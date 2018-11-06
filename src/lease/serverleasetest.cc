#include "../lease/serverlease.h"

#include <vector>
#include <iostream>
#include <unistd.h>

/**
dir0
|
file1-file2-file3-file4-dir5
                          |
                        file6-file7-file8-file9-dir10
*/

namespace leaffs {

static std::vector<int> deleted_keys_;
static const int kTotalEntry = 10;
static const int kFileNumperDir = 5;
/*
混合文件和目录的元数据 cache 的测试
*/

static std::string EncodeKey(int k) {
  std::string result;
  char path[MAX_PATH_LENGTH] = "";
  for (int i = 0; i <= k/kFileNumperDir; i++) {
    sprintf(path, "%s/dir_%d", path, i*kFileNumperDir);
  }
  if (k % kFileNumperDir) {
    sprintf(path, "%s/file_%d", path, k);
  }
  result.append(path);
  std::cout<<"EncodeKey : "<<result<<std::endl;
  return result;
}

static int CreateMetaInfo(int k, MetaInfo* info) {
    info->metadata_type = (k % kFileNumperDir) ? kFile : kDir;
    strcpy(info->name, EncodeKey(k).data());
    info->index = k;
    return 0;
}

static int CreateLeaseInfo(int k, ServerLeaseInfo* info) {
    info->dst_server = (k % kFileNumperDir) ? 1 : 2;
    info->lease_state = (k % kFileNumperDir) ? kWrite : kRead;
    //write的lease太多
    info->lease_due = Debug::NowMicros() + kLeaseTime;
    return 0;
}

static int CreateLeaseEntry(int k, ServerLeaseEntry* entry) {
    CreateMetaInfo(k, &(entry->meta_info));
    ServerLeaseInfo *p = new ServerLeaseInfo();
    CreateLeaseInfo(k, p);
    (entry->lease_list)->InsertHead(*p);
    //问题：数据成员是私有的，需要增加接口
    return 0;
}

}//end of namespace leaffs

using namespace leaffs;

int main(int argc, char** argv) {
    printf("start\n");
    LeaseTable *cache = new LeaseTable();
    ServerLeaseEntry* entry = NULL;
    printf("stage 0\n");
    int k = 0;
    while (k < kTotalEntry) {
        entry = new ServerLeaseEntry();
        CreateLeaseEntry(k, entry);
        cache->Put(EncodeKey(k), entry);
        k++;
    }
    k = 0;
    printf("stage 1\n");
    while (k < kTotalEntry) {
        printf("k = %d\n", k);
        entry = cache->Get(EncodeKey(k));
        if (entry != NULL) {
            printf("%s\n", "hit");
            cache->Release(entry);
        }
        else
            printf("%s\n", "miss");
        k++;
    }

    printf("end\n");
    return 0;
}
