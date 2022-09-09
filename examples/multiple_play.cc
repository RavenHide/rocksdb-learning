//
// Created by 赵友清 on 2022/8/27.
//

#include <unistd.h>

#include <bitset>
#include <cstdio>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/status.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/slice_transform.h"

using ROCKSDB_NAMESPACE::ColumnFamilyDescriptor;
using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
using ROCKSDB_NAMESPACE::ColumnFamilyOptions;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::FlushOptions;
using ROCKSDB_NAMESPACE::Iterator;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::BlockBasedTableOptions;

const std::string kDBPath = "./storage/rocksdb_multiple_play";
const uint64_t kMaxKeySize = 600000;  // 最大生成的key的数量

const size_t kMaxValueLength = 256;  // key 最大长度
const size_t kNumKeysPerFlush = 1000;
const size_t kNumKeyPerIteration = 10; // 每次遍历的个数

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

static void PrintStringBinary(std::string& str) {
  std::string print_str;
  for (auto i : str) {
    std::bitset<8> bs(i);
    print_str.append(bs.to_string());
    print_str.append(" ");
  }
  printf("string: %s, binary: %s\n", str.c_str(), print_str.c_str());
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
    ret.append(buf, sizeof(k));
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
            s.ToString().c_str());
    assert(false);
  }
  assert(handles.size() == GetColumnFamilyNames().size());

  uint64_t cur_key_size = 0;
  //  char val_buf[kMaxValueLength] = {0};
  WriteOptions write_opt;
  while (cur_key_size < kMaxKeySize) {
    for (auto h : handles) {
      assert(h != nullptr);
      for (uint64_t i = 0; i != kNumKeysPerFlush; ++i) {
        // gen key and write into db
        //        Slice key = Key(cur_key_size + static_cast<uint64_t>(i));
        Slice key(std::to_string(cur_key_size + i));
        Slice val(std::to_string(cur_key_size + i));
        s = db->Put(write_opt, h, key, val);
        if (!s.ok()) {
          fprintf(stderr, "[process %ld] Failed to insert\n", my_pid);
          assert(false);
        }
      }

      // flush data
      s = db->Flush(FlushOptions(), h);
      if (!s.ok()) {
        fprintf(stderr, "[process %ld] Failed to flush data\n", my_pid);
        assert(false);
      }
    }
    cur_key_size += static_cast<uint64_t>(kNumKeysPerFlush);
    printf("current key size: %lu, max_key_size: %lu\n", cur_key_size,
           kMaxKeySize);
  }

  for (auto h : handles) {
    delete h;
  }
  handles.clear();
  delete db;
  db = nullptr;
  fprintf(stdout, "[process %ld] Finished adding keys\n", my_pid);
}

void IteratorDB() {
  // open db
  Options opt;
  std::vector<ColumnFamilyDescriptor> column_families;
  for (const auto& cf_name : GetColumnFamilyNames()) {
    column_families.push_back(ColumnFamilyDescriptor(cf_name, opt));
  }

  DB* db;
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(opt, kDBPath, column_families, &handles, &db);
  if (!s.ok()) {
    fprintf(stderr, "Failed to open db\n");
    assert(false);
  } else if (handles.size() != GetColumnFamilyNames().size()) {
    fprintf(stderr, "handles's size is invalid\n");
    assert(false);
  } else {
    if (handles[0] == nullptr) {
      fprintf(stderr, "handles[0]  is nullptr \n");
      assert(false);
    } else {
      ReadOptions read_opt;
      std::unique_ptr<Iterator> iter(db->NewIterator(read_opt, handles[0]));
      size_t counter = 0;
      for (iter->SeekToFirst(); iter->Valid() && counter <= kNumKeyPerIteration; iter->Next()) {
        printf("column_family: %s, key: %s, val: %s\n",
               handles[0]->GetName().c_str(),
               iter->key().ToString().c_str(),
               iter->value().ToString().c_str());
        ++counter;
      }
      printf("iterator is done ============ \n");
    }
  }
  for (auto h : handles) {
    delete h;
  }
  handles.clear();
  delete db;
  db = nullptr;
}


