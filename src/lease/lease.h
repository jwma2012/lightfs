#ifndef LEAFFS_COMMON_LEASE_H_
#define LEAFFS_COMMON_LEASE_H_

namespace leaffs {

namespace {
enum LeaseState {
    kFree,
    kRead,
    kWrite,
};
}

class LeaseEntry {
public:
    LeaseEntry();

 private:
    int16_t src_server;
    int lease_state_;
    uint64_t lease_due_;
    friend class ReadLock;
    friend class WriteLock;

    LeaseTable* table_;
    Cache::Handle* handle_;
    friend class LeaseTable;

}

class ReadLock {
 public:

  explicit ReadLock(LeaseEntry* entry);

  ~ReadLock();

 private:
  Env* env_;
  DirGuard* guard_;
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
  Env* env_;
  DirGuard* guard_;
  LeaseEntry* entry_;

  // No copying allowed
  WriteLock(const WriteLock&);
  WriteLock& operator=(const WriteLock&);
};

}

#endif /* LEAFFS_COMMON_LEASE_H_ */