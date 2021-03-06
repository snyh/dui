/*
 * Copyright (C) 2006, 2007, 2009, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2012, Samsung Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "page/Chrome.h"

#include "page/ChromeClient.h"
#include "platform/network/DNS.h"
#include "platform/DateTimeChooser.h"
#include "dom/Document.h"
#include "html/forms/FileIconLoader.h"
#include "platform/FileChooser.h"
#include "fileapi/FileList.h"
#include "platform/graphics/FloatRect.h"
#include "page/Frame.h"
#include "page/FrameTree.h"
#include "html/HTMLFormElement.h"
#include "html/HTMLInputElement.h"
#include "HTMLNames.h"
#include "rendering/HitTestResult.h"
#include "platform/graphics/Icon.h"
#include "page/Page.h"
#include "page/PopupOpeningObserver.h"
#include "rendering/RenderObject.h"
#include "platform/network/ResourceHandle.h"
#include "page/Settings.h"
#include "page/WindowFeatures.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/StringBuilder.h>

#if ENABLE(INPUT_TYPE_COLOR)
#include "platform/ColorChooser.h"
#endif

namespace WebCore {

using namespace HTMLNames;
using namespace std;

Chrome::Chrome(Page* page, ChromeClient* client)
    : m_page(page)
    , m_client(client)
    , m_displayID(0)
{
    ASSERT(m_client);
}

Chrome::~Chrome()
{
    m_client->chromeDestroyed();
}

PassOwnPtr<Chrome> Chrome::create(Page* page, ChromeClient* client)
{
    return adoptPtr(new Chrome(page, client));
}

void Chrome::invalidateRootView(const IntRect& updateRect, bool immediate)
{
    m_client->invalidateRootView(updateRect, immediate);
}

void Chrome::invalidateContentsAndRootView(const IntRect& updateRect, bool immediate)
{
    m_client->invalidateContentsAndRootView(updateRect, immediate);
}

void Chrome::invalidateContentsForSlowScroll(const IntRect& updateRect, bool immediate)
{
    m_client->invalidateContentsForSlowScroll(updateRect, immediate);
}

void Chrome::scroll(const IntSize& scrollDelta, const IntRect& rectToScroll, const IntRect& clipRect)
{
    m_client->scroll(scrollDelta, rectToScroll, clipRect);
}

#if USE(TILED_BACKING_STORE)
void Chrome::delegatedScrollRequested(const IntPoint& scrollPoint)
{
    m_client->delegatedScrollRequested(scrollPoint);
}
#endif

IntPoint Chrome::screenToRootView(const IntPoint& point) const
{
    return m_client->screenToRootView(point);
}

IntRect Chrome::rootViewToScreen(const IntRect& rect) const
{
    return m_client->rootViewToScreen(rect);
}

PlatformPageClient Chrome::platformPageClient() const
{
    return m_client->platformPageClient();
}

void Chrome::contentsSizeChanged(Frame* frame, const IntSize& size) const
{
    m_client->contentsSizeChanged(frame, size);
}

void Chrome::layoutUpdated(Frame* frame) const
{
    m_client->layoutUpdated(frame);
}

void Chrome::scrollRectIntoView(const IntRect& rect) const
{
    m_client->scrollRectIntoView(rect);
}

void Chrome::scrollbarsModeDidChange() const
{
    m_client->scrollbarsModeDidChange();
}

void Chrome::setWindowRect(const FloatRect& rect) const
{
    m_client->setWindowRect(rect);
}

FloatRect Chrome::windowRect() const
{
    return m_client->windowRect();
}

FloatRect Chrome::pageRect() const
{
    return m_client->pageRect();
}

void Chrome::focus() const
{
    m_client->focus();
}

void Chrome::unfocus() const
{
    m_client->unfocus();
}

bool Chrome::canTakeFocus(FocusDirection direction) const
{
    return m_client->canTakeFocus(direction);
}

void Chrome::takeFocus(FocusDirection direction) const
{
    m_client->takeFocus(direction);
}

void Chrome::focusedNodeChanged(Node* node) const
{
    m_client->focusedNodeChanged(node);
}

void Chrome::focusedFrameChanged(Frame* frame) const
{
    m_client->focusedFrameChanged(frame);
}

void Chrome::show() const
{
    m_client->show();
}

static bool canRunModalIfDuringPageDismissal(Page* page, ChromeClient::DialogType dialog, const String& message)
{
    for (Frame* frame = page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        FrameLoader::PageDismissalType dismissal = frame->loader()->pageDismissalEventBeingDispatched();
        if (dismissal != FrameLoader::NoDismissal)
            return page->chrome().client()->shouldRunModalDialogDuringPageDismissal(dialog, message, dismissal);
    }
    return true;
}

void Chrome::closeWindowSoon()
{
    m_client->closeWindowSoon();
}

IntRect Chrome::windowResizerRect() const
{
    return m_client->windowResizerRect();
}

void Chrome::mouseDidMoveOverElement(const HitTestResult& result, unsigned modifierFlags)
{
    if (result.innerNode()) {
        Document* document = result.innerNode()->document();
        if (document && document->isDNSPrefetchEnabled())
            prefetchDNS(result.absoluteLinkURL().host());
    }
    m_client->mouseDidMoveOverElement(result, modifierFlags);
}

void Chrome::setToolTip(const HitTestResult& result)
{
    // First priority is a potential toolTip representing a spelling or grammar error
    TextDirection toolTipDirection;
    String toolTip = result.spellingToolTip(toolTipDirection);

    // Next priority is a toolTip from a URL beneath the mouse (if preference is set to show those).
    if (toolTip.isEmpty() && m_page->settings()->showsURLsInToolTips()) {
        if (Node* node = result.innerNonSharedNode()) {
            // Get tooltip representing form action, if relevant
            if (isHTMLInputElement(node)) {
                HTMLInputElement* input = toHTMLInputElement(node);
                if (input->isSubmitButton())
                    if (HTMLFormElement* form = input->form()) {
                        toolTip = form->action();
                        if (form->renderer())
                            toolTipDirection = form->renderer()->style()->direction();
                        else
                            toolTipDirection = LTR;
                    }
            }
        }

        // Get tooltip representing link's URL
        if (toolTip.isEmpty()) {
            // FIXME: Need to pass this URL through userVisibleString once that's in WebCore
            toolTip = result.absoluteLinkURL().string();
            // URL always display as LTR.
            toolTipDirection = LTR;
        }
    }

    // Next we'll consider a tooltip for element with "title" attribute
    if (toolTip.isEmpty())
        toolTip = result.title(toolTipDirection);

    if (toolTip.isEmpty() && m_page->settings()->showsToolTipOverTruncatedText())
        toolTip = result.innerTextIfTruncated(toolTipDirection);

    // Lastly, for <input type="file"> that allow multiple files, we'll consider a tooltip for the selected filenames
    if (toolTip.isEmpty()) {
        if (Node* node = result.innerNonSharedNode()) {
            if (isHTMLInputElement(node)) {
                HTMLInputElement* input = toHTMLInputElement(node);
                toolTip = input->defaultToolTip();

                // FIXME: We should obtain text direction of tooltip from
                // ChromeClient or platform. As of October 2011, all client
                // implementations don't use text direction information for
                // ChromeClient::setToolTip. We'll work on tooltip text
                // direction during bidi cleanup in form inputs.
                toolTipDirection = LTR;
            }
        }
    }

    m_client->setToolTip(toolTip, toolTipDirection);
}

void Chrome::enableSuddenTermination()
{
    m_client->enableSuddenTermination();
}

void Chrome::disableSuddenTermination()
{
    m_client->disableSuddenTermination();
}

#if ENABLE(INPUT_TYPE_COLOR)
PassOwnPtr<ColorChooser> Chrome::createColorChooser(ColorChooserClient* client, const Color& initialColor)
{
    notifyPopupOpeningObservers();
    return m_client->createColorChooser(client, initialColor);
}
#endif

#if ENABLE(DATE_AND_TIME_INPUT_TYPES)
PassRefPtr<DateTimeChooser> Chrome::openDateTimeChooser(DateTimeChooserClient* client, const DateTimeChooserParameters& parameters)
{
    notifyPopupOpeningObservers();
    return m_client->openDateTimeChooser(client, parameters);
}
#endif

void Chrome::runOpenPanel(Frame* frame, PassRefPtr<FileChooser> fileChooser)
{
    notifyPopupOpeningObservers();
    m_client->runOpenPanel(frame, fileChooser);
}

void Chrome::dispatchViewportPropertiesDidChange(const ViewportArguments& arguments) const
{
    m_client->dispatchViewportPropertiesDidChange(arguments);
}

void Chrome::setCursor(const Cursor& cursor)
{
    m_client->setCursor(cursor);
}

void Chrome::setCursorHiddenUntilMouseMoves(bool hiddenUntilMouseMoves)
{
    m_client->setCursorHiddenUntilMouseMoves(hiddenUntilMouseMoves);
}

#if ENABLE(REQUEST_ANIMATION_FRAME)
void Chrome::scheduleAnimation()
{
#if !USE(REQUEST_ANIMATION_FRAME_TIMER)
    m_client->scheduleAnimation();
#endif
}
#endif

PlatformDisplayID Chrome::displayID() const
{
    return m_displayID;
}

void Chrome::windowScreenDidChange(PlatformDisplayID displayID)
{
    if (displayID == m_displayID)
        return;

    m_displayID = displayID;

    for (Frame* frame = m_page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        if (frame->document())
            frame->document()->windowScreenDidChange(displayID);
    }
}

#if ENABLE(DRAGGABLE_REGION)
void ChromeClient::annotatedRegionsChanged()
{
}
#endif

void ChromeClient::populateVisitedLinks()
{
}

FloatRect ChromeClient::customHighlightRect(Node*, const AtomicString&, const FloatRect&)
{
    return FloatRect();
}

void ChromeClient::paintCustomHighlight(Node*, const AtomicString&, const FloatRect&, const FloatRect&, bool, bool)
{
}

bool ChromeClient::shouldReplaceWithGeneratedFileForUpload(const String&, String&)
{
    return false;
}

String ChromeClient::generateReplacementFile(const String&)
{
    ASSERT_NOT_REACHED();
    return String();
}

bool ChromeClient::paintCustomOverhangArea(GraphicsContext*, const IntRect&, const IntRect&, const IntRect&)
{
    return false;
}

bool Chrome::selectItemWritingDirectionIsNatural()
{
    return m_client->selectItemWritingDirectionIsNatural();
}

bool Chrome::selectItemAlignmentFollowsMenuWritingDirection()
{
    return m_client->selectItemAlignmentFollowsMenuWritingDirection();
}

PassRefPtr<PopupMenu> Chrome::createPopupMenu(PopupMenuClient* client) const
{
    notifyPopupOpeningObservers();
    return m_client->createPopupMenu(client);
}

PassRefPtr<SearchPopupMenu> Chrome::createSearchPopupMenu(PopupMenuClient* client) const
{
    notifyPopupOpeningObservers();
    return m_client->createSearchPopupMenu(client);
}

bool Chrome::requiresFullscreenForVideoPlayback()
{
    return m_client->requiresFullscreenForVideoPlayback();
}

void Chrome::registerPopupOpeningObserver(PopupOpeningObserver* observer)
{
    ASSERT(observer);
    m_popupOpeningObservers.append(observer);
}

void Chrome::unregisterPopupOpeningObserver(PopupOpeningObserver* observer)
{
    size_t index = m_popupOpeningObservers.find(observer);
    ASSERT(index != notFound);
    m_popupOpeningObservers.remove(index);
}

void Chrome::notifyPopupOpeningObservers() const
{
    const Vector<PopupOpeningObserver*> observers(m_popupOpeningObservers);
    for (size_t i = 0; i < observers.size(); ++i)
        observers[i]->willOpenPopup();
}

} // namespace WebCore
