#pragma once

#include "value.h"

enum OpCode {
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

struct LineInfo {
  int count{};
  int capacity{};

  struct Line {
    int number{};
    int count{};
  };

  Line* lines{};

  void Append(int line);

  ~LineInfo();

  int GetLine(int offset);

  bool IsInSameLine(int offset1, int offset2);
};

struct Chunk {
  int count{};
  int capacity{};
  uint8_t* code{};
  LineInfo line_info;
  ValueArray constants;

  void Write(uint8_t byte, int line);
  void WriteConstant(Value value, int line);

  ~Chunk();

  void Disassemble(const char* name);

  int AddConstant(Value value);
};
