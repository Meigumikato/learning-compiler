#pragma once

#include <string>
#include <variant>
#include <vector>

#include "common.h"
#include "scanner.h"

enum class ObjectType {
  String,
  Function,
};

struct Object {
  ObjectType type;
  Object* next{};
};

struct String : Object {
  std::string content;
};

using Value = std::variant<std::monostate, bool, double, Object*>;

constexpr inline bool IsNil(Value value) {
  return std::holds_alternative<std::monostate>(value);
}

constexpr inline bool IsString(Value value) {
  if (std::holds_alternative<Object*>(value)) {
    return false;
  }
  return std::get<Object*>(value)->type == ObjectType::String;
}

inline String* AsString(Value value) {
  return reinterpret_cast<String*>(std::get<Object*>(value));
}

constexpr inline bool IsNumber(Value value) {
  return std::holds_alternative<double>(value);
}

constexpr inline double AsNumber(Value value) {
  return std::get<double>(value);
}

bool ValuesEqual(Value a, Value b);
void PrintValue(Value value);
