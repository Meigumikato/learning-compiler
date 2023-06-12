#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "chunk.h"
#include "table.h"
#include "value.h"

enum class InterpreteResult {
  Ok,
  CompilerError,
  RuntimeError,
};

class VM {
 private:
  inline static constexpr int FRAMES_MAX = 64;
  inline static constexpr int STACK_MAX = FRAMES_MAX * UINT8_MAX;

  Chunk* chunk{};
  uint8_t* ip{};  // pc
  Value stack[STACK_MAX];
  Value* stack_top{stack};

  Object* objects{};

  std::unordered_set<std::string> strings;
  std::unordered_map<String*, Value> globals;

  struct CallFrame {
    Function* function{};
    uint8_t* ip{};
    Value* slots{};

    ~CallFrame() {
      ip = nullptr;
      slots = nullptr;
      // delete function;
    }
  };

  std::vector<CallFrame> frames;

  InterpreteResult Run();

  void ResetStack() {
    stack_top = stack;
    objects = nullptr;
  }

  uint8_t ReadByte();

  uint16_t ReadShort();

  Value ReadConstant();

  String* ReadString();

  bool Call(Function* function, int arg_count);

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

  void Debug();

  CallFrame* current_frame_;

  void InsertObject(Object* object);

  bool InsertString(String* string);

  String* FindString(const char* chars, int length, uint32_t hash);
};
