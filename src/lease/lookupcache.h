#ifndef LEAFFS_COMMON_LOOKUPCACHE_H_
#define LEAFFS_COMMON_LOOKUPCACHE_H_

#include "util/cache.h"

namespace leaffs {

class LookupCache {
 public:
    static const int kCacheSize = 256;

    void Evict(const Slice& path);
    void Release(LeaseEntry* entry);
    LeaseEntry* Get(const Slice& path);
    LeaseEntry* New(const Slice& path, const DirectoryMeta* dir_meta);
    LeaseEntry* New(const Slice& path, const FileMeta* file_meta);

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