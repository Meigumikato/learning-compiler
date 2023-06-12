#pragma once

#include <cstring>

#include "object.h"
#include "value.h"

// class HashTable {
//  private:
//   inline constexpr static double load_factor_ = 0.75;
//
//   struct Entry {
//     String* key{};
//     Value value{};
//   };
//
//  public:
//   Entry* begin();
//
//   Entry* end();
//
//   Value* Get(String* key);
//
//   bool Insert(String* key, Value value);
//
//   bool Delete(String* key);
//
//   void Merge(HashTable* other_table);
//
//   String* FindString(const char* chars, int length, uint32_t hash);
//
//   ~HashTable();
//
//  private:
//   void Adjust(int new_capacity);
//
//   Entry* FindEntry(Entry* entries, int capacity, String* key);
//
//   int capacity_{};
//   int count_{};
//   Entry* entries_{};
// };
