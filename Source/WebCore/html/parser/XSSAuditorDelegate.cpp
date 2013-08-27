/*
 * Copyright (C) 2013 Google, Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "html/parser/XSSAuditorDelegate.h"

#include "page/DOMWindow.h"
#include "dom/Document.h"
#include "loader/DocumentLoader.h"
#include "platform/network/FormData.h"
#include "page/Frame.h"
#include "loader/FrameLoader.h"
#include "loader/FrameLoaderClient.h"
#include "html/parser/HTMLParserIdioms.h"
#include <wtf/text/StringBuilder.h>
#include <wtf/text/CString.h>

namespace WebCore {

XSSAuditorDelegate::XSSAuditorDelegate(Document* document)
    : m_document(document)
    , m_didSendNotifications(false)
{
    ASSERT(isMainThread());
    ASSERT(m_document);
}

static inline String buildConsoleError(const XSSInfo& xssInfo, const String& url)
{
    StringBuilder message;
    message.append("The XSS Auditor ");
    message.append(xssInfo.m_didBlockEntirePage ? "blocked access to" : "refused to execute a script in");
    message.append(" '");
    message.append(url);
    message.append("' because ");
    message.append(xssInfo.m_didBlockEntirePage ? "the source code of a script" : "its source code");
    message.append(" was found within the request.");

    if (xssInfo.m_didSendCSPHeader)
        message.append(" The server sent a 'Content-Security-Policy' header requesting this behavior.");
    else if (xssInfo.m_didSendXSSProtectionHeader)
        message.append(" The server sent an 'X-XSS-Protection' header requesting this behavior.");
    else
        message.append(" The auditor was enabled as the server sent neither an 'X-XSS-Protection' nor 'Content-Security-Policy' header.");

    return message.toString();
}

PassRefPtr<FormData> XSSAuditorDelegate::generateViolationReport()
{
    ASSERT(isMainThread());
    return FormData::create();
}

void XSSAuditorDelegate::didBlockScript(const XSSInfo& xssInfo)
{
    ASSERT(isMainThread());

    m_document->addConsoleMessage(JSMessageSource, ErrorMessageLevel, buildConsoleError(xssInfo, m_document->url().string()));

    FrameLoader* frameLoader = m_document->frame()->loader();
    if (xssInfo.m_didBlockEntirePage)
        frameLoader->stopAllLoaders();

    if (!m_didSendNotifications) {
        m_didSendNotifications = true;

        frameLoader->client()->didDetectXSS(m_document->url(), xssInfo.m_didBlockEntirePage);
    }
}

} // namespace WebCore
