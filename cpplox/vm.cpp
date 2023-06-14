#include "vm.h"

#include <cstdio>
#include <iterator>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "ffi.h"
#include "memory.h"
#include "object.h"
#include "opcode.h"
#include "parser.h"
#include "value.h"

int FFI = []() -> int {
  DefineNativeFunction("unix", &Unix);
  return 10;
}();

VM::VM() : stack(8191), stack_top(stack.begin()), frames(256), frame_pointer_(frames.begin()) {
  // DefineNativeFunction("unix", &Unix);
}

InterpreteResult VM::Interpret(const char* source) {
  Compiler compiler(source, this);

  auto function = compiler.Compile().release();

  if (!function) return InterpreteResult::CompilerError;

  InsertObject(function);

  Push(function);

  Call(function, 0);

  return Run();
  //
  // Parser parser(source);
  //
  // parser.Parse();
  //
  // return InterpreteResult::Ok;
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

    size_t instruction = frame->ip - function->chunk->code.begin() - 1;

    fprintf(stderr, "[line %d] in ", function->chunk->line_info.GetLine(instruction));

    if (function->name == nullptr) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->GetCString());
    }
  }

  auto frame = frame_pointer_ - 1;

  auto instruction = frame->ip - frame->function->chunk->code.begin() - 1;
  auto line = frame->function->chunk->line_info.GetLine(instruction);

  fprintf(stderr, "[line %d] in script", line);

  ResetStack();
}

void VM::Concatenate() {
  String* b = reinterpret_cast<String*>(std::get<Object*>(Pop()));
  String* a = reinterpret_cast<String*>(std::get<Object*>(Pop()));

  auto result = a->GetString() + b->GetString();

  Push(AllocateString(result));
}

static bool IsFalsey(Value value) {
  return std::holds_alternative<std::monostate>(value) ||
         (std::holds_alternative<bool>(value) && !std::get<bool>(value));
}

bool VM::Call(Function* function, int arg_count) {
  if (arg_count != function->arity) {
    RuntimeError("Expected %d arguments but got %d", function->arity, arg_count);
    return false;
  }

  auto frame = frame_pointer_++;
  frame->function = function;
  frame->ip = function->chunk->code.begin();
  frame->slots = stack_top - arg_count - 1;

  return true;
}

bool VM::CallNative(NativeFunction* function, int arg_count) {
  double result = function->native_functor(arg_count, std::addressof(*(stack_top - arg_count - 1)));

  // auto frame = frame_pointer_ - 1;
  // frame->slots = stack_top - arg_count - 1;
  stack_top -= arg_count + 1;

  Push(result);

  return true;
}

bool VM::CallValue(Value callee, int arg_count) {
  if (std::holds_alternative<Object*>(callee)) {
    auto obj = std::get<Object*>(callee);
    switch (obj->type) {
      case ObjectType::Function:
        return Call(reinterpret_cast<Function*>(obj), arg_count);
      case ObjectType::NativeFunction:
        return CallNative(reinterpret_cast<NativeFunction*>(obj), arg_count);
      default:
        break;
    }
  }

  RuntimeError("Can only call functions and classes.");

  return false;
}

uint8_t VM::ReadByte() {
  auto frame = frame_pointer_ - 1;
  return *(frame->ip++);
}

uint16_t VM::ReadShort() {
  auto frame = frame_pointer_ - 1;
  frame->ip += 2;

  return (frame->ip[-2] << 8) | frame->ip[-1];
}

Value VM::ReadConstant() {
  auto frame = frame_pointer_ - 1;
  return frame->function->chunk->constants[ReadByte()];
}

String* VM::ReadString() { return AsString(ReadConstant()); }

enum class Operation { Add, Sub, Mul, Div, Greater, Less, Equal };

template <Operation op, typename T>
T BinaryOp(VM* vm) {
  double b = AsNumber(vm->Pop());
  double a = AsNumber(vm->Pop());

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
  }
}

template <Operation op>
void BinaryOp(VM* vm) {
  switch (op) {
    case Operation::Add:
      vm->Concatenate();
    default:
      abort();
  }
}

