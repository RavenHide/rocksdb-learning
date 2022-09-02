//
// Created by 赵友清 on 2022/8/27.
//

#include <unistd.h>

#include <cstdio>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/status.h"

using ROCKSDB_NAMESPACE::ColumnFamilyDescriptor;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
using ROCKSDB_NAMESPACE::ColumnFamilyOptions;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteOptions;

const std::string kDBPath = "./storage/rocksdb_multiple_play";
const uint64_t kMaxKeySize = 600000;  // 最大生成的key的数量

const size_t kMaxValueLength = 256;  // key 最大长度
const size_t kNumKeysPerFlush = 1000;

const std::vector<std::string>& GetColumnFamilyNames() {
  static std::vector<std::string> column_family_names = {
      ROCKSDB_NAMESPACE::kDefaultColumnFamilyName, "youqing"};
  return column_family_names;
}

inline bool IsLittleEndian() {
  uint32_t x = 1;
  return *reinterpret_cast<char*>(&x) != 0;
}

void CreateDB() {
  long my_pid = static_cast<long>(getpid());
  fprintf(stdout, "[process %ld]", my_pid);
  Options options;
  Status s = ROCKSDB_NAMESPACE::DestroyDB(kDBPath, options);
  if (!s.ok()) {
    fprintf(stderr, "[process %ld] Failed to destroy DB: %s\n", my_pid,
            s.ToString().c_str());
    assert(false);
  } else {
    options.create_if_missing = true;
    DB* db = nullptr;
    s = DB::Open(options, kDBPath, &db);
    if (!s.ok()) {
      fprintf(stderr, "[process %ld] Failed to open DB: %s\n", my_pid,
              s.ToString().c_str());
      assert(false);
    } else {
      std::vector<ColumnFamilyHandle*> handles;
      ColumnFamilyOptions cf_opts(options);
      for (const auto& cf_name : GetColumnFamilyNames()) {
        if (cf_name != ROCKSDB_NAMESPACE::kDefaultColumnFamilyName) {
          ColumnFamilyHandle* handle = nullptr;
          s = db->CreateColumnFamily(cf_opts, cf_name, &handle);
          if (!s.ok()) {
            fprintf(stderr,
                    "[process %ld] Failed to create column family: %s\n",
                    my_pid, s.ToString().c_str());
            assert(false);
          } else {
            handles.push_back(handle);
          }
        }

        fprintf(stdout, "[process %ld] Column families created\n", my_pid);
        for (auto h : handles) {
          delete h;
        }
        handles.clear();
      }
    }

    delete db;
    fprintf(stdout, "[process %ld] create db is done, and delete db\n", my_pid);
  }
}

static std::string Key(uint64_t k) {
  std::string ret;
  if (IsLittleEndian()) {
    ret.append(reinterpret_cast<char*>(&k), sizeof(k));
  } else {
    char buf[sizeof(k)];
    for (size_t i = 0; i < sizeof(k); ++i) {
      buf[i] = (k >> (i * 8)) & 0xff;
    }
    ret.append(buf, sizeof(k))
  }
  size_t i = 0;
  size_t j = ret.size() - 1;
  while (i < j) {
    char tmp = ret[i];
    ret[i] = ret[j];
    ret[j] = tmp;
    ++i;
    --j;
  }

  return ret;
}

static Slice GenerateRandomValue(const size_t max_length, char scratch[]) {
  // random size
  int base_rand_value = std::rand();
  size_t sz = (std::rand() % max_length) + 1;
  for (size_t i = 0; i < sz; ++i) {
    scratch[i] = static_cast<char>(base_rand_value ^ i);
  }
  return Slice(scratch, sz);
}

void RunPrimary() {
  long my_pid = static_cast<long>(getpid());
  fprintf(stdout, "[process %ld] Primary instance starts \n", my_pid);
  CreateDB();

  // 设置随机种子
  std::srand(time(0));

  // 初始化DB
  Options options;
  options.create_if_missing = true;
  std::vector<ColumnFamilyDescriptor> column_families;
  for (const auto& cf_name : GetColumnFamilyNames()) {
    column_families.push_back(ColumnFamilyDescriptor(cf_name, options));
  }
  std::vector<ColumnFamilyHandle*> handles;
  DB* db;
  Status s = DB::Open(options, kDBPath, column_families, &handles, &db);
  if (!s.ok()) {
    fprintf(stderr, "[process %ld] Failed to open db: %s", my_pid,
            s.ToString());
    assert(false);
  }
  assert(handles.size() == GetColumnFamilyNames().size())

      // todo batch write data into db
      uint64_t cur_key_size = 0;
  while (cur_key_size < kMaxKeySize) {
    for (auto h : handles) {
      assert(h != nullptr) for (size_t i = 0; i != kNumKeysPerFlush, ++i) {
        // gen key and write into db
        Slice key = Key(cur_key_size + static_cast<uint64_t>(i));
        // gen value
        //        Slice value =
      }
    }
  }

  Status s = DB::Open(options, kDBPath, column_families, &handles, &db);
}

// clang++ multiple_play.cc  ../librocksdb.a -o a.out -I ../include/  -std=c++17
// -lpthread -lz  -lbz2 -lzstd -ldl
int main() { CreateDB(); }