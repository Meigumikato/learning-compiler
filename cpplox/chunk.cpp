#include "chunk.h"

#include "debug.h"
#include "opcode.h"

void LineInfo::Append(int line) {
  if (lines.size() > 0 && line == lines.back().number) {
    lines.back().count++;
  } else {
    lines.push_back({line, 1});
  }
}

LineInfo::~LineInfo() {}

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
  code.push_back(byte);
  line_info.Append(line);
}

void Chunk::WriteConstant(Value value, int line) {
  int constant = AddConstant(value);
  if (constant < 256) {
    Write(static_cast<uint8_t>(OpCode::OP_CONSTANT), line);
    Write(constant, line);
  } else {
    Write(static_cast<uint8_t>(OpCode::OP_CONSTANT_LONG), line);

    uint8_t* long_constant = (uint8_t*)&constant;
    if (constant >= (1 << 24)) {
      exit(10);
    }

    Write(long_constant[0], line);
    Write(long_constant[1], line);
    Write(long_constant[2], line);
  }
}

void Chunk::Disassemble(const char* name) { DisassembleChunk(this, name); }

int Chunk::AddConstant(Value value) {
  constants.push_back(value);
  return constants.size() - 1;
}
