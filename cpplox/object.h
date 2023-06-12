#pragma once

#include "chunk.h"
#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) IsObjType(value, OBJ_STRING)
#define IS_FUNCTION(value) IsObjType(vale, OBJ_FUNCTION)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))

#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))

enum ObjType {
  OBJ_STRING,
  OBJ_FUNCTION,
};

struct Obj {
  ObjType type{};
  Obj* next{};
};

struct ObjFunction {
  Obj obj{};
  int arity{};
  Chunk chunk{};
  ObjString* name{};

  ~ObjFunction() { delete name; }
};

struct ObjString {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;
};

ObjFunction* NewFunction();
ObjString* TakeString(char* chars, int length);
ObjString* CopyString(const char* chars, int length);

void PrintObject(Value value);

inline bool IsObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}
