#include "memory.h"

#include <cstdlib>

#include "object.h"

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

void FreeObject(Obj* object) {
  switch (object->type) {
    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length);
      FREE(ObjString, object);
      break;
    }
    case OBJ_FUNCTION:
      ObjFunction* function = (ObjFunction*)object;
      FREE(OBJ_FUNCTION, function);
      break;
  }
}
