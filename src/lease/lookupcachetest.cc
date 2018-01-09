#include "lease/lookupcache.h"
#include "util/common.hpp"

//#include <string.h>
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

static int CreateFileMeta(FileMeta *pFileMeta, int k) {
    pFileMeta->isDirMeta = false;
    pFileMeta->count = k;
    pFileMeta->size = k * 1024;
    pFileMeta->tuple[0].hashNode = 0x1234567890;
    pFileMeta->tuple[0].indexExtentStartBlock = 1;
    pFileMeta->tuple[0].countExtentBlock = 1;
    return 0;
}

static int CreateDirMeta(DirectoryMeta *pDirMeta, int k) {
    pDirMeta->isDirMeta = true;
    pDirMeta->count = 5;
    for (int i = 0; i < pDirMeta->count; i++) {
        if (k+1+i < kTotalEntry) {
            strcpy(pDirMeta->tuple[i].names, EncodeKey(k+1+i).c_str());
            pDirMeta->tuple[i].isDirectories = false;
            if (!(k+1+i % kFileNumperDir)) {
                pDirMeta->tuple[i].isDirectories = true;
            }
        }
    }
    return 0;
}

static int DecodeValue(LeaseEntry* entry) {
    sleep(1);
    printf("wwwwwwwwwww");
    sleep(1);

    if (entry->GetMetadataType() == kDir) {
        sleep(1);
        printf("123456");

        DirectoryMeta *pDirMeta = entry->GetDirMeta();
        for (int i = 0; i < pDirMeta->count; i++) {
            printf("DecodeValue %d %s\n", i, pDirMeta->tuple[i].names);
        }

    } else {
        printf("789012");

        FileMeta *pFileMeta = entry->GetFileMeta();
        printf("DecodeValue file size = %ld\n", pFileMeta->size);
        printf("DecodeValue count = %ld\n", pFileMeta->count);

    }

    return 0;

}
}

using namespace leaffs;

int main(int argc, char** argv) {
    printf("start\n");
    LookupCache *cache = new LookupCache();
    LeaseEntry* entry = NULL;

    printf("stage 0\n");
    int k = 0;

    while (k < kTotalEntry) {
        if (!(k % kFileNumperDir)) { //dir
            DirectoryMeta* dirMeta = new DirectoryMeta();
            CreateDirMeta(dirMeta, k);
            entry = cache->New(EncodeKey(k), dirMeta);
            cache->Release(entry);

            delete dirMeta;
        } else {
            FileMeta* fileMeta = new FileMeta();
            CreateFileMeta(fileMeta, k);
            entry = cache->New(EncodeKey(k), fileMeta);
            cache->Release(entry);
            delete fileMeta;
        }
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
