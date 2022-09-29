//
// Created by 赵友清 on 2022/9/12.
//

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/utilities/transaction_db.h"

const std::string kDBPath = "storage/transaction";

inline bool init_db(std::unique_ptr<rocksdb::TransactionDB>* db_prt) {
  rocksdb::TransactionDB* db = nullptr;
  rocksdb::Options opts{};
  rocksdb::TransactionDBOptions transaction_opt{};
  opts.create_if_missing = true;
  rocksdb::Status s = rocksdb::TransactionDB::Open(
      opts, transaction_opt, kDBPath, &db);
  if (!s.ok()) {
    fprintf(stderr, "failed to open db, err: %s\n", s.ToString().c_str());
    return false;
  } else {
    db_prt->reset(db);
    return true;
  }
}

void play_repeated_read_transaction() {
  // init db
  std::unique_ptr<rocksdb::TransactionDB> txn_db{};
  bool ok = init_db(&txn_db);
  assert(ok);

  ////////////////////////////////////////////////////////
  //
  // "Repeatable Read" (Snapshot Isolation) Example
  //   -- Using a single Snapshot
  //
  ////////////////////////////////////////////////////////

  rocksdb::Slice key = "abc";

  rocksdb::WriteOptions w_opts{};
  rocksdb::Status s = txn_db->Put(w_opts, key, "abc1");
  assert(s.ok());

  rocksdb::TransactionOptions txn_opts{};
  txn_opts.set_snapshot = true;

  // start transaction
  rocksdb::Transaction* txn = txn_db->BeginTransaction(w_opts, txn_opts);

  //read latest committed value
  rocksdb::ReadOptions r_opts{};
  std::string val{};
  s = txn->Get(r_opts, key, &val);
  assert(s.ok());
  assert(val == "abc1");
  val.clear();

//  s = txn_db->Put(w_opts, key, "abc2");
//  assert(s.ok());
//
//  // read the latest committed value
//  s = txn->Get(r_opts, key, &val);
//  assert(s.ok());
//  assert(val == "abc2");
//  val.clear();

  // Read the snapshotted value.
  r_opts.snapshot = txn->GetSnapshot();

  // lock key and no one else can write with this key outside this transaction
  s = txn->GetForUpdate(r_opts, key, &val);
  assert(s.ok());
  assert(val == "abc1");
  val.clear();

  s = txn_db->Put(w_opts, key, "abcabc");
  printf("msg: %s\n", s.ToString().c_str());
  assert(!s.ok());

  // update in this transaction
  s = txn->Put(key, "abc3");
  printf("msg: %s\n", s.ToString().c_str());
  assert(s.ok());

  // repeated read in snapshot isolation
  s = txn->Get(r_opts, key, &val);
  assert(s.ok());
  assert(val == "abc3");
  val.clear();

  // read with lock
  s = txn->GetForUpdate(r_opts, key, &val);
  assert(s.ok());
  assert(val == "abc3");
  val.clear();

  s = txn->Commit();
  assert(s.ok());
  delete txn;
  r_opts.snapshot = nullptr;
}

void play_read_commit_transaction() {
  std::unique_ptr<rocksdb::TransactionDB> db_ptr{};
  bool ok = init_db(&db_ptr);
  assert(ok);

  ////////////////////////////////////////////////////////
  //
  // Simple Transaction Example ("Read Committed")
  //
  ////////////////////////////////////////////////////////
  rocksdb::WriteOptions w_opts{};

  // write data;
  rocksdb::WriteBatch write_batch{};
  std::vector<std::string> keys{};
  keys.reserve(10);
  for (int i = 0; i < 10; ++i) {
    std::string key = std::to_string(i);
    keys.push_back(key);
    std::string val = "value_" + key;
    printf("key: %s, val: %s, vector: %s\n", key.c_str(), val.c_str(), keys[i].c_str());
    write_batch.Put(key, val);
  }
  rocksdb::Status s = db_ptr->Write(w_opts, &write_batch);
  assert(s.ok());

  rocksdb::ReadOptions r_opts{};
  for (int i = 0; i < 10; ++i) {
    std::string val{};
    s = db_ptr->Get(r_opts, keys[i], &val);
    printf("get from db, key: %s, val: %s\n", keys[i].c_str(), val.c_str());
  }
  printf("####### start transaction ############\n");

  // Start a transaction
  rocksdb::Transaction* txn = db_ptr->BeginTransaction(rocksdb::WriteOptions{});
  assert(txn);
  txn->Put(keys[0], keys[0] + "_txn");

  std::string val_0{};
  s = db_ptr->Get(r_opts, keys[0], &val_0);
  assert(s.ok());
  printf("get data outside the transaction, key: %s, val: %s\n", keys[0].c_str(), val_0.c_str());

  // update key[1] outside this transaction
  s = db_ptr->Put(w_opts, keys[1], keys[1] + "_txn");

  // get key[1] lastest value due to the read commit level
  std::string val_1{};
  s = txn->Get(r_opts, keys[1], &val_1);
  assert(s.ok());
  printf("get data in this transaction after outside update, key: %s, val: %s\n", keys[1].c_str(), val_1.c_str());

  // commit transaction
  s = txn->Commit();
  assert(s.ok());

  printf("####### transaction end ############\n");
  for (int i = 0; i < 10; ++i) {
    std::string val{};
    s = db_ptr->Get(r_opts, keys[i], &val);
    printf("get from db after transaction, key: %s, val: %s\n", keys[i].c_str(), val.c_str());
  }
  delete txn;
  keys.clear();
}


// build cmd
// linux
// g++-7 play_transaction_example.cc ../librocksdb.a -o play_transaction_example.out -I ../include/ -I ../ -std=c++17 -lpthread -lz  -lbz2 -lzstd -ldl -lsnappy -llz4  -DROCKSDB_PLATFORM_POSIX -fno-rtti
//
// macos
// clang++ merge_example.cc ../librocksdb.a -o merge_example.out -I ../include/ -I ../ -std=c++17 -lpthread -lz  -lbz2 -lzstd -ldl   -DROCKSDB_PLATFORM_POSIX -fno-rtti -DOS_MACOSX

int main() {
//  play_read_commit_transaction();
  play_repeated_read_transaction();
}