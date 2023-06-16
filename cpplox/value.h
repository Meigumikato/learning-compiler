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
};

struct String : Object {
  size_t hash;
  std::weak_ptr<std::string> content;

  String() : Object() { type = ObjectType::String; }

  const char* GetCString() {
    auto sp = content.lock();
    if (sp == nullptr) {
      abort();
    }

    return sp->c_str();
  }

  const std::string GetString() {
    auto sp = content.lock();
    if (sp == nullptr) {
      abort();
    }

    return *sp;
  }
};

struct Function : Object {
  int arity{};
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

  Upvalue(Value* slot) : location(slot) { type = ObjectType::Upvalue; }
};

struct Closure : Object {
  Function* func;
  std::vector<Upvalue*> upvalues;
  // int upvalue_count;

  Closure(Function* func) : func(func) { type = ObjectType::Closure; }
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
