#include "vm.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "value.h"

void VM::Free() { FreeObjects(); }

void VM::FreeObjects() {
  auto object = objects;
  while (object) {
    auto next = object->next;
    FreeObject(object);
    object = next;
  }
}

ObjString* VM::FindString(const char* chars, int length, uint32_t hash) {
  return strings.FindString(chars, length, hash);
}

bool VM::InsertString(ObjString* string) {
  return strings.Insert(string, NIL_VAL);
}

InterpreteResult VM::Interpret(const char* source) {
  Compiler compiler(source);

  auto function = compiler.Compile();

  if (!function) return INTERPRET_COMPILE_ERROR;

  Push(OBJ_VAL(function));

  Call(function, 0);

  return Run();
}

void VM::InsertObject(Obj* object) {
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

    size_t instruction = frame->ip - function->chunk.code - 1;

    fprintf(stderr, "[line %d] in ",
            function->chunk.line_info.GetLine(instruction));

    if (function->name == nullptr) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->chars);
    }
  }

  auto frame = &frames.back();

  auto instruction = frame->ip - frame->function->chunk.code - 1;
  auto line = frame->function->chunk.line_info.GetLine(instruction);
  fprintf(stderr, "[line %d] in script", line);

  ResetStack();
}

void VM::Concatenate() {
  ObjString* b = AS_STRING(Pop());
  ObjString* a = AS_STRING(Pop());

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);

  chars[length] = '\0';

  ObjString* result = TakeString(chars, length);

  Push(OBJ_VAL(result));
}

static bool IsFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static bool ValuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;

  switch (a.type) {
    case VAL_BOOL:
      return AS_BOOL(a) == AS_BOOL(b);

    case VAL_NIL:
      return false;

    case VAL_NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);

    case VAL_OBJ:
      return AS_OBJ(a) == AS_OBJ(b);

    default:
      return false;
  }
}

bool VM::Call(ObjFunction* function, int arg_count) {
  if (arg_count != function->arity) {
    RuntimeError("Expected %d arguments but got %d", function->arity,
                 arg_count);
    return false;
  }

  frames.emplace_back(function, function->chunk.code,
                      stack_top - arg_count - 1);
  auto frame = &frames.back();

  return true;
}

bool VM::CallValue(Value callee, int arg_count) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
      case OBJ_FUNCTION:
        return Call(AS_FUNCTION(callee), arg_count);
      default:
        break;
    }
  }

  RuntimeError("Can only call functions and classes.");

  return false;
}

InterpreteResult VM::Run() {
  CallFrame* frame = &frames.back();

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(value_type, op)                     \
  do {                                                \
    if (!IS_NUMBER(Peek(0)) || !IS_NUMBER(Peek(1))) { \
      RuntimeError("Operand must be a number.");      \
      return INTERPRET_RUNTIME_ERROR;                 \
    }                                                 \
    double b = AS_NUMBER(Pop());                      \
    double a = AS_NUMBER(Pop());                      \
    Push(value_type(a op b));                         \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION

    printf("              ");
    for (auto* slot = stack; slot < stack_top; slot++) {
      printf("[ ");
      PrintValue(*slot);
      printf(" ]");
    }
    printf("\n");

    disassembleInstruction(&frame->function->chunk,
                           (int)(frame->ip - chunk->code));
#endif

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_ADD: {
        if (IS_STRING(Peek(0)) && IS_STRING(Peek(1))) {
          Concatenate();
        } else if (IS_NUMBER(Peek(0)) && IS_NUMBER(Peek(1))) {
          BINARY_OP(NUMBER_VAL, +);
        } else {
          RuntimeError("Operands must be tow numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_SUBTRACT:
        BINARY_OP(NUMBER_VAL, -);
        break;
      case OP_MULTIPLY:
        BINARY_OP(NUMBER_VAL, *);
        break;
      case OP_DIVIDE:
        BINARY_OP(NUMBER_VAL, /);
        break;

      case OP_NOT:
        Push(BOOL_VAL(IsFalsey(Pop())));
        break;

      case OP_NEGATE: {
        // OPTIM:
        if (IS_NUMBER(Peek(0))) {
          RuntimeError("Operand is must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        Push(NUMBER_VAL(-AS_NUMBER(Pop())));
        break;
      }

      case OP_PRINT: {
        PrintValue(Pop());
        printf("\n");
        break;
      }

      case OP_POP:
        Pop();
        break;

      case OP_RETURN: {
        auto result = Pop();

        if (frames.size() - 1 == 0) {
          Pop();
          return INTERPRET_OK;
        }

        stack_top = frame->slots;
        Push(result);

        frames.pop_back();
        frame = &frames.back();

        return INTERPRET_OK;
      }

      case OP_CONSTANT: {
        auto constant = READ_CONSTANT();
        Push(constant);
        // PrintValue(constant);
        // printf("\n");
        break;
      }

      case OP_NIL:
        Push(NIL_VAL);
        break;
      case OP_TRUE:
        Push(BOOL_VAL(true));
        break;
      case OP_FALSE:
        Push(BOOL_VAL(false));
        break;

      case OP_EQUAL: {
        Value b = Pop();
        Value a = Pop();
        Push(BOOL_VAL(ValuesEqual(a, b)));
      } break;

      case OP_GREATER:
        BINARY_OP(BOOL_VAL, >);
        break;

      case OP_LESS:
        BINARY_OP(BOOL_VAL, <);
        break;

      case OP_DEFINE_GLOBAL: {
        auto name = READ_STRING();
        globals.Insert(name, Pop());
        break;
      }
      case OP_GET_GLOBAL: {
        auto name = READ_STRING();
        Value* value = nullptr;
        if (value = globals.Get(name); value == nullptr) {
          RuntimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }

        Push(*value);
        break;
      }

      case OP_SET_GLOBAL: {
        auto name = READ_STRING();
        if (globals.Insert(name, Peek(0))) {
          globals.Delete(name);
          RuntimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }

        break;
      }

      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        Push(frame->slots[slot]);
        break;
      }

      case OP_SET_LOCAL: {
        auto slot = READ_BYTE();
        frame->slots[slot] = Peek(0);
        break;
      }

      case OP_JUMP: {
        auto offset = READ_SHORT();
        frame->ip += offset;
        break;
      }

      case OP_JUMP_IF_FALSE: {
        auto offset = READ_SHORT();
        if (IsFalsey(Peek(0))) frame->ip += offset;
        break;
      }

      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->ip -= offset;
        break;
      }

      case OP_CALL: {
        int arg_count = READ_BYTE();
        if (!CallValue(Peek(arg_count), arg_count)) {
          return INTERPRET_RUNTIME_ERROR;
        }

        // leave scope
        frame = &frames.back();
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}
