#ifndef LEAFFS_SERVER_LEASE_H_
#define LEAFFS_SERVER_LEASE_H_


#include "leveldb/cache.h"
#include "debug.hpp"
#include "common.hpp"
#include "leasecommon.h"
#include <iostream>

#define MAX_NODE 24

namespace leaffs {

using namespace std;
using namespace leveldb;

class LeaseTable;

struct ServerLeaseInfo {
    int16_t dst_server;
    int lease_state;
    uint64_t lease_due;

    int equal(ServerLeaseInfo &info) {
        if (dst_server == info.dst_server
            && lease_state == info.lease_state
            && lease_due == info.lease_due)
            return 1;
        return 0;
    }

    void PrintInfo() {
        cout<<"dst_server = "<<dst_server<<endl;
        cout<<"lease_state = "<<lease_state<<endl;
        cout<<"lease_due = "<<lease_due<<endl;
    }

     ServerLeaseInfo& operator=(ServerLeaseInfo& info) {
        this->dst_server = info.dst_server;
        this->lease_due = info.lease_due;
        this->lease_state = info.lease_state;
        return *this;
     }
};

struct ServerLeaseNode {

    ServerLeaseNode(ServerLeaseInfo info) {
        lease = info;
        next = NULL;
    }
    ~ServerLeaseNode() {
        printf("leaseNode\n");
    }

    ServerLeaseInfo lease;
    ServerLeaseNode* next;
};

class ServerLeaseList {
public:
   //构造函数
    ServerLeaseList();
    //在链表头部插入结点
    void InsertHead(ServerLeaseInfo val);
    //在链表尾部插入结点
    void InsertTail(ServerLeaseInfo val);
    //插入结点
    void Insert(ServerLeaseInfo val,int pos);
    //删除结点
    void Remove(ServerLeaseInfo val);
    //得到链表长度
    int GetLength();
    //链表反序
    void Reverse();
    //查找结点位置
    int Find(ServerLeaseInfo val);
    //打印链表
    void Print();

    ServerLeaseNode* GetHead(){
        return head_;
    }
    //析构函数
    ~ServerLeaseList();

private:
    int length_;
    ServerLeaseNode* head_;

    // No copying allowed
    ServerLeaseList(const ServerLeaseList&);
    ServerLeaseList& operator=(const ServerLeaseList&);
};

struct  MetaInfo {
    int metadata_type;
    char name[MAX_FILE_NAME_LENGTH];
    int index;  //租约在服务器节点上的table里的索引，方便直接取数据。

    MetaInfo& operator=(const MetaInfo& info) {
        this->metadata_type = info.metadata_type;
        memcpy(this->name, info.name, MAX_FILE_NAME_LENGTH);
        this->index = info.index;
        return *this;
     }
};

class ServerLeaseEntry {
public:
    ServerLeaseEntry() {
        lease_list = new ServerLeaseList();
        table_ = NULL;
        handle_ = NULL;
    };

    ServerLeaseEntry(MetaInfo *info, ServerLeaseInfo *p) {
        lease_list = new ServerLeaseList();
        meta_info = *info;
        lease_list->InsertHead(*p);
        table_ = NULL;
        handle_ = NULL;
    };

    ~ServerLeaseEntry(){
        delete lease_list;
    }

    uint64_t GetLastLeaseDue() {
        return (lease_list->GetHead()->lease).lease_due;
    }

    ServerLeaseInfo* GetLeaseInfo() {
        if (lease_list->GetLength() == 1)
            return &((lease_list->GetHead())->lease);
        return NULL;
    }

    int AddLeaseInfo(ServerLeaseInfo info){
        lease_list->InsertHead(info);
        return 0;
    }

    MetaInfo meta_info;
    ServerLeaseList* lease_list;
private:

    //原本设计一个元数据最多授予24个节点租约，所以系统最大的节点数是25，为了简单没有用链表。
    //后来想链表不会费很大的事就用链表
    //其实可以用stl自带的linkedList的，就不用这么麻烦了

    LeaseTable* table_;
    Cache::Handle* handle_;
    friend class LeaseTable;
};

class ReadLock {
 public:

  explicit ReadLock(ServerLeaseEntry* entry);

  ~ReadLock();

 private:
  ServerLeaseEntry* entry_;

  // No copying allowed
  ReadLock(const ReadLock&);
  ReadLock& operator=(const ReadLock&);
};

class WriteLock {
 public:

  explicit WriteLock(ServerLeaseEntry* entry);

  ~WriteLock();

 private:
  ServerLeaseEntry* entry_;

  // No copying allowed
  WriteLock(const WriteLock&);
  WriteLock& operator=(const WriteLock&);
};

class LeaseTable {
 public:
    static const int kCacheSize = 256;

    void Evict(const Slice& path);
    void Release(ServerLeaseEntry* entry);
    ServerLeaseEntry* Get(const Slice& path);
    ServerLeaseEntry* GetValidCache(const Slice& path);
    ServerLeaseEntry* Put(const Slice& path, ServerLeaseEntry *lease);

    LeaseTable(int cap = kCacheSize) { cache_ = NewLRUCache(cap); }

    virtual ~LeaseTable() { delete cache_; }//目前为了简单，不需要做所有lease的删除工作，以后可以加上

private:
    Cache* cache_;

    // No copying allowed
    //禁止拷贝构造函数
    LeaseTable(const LeaseTable&);
    LeaseTable& operator=(const LeaseTable&);
};


} /* namespace leaffs*/
#endif /* LEAFFS_SERVER_LEASE_H_ */