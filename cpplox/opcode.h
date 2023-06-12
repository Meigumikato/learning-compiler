#pragma once

#include <cstdint>
#include <utility>

enum class OpCode : uint8_t {
  OP_RETURN,
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,

  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,

  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_POP,

  OP_GET_GLOBAL,
  OP_SET_GLOBAL,

  OP_SET_LOCAL,
  OP_GET_LOCAL,

  OP_JUMP,
  OP_JUMP_IF_FALSE,

  OP_LOOP,

  OP_CALL,

  OP_DEFINE_GLOBAL,
  OP_CONSTANT_LONG,
};

constexpr auto operator+(OpCode a) noexcept {
  return static_cast<std::underlying_type_t<OpCode>>(a);
}