void VM::Debug() {
  printf("     stack        ");
  for (auto slot = stack.begin(); slot < stack_top; ++slot) {
    printf("[");
    PrintValue(*slot);
    printf(":%ld]", stack_top - slot);
  }

  printf("[*top*]");
  printf("\n");

  auto frame = frame_pointer_ - 1;

  int diff = std::distance(frame->function->chunk->GetCodeBegin(), frame->ip);
  disassembleInstruction(frame->function->chunk.get(), diff);
}

InterpreteResult VM::Run() {
  auto current_frame = frame_pointer_ - 1;

  printf("\n======================= run trace ============================\n");
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
          Push(BinaryOp<Operation::Add, double>(this));
        } else {
          RuntimeError("Operands must be tow numbers or two strings.");
          return InterpreteResult::RuntimeError;
        }
        break;
      }
      case +OP_SUBTRACT: {
        Push(BinaryOp<Operation::Sub, double>(this));
        break;
      }

      case +OP_MULTIPLY: {
        Push(BinaryOp<Operation::Mul, double>(this));
        break;
      }

      case +OP_DIVIDE: {
        Push(BinaryOp<Operation::Div, double>(this));
        break;
      }

      case +OP_GREATER: {
        Push(BinaryOp<Operation::Greater, bool>(this));
        break;
      }

      case +OP_LESS: {
        Push(BinaryOp<Operation::Less, bool>(this));
        break;
      }

      case +OP_NOT:
        Push(IsFalsey(Pop()));
        break;

      case +OP_NEGATE: {
        if (!IsNumber(Peek(0))) {
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
        frame_pointer_--;

        if (frame_pointer_ == frames.begin()) {
          Pop();
          return InterpreteResult::Ok;
        }

        stack_top = current_frame->slots;
        Push(result);

        current_frame = frame_pointer_ - 1;

        break;
      }

      case +OP_CONSTANT: {
        Push(ReadConstant());
        break;
      }

      case +OP_NIL:
        Push(Nil{});
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
        break;
      }

      case +OP_COMPARE: {
        double b = std::get<double>(Pop());
        double a = std::get<double>(Peek(0));

        if (a > b) {
          Push(1.0);
        } else if (a == b) {
          Push(0.0);
        } else {
          Push(-1.0);
        }

        break;
      }

      case +OP_DEFINE_GLOBAL: {
        auto name = ReadString();
        // allow global variable redefine
        globals.insert({name->hash, Pop()});
        break;
      }

      case +OP_GET_GLOBAL: {
        auto name = ReadString();

        if (!globals.contains(name->hash)) {
          RuntimeError("Undefined variable '%s'.", name->GetCString());
          return InterpreteResult::RuntimeError;
        } else {
          Push(globals[name->hash]);
        }

        break;
      }

      case +OP_SET_GLOBAL: {
        auto name = ReadString();
        if (!globals.contains(name->hash)) {
          RuntimeError("Undefined variable '%s'.", name->GetCString());
          return InterpreteResult::RuntimeError;
        }

        globals[name->hash] = Peek(0);
        break;
      }

      case +OP_GET_LOCAL: {
        uint8_t slot = ReadByte();
        Push(current_frame->slots[slot]);
        break;
      }

      case +OP_SET_LOCAL: {
        auto slot = ReadByte();
        current_frame->slots[slot] = Peek(0);
        break;
      }

      case +OP_JUMP: {
        auto offset = ReadShort();
        current_frame->ip += offset;
        break;
      }

      case +OP_JUMP_IF_FALSE: {
        auto offset = ReadShort();
        if (IsFalsey(Peek(0))) current_frame->ip += offset;
        break;
      }

      case +OP_JUMP_IF_EQUAL: {
        auto offset = ReadShort();
        if (std::get<double>(Peek(0)) == 0) current_frame->ip += offset;
        break;
      }

      case +OP_JUMP_IF_NO_EQUAL: {
        auto offset = ReadShort();
        if (std::get<double>(Peek(0)) != 0) current_frame->ip += offset;
        break;
      }

      case +OP_LOOP: {
        uint16_t offset = ReadShort();
        current_frame->ip -= offset;
        break;
      }

      case +OP_CALL: {
        int arg_count = ReadByte();
        if (!CallValue(Peek(arg_count), arg_count)) {
          return InterpreteResult::RuntimeError;
        }

        // change to call frame
        current_frame = frame_pointer_ - 1;
        break;
      }
    }
  }
}
