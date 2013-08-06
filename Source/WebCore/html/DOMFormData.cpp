/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "html/DOMFormData.h"

#include "fileapi/Blob.h"
#include "html/HTMLFormControlElement.h"
#include "html/HTMLFormElement.h"
#include "platform/text/TextEncoding.h"
#include <wtf/text/WTFString.h>

namespace WebCore {

DOMFormData::DOMFormData(const TextEncoding& encoding)
    : FormDataList(encoding)
{
}

DOMFormData::DOMFormData(HTMLFormElement* form)
    : FormDataList(UTF8Encoding())
{
    if (!form)
        return;

    for (unsigned i = 0; i < form->associatedElements().size(); ++i) {
        FormAssociatedElement* element = form->associatedElements()[i];
        if (!toHTMLElement(element)->isDisabledFormControl())
            element->appendFormData(*this, true);
    }
}

void DOMFormData::append(const String& name, const String& value)
{
    if (!name.isEmpty())
        appendData(name, value);
}

void DOMFormData::append(const String& name, Blob* blob, const String& filename)
{
    if (!name.isEmpty())
        appendBlob(name, blob, filename);
}

} // namespace WebCore
