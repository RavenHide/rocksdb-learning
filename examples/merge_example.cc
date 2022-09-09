//
// Created by zhaoyouqing on 2022/9/7.
//

#include <error.h>
#include "iostream"
#include "rocksdb/merge_operator.h"
#include "rocksdb/env.h"
#include "rocksdb/db.h"
#include "util/coding_lean.h"

using ROCKSDB_NAMESPACE::AssociativeMergeOperator;
using ROCKSDB_NAMESPACE::Slice;
using ROCKSDB_NAMESPACE::Logger;

static bool StringToUint64(const char* str, uint64_t& val) {
  char* eptr = nullptr;
  errno = 0;
  if (*str == '-') {
    fprintf(stderr, "is not a valid uint64, str: %s\n", str);
    return false;
  } else {
    uint64_t convert_val = std::strtoull(str, &eptr, 10);
    if ((errno == ERANGE) && (convert_val == UINT64_MAX)) {
      fprintf(stderr, "occur erange, err: %s, str: %s\n", std::strerror(errno), str);
      return false;
    } if (errno != 0 && convert_val == 0) {
      fprintf(stderr, "occur error, err: %s, str: %s\n", std::strerror(errno), str);
      return false;
    } else if (eptr == str) { // no digit found in str
      fprintf(stderr, "is not a number, str: %s\n", str);
      return false;
    } else if (*eptr != '\0') {
      fprintf(stderr, "is not a number, str: %s\n", str);
      return false;
    } else {
      val = convert_val;
      return true;
    }
  }
}

class Uint64AddOperator : public rocksdb::AssociativeMergeOperator{
 public:
  ~Uint64AddOperator() override {}

  bool Merge(const Slice& key, const Slice* existing_value,
                     const Slice& value, std::string* new_value,
                     Logger* logger) const override {

    uint64_t existing = 0;
    if (existing_value != nullptr) {
      auto ok = StringToUint64(existing_value->data(), existing);
      if (!ok) {
        rocksdb::Log(logger, "failed to convert data to uint64, val: %s\n",
                     existing_value->data());
        existing = 0;
      }
    } else {
      existing = 0;
    }

    uint64_t cur_val = 0;
    auto ok = StringToUint64(value.data(), cur_val);
    if (!ok) {
      rocksdb::Log(logger, "failed to convert data to uint64, val: %s\n",
                   value.data());
      cur_val = 0;
    }

    *new_value = std::to_string(existing + cur_val);
    return true;
  }

  static const char* kClassName() { return "uint64_add_operation"; }
  const char* Name() const override {
    return kClassName();
  }

};

void testStringConvertToUint64() {
  std::cout << "input str" << std::endl;
  std::string str{};
  std::getline(std::cin, str);
  std::cout << str << std::endl;

  uint64_t val = rocksdb::DecodeFixed64(str.c_str());
  fprintf(stdout, "val: %llu\n", val);
  bool ok = StringToUint64(str.c_str(), val);
  fprintf(stderr, "ok: %d, val: %llu\n", ok, val);
}

// build cmd
//
// g++-7 merge_example.cc ../librocksdb.a -o merge_example.out -I ../include/ -I ../ -std=c++17 -lpthread -lz  -lbz2 -lzstd -ldl -lsnappy -llz4  -DROCKSDB_PLATFORM_POSIX -fno-rtti

int main() {

  // open db and merge
  rocksdb::DB* db;
  rocksdb::Options opt;
  opt.create_if_missing = true;
  opt.merge_operator.reset(new Uint64AddOperator);
  rocksdb::Status s = rocksdb::DB::Open(opt, "storage/merge_example", &db);
  if (!s.ok()) {
    fprintf(stderr, "failed to open db, status: %s\n", s.ToString().c_str());
    assert(false);
  }

  rocksdb::WriteOptions write_opt;
  std::string key = "test_merge";
  std::string val = "1";
  s = db->Merge(write_opt, key, val);
  assert(s.ok());

  s = db->Merge(write_opt, key, val);
  assert(s.ok());

 std::string get_val = "";
  s = db->Get(rocksdb::ReadOptions(), key, &get_val);
  assert(s.ok());
  fprintf(stdout, "val: %s\n", get_val.c_str());
  delete db;
}

