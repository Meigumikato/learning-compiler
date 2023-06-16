#pragma once

#include <_types/_uint8_t.h>

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
 public:
  inline static constexpr int FRAMES_MAX = 64;
  inline static constexpr int STACK_MAX = FRAMES_MAX * UINT8_MAX;

  struct CallFrame {
    Closure* closure{};
    std::vector<uint8_t>::iterator ip;
    std::vector<Value>::iterator slots;
  };

  VM();

  std::vector<Value> stack;
  std::vector<Value>::iterator stack_top;

  Object* objects{};

  std::unordered_map<size_t, Value> globals;
  std::unordered_map<size_t, std::shared_ptr<std::string>> strings;

  std::vector<CallFrame> frames;
  std::vector<CallFrame>::iterator frame_pointer_;

  InterpreteResult Run();

  void ResetStack() {
    stack_top = stack.begin();
    objects = nullptr;
  }

  uint8_t ReadByte();

  uint16_t ReadShort();

  Value ReadConstant();

  String* ReadString();

  bool Call(Closure* closure, int arg_count);

  bool CallNative(NativeFunction* function, int arg_count);

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

    InsertObject(string);

    return string;
  }

  InterpreteResult Interpret(const char* source);

  static VM* GetInstance() {
    static VM vm;
    return &vm;
  }

  void Debug();

  void InsertObject(Object* object);
};
