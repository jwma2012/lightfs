#ifndef LEAFFS_COMMON_LOOKUPCACHE_H_
#define LEAFFS_COMMON_LOOKUPCACHE_H_

#include <assert.h>

namespace leaffs {

namespace {

static
void DeleteLookupEntry(const Slice& key, void* value) {
  if (value != NULL) {
    delete reinterpret_cast<LeaseEntry*>(value);
  }
}
}

void LookupCache::Evict(std::string path) {
  cache_->Erase(key);
}

void LookupCache::Release(LeaseEntry* entry) {
  if (entry != NULL) {
    assert(entry->handle_ != NULL);
    cache_->Release(entry->handle_);
  }
}

LookupEntry* LookupCache::Get(const OID& oid) {
  std::string key;
  PutFixed64(&key, oid.dir_id);
  key.append(oid.obj_name);
  Cache::Handle* handle = cache_->Lookup(key);
  if (handle == NULL) {
    return NULL;
  }
  void* value = cache_->Value(handle);
  LookupEntry* entry = reinterpret_cast<LookupEntry*>(value);
  DLOG_ASSERT(entry->handle_ == handle);
  return entry;
}

LookupEntry* LookupCache::New(const OID& oid, const LookupInfo& info) {
  std::string key;
  PutFixed64(&key, oid.dir_id);
  key.append(oid.obj_name);
  DLOG_ASSERT(cache_->Lookup(key) == NULL);
  LookupEntry* entry = new LookupEntry();
  entry->inode_no = info.id;
  entry->uid = info.uid;
  entry->gid = info.gid;
  entry->perm = info.perm;
  entry->zeroth_server = info.zeroth_server;
  entry->lease_due = info.lease_due;
  Cache::Handle* handle = cache_->Insert(key, entry, 1, &DeleteLookupEntry);
  entry->cache_ = this;
  entry->handle_ = handle;
  return entry;

#endif /* LEAFFS_COMMON_LOOKUPCACHE_H_ */