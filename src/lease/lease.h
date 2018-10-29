#ifndef LEAFFS_LEASE_H_
#define LEAFFS_LEASE_H_

#include "common.hpp"
#include "leveldb/cache.h"
#include "leasecommon.h"
using namespace leveldb;

namespace leaffs {

class LookupCache;
class LeaseEntry {
public:
    LeaseEntry() { }
    virtual ~LeaseEntry() {
        printf("lease\n");
    }

    uint64_t GetLeaseDue() {
        return lease_due_;
    }

    void SetMetadataType(int e) {
        metadata_type_ = e;
    }

    int GetMetadataType() const{
        return metadata_type_;
    }

    virtual DirectoryMeta *GetDirMeta(){
        //原来有const关键字的时候可以编译通过，但运行会报段错误。
        //有const修饰的函数不能改值。
        /*  一般放在函数体后，形如：void   fun()   const;
  如果一个成员函数的不会修改数据成员，那么最好将其声明为const，因为const成员函数中不允许对数据成员进行修改，如果修改，编译器将报错，这大 大提高了程序的健壮性。*/
        return NULL;
    }

    virtual FileMeta *GetFileMeta(){
        return NULL;
    }

private:
    int16_t src_server_;
    int lease_state_;
    uint64_t lease_due_;
    int metadata_type_;

    LookupCache* cache_;
    Cache::Handle* handle_;
    friend class LookupCache;

    // No copying allowed
    LeaseEntry(const LeaseEntry&);
    LeaseEntry& operator=(const LeaseEntry&);
};

class DirMetaEntry : public LeaseEntry{
public:
    DirMetaEntry(DirectoryMeta* m) {
        ptr_dir_meta_ = new DirectoryMeta();
        memcpy(ptr_dir_meta_, m, sizeof(DirectoryMeta)); //深拷贝
        LeaseEntry::SetMetadataType(kDir);
    }
    virtual ~DirMetaEntry() {
        printf("DirMetaEntry\n");
        delete ptr_dir_meta_;
    }
    DirectoryMeta *GetDirMeta() {
        return ptr_dir_meta_;
    }

private:
    DirectoryMeta* ptr_dir_meta_;

    // No copying allowed
    DirMetaEntry(const DirMetaEntry&);
    DirMetaEntry& operator=(const DirMetaEntry&);
};

class FileMetaEntry : public LeaseEntry{
public:
    FileMetaEntry(FileMeta *m) {
        ptr_file_meta_ = new FileMeta();
        memcpy(ptr_file_meta_, m, sizeof(FileMeta)); //深拷贝
        LeaseEntry::SetMetadataType(kFile);
    }
    virtual ~FileMetaEntry() {
        printf("FileMetaEntry\n");
        delete ptr_file_meta_;
    }
    FileMeta *GetFileMeta() {
        return ptr_file_meta_;
    }

private:
    FileMeta* ptr_file_meta_;

    // No copying allowed
    FileMetaEntry(const FileMetaEntry&);
    FileMetaEntry& operator=(const FileMetaEntry&);
};


} /* namespace leaffs*/
#endif /* LEAFFS_LEASE_H_ */