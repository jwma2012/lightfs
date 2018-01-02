// Copyright (c) 2014 The IndexFS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef _INDEXFS_COMMON_LEASECTRL_H_
#define _INDEXFS_COMMON_LEASECTRL_H_

//#include "common/common.h"
//#include "common/options.h"
#include "lock.h"

namespace indexfs {

class ReadLock;
class WriteLock;
class LeaseTable;

class LeaseEntry {
 public:

  LeaseEntry();

  int16_t src_server;

  LeaseTable* GetTable() { return table_; }

 private:
  int lease_state_;
  uint64_t lease_due_;
  friend class ReadLock;
  friend class WriteLock;

  LeaseTable* table_;
  Cache::Handle* handle_;
  friend class LeaseTable;

  // No copying allowed
  LeaseEntry(const LeaseEntry&);
  LeaseEntry& operator=(const LeaseEntry&);
};

class ReadLock {
 public:

  explicit ReadLock(LeaseEntry* entry);

  ~ReadLock();

 private:
  LockService* Lock;
  //锁需额外的8byte空间。
  LeaseEntry* entry_;

  // No copying allowed
  ReadLock(const ReadLock&);
  ReadLock& operator=(const ReadLock&);
};

class WriteLock {
 public:

  explicit WriteLock(LeaseEntry* entry, DirGuard* guard, Env* env);

  ~WriteLock();

 private:
  LockService* Lock;
  LeaseEntry* entry_;

  // No copying allowed
  WriteLock(const WriteLock&);
  WriteLock& operator=(const WriteLock&);
};

class LeaseTable {
 public:

  static uint64_t FetchLeaseDue(LeaseEntry* entry) {
    return entry->lease_due_;
  }

  void Evict(const OID& oid);
  void Release(LeaseEntry* entry);
  LeaseEntry* Get(const OID& oid);
  LeaseEntry* New(const OID& oid, const StatInfo& info);

  LeaseTable(int cap = (1 << 30)) { cache_ = NewLRUCache(cap); }
  //2的30次方，1G的空间

  virtual ~LeaseTable() { delete cache_; }

 private:
  Cache* cache_;

  // No copying allowed
  LeaseTable(const LeaseTable&);
  LeaseTable& operator=(const LeaseTable&);
};

class LeaseGuard {
 public:
  explicit LeaseGuard(LeaseEntry* le) : le_(le) {
    table_ = le_->GetTable();
  }
  ~LeaseGuard() {
    table_->Release(le_);
  }

 private:
  LeaseEntry* le_;
  LeaseTable* table_;

  // No copying allowed
  LeaseGuard(const LeaseGuard&);
  LeaseGuard& operator=(const LeaseGuard&);
};

} // namespace indexfs

#endif /* _INDEXFS_COMMON_LEASECTRL_H_ */