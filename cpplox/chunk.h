#pragma once

#include "common.h"
#include "value.h"

struct LineInfo {
  struct Line {
    int number{};
    int count{};
  };

  std::vector<Line> lines;

  void Append(int line);

  int GetLine(int offset);

  bool IsInSameLine(int offset1, int offset2);
};

class Chunk {
 public:
  size_t Count() { return code.size(); }

  std::vector<uint8_t> code;
  LineInfo line_info;
  std::vector<Value> constants;

  auto GetCodeBegin() { return code.begin(); }

  auto Write(uint8_t byte, int line) -> void;
  auto WriteConstant(Value value, int line);

  auto Disassemble(const char* name) -> void;

  auto AddConstant(Value value) -> int;
};
