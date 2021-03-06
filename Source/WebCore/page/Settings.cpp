/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2011, 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "page/Settings.h"

#include "loader/cache/CachedResourceLoader.h"
#include "dom/Document.h"
#include "platform/graphics/Font.h"
#include "platform/graphics/FontGenericFamilies.h"
#include "page/Frame.h"
#include "page/FrameTree.h"
#include "page/FrameView.h"
#include "page/Page.h"
#include "rendering/TextAutosizer.h"
#include <limits>

using namespace std;

namespace WebCore {

static void setImageLoadingSettings(Page* page)
{
    for (Frame* frame = page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        frame->document()->cachedResourceLoader()->setImagesEnabled(page->settings()->areImagesEnabled());
        frame->document()->cachedResourceLoader()->setAutoLoadImages(page->settings()->loadsImagesAutomatically());
    }
}

static void invalidateAfterGenericFamilyChange(Page* page)
{
    invalidateFontGlyphsCache();
    if (page)
        page->setNeedsRecalcStyleInAllFrames();
}


#if USE(SAFARI_THEME)
bool Settings::gShouldPaintNativeControls = true;
#endif

#if USE(AVFOUNDATION)
bool Settings::gAVFoundationEnabled = false;
#endif

#if PLATFORM(MAC)
bool Settings::gQTKitEnabled = true;
#endif

bool Settings::gMockScrollbarsEnabled = false;
bool Settings::gUsesOverlayScrollbars = false;

#if PLATFORM(WIN)
bool Settings::gShouldUseHighResolutionTimers = true;
#endif
    
bool Settings::gShouldRespectPriorityInCSSAttributeSetters = false;
bool Settings::gLowPowerVideoAudioBufferSizeEnabled = false;

// NOTEs
//  1) EditingMacBehavior comprises Tiger, Leopard, SnowLeopard and iOS builds, as well as QtWebKit when built on Mac;
//  2) EditingWindowsBehavior comprises Win32 and WinCE builds, as well as QtWebKit and Chromium when built on Windows;
//  3) EditingUnixBehavior comprises all unix-based systems, but Darwin/MacOS (and then abusing the terminology);
// 99) MacEditingBehavior is used as a fallback.
static EditingBehaviorType editingBehaviorTypeForPlatform()
{
    return
#if OS(DARWIN)
    EditingMacBehavior
#elif OS(WINDOWS)
    EditingWindowsBehavior
#elif OS(UNIX)
    EditingUnixBehavior
#else
    // Fallback
    EditingMacBehavior
#endif
    ;
}

static const double defaultIncrementalRenderingSuppressionTimeoutInSeconds = 5;
#if USE(UNIFIED_TEXT_CHECKING)
static const bool defaultUnifiedTextCheckerEnabled = true;
#else
static const bool defaultUnifiedTextCheckerEnabled = false;
#endif
static const bool defaultSmartInsertDeleteEnabled = true;
static const bool defaultSelectTrailingWhitespaceEnabled = false;

Settings::Settings(Page* page)
    : m_page(0)
    , m_mediaTypeOverride("screen")
    , m_fontGenericFamilies(FontGenericFamilies::create())
#if ENABLE(TEXT_AUTOSIZING)
    , m_textAutosizingFontScaleFactor(1)
#if HACK_FORCE_TEXT_AUTOSIZING_ON_DESKTOP
    , m_textAutosizingWindowSizeOverride(320, 480)
    , m_textAutosizingEnabled(true)
#else
    , m_textAutosizingEnabled(false)
#endif
#endif
    SETTINGS_INITIALIZER_LIST
    , m_isJavaEnabled(false)
    , m_isJavaEnabledForLocalFiles(true)
    , m_loadsImagesAutomatically(false)
    , m_privateBrowsingEnabled(false)
    , m_areImagesEnabled(true)
    , m_arePluginsEnabled(false)
    , m_isScriptEnabled(false)
    , m_needsAdobeFrameReloadingQuirk(false)
    , m_usesPageCache(false)
    , m_fontRenderingMode(0)
    , m_isCSSCustomFilterEnabled(false)
#if ENABLE(CSS_STICKY_POSITION)
    , m_cssStickyPositionEnabled(true)
#endif
#if ENABLE(CSS_VARIABLES)
    , m_cssVariablesEnabled(false)
#endif
    , m_showTiledScrollingIndicator(false)
    , m_tiledBackingStoreEnabled(false)
    , m_dnsPrefetchingEnabled(false)
#if ENABLE(TOUCH_EVENTS)
    , m_touchEventEmulationEnabled(false)
#endif
    , m_scrollingPerformanceLoggingEnabled(false)
    , m_aggressiveTileRetentionEnabled(false)
    , m_timeWithoutMouseMovementBeforeHidingControls(3)
    , m_setImageLoadingSettingsTimer(this, &Settings::imageLoadingSettingsTimerFired)
{
    // A Frame may not have been created yet, so we initialize the AtomicString
    // hash before trying to use it.
    AtomicString::init();
    initializeDefaultFontFamilies();
    m_page = page; // Page is not yet fully initialized wen constructing Settings, so keeping m_page null over initializeDefaultFontFamilies() call.
}

Settings::~Settings()
{
}

PassOwnPtr<Settings> Settings::create(Page* page)
{
    return adoptPtr(new Settings(page));
}

SETTINGS_SETTER_BODIES

#if !PLATFORM(MAC) && !PLATFORM(BLACKBERRY)
void Settings::initializeDefaultFontFamilies()
{
    // Other platforms can set up fonts from a client, but on Mac, we want it in WebCore to share code between WebKit1 and WebKit2.
}
#endif

const AtomicString& Settings::standardFontFamily(UScriptCode script) const
{
    return m_fontGenericFamilies->standardFontFamily(script);
}

void Settings::setStandardFontFamily(const AtomicString& family, UScriptCode script)
{
    bool changes = m_fontGenericFamilies->setStandardFontFamily(family, script);
    if (changes)
        invalidateAfterGenericFamilyChange(m_page);
}

const AtomicString& Settings::fixedFontFamily(UScriptCode script) const
{
    return m_fontGenericFamilies->fixedFontFamily(script);
}

void Settings::setFixedFontFamily(const AtomicString& family, UScriptCode script)
{
    bool changes = m_fontGenericFamilies->setFixedFontFamily(family, script);
    if (changes)
        invalidateAfterGenericFamilyChange(m_page);
}

const AtomicString& Settings::serifFontFamily(UScriptCode script) const
{
    return m_fontGenericFamilies->serifFontFamily(script);
}

void Settings::setSerifFontFamily(const AtomicString& family, UScriptCode script)
{
    bool changes = m_fontGenericFamilies->setSerifFontFamily(family, script);
    if (changes)
        invalidateAfterGenericFamilyChange(m_page);
}

const AtomicString& Settings::sansSerifFontFamily(UScriptCode script) const
{
    return m_fontGenericFamilies->sansSerifFontFamily(script);
}

void Settings::setSansSerifFontFamily(const AtomicString& family, UScriptCode script)
{
    bool changes = m_fontGenericFamilies->setSansSerifFontFamily(family, script);
    if (changes)
        invalidateAfterGenericFamilyChange(m_page);
}

const AtomicString& Settings::cursiveFontFamily(UScriptCode script) const
{
    return m_fontGenericFamilies->cursiveFontFamily(script);
}

void Settings::setCursiveFontFamily(const AtomicString& family, UScriptCode script)
{
    bool changes = m_fontGenericFamilies->setCursiveFontFamily(family, script);
    if (changes)
        invalidateAfterGenericFamilyChange(m_page);
}

const AtomicString& Settings::fantasyFontFamily(UScriptCode script) const
{
    return m_fontGenericFamilies->fantasyFontFamily(script);
}

void Settings::setFantasyFontFamily(const AtomicString& family, UScriptCode script)
{
    bool changes = m_fontGenericFamilies->setFantasyFontFamily(family, script);
    if (changes)
        invalidateAfterGenericFamilyChange(m_page);
}

const AtomicString& Settings::pictographFontFamily(UScriptCode script) const
{
    return m_fontGenericFamilies->pictographFontFamily(script);
}

void Settings::setPictographFontFamily(const AtomicString& family, UScriptCode script)
{
    bool changes = m_fontGenericFamilies->setPictographFontFamily(family, script);
    if (changes)
        invalidateAfterGenericFamilyChange(m_page);
}

#if ENABLE(TEXT_AUTOSIZING)
void Settings::setTextAutosizingEnabled(bool textAutosizingEnabled)
{
    if (m_textAutosizingEnabled == textAutosizingEnabled)
        return;

    m_textAutosizingEnabled = textAutosizingEnabled;
    m_page->setNeedsRecalcStyleInAllFrames();
}

void Settings::setTextAutosizingWindowSizeOverride(const IntSize& textAutosizingWindowSizeOverride)
{
    if (m_textAutosizingWindowSizeOverride == textAutosizingWindowSizeOverride)
        return;

    m_textAutosizingWindowSizeOverride = textAutosizingWindowSizeOverride;
    m_page->setNeedsRecalcStyleInAllFrames();
}

void Settings::setTextAutosizingFontScaleFactor(float fontScaleFactor)
{
    m_textAutosizingFontScaleFactor = fontScaleFactor;

    // FIXME: I wonder if this needs to traverse frames like in WebViewImpl::resize, or whether there is only one document per Settings instance?
    for (Frame* frame = m_page->mainFrame(); frame; frame = frame->tree()->traverseNext())
        frame->document()->textAutosizer()->recalculateMultipliers();

    m_page->setNeedsRecalcStyleInAllFrames();
}

#endif

void Settings::setMediaTypeOverride(const String& mediaTypeOverride)
{
    if (m_mediaTypeOverride == mediaTypeOverride)
        return;

    m_mediaTypeOverride = mediaTypeOverride;

    Frame* mainFrame = m_page->mainFrame();
    ASSERT(mainFrame);
    FrameView* view = mainFrame->view();
    ASSERT(view);

    view->setMediaType(mediaTypeOverride);
    m_page->setNeedsRecalcStyleInAllFrames();
}

void Settings::setLoadsImagesAutomatically(bool loadsImagesAutomatically)
{
    m_loadsImagesAutomatically = loadsImagesAutomatically;
    
    // Changing this setting to true might immediately start new loads for images that had previously had loading disabled.
    // If this happens while a WebView is being dealloc'ed, and we don't know the WebView is being dealloc'ed, these new loads
    // can cause crashes downstream when the WebView memory has actually been free'd.
    // One example where this can happen is in Mac apps that subclass WebView then do work in their overridden dealloc methods.
    // Starting these loads synchronously is not important.  By putting it on a 0-delay, properly closing the Page cancels them
    // before they have a chance to really start.
    // See http://webkit.org/b/60572 for more discussion.
    m_setImageLoadingSettingsTimer.startOneShot(0);
}

void Settings::imageLoadingSettingsTimerFired(Timer<Settings>*)
{
    setImageLoadingSettings(m_page);
}

void Settings::setScriptEnabled(bool isScriptEnabled)
{
    m_isScriptEnabled = isScriptEnabled;
}

void Settings::setJavaEnabled(bool isJavaEnabled)
{
    m_isJavaEnabled = isJavaEnabled;
}

void Settings::setJavaEnabledForLocalFiles(bool isJavaEnabledForLocalFiles)
{
    m_isJavaEnabledForLocalFiles = isJavaEnabledForLocalFiles;
}

void Settings::setImagesEnabled(bool areImagesEnabled)
{
    m_areImagesEnabled = areImagesEnabled;

    // See comment in setLoadsImagesAutomatically.
    m_setImageLoadingSettingsTimer.startOneShot(0);
}

void Settings::setPluginsEnabled(bool arePluginsEnabled)
{
    m_arePluginsEnabled = arePluginsEnabled;
}

void Settings::setPrivateBrowsingEnabled(bool privateBrowsingEnabled)
{
    if (m_privateBrowsingEnabled == privateBrowsingEnabled)
        return;

    m_privateBrowsingEnabled = privateBrowsingEnabled;
    m_page->privateBrowsingStateChanged();
}

void Settings::setUserStyleSheetLocation(const KURL& userStyleSheetLocation)
{
    if (m_userStyleSheetLocation == userStyleSheetLocation)
        return;

    m_userStyleSheetLocation = userStyleSheetLocation;

    m_page->userStyleSheetLocationChanged();
}

// FIXME: This quirk is needed because of Radar 4674537 and 5211271. We need to phase it out once Adobe
// can fix the bug from their end.
void Settings::setNeedsAdobeFrameReloadingQuirk(bool shouldNotReloadIFramesForUnchangedSRC)
{
    m_needsAdobeFrameReloadingQuirk = shouldNotReloadIFramesForUnchangedSRC;
}

void Settings::setUsesPageCache(bool usesPageCache)
{
    if (m_usesPageCache == usesPageCache)
        return;
        
    m_usesPageCache = usesPageCache;
}

void Settings::setFontRenderingMode(FontRenderingMode mode)
{
    if (fontRenderingMode() == mode)
        return;
    m_fontRenderingMode = mode;
    m_page->setNeedsRecalcStyleInAllFrames();
}

FontRenderingMode Settings::fontRenderingMode() const
{
    return static_cast<FontRenderingMode>(m_fontRenderingMode);
}

#if USE(SAFARI_THEME)
void Settings::setShouldPaintNativeControls(bool shouldPaintNativeControls)
{
    gShouldPaintNativeControls = shouldPaintNativeControls;
}
#endif

void Settings::setDNSPrefetchingEnabled(bool dnsPrefetchingEnabled)
{
    if (m_dnsPrefetchingEnabled == dnsPrefetchingEnabled)
        return;

    m_dnsPrefetchingEnabled = dnsPrefetchingEnabled;
    m_page->dnsPrefetchingStateChanged();
}

void Settings::setShowTiledScrollingIndicator(bool enabled)
{
    if (m_showTiledScrollingIndicator == enabled)
        return;
        
    m_showTiledScrollingIndicator = enabled;
}

#if PLATFORM(WIN)
void Settings::setShouldUseHighResolutionTimers(bool shouldUseHighResolutionTimers)
{
    gShouldUseHighResolutionTimers = shouldUseHighResolutionTimers;
}
#endif

void Settings::setTiledBackingStoreEnabled(bool enabled)
{
    m_tiledBackingStoreEnabled = enabled;
#if USE(TILED_BACKING_STORE)
    if (m_page->mainFrame())
        m_page->mainFrame()->setTiledBackingStoreEnabled(enabled);
#endif
}

#if USE(AVFOUNDATION)
void Settings::setAVFoundationEnabled(bool enabled)
{
    if (gAVFoundationEnabled == enabled)
        return;

    gAVFoundationEnabled = enabled;
}
#endif

#if PLATFORM(MAC)
void Settings::setQTKitEnabled(bool enabled)
{
    if (gQTKitEnabled == enabled)
        return;

    gQTKitEnabled = enabled;
}
#endif

void Settings::setScrollingPerformanceLoggingEnabled(bool enabled)
{
    m_scrollingPerformanceLoggingEnabled = enabled;

    if (m_page->mainFrame() && m_page->mainFrame()->view())
        m_page->mainFrame()->view()->setScrollingPerformanceLoggingEnabled(enabled);
}
    
void Settings::setAggressiveTileRetentionEnabled(bool enabled)
{
    m_aggressiveTileRetentionEnabled = enabled;
}

void Settings::setMockScrollbarsEnabled(bool flag)
{
    gMockScrollbarsEnabled = flag;
}

bool Settings::mockScrollbarsEnabled()
{
    return gMockScrollbarsEnabled;
}

void Settings::setUsesOverlayScrollbars(bool flag)
{
    gUsesOverlayScrollbars = flag;
}

bool Settings::usesOverlayScrollbars()
{
    return gUsesOverlayScrollbars;
}

void Settings::setShouldRespectPriorityInCSSAttributeSetters(bool flag)
{
    gShouldRespectPriorityInCSSAttributeSetters = flag;
}

bool Settings::shouldRespectPriorityInCSSAttributeSetters()
{
    return gShouldRespectPriorityInCSSAttributeSetters;
}

void Settings::setLowPowerVideoAudioBufferSizeEnabled(bool flag)
{
    gLowPowerVideoAudioBufferSizeEnabled = flag;
}

} // namespace WebCore
