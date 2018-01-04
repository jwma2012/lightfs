#ifndef LEAFFS_COMMON_LEASE_H_
#define LEAFFS_COMMON_LEASE_H_

#include "common.hpp"

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
} //anonymous namespace

class LeaseEntry() {
public:
    LeaseEntry() { }
    virtual ~LeaseEntry() {printf("lease\n")};
    struct Metadata { }; //源自leveldb cache的写法

    static uint64_t GetLeaseDue() {
        return this->lease_due_;
    }

    virtual void Evict(const Slice& path) = 0;
    virtual void Release(Metadata* meta) = 0;
    virtual Metadata* Get(const Slice& path)  = 0;
    virtual Metadata* New(const Slice& path, void* value) = 0;
    virtual void* Value(Metadata* meta) = 0;
    static void SetMetadataType(MetadataType e) {
        metadata_type_ = e;
    }

private:
    int16_t src_server_;
    int lease_state_;
    uint64_t lease_due_;
    int metadata_type_;

    // No copying allowed
    LeaseEntry(const LeaseEntry&);
    LeaseEntry& LeaseEntry=(const LeaseEntry&);
}

class DirMetaEntry : public LeaseEntry{
public:
    DirMetaEntry() {
        LeaseEntry::SetMetadataType(kDir);
    }
    virtual ~DirMetaEntry() { {printf("DirMetaEntry\n")}; delete ptr_dir_meta_}
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
    FileMetaEntry() {
        LeaseEntry::SetMetadataType(kFile);
    }
    virtual ~FileMetaEntry() { {printf("FileMetaEntry\n")}; delete ptr_file_meta_}
    DirectoryMeta *GetFileMeta() {
        return ptr_file_meta_;
    }

private:
    FileMeta* ptr_file_meta_;

    // No copying allowed
    FileMetaEntry(const FileMetaEntry&);
    FileMetaEntry& operator=(const FileMetaEntry&);
};


} /* namespace leaffs*/
#endif /* LEAFFS_COMMON_LEASE_H_ */