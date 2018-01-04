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
// Copyright (c) 2014 The IndexFS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef _INDEXFS_COMMON_LOOKUPCACHE_H_
#define _INDEXFS_COMMON_LOOKUPCACHE_H_

#include "common/common.h"
#include "common/options.h"

namespace indexfs {

class LookupCache;
class LookupEntry {
public:

    LookupEntry(DirectoryMeta *dir_meta) {
        //深拷贝
        metadata_type_ = kDir;
        lease_info_ = new DirectoryMeta();
        memcpy(lease_info_,)
    }

    LookupCache* GetCache() { return cache_; }


private:
    LookupCache* cache_;
    Cache::Handle* handle_;
    int metadata_type_;
    friend class LookupCache;
    LeaseInfo *lease_info_;
    // No copying allowed
    LookupEntry(const LookupEntry&);
    LookupEntry& operator=(const LookupEntry&);
};

class LookupCache {
 public:
    static const int kCacheSize = 256;

    void Evict(const std::string& path);
    void Release(LookupEntry* entry);
    LookupEntry* Get(const std::string& path);
    LookupEntry* New(const std::string& path, const DirectoryMeta& dir_meta);
    LookupEntry* New(const std::string& path, const FileMeta& dir_meta);

    LookupCache(int cap = kCacheSize) { cache_ = NewLRUCache(cap); }

    virtual ~LookupCache() { delete cache_; }

private:
    Cache* cache_;

    // No copying allowed
    LookupCache(const LookupCache&);
    LookupCache& operator=(const LookupCache&);
};

} // namespace leaffs
#endif /* LEAFFS_COMMON_LOOKUPCACHE_H_ */