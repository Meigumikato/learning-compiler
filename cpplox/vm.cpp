#include "vm.h"

#include <cstdarg>
#include <cstdio>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"

VM vm;

void VM::Free() {}

InterpreteResult interpret(const char* source) {
  Compiler compiler(source);

  if (!compiler.Compile()) {
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = compiler.GetChunk();
  vm.ip = vm.chunk->code;

  auto result = vm.run();

  return INTERPRET_OK;
}

void VM::RuntimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);

  fprintf(stderr, format, args);

  va_end(args);

  fputs("\n", stderr);

  auto instruction = vm.ip - vm.chunk->code - 1;
  auto line = vm.chunk->line_info.GetLine(instruction);
  fprintf(stderr, "[line %d] in script", line);
  ResetStack();
}

InterpreteResult VM::run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() vm.chunk->constants.values[READ_BYTE()]
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
    for (auto* slot = stack; slot < vm.stack_top; slot++) {
      printf("[ ");
      PrintValue(*slot);
      printf(" ]");
    }
    printf("\n");

    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_ADD:
        BINARY_OP(NUMBER_VAL, +);
        break;
      case OP_SUBTRACT:
        BINARY_OP(NUMBER_VAL, -);
        break;
      case OP_MULTIPLY:
        BINARY_OP(NUMBER_VAL, *);
        break;
      case OP_DIVIDE:
        BINARY_OP(NUMBER_VAL, /);
        break;

      case OP_NEGATE:
        // OPTIM:
        if (IS_NUMBER(Peek(0))) {
          RuntimeError("Operand is must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        Push(NUMBER_VAL(-AS_NUMBER(Pop())));
        break;
      case OP_RETURN: {
        PrintValue(Pop());
        printf("\n");
        return INTERPRET_OK;
      }
      case OP_CONSTANT: {
        auto constant = READ_CONSTANT();
        Push(constant);
        // PrintValue(constant);
        // printf("\n");
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}
