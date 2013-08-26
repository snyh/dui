/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2008 Alp Toker <alp@atoker.com>
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "loader/SubframeLoader.h"

#include "page/Chrome.h"
#include "page/ChromeClient.h"
#include "page/ContentSecurityPolicy.h"
#include "page/DiagnosticLoggingKeys.h"
#include "page/Frame.h"
#include "loader/FrameLoader.h"
#include "loader/FrameLoaderClient.h"
#include "html/HTMLFrameElementBase.h"
#include "HTMLNames.h"
#include "platform/MIMETypeRegistry.h"
#include "page/Page.h"
#include "rendering/RenderWidget.h"
#include "rendering/RenderView.h"
#include "page/SecurityPolicy.h"
#include "page/Settings.h"

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
#include "html/HTMLMediaElement.h"
#include "rendering/RenderVideo.h"
#endif

namespace WebCore {
    
using namespace HTMLNames;

SubframeLoader::SubframeLoader(Frame* frame)
    : m_containsPlugins(false)
    , m_frame(frame)
{
}

void SubframeLoader::clear()
{
    m_containsPlugins = false;
}

bool SubframeLoader::requestFrame(HTMLFrameOwnerElement* ownerElement, const String& urlString, const AtomicString& frameName, bool lockHistory, bool lockBackForwardList)
{
    // Support for <frame src="javascript:string">
    KURL scriptURL;
    KURL url;
    if (protocolIsJavaScript(urlString)) {
        scriptURL = completeURL(urlString); // completeURL() encodes the URL.
        url = blankURL();
    } else
        url = completeURL(urlString);

    Frame* frame = loadOrRedirectSubframe(ownerElement, url, frameName, lockHistory, lockBackForwardList);
    if (!frame)
        return false;

    return true;
}
    
bool SubframeLoader::resourceWillUsePlugin(const String& url, const String& mimeType, bool shouldPreferPlugInsForImages)
{
    KURL completedURL;
    if (!url.isEmpty())
        completedURL = completeURL(url);

    bool useFallback;
    return shouldUsePlugin(completedURL, mimeType, shouldPreferPlugInsForImages, false, useFallback);
}

static String findPluginMIMETypeFromURL(Page* page, const String& url)
{
    return String();
}


#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
PassRefPtr<Widget> SubframeLoader::loadMediaPlayerProxyPlugin(Node* node, const KURL& url,
    const Vector<String>& paramNames, const Vector<String>& paramValues)
{
    ASSERT(node->hasTagName(videoTag) || isHTMLAudioElement(node));

    KURL completedURL;
    if (!url.isEmpty())
        completedURL = completeURL(url);

    if (!m_frame->document()->contentSecurityPolicy()->allowMediaFromSource(completedURL))
        return 0;

    HTMLMediaElement* mediaElement = static_cast<HTMLMediaElement*>(node);
    RenderPart* renderer = toRenderPart(node->renderer());
    IntSize size;

    if (renderer)
        size = roundedIntSize(LayoutSize(renderer->contentWidth(), renderer->contentHeight()));
    else if (mediaElement->isVideo())
        size = RenderVideo::defaultSize();

    RefPtr<Widget> widget = m_frame->loader()->client()->createMediaPlayerProxyPlugin(size, mediaElement, completedURL,
                                         paramNames, paramValues, "application/x-media-element-proxy-plugin");

    if (widget && renderer) {
        renderer->setWidget(widget);
        renderer->node()->setNeedsStyleRecalc(SyntheticStyleChange);
    }
    m_containsPlugins = true;

    return widget ? widget.release() : 0;
}
#endif // ENABLE(PLUGIN_PROXY_FOR_VIDEO)

Frame* SubframeLoader::loadOrRedirectSubframe(HTMLFrameOwnerElement* ownerElement, const KURL& url, const AtomicString& frameName, bool lockHistory, bool lockBackForwardList)
{
    Frame* frame = ownerElement->contentFrame();

    if (!frame)
        frame = loadSubframe(ownerElement, url, frameName, m_frame->loader()->outgoingReferrer());

    ASSERT(ownerElement->contentFrame() == frame || !ownerElement->contentFrame());
    return ownerElement->contentFrame();
}

Frame* SubframeLoader::loadSubframe(HTMLFrameOwnerElement* ownerElement, const KURL& url, const String& name, const String& referrer)
{
    RefPtr<Frame> protect(m_frame);

    bool allowsScrolling = true;
    int marginWidth = -1;
    int marginHeight = -1;
    if (ownerElement->hasTagName(frameTag) || ownerElement->hasTagName(iframeTag)) {
        HTMLFrameElementBase* o = static_cast<HTMLFrameElementBase*>(ownerElement);
        allowsScrolling = o->scrollingMode() != ScrollbarAlwaysOff;
        marginWidth = o->marginWidth();
        marginHeight = o->marginHeight();
    }

    String referrerToUse = SecurityPolicy::generateReferrerHeader(ownerElement->document()->referrerPolicy(), url, referrer);
    RefPtr<Frame> frame = m_frame->loader()->client()->createFrame(url, name, ownerElement, referrerToUse, allowsScrolling, marginWidth, marginHeight);

    if (!frame)  {
        m_frame->loader()->checkCallImplicitClose();
        return 0;
    }
    
    // All new frames will have m_isComplete set to true at this point due to synchronously loading
    // an empty document in FrameLoader::init(). But many frames will now be starting an
    // asynchronous load of url, so we set m_isComplete to false and then check if the load is
    // actually completed below. (Note that we set m_isComplete to false even for synchronous
    // loads, so that checkCompleted() below won't bail early.)
    // FIXME: Can we remove this entirely? m_isComplete normally gets set to false when a load is committed.
    frame->loader()->started();
   
    RenderObject* renderer = ownerElement->renderer();
    FrameView* view = frame->view();
    if (renderer && renderer->isWidget() && view)
        toRenderWidget(renderer)->setWidget(view);
    
    m_frame->loader()->checkCallImplicitClose();
    
    // Some loads are performed synchronously (e.g., about:blank and loads
    // cancelled by returning a null ResourceRequest from requestFromDelegate).
    // In these cases, the synchronous load would have finished
    // before we could connect the signals, so make sure to send the 
    // completed() signal for the child by hand and mark the load as being
    // complete.
    // FIXME: In this case the Frame will have finished loading before 
    // it's being added to the child list. It would be a good idea to
    // create the child first, then invoke the loader separately.
    if (frame->loader()->state() == FrameStateComplete && !frame->loader()->policyDocumentLoader())
        frame->loader()->checkCompleted();

    return frame.get();
}

bool SubframeLoader::allowPlugins(ReasonForCallingAllowPlugins reason)
{
    Settings* settings = m_frame->settings();
    bool allowed = m_frame->loader()->client()->allowPlugins(settings && settings->arePluginsEnabled());
    if (!allowed && reason == AboutToInstantiatePlugin)
        m_frame->loader()->client()->didNotAllowPlugins();
    return allowed;
}

bool SubframeLoader::shouldUsePlugin(const KURL& url, const String& mimeType, bool shouldPreferPlugInsForImages, bool hasFallback, bool& useFallback)
{
    return false;
}

Document* SubframeLoader::document() const
{
    return m_frame->document();
}

KURL SubframeLoader::completeURL(const String& url) const
{
    ASSERT(m_frame->document());
    return m_frame->document()->completeURL(url);
}

} // namespace WebCore
