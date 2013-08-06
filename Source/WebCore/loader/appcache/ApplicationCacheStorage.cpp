/*
 * Copyright (C) 2008, 2009, 2010, 2011 Apple Inc. All Rights Reserved.
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
#include "loader/appcache/ApplicationCacheStorage.h"

#include "loader/appcache/ApplicationCache.h"
#include "loader/appcache/ApplicationCacheGroup.h"
#include "loader/appcache/ApplicationCacheHost.h"
#include "loader/appcache/ApplicationCacheResource.h"
#include "platform/FileSystem.h"
#include "platform/KURL.h"
#include "platform/NotImplemented.h"
#include "page/SecurityOrigin.h"
#include "platform/UUID.h"
#include <wtf/text/CString.h>
#include <wtf/StdLibExtras.h>
#include <wtf/StringExtras.h>
#include <wtf/text/StringBuilder.h>

using namespace std;

namespace WebCore {

static const char flatFileSubdirectory[] = "ApplicationCache";

template <class T>
class StorageIDJournal {
public:  
    ~StorageIDJournal()
    {
        size_t size = m_records.size();
        for (size_t i = 0; i < size; ++i)
            m_records[i].restore();
    }

    void add(T* resource, unsigned storageID)
    {
        m_records.append(Record(resource, storageID));
    }

    void commit()
    {
        m_records.clear();
    }

private:
    class Record {
    public:
        Record() : m_resource(0), m_storageID(0) { }
        Record(T* resource, unsigned storageID) : m_resource(resource), m_storageID(storageID) { }

        void restore()
        {
            m_resource->setStorageID(m_storageID);
        }

    private:
        T* m_resource;
        unsigned m_storageID;
    };

    Vector<Record> m_records;
};

static unsigned urlHostHash(const KURL& url)
{
    unsigned hostStart = url.hostStart();
    unsigned hostEnd = url.hostEnd();

    const String& urlString = url.string();

    if (urlString.is8Bit())
        return AlreadyHashed::avoidDeletedValue(StringHasher::computeHashAndMaskTop8Bits(urlString.characters8() + hostStart, hostEnd - hostStart));
    
    return AlreadyHashed::avoidDeletedValue(StringHasher::computeHashAndMaskTop8Bits(urlString.characters16() + hostStart, hostEnd - hostStart));
}

ApplicationCacheGroup* ApplicationCacheStorage::loadCacheGroup(const KURL& manifestURL)
{
    return 0;
}    

ApplicationCacheGroup* ApplicationCacheStorage::findOrCreateCacheGroup(const KURL& manifestURL)
{
    ASSERT(!manifestURL.hasFragmentIdentifier());

    CacheGroupMap::AddResult result = m_cachesInMemory.add(manifestURL, 0);
    
    if (!result.isNewEntry) {
        ASSERT(result.iterator->value);
        return result.iterator->value;
    }

    // Look up the group in the database
    ApplicationCacheGroup* group = loadCacheGroup(manifestURL);
    
    // If the group was not found we need to create it
    if (!group) {
        group = new ApplicationCacheGroup(manifestURL);
        m_cacheHostSet.add(urlHostHash(manifestURL));
    }
    
    result.iterator->value = group;
    
    return group;
}

ApplicationCacheGroup* ApplicationCacheStorage::findInMemoryCacheGroup(const KURL& manifestURL) const
{
    return m_cachesInMemory.get(manifestURL);
}

void ApplicationCacheStorage::loadManifestHostHashes()
{
}    

ApplicationCacheGroup* ApplicationCacheStorage::cacheGroupForURL(const KURL& url)
{
    return 0;
}
ApplicationCacheGroup* ApplicationCacheStorage::fallbackCacheGroupForURL(const KURL& url)
{
    return 0;
}

void ApplicationCacheStorage::cacheGroupDestroyed(ApplicationCacheGroup* group)
{
    if (group->isObsolete()) {
        ASSERT(!group->storageID());
        ASSERT(m_cachesInMemory.get(group->manifestURL()) != group);
        return;
    }

    ASSERT(m_cachesInMemory.get(group->manifestURL()) == group);

    m_cachesInMemory.remove(group->manifestURL());
    
    // If the cache group is half-created, we don't want it in the saved set (as it is not stored in database).
    if (!group->storageID())
        m_cacheHostSet.remove(urlHostHash(group->manifestURL()));
}

void ApplicationCacheStorage::cacheGroupMadeObsolete(ApplicationCacheGroup* group)
{
    ASSERT(m_cachesInMemory.get(group->manifestURL()) == group);
    ASSERT(m_cacheHostSet.contains(urlHostHash(group->manifestURL())));

    if (ApplicationCache* newestCache = group->newestCache())
        remove(newestCache);

    m_cachesInMemory.remove(group->manifestURL());
    m_cacheHostSet.remove(urlHostHash(group->manifestURL()));
}

void ApplicationCacheStorage::setCacheDirectory(const String& cacheDirectory)
{
    ASSERT(m_cacheDirectory.isNull());
    ASSERT(!cacheDirectory.isNull());
    
    m_cacheDirectory = cacheDirectory;
}

const String& ApplicationCacheStorage::cacheDirectory() const
{
    return m_cacheDirectory;
}

void ApplicationCacheStorage::setMaximumSize(int64_t size)
{
    m_maximumSize = size;
}

int64_t ApplicationCacheStorage::maximumSize() const
{
    return m_maximumSize;
}

bool ApplicationCacheStorage::isMaximumSizeReached() const
{
    return m_isMaximumSizeReached;
}

int64_t ApplicationCacheStorage::spaceNeeded(int64_t cacheToSave)
{
    int64_t spaceNeeded = 0;
    return spaceNeeded;
}

void ApplicationCacheStorage::setDefaultOriginQuota(int64_t quota)
{
    m_defaultOriginQuota = quota;
}

bool ApplicationCacheStorage::calculateQuotaForOrigin(const SecurityOrigin* origin, int64_t& quota)
{
    return false;
}

bool ApplicationCacheStorage::calculateUsageForOrigin(const SecurityOrigin* origin, int64_t& usage)
{
    return false;
}

bool ApplicationCacheStorage::calculateRemainingSizeForOriginExcludingCache(const SecurityOrigin* origin, ApplicationCache* cache, int64_t& remainingSize)
{
    return false;
}

bool ApplicationCacheStorage::storeUpdatedQuotaForOrigin(const SecurityOrigin* origin, int64_t quota)
{
    return false;
}

static const int schemaVersion = 7;
    
void ApplicationCacheStorage::verifySchemaVersion()
{
}
    
void ApplicationCacheStorage::openDatabase(bool createIfDoesNotExist)
{
}

bool ApplicationCacheStorage::store(ApplicationCacheGroup* group, GroupStorageIDJournal* journal)
{
    return false;
}    

bool ApplicationCacheStorage::store(ApplicationCache* cache, ResourceStorageIDJournal* storageIDJournal)
{
    return false;
}

bool ApplicationCacheStorage::store(ApplicationCacheResource* resource, unsigned cacheStorageID)
{
    return false;
}

bool ApplicationCacheStorage::storeUpdatedType(ApplicationCacheResource* resource, ApplicationCache* cache)
{
    return false;
}

bool ApplicationCacheStorage::store(ApplicationCacheResource* resource, ApplicationCache* cache)
{
    return false;
}

bool ApplicationCacheStorage::ensureOriginRecord(const SecurityOrigin* origin)
{
    return true;
}

bool ApplicationCacheStorage::checkOriginQuota(ApplicationCacheGroup* group, ApplicationCache* oldCache, ApplicationCache* newCache, int64_t& totalSpaceNeeded)
{
    return false;
}

bool ApplicationCacheStorage::storeNewestCache(ApplicationCacheGroup* group, ApplicationCache* oldCache, FailureReason& failureReason)
{
    return false;
}

bool ApplicationCacheStorage::storeNewestCache(ApplicationCacheGroup* group)
{
    return false;
}

template <typename CharacterType>
static inline void parseHeader(const CharacterType* header, size_t headerLength, ResourceResponse& response)
{
}

static inline void parseHeaders(const String& headers, ResourceResponse& response)
{
}
    
PassRefPtr<ApplicationCache> ApplicationCacheStorage::loadCache(unsigned storageID)
{
    return 0;
}    
    
void ApplicationCacheStorage::remove(ApplicationCache* cache)
{
}

void ApplicationCacheStorage::empty()
{
}
    
void ApplicationCacheStorage::deleteTables()
{
}
    
bool ApplicationCacheStorage::shouldStoreResourceAsFlatFile(ApplicationCacheResource* resource)
{
    return false;
}
    
bool ApplicationCacheStorage::writeDataToUniqueFileInDirectory(SharedBuffer* data, const String& directory, String& path, const String& fileExtension)
{
    return true;
}

bool ApplicationCacheStorage::storeCopyOfCache(const String& cacheDirectory, ApplicationCacheHost* cacheHost)
{
    return false;
}

bool ApplicationCacheStorage::manifestURLs(Vector<KURL>* urls)
{
    return true;
}

bool ApplicationCacheStorage::cacheGroupSize(const String& manifestURL, int64_t* size)
{
    return false;
}

bool ApplicationCacheStorage::deleteCacheGroup(const String& manifestURL)
{
    return false;
}

void ApplicationCacheStorage::vacuumDatabaseFile()
{
}

void ApplicationCacheStorage::checkForMaxSizeReached()
{
}
    
void ApplicationCacheStorage::checkForDeletedResources()
{
}
    
long long ApplicationCacheStorage::flatFileAreaSize()
{
    return 0;
}
void ApplicationCacheStorage::getOriginsWithCache(HashSet<RefPtr<SecurityOrigin> >& origins)
{
}

void ApplicationCacheStorage::deleteAllEntries()
{
}

ApplicationCacheStorage::ApplicationCacheStorage() 
    : m_maximumSize(ApplicationCacheStorage::noQuota())
    , m_isMaximumSizeReached(false)
    , m_defaultOriginQuota(ApplicationCacheStorage::noQuota())
{
}

ApplicationCacheStorage& cacheStorage()
{
    DEFINE_STATIC_LOCAL(ApplicationCacheStorage, storage, ());
    
    return storage;
}

} // namespace WebCore
