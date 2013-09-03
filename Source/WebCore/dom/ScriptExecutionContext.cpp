/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (C) 2012 Google Inc. All Rights Reserved.
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
 *
 */

#include "config.h"
#include "dom/ScriptExecutionContext.h"

#include "dom/EventTarget.h"
#include "loader/cache/CachedScript.h"
#include "dom/ErrorEvent.h"
#include "html/PublicURLManager.h"
#include "page/Settings.h"
#include "bindings/dui/ScriptState.h"
#include <wtf/MainThread.h>

namespace WebCore {

void ScriptExecutionContext::AddConsoleMessageTask::performTask(ScriptExecutionContext* context)
{
}

ScriptExecutionContext::ScriptExecutionContext()
    : m_iteratingActiveDOMObjects(false)
    , m_inDestructor(false)
    , m_circularSequentialID(0)
    , m_inDispatchErrorEvent(false)
    , m_activeDOMObjectsAreSuspended(false)
    , m_reasonForSuspendingActiveDOMObjects(static_cast<ActiveDOMObject::ReasonForSuspension>(-1))
    , m_activeDOMObjectsAreStopped(false)
{
}

ScriptExecutionContext::~ScriptExecutionContext()
{
    m_inDestructor = true;
    for (HashSet<ContextDestructionObserver*>::iterator iter = m_destructionObservers.begin(); iter != m_destructionObservers.end(); iter = m_destructionObservers.begin()) {
        ContextDestructionObserver* observer = *iter;
        m_destructionObservers.remove(observer);
        ASSERT(observer->scriptExecutionContext() == this);
        observer->contextDestroyed();
    }

#if ENABLE(BLOB)
    if (m_publicURLManager)
        m_publicURLManager->contextDestroyed();
#endif
}

bool ScriptExecutionContext::canSuspendActiveDOMObjects()
{
    // No protection against m_activeDOMObjects changing during iteration: canSuspend() shouldn't execute arbitrary JS.
    m_iteratingActiveDOMObjects = true;
    ActiveDOMObjectsSet::iterator activeObjectsEnd = m_activeDOMObjects.end();
    for (ActiveDOMObjectsSet::iterator iter = m_activeDOMObjects.begin(); iter != activeObjectsEnd; ++iter) {
        ASSERT((*iter)->scriptExecutionContext() == this);
        ASSERT((*iter)->suspendIfNeededCalled());
        if (!(*iter)->canSuspend()) {
            m_iteratingActiveDOMObjects = false;
            return false;
        }
    }
    m_iteratingActiveDOMObjects = false;
    return true;
}

void ScriptExecutionContext::suspendActiveDOMObjects(ActiveDOMObject::ReasonForSuspension why)
{
    // No protection against m_activeDOMObjects changing during iteration: suspend() shouldn't execute arbitrary JS.
    m_iteratingActiveDOMObjects = true;
    ActiveDOMObjectsSet::iterator activeObjectsEnd = m_activeDOMObjects.end();
    for (ActiveDOMObjectsSet::iterator iter = m_activeDOMObjects.begin(); iter != activeObjectsEnd; ++iter) {
        ASSERT((*iter)->scriptExecutionContext() == this);
        ASSERT((*iter)->suspendIfNeededCalled());
        (*iter)->suspend(why);
    }
    m_iteratingActiveDOMObjects = false;
    m_activeDOMObjectsAreSuspended = true;
    m_reasonForSuspendingActiveDOMObjects = why;
}

void ScriptExecutionContext::resumeActiveDOMObjects(ActiveDOMObject::ReasonForSuspension why)
{
    if (m_reasonForSuspendingActiveDOMObjects != why)
        return;

    m_activeDOMObjectsAreSuspended = false;
    // No protection against m_activeDOMObjects changing during iteration: resume() shouldn't execute arbitrary JS.
    m_iteratingActiveDOMObjects = true;
    ActiveDOMObjectsSet::iterator activeObjectsEnd = m_activeDOMObjects.end();
    for (ActiveDOMObjectsSet::iterator iter = m_activeDOMObjects.begin(); iter != activeObjectsEnd; ++iter) {
        ASSERT((*iter)->scriptExecutionContext() == this);
        ASSERT((*iter)->suspendIfNeededCalled());
        (*iter)->resume();
    }
    m_iteratingActiveDOMObjects = false;
}

void ScriptExecutionContext::stopActiveDOMObjects()
{
    if (m_activeDOMObjectsAreStopped)
        return;
    m_activeDOMObjectsAreStopped = true;
    // No protection against m_activeDOMObjects changing during iteration: stop() shouldn't execute arbitrary JS.
    m_iteratingActiveDOMObjects = true;
    ActiveDOMObjectsSet::iterator activeObjectsEnd = m_activeDOMObjects.end();
    for (ActiveDOMObjectsSet::iterator iter = m_activeDOMObjects.begin(); iter != activeObjectsEnd; ++iter) {
        ASSERT((*iter)->scriptExecutionContext() == this);
        ASSERT((*iter)->suspendIfNeededCalled());
        (*iter)->stop();
    }
    m_iteratingActiveDOMObjects = false;
}

void ScriptExecutionContext::suspendActiveDOMObjectIfNeeded(ActiveDOMObject* object)
{
    ASSERT(m_activeDOMObjects.contains(object));
    // Ensure all ActiveDOMObjects are suspended also newly created ones.
    if (m_activeDOMObjectsAreSuspended)
        object->suspend(m_reasonForSuspendingActiveDOMObjects);
    if (m_activeDOMObjectsAreStopped)
        object->stop();
}

void ScriptExecutionContext::didCreateActiveDOMObject(ActiveDOMObject* object)
{
    ASSERT(object);
    ASSERT(!m_inDestructor);
    if (m_iteratingActiveDOMObjects)
        CRASH();
    m_activeDOMObjects.add(object);
}

void ScriptExecutionContext::willDestroyActiveDOMObject(ActiveDOMObject* object)
{
    ASSERT(object);
    if (m_iteratingActiveDOMObjects)
        CRASH();
    m_activeDOMObjects.remove(object);
}

void ScriptExecutionContext::didCreateDestructionObserver(ContextDestructionObserver* observer)
{
    ASSERT(observer);
    ASSERT(!m_inDestructor);
    m_destructionObservers.add(observer);
}

void ScriptExecutionContext::willDestroyDestructionObserver(ContextDestructionObserver* observer)
{
    ASSERT(observer);
    m_destructionObservers.remove(observer);
}

bool ScriptExecutionContext::sanitizeScriptError(String& errorMessage, int& lineNumber, String& sourceURL, CachedScript* cachedScript)
{
    return false;
}

bool ScriptExecutionContext::dispatchErrorEvent(const String& errorMessage, int lineNumber, const String& sourceURL, CachedScript* cachedScript)
{
    EventTarget* target = errorEventTarget();
    if (!target)
        return false;

    String message = errorMessage;
    int line = lineNumber;
    String sourceName = sourceURL;
    sanitizeScriptError(message, line, sourceName, cachedScript);

    ASSERT(!m_inDispatchErrorEvent);
    m_inDispatchErrorEvent = true;
    RefPtr<ErrorEvent> errorEvent = ErrorEvent::create(message, sourceName, line);
    target->dispatchEvent(errorEvent);
    m_inDispatchErrorEvent = false;
    return errorEvent->defaultPrevented();
}

int ScriptExecutionContext::circularSequentialID()
{
    ++m_circularSequentialID;
    if (m_circularSequentialID <= 0)
        m_circularSequentialID = 1;
    return m_circularSequentialID;
}

#if ENABLE(BLOB)
PublicURLManager& ScriptExecutionContext::publicURLManager()
{
    if (!m_publicURLManager)
        m_publicURLManager = PublicURLManager::create();
    return *m_publicURLManager;
}
#endif

ScriptExecutionContext::Task::~Task()
{
}

JSC::VM* ScriptExecutionContext::vm()
{
    return 0;
}

} // namespace WebCore
