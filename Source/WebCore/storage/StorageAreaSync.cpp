/*
 * Copyright (C) 2008, 2009, 2010 Apple Inc. All Rights Reserved.
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
#include "StorageAreaSync.h"

#include "EventNames.h"
#include "FileSystem.h"
#include "HTMLElement.h"
#include "SecurityOrigin.h"
#include "StorageAreaImpl.h"
#include "StorageSyncManager.h"
#include "StorageTracker.h"
#include "SuddenTermination.h"
#include <wtf/Functional.h>
#include <wtf/MainThread.h>
#include <wtf/text/CString.h>

namespace WebCore {

// If the StorageArea undergoes rapid changes, don't sync each change to disk.
// Instead, queue up a batch of items to sync and actually do the sync at the following interval.
static const double StorageSyncInterval = 1.0;

// A sane limit on how many items we'll schedule to sync all at once.  This makes it
// much harder to starve the rest of LocalStorage and the OS's IO subsystem in general.
static const int MaxiumItemsToSync = 100;

inline StorageAreaSync::StorageAreaSync(PassRefPtr<StorageSyncManager> storageSyncManager, PassRefPtr<StorageAreaImpl> storageArea, const String& databaseIdentifier)
    : m_syncTimer(this, &StorageAreaSync::syncTimerFired)
    , m_itemsCleared(false)
    , m_finalSyncScheduled(false)
    , m_storageArea(storageArea)
    , m_syncManager(storageSyncManager)
    , m_databaseIdentifier(databaseIdentifier.isolatedCopy())
    , m_clearItemsWhileSyncing(false)
    , m_syncScheduled(false)
    , m_syncInProgress(false)
    , m_databaseOpenFailed(false)
    , m_syncCloseDatabase(false)
    , m_importComplete(false)
{
    ASSERT(isMainThread());
    ASSERT(m_storageArea);
    ASSERT(m_syncManager);

    // FIXME: If it can't import, then the default WebKit behavior should be that of private browsing,
    // not silently ignoring it. https://bugs.webkit.org/show_bug.cgi?id=25894
    m_syncManager->dispatch(bind(&StorageAreaSync::performImport, this));
}

PassRefPtr<StorageAreaSync> StorageAreaSync::create(PassRefPtr<StorageSyncManager> storageSyncManager, PassRefPtr<StorageAreaImpl> storageArea, const String& databaseIdentifier)
{
    RefPtr<StorageAreaSync> area = adoptRef(new StorageAreaSync(storageSyncManager, storageArea, databaseIdentifier));

    return area.release();
}

StorageAreaSync::~StorageAreaSync()
{
    ASSERT(isMainThread());
    ASSERT(!m_syncTimer.isActive());
    ASSERT(m_finalSyncScheduled);
}

void StorageAreaSync::scheduleFinalSync()
{
    ASSERT(isMainThread());
    // FIXME: We do this to avoid races, but it'd be better to make things safe without blocking.
    blockUntilImportComplete();
    m_storageArea = 0;  // This is done in blockUntilImportComplete() but this is here as a form of documentation that we must be absolutely sure the ref count cycle is broken.

    if (m_syncTimer.isActive())
        m_syncTimer.stop();
    else {
        // The following is balanced by the call to enableSuddenTermination in the
        // syncTimerFired function.
        disableSuddenTermination();
    }
    // FIXME: This is synchronous.  We should do it on the background process, but
    // we should do it safely.
    m_finalSyncScheduled = true;
    syncTimerFired(&m_syncTimer);

    m_syncManager->dispatch(bind(&StorageAreaSync::deleteEmptyDatabase, this));
}

void StorageAreaSync::scheduleItemForSync(const String& key, const String& value)
{
    ASSERT(isMainThread());
    ASSERT(!m_finalSyncScheduled);

    m_changedItems.set(key, value);
    if (!m_syncTimer.isActive()) {
        m_syncTimer.startOneShot(StorageSyncInterval);

        // The following is balanced by the call to enableSuddenTermination in the
        // syncTimerFired function.
        disableSuddenTermination();
    }
}

void StorageAreaSync::scheduleClear()
{
    ASSERT(isMainThread());
    ASSERT(!m_finalSyncScheduled);

    m_changedItems.clear();
    m_itemsCleared = true;
    if (!m_syncTimer.isActive()) {
        m_syncTimer.startOneShot(StorageSyncInterval);

        // The following is balanced by the call to enableSuddenTermination in the
        // syncTimerFired function.
        disableSuddenTermination();
    }
}

void StorageAreaSync::scheduleCloseDatabase()
{
}

void StorageAreaSync::syncTimerFired(Timer<StorageAreaSync>*)
{
    ASSERT(isMainThread());

    bool partialSync = false;
    {
        MutexLocker locker(m_syncLock);

        // Do not schedule another sync if we're still trying to complete the
        // previous one.  But, if we're shutting down, schedule it anyway.
        if (m_syncInProgress && !m_finalSyncScheduled) {
            ASSERT(!m_syncTimer.isActive());
            m_syncTimer.startOneShot(StorageSyncInterval);
            return;
        }

        if (m_itemsCleared) {
            m_itemsPendingSync.clear();
            m_clearItemsWhileSyncing = true;
            m_itemsCleared = false;
        }

        HashMap<String, String>::iterator changed_it = m_changedItems.begin();
        HashMap<String, String>::iterator changed_end = m_changedItems.end();
        for (int count = 0; changed_it != changed_end; ++count, ++changed_it) {
            if (count >= MaxiumItemsToSync && !m_finalSyncScheduled) {
                partialSync = true;
                break;
            }
            m_itemsPendingSync.set(changed_it->key.isolatedCopy(), changed_it->value.isolatedCopy());
        }

        if (partialSync) {
            // We can't do the fast path of simply clearing all items, so we'll need to manually
            // remove them one by one.  Done under lock since m_itemsPendingSync is modified by
            // the background thread.
            HashMap<String, String>::iterator pending_it = m_itemsPendingSync.begin();
            HashMap<String, String>::iterator pending_end = m_itemsPendingSync.end();
            for (; pending_it != pending_end; ++pending_it)
                m_changedItems.remove(pending_it->key);
        }

        if (!m_syncScheduled) {
            m_syncScheduled = true;

            // The following is balanced by the call to enableSuddenTermination in the
            // performSync function.
            disableSuddenTermination();

            m_syncManager->dispatch(bind(&StorageAreaSync::performSync, this));
        }
    }

    if (partialSync) {
        // If we didn't finish syncing, then we need to finish the job later.
        ASSERT(!m_syncTimer.isActive());
        m_syncTimer.startOneShot(StorageSyncInterval);
    } else {
        // The following is balanced by the calls to disableSuddenTermination in the
        // scheduleItemForSync, scheduleClear, and scheduleFinalSync functions.
        enableSuddenTermination();

        m_changedItems.clear();
    }
}

void StorageAreaSync::openDatabase(OpenDatabaseParamType openingStrategy)
{
}

void StorageAreaSync::migrateItemTableIfNeeded()
{
}

void StorageAreaSync::performImport()
{
}

void StorageAreaSync::markImported()
{
}

// FIXME: In the future, we should allow use of StorageAreas while it's importing (when safe to do so).
// Blocking everything until the import is complete is by far the simplest and safest thing to do, but
// there is certainly room for safe optimization: Key/length will never be able to make use of such an
// optimization (since the order of iteration can change as items are being added). Get can return any
// item currently in the map. Get/remove can work whether or not it's in the map, but we'll need a list
// of items the import should not overwrite. Clear can also work, but it'll need to kill the import
// job first.
void StorageAreaSync::blockUntilImportComplete()
{
}


void StorageAreaSync::sync(bool clearItems, const HashMap<String, String>& items)
{
}

void StorageAreaSync::performSync()
{
}

void StorageAreaSync::deleteEmptyDatabase()
{
}

void StorageAreaSync::scheduleSync()
{
    syncTimerFired(&m_syncTimer);
}

} // namespace WebCore
