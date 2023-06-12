#pragma once

#include "common.h"
#include "value.h"

struct LineInfo {
  int count{};
  int capacity{};

  struct Line {
    int number{};
    int count{};
  };

  std::vector<Line> lines{};

  void Append(int line);

  ~LineInfo();

  int GetLine(int offset);

  bool IsInSameLine(int offset1, int offset2);
};

class Chunk {
 public:
  size_t Count() { return code.size(); }

  std::vector<uint8_t> code;
  LineInfo line_info;
  std::vector<Value> constants;

  void Write(uint8_t byte, int line);
  void WriteConstant(Value value, int line);

  ~Chunk();

  void Disassemble(const char* name);

  int AddConstant(Value value);
};

struct Function : Object {
  int arity{};
  Chunk chunk{};
  String* name{};
};
