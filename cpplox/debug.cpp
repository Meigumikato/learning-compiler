#include "debug.h"

#include <cstdint>
#include <cstdio>

#include "chunk.h"

void disassembleChunk(Chunk *chunk, const char *name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

static int constantInstruction(const char *name, Chunk *chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  chunk->constants.Print(constant);
  printf("'\n");

  return offset + 2;
}

static int longConstantInstruction(const char *name, Chunk *chunk, int offset) {
  struct LongConstant {
    uint8_t instruct;
    int operand : 24;
  };

  static_assert(sizeof(LongConstant) == 4, "LongConstant size not equal 4");

  LongConstant *instruct = (LongConstant *)(chunk->code + offset);

  printf("%-16s %4d '", name, instruct->operand);
  chunk->constants.Print(instruct->operand);
  printf("'\n");

  return offset + 4;
}

static int simpleInstruction(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

int disassembleInstruction(Chunk *chunk, int offset) {
  printf("%04d ", offset);
  if (offset > 0 && chunk->line_info.IsInSameLine(offset, offset - 1)) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->line_info.GetLine(offset));
  }

  uint8_t instruction = chunk->code[offset];

  switch (instruction) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);

    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);

    case OP_CONSTANT_LONG:
      return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);

    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);

    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);

    default:
      printf("Unknow opcode %d\n", instruction);
      return offset + 1;
  }
}
