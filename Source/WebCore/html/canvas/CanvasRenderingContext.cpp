/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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
#include "html/canvas/CanvasRenderingContext.h"

#include "loader/cache/CachedImage.h"
#include "html/canvas/CanvasPattern.h"
#include "html/HTMLCanvasElement.h"
#include "html/HTMLImageElement.h"
#include "platform/graphics/Image.h"
#include "platform/KURL.h"

namespace WebCore {

CanvasRenderingContext::CanvasRenderingContext(HTMLCanvasElement* canvas)
    : m_canvas(canvas)
{
}

bool CanvasRenderingContext::wouldTaintOrigin(const CanvasPattern* pattern)
{
    if (canvas()->originClean() && pattern && !pattern->originClean())
        return true;
    return false;
}

bool CanvasRenderingContext::wouldTaintOrigin(const HTMLCanvasElement* sourceCanvas)
{
    if (canvas()->originClean() && sourceCanvas && !sourceCanvas->originClean())
        return true;
    return false;
}

bool CanvasRenderingContext::wouldTaintOrigin(const HTMLImageElement* image)
{
    return false;
}

bool CanvasRenderingContext::wouldTaintOrigin(const KURL& url)
{
    return false;
}

void CanvasRenderingContext::checkOrigin(const KURL& url)
{
    if (wouldTaintOrigin(url))
        canvas()->setOriginTainted();
}

} // namespace WebCore
