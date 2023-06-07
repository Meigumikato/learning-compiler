#pragma once

#include "memory.h"
#include "value.h"

enum OpCode {
  OP_RETURN,
  OP_CONSTANT,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,

  OP_CONSTANT_LONG,
};

struct LineInfo {
  int count{};
  int capacity{};

  struct Line {
    int number{};
    int count{};
  };

  Line* lines{nullptr};

  void Append(int line) {
    if (capacity < count + 1) {
      int old_capacity = capacity;
      capacity = GROW_CAPACITY(old_capacity);
      lines = GROW_ARRAY(Line, lines, old_capacity, capacity);
    }

    if (count > 0 && line == lines[count - 1].number) {
      lines[count - 1].count++;
    } else {
      lines[count].number = line;
      lines[count].count = 1;
      count++;
    }
  }

  ~LineInfo() { FREE_ARRAY(Line, lines, capacity); }

  int GetLine(int offset) {
    int acc = 0;
    for (int i = 0; i < count; ++i) {
      acc += lines[i].count;
      if (acc >= offset) {
        return lines[i].number;
      }
    }
    assert(0);
  }

  bool IsInSameLine(int offset1, int offset2) {
    int acc = 0;
    for (int i = 0; i < count; ++i) {
      acc += lines[i].count;
      if (acc >= offset1) {
        if (acc >= offset2)
          return true;
        else
          return false;
      }
      if (acc >= offset2) {
        if (acc >= offset1)
          return true;
        else
          return false;
      }
    }
    assert(0);
  }
};

struct Chunk {
  int count{};
  int capacity{};
  uint8_t* code{nullptr};
  LineInfo line_info;
  ValueArray constants;

  void Write(uint8_t byte, int line);
  void WriteConstant(Value value, int line);

  ~Chunk();

  void Disassemble(const char* name);

  int AddConstant(Value value);
};
