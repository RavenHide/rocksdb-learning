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
}