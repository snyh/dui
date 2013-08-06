/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "storage/StorageTracker.h"

#include "platform/FileSystem.h"
#include "storage/StorageThread.h"
#include "platform/Logging.h"
#include "page/PageGroup.h"
#include "page/SecurityOrigin.h"
#include "storage/StorageTrackerClient.h"
#include "platform/text/TextEncoding.h"
#include <wtf/Functional.h>
#include <wtf/MainThread.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>
#include <wtf/text/CString.h>

namespace WebCore {

static StorageTracker* storageTracker = 0;

// If there is no document referencing a storage database, close the underlying database
// after it has been idle for m_StorageDatabaseIdleInterval seconds.
static const double DefaultStorageDatabaseIdleInterval = 300;
    
void StorageTracker::initializeTracker(const String& storagePath, StorageTrackerClient* client)
{
    ASSERT(isMainThread());
    ASSERT(!storageTracker || !storageTracker->m_client);
    
    if (!storageTracker)
        storageTracker = new StorageTracker(storagePath);
    
    storageTracker->m_client = client;
    storageTracker->m_needsInitialization = true;
}

void StorageTracker::internalInitialize()
{
    m_needsInitialization = false;

    ASSERT(isMainThread());

    // Make sure text encoding maps have been built on the main thread, as the StorageTracker thread might try to do it there instead.
    // FIXME (<rdar://problem/9127819>): Is there a more explicit way of doing this besides accessing the UTF8Encoding?
    UTF8Encoding();
    
    storageTracker->setIsActive(true);
    storageTracker->m_thread->start();  
    storageTracker->importOriginIdentifiers();
}

StorageTracker& StorageTracker::tracker()
{
    if (!storageTracker)
        storageTracker = new StorageTracker("");
    if (storageTracker->m_needsInitialization)
        storageTracker->internalInitialize();
    
    return *storageTracker;
}

StorageTracker::StorageTracker(const String& storagePath)
    : m_storageDirectoryPath(storagePath.isolatedCopy())
    , m_client(0)
    , m_thread(StorageThread::create())
    , m_isActive(false)
    , m_needsInitialization(false)
    , m_StorageDatabaseIdleInterval(DefaultStorageDatabaseIdleInterval)
{
}

void StorageTracker::setDatabaseDirectoryPath(const String& path)
{
}

String StorageTracker::databaseDirectoryPath() const
{
    return m_storageDirectoryPath.isolatedCopy();
}

String StorageTracker::trackerDatabasePath()
{
    return String();
}

void StorageTracker::openTrackerDatabase(bool createIfDoesNotExist)
{
}

void StorageTracker::importOriginIdentifiers()
{   
}

void StorageTracker::finishedImportingOriginIdentifiers()
{
}

void StorageTracker::syncImportOriginIdentifiers()
{
}
    
void StorageTracker::syncFileSystemAndTrackerDatabase()
{
}

void StorageTracker::setOriginDetails(const String& originIdentifier, const String& databaseFile)
{
}

void StorageTracker::syncSetOriginDetails(const String& originIdentifier, const String& databaseFile)
{
}

void StorageTracker::origins(Vector<RefPtr<SecurityOrigin> >& result)
{
}

void StorageTracker::deleteAllOrigins()
{
}
    
void StorageTracker::syncDeleteAllOrigins()
{
}

void StorageTracker::deleteOriginWithIdentifier(const String& originIdentifier)
{
}

void StorageTracker::deleteOrigin(SecurityOrigin* origin)
{
}

void StorageTracker::syncDeleteOrigin(const String& originIdentifier)
{
}
    
void StorageTracker::willDeleteAllOrigins()
{
}

void StorageTracker::willDeleteOrigin(const String& originIdentifier)
{
}

    
bool StorageTracker::canDeleteOrigin(const String& originIdentifier)
{
    return false;
}

void StorageTracker::cancelDeletingOrigin(const String& originIdentifier)
{
}

bool StorageTracker::isActive()
{
    return m_isActive;
}

void StorageTracker::setIsActive(bool flag)
{
    m_isActive = flag;
}
    
String StorageTracker::databasePathForOrigin(const String& originIdentifier)
{
    return String();
}
    
long long StorageTracker::diskUsageForOrigin(SecurityOrigin* origin)
{
    return 0;
}

} // namespace WebCore
