#ifndef LEAFFS_LEASE_COMMON_H_
#define LEAFFS_LEASE_COMMON_H_

namespace leaffs {
namespace {
enum LeaseState {
    kFree,
    kRead,
    kWrite,
};

enum MetadataType {
    kDir,
    kFile,
};

static const uint64_t kEpsilon = 10 * 1000;
static const uint64_t kLeaseTime = 1000 * 1000; //1秒钟

} //anonymous namespace
} //namespace leaffs

#endif/* LEAFFS_LEASE_COMMON_H_ */