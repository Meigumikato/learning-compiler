#pragma once

#include "chunk.h"

#define STACK_MAX 256

enum InterpreteResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,

};

struct VM {
  Chunk* chunk;
  uint8_t* ip;  // pc
  Value stack[STACK_MAX];
  Value* stack_top = stack;

  VM& GetInstance();

  void ResetStack() { stack_top = stack; }

  Value* Top() { return stack_top; }

  void Push(Value value) {
    *stack_top = value;
    stack_top++;
  }
  Value Pop() {
    stack_top--;
    return *stack_top;
  }

  void Free();

  InterpreteResult run();
};

InterpreteResult interpret(const char* source);
