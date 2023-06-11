#pragma once

#include <cstring>

#include "memory.h"
#include "object.h"
#include "value.h"

class HashTable {
 private:
  inline constexpr static double load_factor_ = 0.75;

  struct Entry {
    ObjString* key{};
    Value value{NIL_VAL};
  };

 public:
  Entry* begin();

  Entry* end();

  Value* Get(ObjString* key);

  bool Insert(ObjString* key, Value value);

  bool Delete(ObjString* key);

  void Merge(HashTable* other_table);

  ObjString* FindString(const char* chars, int length, uint32_t hash);

  ~HashTable();

 private:
  void Adjust(int new_capacity);

  Entry* FindEntry(Entry* entries, int capacity, ObjString* key);

  int capacity_{};
  int count_{};
  Entry* entries_{};
};
