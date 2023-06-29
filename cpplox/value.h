#pragma once

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <variant>

#include "common.h"
#include "scanner.h"

class Chunk;

enum class ObjectType {
  String,
  Function,
  NativeFunction,
  Closure,
  Upvalue,
};

struct Object {
  ObjectType type{};
  Object* next{};
  bool is_marked{};
};

struct String : Object {
  size_t hash;
  char* content;
  int length;

  String() : Object() { type = ObjectType::String; }

  const char* GetCString() {
    return content;
  }

  const std::string GetString() {
    return std::string(content, length);
  }
};

struct Function : Object {
  int arity{};
  int upvalue_count{};
  std::unique_ptr<Chunk> chunk{};
  String* name{};

  Function();

  const char* GetName() {
    assert(name != nullptr);
    return name->GetCString();
  }
};

using Nil = std::monostate;
using Value = std::variant<Nil, bool, double, Object*>;

using NativeFunctor = std::function<Value(int argc, Value* argv)>;
struct NativeFunction : Object {
  String* name{};
  NativeFunctor native_functor;
};

struct Upvalue : Object {
  Value* location;
  Value closed;

  Upvalue* next;

  Upvalue(Value* slot) : location(slot), closed(), next(nullptr) { type = ObjectType::Upvalue; }
};

struct Closure : Object {
  Function* func;
  std::vector<Upvalue*> upvalues;

  Closure(Function* func) : func(func), upvalues(func->upvalue_count) { 
    type = ObjectType::Closure; 
  }
};

inline NativeFunction* NewNativeFunction(String* name, NativeFunctor functor) {
  NativeFunction* nf = new NativeFunction{.name = name, .native_functor = functor};
  nf->type = ObjectType::NativeFunction;
  return nf;
}

constexpr inline bool IsNil(Value value) { return std::holds_alternative<std::monostate>(value); }

constexpr inline bool IsString(Value value) {
  if (!std::holds_alternative<Object*>(value)) {
    return false;
  }
  return std::get<Object*>(value)->type == ObjectType::String;
}

inline String* AsString(Value value) { return reinterpret_cast<String*>(std::get<Object*>(value)); }

constexpr inline bool IsNumber(Value value) { return std::holds_alternative<double>(value); }

constexpr inline double AsNumber(Value value) { return std::get<double>(value); }

bool ValuesEqual(Value a, Value b);
void PrintValue(Value value);
