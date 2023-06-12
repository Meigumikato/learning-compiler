#include "vm.h"

#include <_ctype.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <variant>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "opcode.h"
#include "value.h"

String* VM::FindString(char* chars, int length, uint32_t hash) {
  std::string temp(chars, length);
  if (strings.contains(temp)) {
    return;
  }

  return nullptr;
}

bool VM::InsertString(String* string) {
  // return strings.insert(string, Value{});
  return strings.insert(string).second;
}

InterpreteResult VM::Interpret(const char* source) {
  Compiler compiler(source);

  auto function = compiler.Compile();

  if (!function) return InterpreteResult::CompilerError;

  Push(function.get());

  Call(function.get(), 0);

  return Run();
}

void VM::InsertObject(Object* object) {
  object->next = objects;
  objects = object;
}

void VM::RuntimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);

  fprintf(stderr, format, args);

  va_end(args);

  fputs("\n", stderr);

  for (int i = frames.size() - 1; i >= 0; --i) {
    auto frame = &frames[i];
    auto function = frame->function;

    size_t instruction = frame->ip - function->chunk.code.data() - 1;

    fprintf(stderr, "[line %d] in ",
            function->chunk.line_info.GetLine(instruction));

    if (function->name == nullptr) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  auto frame = &frames.back();

  auto instruction = frame->ip - frame->function->chunk.code.data() - 1;
  auto line = frame->function->chunk.line_info.GetLine(instruction);
  fprintf(stderr, "[line %d] in script", line);

  ResetStack();
}

void VM::Concatenate() {
  String* b = reinterpret_cast<String*>(std::get<Object*>(Pop()));
  String* a = reinterpret_cast<String*>(std::get<Object*>(Pop()));

  int length = a->length + b->length;
  char* chars = new char[length + 1];
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);

  chars[length] = '\0';

  String* result = TakeString(chars, length);

  Push(static_cast<Object*>(result));
}

static bool IsFalsey(Value value) {
  return std::holds_alternative<std::monostate>(value) ||
         (std::holds_alternative<bool>(value) && !std::get<bool>(value));
}

bool VM::Call(Function* function, int arg_count) {
  if (arg_count != function->arity) {
    RuntimeError("Expected %d arguments but got %d", function->arity,
                 arg_count);
    return false;
  }

  frames.emplace_back();
  frames.back().function = function;
  frames.back().ip = function->chunk.code.data();
  frames.back().slots = stack_top - arg_count - 1;

  auto frame = &frames.back();

  return true;
}

bool VM::CallValue(Value callee, int arg_count) {
  if (std::holds_alternative<Object*>(callee)) {
    auto obj = std::get<Object*>(callee);
    switch (obj->type) {
      case ObjectType::Function:
        return Call(reinterpret_cast<Function*>(obj), arg_count);
      default:
        break;
    }
  }

  RuntimeError("Can only call functions and classes.");

  return false;
}

uint8_t VM::ReadByte() { return *current_frame_->ip++; }

uint16_t VM::ReadShort() {
  current_frame_->ip += 2;
  return (current_frame_->ip[-2] << 8) | current_frame_->ip[-1];
}

Value VM::ReadConstant() {
  return current_frame_->function->chunk.constants[ReadByte()];
}

String* VM::ReadString() { return AsString(ReadConstant()); }

enum class Operation { Add, Sub, Mul, Div, Greater, Less, Equal };

template <Operation op, typename T>
T BinaryOp(auto a, auto b) {
  switch (op) {
    case Operation::Add:
      return a + b;
    case Operation::Sub:
      return a - b;
    case Operation::Mul:
      return a * b;
    case Operation::Div:
      return a / b;
    case Operation::Greater:
      return a > b;
    case Operation::Less:
      return a < b;
    case Operation::Equal:
      return a == b;
  }
}

void VM::Debug() {
  printf("              ");
  for (auto* slot = stack; slot < stack_top; slot++) {
    printf("[ ");
    PrintValue(*slot);
    printf(" ]");
  }
  printf("\n");

  disassembleInstruction(
      &current_frame_->function->chunk,
      (int)(current_frame_->ip - current_frame_->function->chunk.code.data()));
}

