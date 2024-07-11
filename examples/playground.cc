//
// Created by 赵友清 on 2022/8/6.
//
#include "cassert"
#include "cstdio"
#include "rocksdb/db.h"

using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteOptions;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::WriteBatch;

std::string DBPath = "/tmp/rocksdb_simple_example";
// compile cmd
//
// clang++ playground.cc  ../librocksdb.a -o a.out -I ../include/  -std=c++17 -lpthread -lz  -lbz2 -lzstd -ldl
int main() {
  DB* db;
  Options opt;
  // optimize RocksDB.
  opt.IncreaseParallelism();
  opt.OptimizeLevelStyleCompaction();
  opt.create_if_missing = true;
  // open DB
  Status s = DB::Open(opt, DBPath, &db);
  assert(s.ok());
  // Put Key-Value
  std::string key = "key1";
  s = db->Put(WriteOptions(), key, "v1");
  assert(s.ok());

  // get from DB
  std::string value;
  s = db->Get(ReadOptions(), key, &value);
  assert(s.ok());
  std::printf("key: %s, val: %s\n", key.c_str(), value.c_str());

  WriteBatch batch;
  batch.Delete(key);
  key = "key2";
  batch.Put(key, "v2");
  s = db->Write(WriteOptions(), &batch);
  assert(s.ok());

  // get key1
  s = db->Get(ReadOptions(), "key1", &value);
  assert(s.IsNotFound());

  s = db->Get(ReadOptions(), key, &value);
  std::printf("key: %s, val: %s\n", key.c_str(), value.c_str());

  // single delete test
  std::string single_delete_key = "single_delete_test";
  db->Put(rocksdb::WriteOptions(), single_delete_key, "11");
  db->SingleDelete(rocksdb::WriteOptions(), single_delete_key);

  std::string single_delete_value{};
  s = db->Get(ReadOptions(), single_delete_key, &single_delete_value);
  assert(s.IsNotFound());
  std::printf("status: %s, single_delete: key: %s, val: %s\n",
              s.ToString().c_str(), single_delete_key.c_str(), single_delete_value.c_str());

  db->Put(rocksdb::WriteOptions(), single_delete_key, "12");
  db->Put(rocksdb::WriteOptions(), single_delete_key, "13");
  db->Put(rocksdb::WriteOptions(), single_delete_key, "14");
  db->Put(rocksdb::WriteOptions(), single_delete_key, "15");
  s = db->SingleDelete(rocksdb::WriteOptions(), single_delete_key);

  std::printf("single_delete: result: %s, key: %s, val: %s\n",
              s.ToString().c_str(), single_delete_key.c_str(), single_delete_value.c_str());

  s = db->Get(ReadOptions(), single_delete_key, &single_delete_value);
//  assert(s.ok());
  std::printf("status: %s, single_delete: key: %s, val: %s\n",
              s.ToString().c_str(), single_delete_key.c_str(), single_delete_value.c_str());

  delete db;
}