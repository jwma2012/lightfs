#include "serverlease.h"
#include <iostream>

using namespace std;

ServerLeaseList::ServerLeaseList() {
    head_ = NULL;
    length_ = 0;
}

ServerLeaseList::~ServerLeaseList() {
    ServerLeaseNode *temp;
    for(int i = 0; i < length_; i++)
    {
        temp = head_;
        head_= head_->next;
        delete temp;
    }
}

//得到链表长度
int ServerLeaseList::GetLength() {
    return length_;
}

//在链表头部插入结点
void ServerLeaseList::InsertHead(ServerLeaseInfo val) {
    Insert(val,0);
}

//插入结点
void ServerLeaseList::Insert(ServerLeaseInfo val, int pos) {
    if (pos < 0) {
        cout<<"pos must be greater than zero"<<endl;
        return;
    }
    int index = 1;
    ServerLeaseNode *temp = head;
    ServerLeaseNode *node = new ServerLeaseNode(val);
    if(pos == 0) {
        node->next = temp;
        head = node;
        length_++;
        return;
    }
    while(temp != NULL && index < pos) {
        temp = temp->next;
        index++;
    }
    if (temp == NULL) {
        cout<<"Insert failed"<<endl;
        return;
    }
    node->next = temp->next;
    temp->next = node;
    length_++;
}

//删除结点
void ServerLeaseList::Remove(ServerLeaseInfo val) {
    int pos = Find(val);
    if (pos == -1) {
        cout<<"Delete failed"<<endl;
        return;
    }
    if (pos == 1) {
        head = head->next;
        length--;
        return;
    }
    int index = 2;
    ServerLeaseNode *temp = head;
    while (index < pos)
        temp = temp->next;
    temp->next = temp->next->next;
    length--;
}

//查找结点位置
int ServerLeaseList::Find(ServerLeaseInfo val) {
    ServerLease *temp = head;
    int index = 1;
    while (temp != NULL) {
        if ((temp->lease).equal(&val))
            return index;
        temp = temp->next;
        index++;
    }
    return -1; //不存在返回-1
}

//链表反序
void ServerLeaseList::Reverse() {
    if(head==NULL)
        return;
    ServerLeaseNode* curNode = head;
    ServerLeaseNode* nextNode = head->next;
    ServerLeaseNode* temp;
    while (nextNode != NULL) {
        temp = nextNode->next;
        nextNode->next = curNode;
        curNode = nextNode;
        nextNode = temp;
    }
    head->next = NULL;
    head = curNode;
}

//打印链表
void ServerLeaseList::Print() {
    if(head == NULL) {
        cout<<"ServerLeaseList is empty"<<endl;
        return;
    }
    ServerLeaseNode* temp = head;
    while (temp != NULL) {
        (temp->lease_).PrintInfo();
        temp = temp->next;
    }
    cout<<endl;
}

static
void DeleteServerLeaseEntry(const Slice& path, void* value) {
    if (value != NULL) {
        delete reinterpret_cast<ServerLeaseEntry*>(value);
    }
}


void LeaseTable::Evict(const Slice& path) {
    cache_->Erase(path);
}

void LeaseTable::Release(ServerLeaseEntry* entry) {
    if (entry != NULL) {
    assert(entry->handle_ != NULL);
        cache_->Release(entry->handle_);
    }
}

ServerLeaseEntry* LeaseTable::Get(const Slice& path) {
    Cache::Handle* handle = cache_->Lookup(path);
    if (handle == NULL) {
        return NULL;
    }
    void* value = cache_->Value(handle);
    ServerLeaseEntry* entry = reinterpret_cast<ServerLeaseEntry*>(value);
    assert(entry->handle_ == handle);
    return entry;
}

ServerLeaseEntry* LeaseTable::GetValidCache(const Slice& path) {
    ServerLeaseEntry* entry = LeaseTable::Get(path);
    if (entry == NULL) {
        return NULL;
    }
    if (entry->GetLeaseDue() < Debug::NowMicros() + kEpsilon) { //invalid
        return NULL;
    }
    return entry;
}

ServerLeaseEntry* Put(const Slice& path, ServerLeaseEntry *entry) {
    Cache::Handle* handle = cache_->Lookup(path);
    if (handle == NULL){
        Cache::Handle* handle = cache_->Insert(path, entry, 1, &DeleteServerLeaseEntry);
        entry->cache_ = this;
        entry->handle_ = handle;
    } else {
        void* value = cache_->Value(handle);
        ServerLeaseEntry* lease_entry = reinterpret_cast<ServerLeaseEntry*>(value);
        assert(lease_entry->handle_ == handle);
        (lease_entry->lease_List_)->InsertHead(entry->lease_List_->head_->lease);
        //后来的，肯定是时间上靠前的
    }
    return entry;
}
