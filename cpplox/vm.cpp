#include "vm.h"

#include <cstdio>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"

VM vm;

void VM::Free() {}

InterpreteResult interpret(const char* source) {
  Chunk chunk;

  if (!compile(source, &chunk)) {
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  auto result = vm.run();

  return INTERPRET_OK;
}

InterpreteResult VM::run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() vm.chunk->constants.values[READ_BYTE()]
#define BINARY_OP(op)     \
  do {                    \
    Value b = Pop();      \
    *Top() = *Top() op b; \
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
        BINARY_OP(+);
        break;
      case OP_SUBTRACT:
        BINARY_OP(-);
        break;
      case OP_MULTIPLY:
        BINARY_OP(*);
        break;
      case OP_DIVIDE:
        BINARY_OP(/);
        break;

      case OP_NEGATE:
        // OPTIM:
        // Push(-Pop());
        *Top() = -*Top();
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
