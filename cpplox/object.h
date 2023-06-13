#pragma once

#include <variant>

#include "common.h"
#include "value.h"

// #define OBJ_TYPE(value) (AS_OBJ(value)->type)
//
// #define IS_STRING(value) IsObjType(value, OBJ_STRING)
// #define IS_FUNCTION(value) IsObjType(vale, OBJ_FUNCTION)
//
// #define AS_STRING(value) ((ObjString*)AS_OBJ(value))
//
// #define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
//
// #define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))

// using Object = std::variant<Obj, ObjString, ObjFunction>;

// ObjFunction* NewFunction();
// String* TakeString(char* chars, int length);
// String* CopyString(const char* chars, int length);
//
// void PrintObject(Value value);
//
// inline bool IsObjType(Value value) {
//   return std::holds_alternative<Obj*>(value);
// }
