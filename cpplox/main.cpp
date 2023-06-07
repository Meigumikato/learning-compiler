
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "chunk.h"
#include "common.h"
#include "vm.h"

static void repl() {
  char line[1024];
  for (;;) {
    printf(">>");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }
  }

  interpret(line);
}

static char* ReadFile(const char* path) {
  FILE* file = fopen(path, "rb");
  if (file == nullptr) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);

  size_t file_size = ftell(file);

  rewind(file);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == nullptr) {
    fprintf(stderr, "Not enough memory to read \"%s\".", path);
    exit(74);
  }

  size_t byte_read = fread(buffer, sizeof(char), file_size, file);
  if (byte_read < file_size) {
    fprintf(stderr, "Count not read file \"%s\".", path);
    exit(74);
  }

  buffer[byte_read] = '\0';

  fclose(file);

  return buffer;
}

static void RunFile(const char* path) {
  char* source = ReadFile(path);
  auto res = interpret(source);
  free(source);

  if (res == INTERPRET_COMPILE_ERROR) exit(65);
  if (res == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, char* argv[]) {
  // Chunk chunk;
  //
  // chunk.WriteConstant(1.2, 123);
  // chunk.Write(OP_NEGATE, 123);
  //
  // chunk.WriteConstant(3.8, 123);
  //
  // chunk.Write(OP_ADD, 123);
  //
  // chunk.WriteConstant(2, 123);
  //
  // chunk.Write(OP_SUBTRACT, 123);
  //
  // chunk.WriteConstant(3, 123);
  // chunk.Write(OP_MULTIPLY, 123);
  //
  // chunk.Write(OP_RETURN, 123);
  //
  // interpret(&chunk);
  // // chunk.Disassemble("test chunk");
  //
  // chunk.Free();

  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    RunFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: clox [path]\n]") exit(64);
  }

  return 0;
}