void PrefixIteratorDB() {
  Options opt;
  // set up prefix iterator config
  // configure prefix bloom filter
  BlockBasedTableOptions table_opts;
  table_opts.filter_policy.reset(rocksdb::NewBloomFilterPolicy(
      10, false));
  table_opts.whole_key_filtering = false;
  opt.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_opts));
  // define prefix
  opt.prefix_extractor.reset(rocksdb::NewCappedPrefixTransform(3));

  // define column families
  std::vector<ColumnFamilyDescriptor> column_families;
  for (const auto& cf_name : GetColumnFamilyNames()) {
    column_families.push_back(ColumnFamilyDescriptor(cf_name, opt));
  }

  // open db
  DB* db;
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(opt, kDBPath, column_families, &handles, &db);
  if (!s.ok()) {
    fprintf(stderr, "Failed to open db\n");
    assert(false);
  } else if (handles.size() != GetColumnFamilyNames().size()) {
    fprintf(stderr, "handles's size is invalid\n");
    assert(false);
  } else {
    if (handles[0] == nullptr) {
      fprintf(stderr, "handles[0]  is nullptr \n");
      assert(false);
    } else {
      ReadOptions read_opt;
      read_opt.auto_prefix_mode = true;
      std::unique_ptr<Iterator> iter(db->NewIterator(read_opt, handles[0]));
      size_t counter = 0;
      for (iter->Seek("200"); iter->Valid() && counter <= kNumKeyPerIteration; iter->Next()) {
        printf("column_family: %s, key: %s, val: %s\n",
               handles[0]->GetName().c_str(),
               iter->key().ToString().c_str(),
               iter->value().ToString().c_str());
        ++counter;
      }
      printf("iterator is done ============ \n");
    }
  }
  for (auto h : handles) {
    delete h;
  }
  handles.clear();
  delete db;
  db = nullptr;
}

void IteratorDBWithForPrev(const std::string& pre) {
  // open db
  Options opt;
  std::vector<ColumnFamilyDescriptor> column_families;
  for (const auto& cf_name : GetColumnFamilyNames()) {
    column_families.push_back(ColumnFamilyDescriptor(cf_name, opt));
  }

  DB* db;
  std::vector<ColumnFamilyHandle*> handles;
  Status s = DB::Open(opt, kDBPath, column_families, &handles, &db);
  if (!s.ok()) {
    fprintf(stderr, "Failed to open db\n");
    assert(false);
  } else if (handles.size() != GetColumnFamilyNames().size()) {
    fprintf(stderr, "handles's size is invalid\n");
    assert(false);
  } else {
    if (handles[0] == nullptr) {
      fprintf(stderr, "handles[0]  is nullptr \n");
      assert(false);
    } else {
      ReadOptions read_opt;
      std::unique_ptr<Iterator> iter(db->NewIterator(read_opt, handles[0]));
      size_t counter = 0;
      for (iter->SeekForPrev(pre); iter->Valid() && counter <= kNumKeyPerIteration; iter->Next()) {
        printf("column_family: %s, key: %s, val: %s\n",
               handles[0]->GetName().c_str(),
               iter->key().ToString().c_str(),
               iter->value().ToString().c_str());
        ++counter;
      }
      printf("iterator is done ============ \n");
    }
  }
  for (auto h : handles) {
    delete h;
  }
  handles.clear();
  delete db;
  db = nullptr;
}

// clang++ multiple_play.cc  ../librocksdb.a -o multiple_play.out -I ../include/
// -std=c++17 -lpthread -lz  -lbz2 -lzstd -ldl
int main() {
//  CreateDB();
//  RunPrimary();
  // iterator data

  IteratorDB();
  printf("iteration is done ========\n");
  PrefixIteratorDB();
  printf("prefix iteration is done ========\n");
  IteratorDBWithForPrev("400");
  printf("iteration with prev is done ========\n");

  // test Key
  //  for (uint64_t i = 0; i < 10; ++i) {
  //    Key(i);
  //  }

  // test Gen
  //  char val_buf[kMaxValueLength] = {0};
  //  for (int i = 0; i < 10; ++i) {
  //    Slice slice = GenerateRandomValue(kMaxValueLength, val_buf);
  //    printf("%s\n", slice.ToString().c_str());
  //  }
}
