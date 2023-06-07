#pragma once

#include "common.h"

using Value = double;

struct ValueArray {
  ~ValueArray();

  int capacity{};
  int count{};
  Value* values{nullptr};

  void Write(Value value);
  void Print(int offset);
};

void PrintValue(Value value);
