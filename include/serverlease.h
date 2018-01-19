#ifndef LEAFFS_SERVER_LEASE_H_
#define LEAFFS_SERVER_LEASE_H_

#define MAX_NODE 24
namespace leaffs {

class LeaseTable;
struct ServerLeaseInfo {
    int16_t dst_server;
    int lease_state;
    uint64_t lease_due;

    int equal(ServerLeaseInfo &info) {
        if (dst_server == info->dst_server
            && lease_state == info->lease_state
            && lease_due == info->lease_due)
            return 1;
        return 0;
    }

    void PrintInfo(){
        cout<<"dst_server = "<<dst_server<<endl;
        cout<<"lease_state = "<<lease_state<<endl;
        cout<<"lease_due = "<<lease_due<<endl;
    }
};

class ServerLeaseNode {
public:
    LeaseEntry(ServerLeaseInfo info) { lease_ = info;}
    virtual ~LeaseEntry() {
        printf("leaseNode\n");
    }
    ServerLeaseInfo& GetLeaseInfo(){
        return &lease_;
    }

private:
    ServerLeaseInfo lease_;
    ServerLeaseNode* next_;
};

class ServerLeaseList {
public:

    ServerLeaseList();
    ~ServerLeaseList();

private:
    int length_;
    ServerLeaseNode* head_;

    // No copying allowed
    ServerLeaseList(const ServerLeaseList&);
    ServerLeaseList& operator=(const ServerLeaseList&);
};

class ServerLeaseEntry {
public:
    ServerLeaseEntry() {
        lease_list_ = NULL;
    };
    ~ServerLeaseEntry(){
        delete lease_list_;
    }

private:
    int metadata_type_;
    char name_[MAX_FILE_NAME_LENGTH];
    int index_;  //租约在服务器节点上的table里的索引，方便直接取数据。
    ServerLeaseList* lease_list_;
    //原本设计一个元数据最多授予24个节点租约，所以系统最大的节点数是25，为了简单没有用链表。
    //后来想链表不会费很大的事就用链表
    //其实可以用stl自带的linkedList的，就不用这么麻烦了

    LeaseTable* table_;
    Cache::Handle* handle_;
    friend class LeaseTable;
};

class ReadLock {
 public:

  explicit ReadLock(LeaseEntry* entry);

  ~ReadLock();

 private:
  LeaseEntry* entry_;

  // No copying allowed
  ReadLock(const ReadLock&);
  ReadLock& operator=(const ReadLock&);
};

class WriteLock {
 public:

  explicit WriteLock(LeaseEntry* entry);

  ~WriteLock();

 private:
  LeaseEntry* entry_;

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