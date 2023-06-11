#pragma once

#include "chunk.h"

void DisassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);
