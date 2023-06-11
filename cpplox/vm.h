#pragma once

#include <cstdint>
#include <vector>

#include "chunk.h"
#include "table.h"
#include "value.h"

enum InterpreteResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR,

};

class VM {
 private:
  inline static constexpr int FRAMES_MAX = 64;
  inline static constexpr int STACK_MAX = FRAMES_MAX * UINT8_MAX;

  Chunk* chunk{};
  uint8_t* ip{};  // pc
  Value stack[STACK_MAX];
  Value* stack_top{stack};
  Obj* objects{};

  HashTable strings;
  HashTable globals;

  struct CallFrame {
    ObjFunction* function{};
    uint8_t* ip{};
    Value* slots{};
  };

  std::vector<CallFrame> frames;

  InterpreteResult Run();

  void ResetStack() {
    stack_top = stack;
    objects = nullptr;
  }

  bool Call(ObjFunction* function, int arg_count);
  bool CallValue(Value callee, int arg_count);

  void RuntimeError(const char* format, ...);

  void Concatenate();

  void Push(Value value) {
    *stack_top = value;
    stack_top++;
  }

  Value Pop() {
    stack_top--;
    return *stack_top;
  }

  Value Peek(int distance) { return stack_top[-1 - distance]; }

  void Free();

  void FreeObjects();

 public:
  ~VM() { Free(); }

  InterpreteResult Interpret(const char* source);

  static VM* GetInstance() {
    static VM vm;
    return &vm;
  }

  void InsertObject(Obj* object);

  bool InsertString(ObjString* string);

  ObjString* FindString(const char* chars, int length, uint32_t hash);
};
