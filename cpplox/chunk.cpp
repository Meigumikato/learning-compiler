#include "chunk.h"

#include <cstdlib>

#include "debug.h"
#include "memory.h"

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

void Chunk::Disassemble(const char* name) { disassembleChunk(this, name); }

int Chunk::AddConstant(Value value) {
  constants.Write(value);
  return constants.count - 1;
}
