#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
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

  struct CallFrame {
    Function* function{};
    uint8_t* ip{};
    std::vector<Value>::iterator slots;

    ~CallFrame() {
      ip = nullptr;
      // slots = nullptr;
      // delete function;
    }
  };

  VM() : stack(8191), stack_top(stack.begin()) {}

  std::vector<Value> stack;
  std::vector<Value>::iterator stack_top;

  Object* objects{};

  std::unordered_map<size_t, std::shared_ptr<std::string>> strings;
  std::unordered_map<size_t, Value> globals;

  std::vector<CallFrame> frames;

  InterpreteResult Run();

  void ResetStack() {
    stack_top = stack.begin();
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

 public:
  Value AllocateString(std::string_view str) {
    size_t hash = std::hash<std::string_view>{}(str);

    if (!strings.contains(hash)) {
      strings[hash] = std::make_shared<std::string>(str);
    }

    String* string = new String();
    string->content = strings[hash];
    string->hash = hash;

    string->next = objects;
    objects = string;

    return string;
  }

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
