#include "memory.h"

#include <cstdlib>

void* reallocate(void* pointer, size_t old_size, size_t new_size) {
  if (new_size == 0) {
    free(pointer);
    return nullptr;
  }

  auto res = realloc(pointer, new_size);

  if (res == nullptr) {
    exit(1);
  }

  return res;
}
