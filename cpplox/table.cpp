#include "table.h"
HashTable::Entry* HashTable::begin() {
  auto idx = 0;
  if (capacity_ <= idx) return nullptr;

  while (idx < capacity_ && entries_[idx].key == nullptr) {
    idx++;
  }
  if (idx == capacity_) return nullptr;

  return entries_ + idx;
}

HashTable::Entry* HashTable::end() { return nullptr; }

Value* HashTable::Get(ObjString* key) {
  if (count_ == 0) return nullptr;

  auto entry = FindEntry(entries_, capacity_, key);

  if (entry->key == nullptr) return nullptr;

  return &entry->value;
}

bool HashTable::Insert(ObjString* key, Value value) {
  if (key == nullptr) return false;

  if (count_ + 1 >= ((double)capacity_ * load_factor_)) {
    auto new_capacity = GROW_CAPACITY(capacity_);
    Adjust(new_capacity);
  }

  auto entry = FindEntry(entries_, capacity_, key);

  // update
  if (entry->key != nullptr) {
    entry->value = value;
    return false;
  }

  // new entry not tombstone
  if (entry->key == nullptr && IS_NIL(entry->value)) {
    count_++;
  }

  entry->key = key;
  entry->value = value;

  return true;
}

bool HashTable::Delete(ObjString* key) {
  if (count_ == 0) return false;

  auto entry = FindEntry(entries_, capacity_, key);

  if (entry->key == nullptr) {
    return false;
  }

  // tombstone
  entry->key = nullptr;
  entry->value = BOOL_VAL(true);

  return true;
}

void HashTable::Merge(HashTable* other_table) {
  for (auto entry : *other_table) {
    if (entry.key != nullptr) Insert(entry.key, entry.value);
  }
}

ObjString* HashTable::FindString(const char* chars, int length, uint32_t hash) {
  if (count_ == 0) return nullptr;

  auto idx = hash % capacity_;

  for (;;) {
    auto entry = entries_ + idx;

    if (entry->key == nullptr) {
      if (IS_NIL(entry->value)) return nullptr;
    } else if (entry->key->hash == hash && entry->key->length == length &&
               memcmp(entry->key->chars, chars, length) == 0) {
      return entry->key;
    }

    idx = (idx + 1) % capacity_;
  }
}

HashTable::~HashTable() {
  capacity_ = 0;
  count_ = 0;
  delete[] entries_;
}

void HashTable::Adjust(int new_capacity) {
  Entry* new_entries = new Entry[new_capacity]{};

  if (entries_ == nullptr) {
    entries_ = new_entries;
    capacity_ = new_capacity;
    return;
  }

  count_ = 0;
  for (int i = 0; i < capacity_; ++i) {
    if (entries_[i].key == nullptr) continue;

    auto new_entry = FindEntry(new_entries, new_capacity, entries_[i].key);
    assert(new_entry->key == nullptr);

    new_entry->key = entries_[i].key;
    new_entry->value = entries_[i].value;
    count_++;
  }

  delete[] entries_;

  entries_ = new_entries;
  capacity_ = new_capacity;
}

HashTable::Entry* HashTable::FindEntry(HashTable::Entry* entries, int capacity,
                                       ObjString* key) {
  auto idx = key->hash % capacity;
  Entry* tombstone = nullptr;

  for (;;) {
    auto entry = entries + idx;

    if (entry->key == nullptr && IS_BOOL(entry->value)) {
      if (tombstone == nullptr) tombstone = entry;
      continue;
    }

    if (entry->key == key) return entry;

    if (entry->key == nullptr) {
      return tombstone == nullptr ? entry : tombstone;
    }

    idx = (idx + 1) % capacity;
  }
}
