#include "vm.h"

#include <cstdarg>
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

VM::VM()
    : stack(8191),
      stack_top(stack.begin()),
      frames(256),
      frame_pointer_(frames.begin()),
      open_upvalues(nullptr) {}

InterpreteResult VM::Interpret(const char* source) {
  Compiler compiler(source, this);

  auto function = compiler.Compile().release();

  if (!function) return InterpreteResult::CompilerError;

  InsertObject(function);

  Push(function);

  Closure* closure = new Closure(function);

  Pop();

  Push(closure);

  Call(closure, 0);

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
    auto closure = frame->closure;

    size_t instruction = frame->ip - closure->func->chunk->code.begin() - 1;

    fprintf(stderr, "[line %d] in ", closure->func->chunk->line_info.GetLine(instruction));

    if (closure->func->name == nullptr) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", closure->func->name->GetCString());
    }
  }

  auto frame = frame_pointer_ - 1;

  auto instruction = frame->ip - frame->closure->func->chunk->code.begin() - 1;
  auto line = frame->closure->func->chunk->line_info.GetLine(instruction);

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

bool VM::Call(Closure* closure, int arg_count) {
  if (arg_count != closure->func->arity) {
    RuntimeError("Expected %d arguments but got %d", closure->func->arity, arg_count);
    return false;
  }

  auto frame = frame_pointer_++;
  frame->closure = closure;
  frame->ip = closure->func->chunk->code.begin();
  frame->slots = stack_top - arg_count - 1;

  return true;
}

bool VM::CallNative(NativeFunction* function, int arg_count) {
  Value result = function->native_functor(arg_count, std::addressof(*(stack_top - arg_count - 1)));

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
      case ObjectType::Closure:
        return Call(reinterpret_cast<Closure*>(obj), arg_count);
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
  return frame->closure->func->chunk->constants[ReadByte()];
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

Upvalue* CaptureUpvalue(Value* local) {
  Upvalue* pre_upvalue = nullptr;
  auto upvalue = VM::GetInstance()->open_upvalues;

  while (upvalue != nullptr && upvalue->location > local) {
    pre_upvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != nullptr && upvalue->location == local) {
    return upvalue;
  }

  Upvalue* created_upvalue = new Upvalue(local);

  created_upvalue->next = upvalue;

  if (pre_upvalue == nullptr) {
    VM::GetInstance()->open_upvalues = created_upvalue;
  } else {
    pre_upvalue->next = created_upvalue;
  }

  return created_upvalue;
}

void CloseUpValue(Value* last) {
  auto open_upvalues = VM::GetInstance()->open_upvalues;

  while (open_upvalues != nullptr && open_upvalues->location >= last) {
    auto upvalue = open_upvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    open_upvalues = open_upvalues->next;
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

  int diff = std::distance(frame->closure->func->chunk->GetCodeBegin(), frame->ip);
  disassembleInstruction(frame->closure->func->chunk.get(), diff);
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
        CloseUpValue(current_frame->slots.base());
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

      case +OP_SET_UPVALUE: {
        auto slot = ReadByte();
        *current_frame->closure->upvalues[slot]->location = Peek(0);
        break;
      }

      case +OP_GET_UPVALUE: {
        auto slot = ReadByte();
        Push(*current_frame->closure->upvalues[slot]->location);
        break;
      }

      case +OP_CLOSE_UPVALUE: {
        CloseUpValue((stack_top - 1).base());
        Pop();
        break;
      }

      case +OP_CLOSURE: {
        Function* function = reinterpret_cast<Function*>(std::get<Object*>(ReadConstant()));
        Closure* closure = new Closure(function);
        Push(closure);
        for (int i = 0; i < closure->upvalues.size(); i++) {
          uint8_t is_local = ReadByte();
          uint8_t index = ReadByte();

          if (is_local) {
            closure->upvalues[i] = CaptureUpvalue(&current_frame->slots[index]);
          } else {
            closure->upvalues[i] = current_frame->closure->upvalues[i];
          }
        }
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
