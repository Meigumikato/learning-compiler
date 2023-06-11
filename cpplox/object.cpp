#include "object.h"

#include <cstdio>
#include <cstring>

#include "memory.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, object_type) \
  (type*)AllocateObject(sizeof(type), object_type)

static Obj* AllocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)ALLOCATE(char, size);
  object->type = type;

  auto vm = VM::GetInstance();
  vm->InsertObject(object);
  return object;
}

static uint32_t HashString(const char* key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

ObjFunction* NewFunction() {
  ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->name = nullptr;
  return function;
}

static ObjString* AllocateString(char* chars, int length) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = HashString(string->chars, string->length);

  auto interned = VM::GetInstance()->InsertString(string);

  return string;
}

ObjString* TakeString(char* chars, int length) {
  auto hash = HashString(chars, length);
  auto interned = VM::GetInstance()->FindString(chars, length, hash);

  if (interned != nullptr) {
    delete[] chars;
    return interned;
  }

  return AllocateString(chars, length);
}

ObjString* CopyString(const char* chars, int length) {
  auto hash = HashString(chars, length);
  auto interned = VM::GetInstance()->FindString(chars, length, hash);

  if (interned != nullptr) {
    return interned;
  }

  char* heap_chars = ALLOCATE(char, length + 1);
  memcpy(heap_chars, chars, length);
  heap_chars[length] = '\0';

  return AllocateString(heap_chars, length);
}

static void PrintFunction(ObjFunction* function) {
  if (function->name == nullptr) {
    printf("<script>");
    return;
  }

  printf("<fn %s>", function->name->chars);
}

void PrintObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
    default:
    case OBJ_FUNCTION:
      PrintFunction(AS_FUNCTION(value));
      break;
  }
}