InterpreteResult VM::Run() {
  current_frame_ = &frames.back();

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    Debug();
#endif

    uint8_t instruction;
    using enum OpCode;
    switch (instruction = ReadByte()) {
      case +OP_ADD: {
        if (IsString(Peek(0)) && IsString(Peek(1))) {
          Concatenate();
        } else if (IsNumber(Peek(0)) && IsNumber(Peek(1))) {
          double b = AsNumber(Pop());
          double a = AsNumber(Pop());
          Push(BinaryOp<Operation::Add, double>(a, b));
        } else {
          RuntimeError("Operands must be tow numbers or two strings.");
          return InterpreteResult::RuntimeError;
        }
        break;
      }
      case +OP_SUBTRACT: {
        double b = AsNumber(Pop());
        double a = AsNumber(Pop());
        Push(BinaryOp<Operation::Sub, double>(a, b));
        break;
      }

      case +OP_MULTIPLY: {
        double b = AsNumber(Pop());
        double a = AsNumber(Pop());
        Push(BinaryOp<Operation::Mul, double>(a, b));
        break;
      }

      case +OP_DIVIDE: {
        double b = AsNumber(Pop());
        double a = AsNumber(Pop());
        Push(BinaryOp<Operation::Div, double>(a, b));
        break;
      }

      case +OP_NOT:
        Push(IsFalsey(Pop()));
        break;

      case +OP_NEGATE: {
        if (IsNumber(Peek(0))) {
          RuntimeError("Operand is must be a number.");
          return InterpreteResult::RuntimeError;
        }

        Push(-AsNumber(Pop()));
        break;
      }

      case +OP_PRINT: {
        PrintValue(Pop());
        printf("\n");
        break;
      }

      case +OP_POP:
        Pop();
        break;

      case +OP_RETURN: {
        auto result = Pop();

        if (frames.size() - 1 == 0) {
          Pop();
          return InterpreteResult::Ok;
        }

        stack_top = current_frame_->slots;
        Push(result);

        frames.pop_back();
        current_frame_ = &frames.back();

        return InterpreteResult::Ok;
      }

      case +OP_CONSTANT: {
        auto constant = ReadConstant();
        Push(constant);
        break;
      }

      case +OP_NIL:
        Push(std::monostate{});
        break;
      case +OP_TRUE:
        Push(true);
        break;
      case +OP_FALSE:
        Push(false);
        break;

      case +OP_EQUAL: {
        Value b = Pop();
        Value a = Pop();
        Push(ValuesEqual(a, b));
      } break;

      case +OP_GREATER: {
        double b = AsNumber(Pop());
        double a = AsNumber(Pop());
        Push(BinaryOp<Operation::Greater, bool>(a, b));
        break;
      }

      case +OP_LESS: {
        double b = AsNumber(Pop());
        double a = AsNumber(Pop());
        Push(BinaryOp<Operation::Less, bool>(a, b));
        break;
      }

      case +OP_DEFINE_GLOBAL: {
        auto name = ReadString();
        globals.emplace(name, Pop());
        break;
      }
      case +OP_GET_GLOBAL: {
        auto name = ReadString();
        if (!globals.contains(name)) {
          RuntimeError("Undefined variable '%s'.", name->chars);
          return InterpreteResult::RuntimeError;
        } else {
          Push(globals[name]);
        }

        break;
      }

      case +OP_SET_GLOBAL: {
        auto name = ReadString();
        if (globals.contains(name)) {
          RuntimeError("Undefined variable '%s'.", name->chars);
          return InterpreteResult::RuntimeError;
        }

        globals.emplace(name, Peek(0));

        break;
      }

      case +OP_GET_LOCAL: {
        uint8_t slot = ReadByte();
        Push(current_frame_->slots[slot]);
        break;
      }

      case +OP_SET_LOCAL: {
        auto slot = ReadByte();
        current_frame_->slots[slot] = Peek(0);
        break;
      }

      case +OP_JUMP: {
        auto offset = ReadShort();
        current_frame_->ip += offset;
        break;
      }

      case +OP_JUMP_IF_FALSE: {
        auto offset = ReadShort();
        if (IsFalsey(Peek(0))) current_frame_->ip += offset;
        break;
      }

      case +OP_LOOP: {
        uint16_t offset = ReadShort();
        current_frame_->ip -= offset;
        break;
      }

      case +OP_CALL: {
        int arg_count = ReadByte();
        if (!CallValue(Peek(arg_count), arg_count)) {
          return InterpreteResult::RuntimeError;
        }
        break;
      }
    }
  }
}
