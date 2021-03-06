#include "lease/serverlease.h"

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

static int DecodeKey(const Slice& k) {
  /*
  int i = 0;
  int num = 0;
  int t = k.data()[k.size()-i-1];
  for (;(t<'9')&&(t>'0');i++) {
    t = k.data()[k.size()-i-1];
    num += (t-'0')*(i==0?1:i*10);
  }
  return num;
  */
  return k.data()[k.size()-1] - '0';//最后一位是数字
}

static int CreateMetaInfo(int k, MetaInfo* info) {
    info->metadata_type = (k % kFileNumperDir) ? kFile : kDir;
    strcpy(info->name, EncodeKey(k).data());
    info->index = k;
    return 0;
}

static int CreateLeaseInfo(int k, ServerLeaseInfo* info) {
    info->dst_server = (k % kFileNumperDir) ? 1 : 2;
    info->lease_state = (k % 2) ? kWrite : kRead;
    info->lease_due = Debug::NowMicros();
    return 0;
}

using namespace leaffs;

int main(int argc, char** argv) {
    printf("start\n");
    LeaseTable *cache = new LeaseTable();
    ServerLeaseEntry* entry = NULL;
    printf("stage 0\n");
    int k = 0;
    while (k < kTotalEntry) {

        k++;
    }
    k = 0;
    int re;
    printf("stage 1\n");
    while (k < kTotalEntry) {
        printf("k = %d\n", k);
        entry = cache->Get(EncodeKey(k));
        //printf("k = %d", k);
        if (entry != NULL) {
            //printf("2.0 k = %d", k);
            sleep(1);
            printf("type = %d,%d\n", entry->GetMetadataType(), kDir);
            printf("1.0 %s\n", "hit");
            DecodeValue(entry);
            printf("2.0 %s\n", "hit");
            cache->Release(entry);
        }
        else
            printf("%s\n", "miss");
        k++;
    }

    printf("end\n");
    return 0;
}
