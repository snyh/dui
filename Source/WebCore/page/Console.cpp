/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
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
#include "Console.h"

#include "Chrome.h"
#include "ChromeClient.h"
#include "ConsoleTypes.h"
#include "Document.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameTree.h"
#include "Page.h"
#include "PageConsole.h"
#include "PageGroup.h"
#include "Settings.h"
#include <stdio.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

Console::Console(Frame* frame)
    : DOMWindowProperty(frame)
{
}

Console::~Console()
{
}

void Console::debug(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::error(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::info(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::log(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::warn(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::dir(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::dirxml(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::table(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::clear(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::trace(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::assertCondition(ScriptState* state, PassRefPtr<ScriptArguments> arguments, bool condition)
{
    if (condition)
        return;

}

void Console::count(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::time(const String& title)
{
}

void Console::timeEnd(ScriptState* state, const String& title)
{
}

void Console::timeStamp(PassRefPtr<ScriptArguments> arguments)
{
}

void Console::group(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::groupCollapsed(ScriptState* state, PassRefPtr<ScriptArguments> arguments)
{
}

void Console::groupEnd()
{
}

Page* Console::page() const
{
    if (!m_frame)
        return 0;
    return m_frame->page();
}

} // namespace WebCore
