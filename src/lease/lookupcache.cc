#include "lease/lookupcache.h"
#include "lease/lease.h"
#include <assert.h>

namespace leaffs {

namespace {
static
void DeleteLeaseEntry(const Slice& path, void* value) {
    if (value != NULL) {
        delete reinterpret_cast<LeaseEntry*>(value);
    }
}
}

void LookupCache::Evict(const Slice& path) {
    cache_->Erase(path);
}

void LookupCache::Release(LeaseEntry* entry) {
    if (entry != NULL) {
    assert(entry->handle_ != NULL);
        cache_->Release(entry->handle_);
    }
}

LeaseEntry* LookupCache::Get(const Slice& path) {
    Cache::Handle* handle = cache_->Lookup(path);
    if (handle == NULL) {
        return NULL;
    }
    void* value = cache_->Value(handle);
    LeaseEntry* entry = reinterpret_cast<LeaseEntry*>(value);
    assert(entry->handle_ == handle);
    return entry;
}

LeaseEntry* LookupCache::GetValidCache(const Slice& path) {
    LeaseEntry* entry = LookupCache::Get(path);
    if (entry == NULL) {
        return NULL;
    }
    if (entry->GetLeaseDue() < Debug::NowMicros() + kEpsilon) { //invalid
        return NULL;
    }
    return entry;
}

LeaseEntry* LookupCache::Get(const Slice& path, MetadataType type) {
    Cache::Handle* handle = cache_->Lookup(path);
    if (handle == NULL) {
        return NULL;
    }
    void* value = cache_->Value(handle);
    LeaseEntry* entry = reinterpret_cast<LeaseEntry*>(value);
    assert(entry->handle_ == handle);
    return entry;
}

LeaseEntry* New(const Slice& path, const DirectoryMeta* dir_meta) {
    assert(cache_->Lookup(path) == NULL);
    LeaseEntry* entry = new LeaseEntry(dir_meta);
    Cache::Handle* handle = cache_->Insert(path, entry, 1, &DeleteLeaseEntry);
    entry->cache_ = this;
    entry->handle_ = handle;
    return entry;
}

LeaseEntry* New(const Slice& path, const FileMeta* file_meta) {
    assert(cache_->Lookup(path) == NULL);
    LeaseEntry* entry = new LeaseEntry(dir_meta);
    Cache::Handle* handle = cache_->Insert(path, entry, 1, &DeleteLeaseEntry);
    entry->cache_ = this;
    entry->handle_ = handle;
    return entry;
}

} // namespace indexfs
