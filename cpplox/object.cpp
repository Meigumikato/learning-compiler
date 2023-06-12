#include "object.h"

#include <cstdio>
#include <cstring>

#include "chunk.h"
#include "value.h"
#include "vm.h"

Function* NewFunction() {
  auto function = new Function;

  function->type = ObjectType::Function;

  return function;
}

String* TakeString(char* chars, int length) {
  auto interned = VM::GetInstance()->FindString(chars, length, 1);

  if (interned != nullptr) {
    delete[] chars;
    return interned;
  }

  return new String{.length = length, .chars = chars};
}

String* CopyString(const char* chars, int length) {
  auto interned = VM::GetInstance()->FindString(chars, length, 1);

  if (interned != nullptr) {
    return interned;
  }

  char* heap_chars = new char[length + 1];
  memcpy(heap_chars, chars, length);
  heap_chars[length] = '\0';

  return new String{.length = length, .chars = heap_chars};
}
