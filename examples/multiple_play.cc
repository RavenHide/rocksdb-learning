//
// Created by 赵友清 on 2022/8/27.
//

#include <unistd.h>

#include <cstdio>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/status.h"

using ROCKSDB_NAMESPACE::ColumnFamilyHandle;
using ROCKSDB_NAMESPACE::ColumnFamilyOptions;
using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteOptions;

const std::string kDBPath = "./storage/rocksdb_multiple_play";

const std::vector<std::string>& GetColumnFamilyNames() {
  static std::vector<std::string> column_family_names = {
      ROCKSDB_NAMESPACE::kDefaultColumnFamilyName, "youqing"};
  return column_family_names;
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
      for (const auto& cf_name: GetColumnFamilyNames()) {
        if (cf_name != ROCKSDB_NAMESPACE::kDefaultColumnFamilyName) {
          ColumnFamilyHandle* handle = nullptr;
          s = db->CreateColumnFamily(cf_opts, cf_name, &handle);
          if (!s.ok()) {
            fprintf(stderr, "[process %ld] Failed to create column family: %s\n", my_pid,
                    s.ToString().c_str());
            assert(false);
          } else {
            handles.push_back(handle);
          }
        }

        fprintf(stdout, "[process %ld] Column families created\n", my_pid);
        for (auto h: handles) {
          delete h;
        }
        handles.clear();
      }

    }

    delete db;
    fprintf(stdout, "[process %ld] create db is done, and delete db\n", my_pid);
  }
}

// clang++ multiple_play.cc  ../librocksdb.a -o a.out -I ../include/  -std=c++17 -lpthread -lz  -lbz2 -lzstd -ldl
int main() { CreateDB(); }