#include "leveldb/cache.h"
//#include <string.h>
#include <vector>
#include <iostream>
#include "util/coding.h"
#include "common.hpp"


//using namespace leveldb;
/**
dir0
|
file1-file2-file3-file4-dir5
                          |
                        file6-file7-file8-file9-dir10
*/
static const int kFileNumperDir = 5;
static const int kTotalEntry = 10;
//static const bool idDirecory = false;

namespace leveldb {

static std::vector<int> deleted_keys_;
static std::vector<int> deleted_values_;
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

/*
static void* EncodeValue(uintptr_t v) {
    if (v % kFileNumperDir) { //dir
        DirectoryMeta dirMeta;
        CreateDirMeta(&dirMeta, v);
        return reinterpret_cast<void *>(&dirMeta);
    }
    else {
        FileMeta fileMeta;
        CreateFileMeta(&fileMeta, v);
        return reinterpret_cast<void *>(&fileMeta);
    }
}
*/
static int DecodeValue(void* v) {
  bool *pBool = (bool *)v;
  if (*pBool) {
    DirectoryMeta *pDirMeta = (DirectoryMeta *)v;
    for (int i = 0; i < pDirMeta->count; i++) {
      printf("DecodeValue %d %s\n", i, pDirMeta->tuple[i].names);
    }
  }
  else {
    FileMeta *pFileMeta = (FileMeta *)v;
    printf("DecodeValue file size = %ld\n", pFileMeta->size);
    printf("DecodeValue count = %ld\n", pFileMeta->count);
  }
  return 0;
}

static void Deleter(const Slice& key, void* v) {
    deleted_keys_.push_back(DecodeKey(key));
    deleted_values_.push_back(DecodeValue(v));
    printf("%s%d\n", "Delete ", DecodeKey(key));
}


class CacheTest {
 public:

  static const int kCacheSize = 16;

  Cache* cache_;

  CacheTest() : cache_(NewLRUCache(kCacheSize)) {
    printf("%s\n", "contractor");
  }

  ~CacheTest() {
    delete cache_;
  }

  int Lookup(int key) {
    Cache::Handle* handle = cache_->Lookup(EncodeKey(key));
    const int r = (handle == NULL) ? -1 : DecodeValue(cache_->Value(handle));
    if (handle != NULL) {
      cache_->Release(handle);
    }
    return r;
  }

  void Insert(int key) {
    int charge;

    void *value = NULL;

    if (!(key % kFileNumperDir)) { //dir
        charge = sizeof(DirectoryMeta);
        DirectoryMeta *pDirMeta;
        pDirMeta = (DirectoryMeta *)malloc(sizeof(DirectoryMeta));
        if (pDirMeta == NULL){
          printf("1.%s\n", "no free memory");
          return;
        }
        memset(pDirMeta, 0, sizeof(DirectoryMeta));
        //printf("dir ptr = %ld\n", (uintptr_t)pDirMeta);
        //printf("isDirectories = %d\n", pDirMeta->isDirMeta);
        CreateDirMeta(pDirMeta, key);
        value = reinterpret_cast<void *>(pDirMeta);
    }
    else {
        charge = sizeof(FileMeta);
        FileMeta *pFileMeta;
        pFileMeta = (FileMeta *)malloc(sizeof(FileMeta));
        if (pFileMeta == NULL){
          printf("2.%s\n", "no free memory");
          return;
        }
        memset(pFileMeta, 0, sizeof(FileMeta));
        //printf("file ptr = %ld\n", (uintptr_t)pFileMeta);
        CreateFileMeta(pFileMeta, key);
        //pFileMeta->count= (uint64_t)key;
       // printf("1.0  key = %d\n", key);
        //printf("1.0  count = %ld\n", pFileMeta->count);
        //printf("isSirMeta = %d\n", pFileMeta->isDirMeta);
        value = reinterpret_cast<void *>(pFileMeta);
    }

    cache_->Release(cache_->Insert(EncodeKey(key), value, charge,
                                   &Deleter));
  }
/*
  Cache::Handle* InsertAndReturnHandle(int key, int value, int charge = 1) {
    return cache_->Insert(EncodeKey(key), EncodeValue(value), charge,
                          &CacheTest::Deleter);
  }
*/
  void Erase(int key) {
    cache_->Erase(EncodeKey(key));
  }
};

}
using namespace leveldb;

int main(int argc, char** argv) {
    printf("start\n");
    //leveldb::CacheTest *current = new leveldb::CacheTest();
    /*
    Cache* cache;
    static const int kCacheSize = 100;
    cache = NewLRUCache(kCacheSize);
    */

    leveldb::CacheTest *current = new leveldb::CacheTest();
    int k = 0;
    printf("stage 0\n");
/*
    current->Insert(k);
    k++;
    printf("k = %d\n", k);
    current->Insert(k);
    k++;
    printf("k = %d\n", k);
    current->Insert(k);
    k++;
    printf("k = %d\n", k);

*/

    while (k < kTotalEntry) {
      //if (k % kFileNumperDir) {
        current->Insert(k);
      //}
      k++;
    }

    k = 0;
    int re;
    /*
    printf("k = %d\n", k);
    re = current->Lookup(k);
    k--;
    printf("k = %d\n", k);
    re = current->Lookup(k);
    k--;
    printf("k = %d\n", k);
    re = current->Lookup(k);
    k--;
    printf("k = %d\n", k);
    */
    printf("stage 1\n");
    while (k < kTotalEntry) {
        re = current->Lookup(k);
        if (re == 0)
          printf("%s\n", "hit");
        else
          printf("%s\n", "miss");
        k++;
    }

    printf("end\n");
    return 0;
}
