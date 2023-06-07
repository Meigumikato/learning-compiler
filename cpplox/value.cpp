#include "value.h"

#include <cstdio>

#include "memory.h"

void ValueArray::Write(Value value) {
  if (capacity < count + 1) {
    int old_capacity = capacity;
    capacity = GROW_CAPACITY(capacity);
    values = GROW_ARRAY(Value, values, old_capacity, capacity);
  }

  values[count++] = value;
}

ValueArray::~ValueArray() {
  FREE_ARRAY(Value, values, capacity);

  capacity = count = 0;
  values = nullptr;
}

void ValueArray::Print(int offset) { printf("%g", values[offset]); }

void PrintValue(Value value) { printf("%g", value); }
