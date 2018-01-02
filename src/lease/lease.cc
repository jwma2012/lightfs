#include "lease/lease.h"

static const uint64_t kEpsilon = 10 * 1000;
static const uint64_t kLeaseTime = 1000 * 1000; //1秒钟

namespace leaffs {

ReadLock::ReadLock(LeaseEntry* entry, Env* env) :
    env_(env), entry_(entry) {

  while (entry_->lease_state_ == kWrite) {
    uint64_t now = env_->NowMicros();
    if (now + kEpsilon < entry_->lease_due_) {
      break;
    }
    guard_->Wait(); // Wait for the current writer
    //为了提升性能直接使用gcc的cas
    //
  }
  if (entry_->lease_state_ == kFree) {
    entry_->lease_state_ = kRead;
  }
}