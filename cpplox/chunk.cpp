#include "chunk.h"

#include <cstdlib>

#include "debug.h"
#include "memory.h"

void LineInfo::Append(int line) {
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

LineInfo::~LineInfo() { FREE_ARRAY(Line, lines, capacity); }

int LineInfo::GetLine(int offset) {
  int acc = 0;
  for (int i = 0; i < count; ++i) {
    acc += lines[i].count;
    if (acc >= offset) {
      return lines[i].number;
    }
  }
  assert(0);
}

bool LineInfo::IsInSameLine(int offset1, int offset2) {
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

void Chunk::Write(uint8_t byte, int line) {
  if (capacity < count + 1) {
    auto old_capacity = capacity;

    capacity = GROW_CAPACITY(old_capacity);
    code = GROW_ARRAY(uint8_t, code, old_capacity, capacity);
  }

  line_info.Append(line);

  code[count] = byte;
  count++;
}

void Chunk::WriteConstant(Value value, int line) {
  int constant = AddConstant(value);
  if (constant < 256) {
    Write(OP_CONSTANT, line);
    Write(constant, line);
  } else {
    Write(OP_CONSTANT_LONG, line);

    uint8_t* long_constant = (uint8_t*)&constant;
    if (constant >= (1 << 24)) {
      exit(10);
    }

    Write(long_constant[0], line);
    Write(long_constant[1], line);
    Write(long_constant[2], line);
  }
}

Chunk::~Chunk() {
  FREE_ARRAY(uint8_t, code, capacity);
  count = capacity = 0;
  code = nullptr;
}

void Chunk::Disassemble(const char* name) { DisassembleChunk(this, name); }

int Chunk::AddConstant(Value value) {
  constants.Write(value);
  return constants.count - 1;
}
