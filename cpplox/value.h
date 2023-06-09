#pragma once

#include "common.h"

// using Value = double;

enum ValueType {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
};

struct Value {
  ValueType type;
  union {
    bool boolean;
    double number;
  } as;
};

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_NIL(value) ((value).type == VAL_NIL)

struct ValueArray {
  ~ValueArray();

  int capacity{};
  int count{};
  Value* values{nullptr};

  void Write(Value value);
};

void PrintValue(Value value);
