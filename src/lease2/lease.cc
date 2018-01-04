#include "lease/lease.h"

static const uint64_t kEpsilon = 10 * 1000;
static const uint64_t kLeaseTime = 1000 * 1000; //1秒钟

namespace leaffs {

LeaseEntry::LeaseEntry() :
    lease_state_(kFree), lease_due_(0) {
}

static
void DeleteLeaseEntry(const Slice& key, void* value) {
  if (value != NULL) {
    delete reinterpret_cast<LeaseEntry*>(value);
  }
}

void LeaseTable::Evict(const OID& oid) {
  std::string key;
  PutFixed64(&key, oid.dir_id);
  key.append(oid.obj_name);
  cache_->Erase(key);
}

void LeaseTable::Release(Cache::Handle* handle) {
    if (entry != NULL) {
    DLOG_ASSERT(entry->handle_ != NULL);
        cache_->Release(entry->handle_);
    }
}

LeaseEntry* LeaseTable::Get(const Slice& key) {

    Cache::Handle* handle = cache_->Lookup(key);
    if (handle == NULL) {
        return NULL;
    }
    void* value = cache_->Value(handle);
    LeaseEntry* entry = reinterpret_cast<LeaseEntry*>(value);
    DLOG_ASSERT(entry->handle_ == handle);
    return entry;
}

LeaseEntry* LeaseTable::New(const Slice& key, void* value) {
    std::string key;
    PutFixed64(&key, oid.dir_id);
    key.append(oid.obj_name);
    DLOG_ASSERT(cache_->Lookup(key) == NULL);
    LeaseEntry* entry = new LeaseEntry();
    entry->inode_no = info.id;
    entry->uid = info.uid;
    entry->gid = info.gid;
    entry->perm = info.mode;
    entry->zeroth_server = info.zeroth_server;
    Cache::Handle* handle = cache_->Insert(key, value, 1, &DeleteLeaseEntry);
    entry->table_ = this;
    entry->handle_ = handle;
    return entry;
}

}