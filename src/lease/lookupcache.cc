#include "../lease/lookupcache.h"

#include <assert.h>

namespace leaffs {

using namespace leveldb;

namespace {
static
void DeleteLeaseEntry(const Slice& path, void* value) {
    if (value != NULL) {
        delete reinterpret_cast<LeaseEntry*>(value);
    }
}
}

//这个没有考虑cache不命中的情况，也就是它的删除假定path对应的是在cache里的。
//通过阅读源码，发现它是有判断的，所以可以在cache不命中的情况下，直接调用它。
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
    printf("%s\n", "get 1.0");
    if (handle == NULL) {
        return NULL;
    }
    void* value = cache_->Value(handle);
    printf("%s\n", "get 2.0");
    LeaseEntry* entry = reinterpret_cast<LeaseEntry*>(value);
    printf("%s\n", "get 3.0");
    assert(entry->handle_ == handle);
    printf("%s\n", "get 4.0");
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

LeaseEntry* LookupCache::New(const Slice& path, DirectoryMeta* dir_meta) {
    assert(cache_->Lookup(path) == NULL);
    LeaseEntry* entry = new DirMetaEntry(dir_meta);
    Cache::Handle* handle = cache_->Insert(path, entry, 1, &DeleteLeaseEntry);
    entry->cache_ = this;
    entry->handle_ = handle;
    return entry;
}

LeaseEntry* LookupCache::New(const Slice& path, FileMeta* file_meta) {
    assert(cache_->Lookup(path) == NULL);
    LeaseEntry* entry = new FileMetaEntry(file_meta);
    Cache::Handle* handle = cache_->Insert(path, entry, 1, &DeleteLeaseEntry);
    entry->cache_ = this;
    entry->handle_ = handle;
    return entry;
}

} // namespace lightfs
