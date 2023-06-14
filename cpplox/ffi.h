#pragma once

#include <ctime>
#include <functional>

#include "value.h"
#include "vm.h"

inline void DefineNativeFunction(const char* name, NativeFunctor func) {
  auto vm = VM::GetInstance();

  String* str = reinterpret_cast<String*>(std::get<Object*>(vm->AllocateString(name)));

  Object* nf = NewNativeFunction(str, func);

  vm->InsertObject(nf);
  vm->globals.insert({str->hash, nf});
}

inline int Unix(int argc, Value* argv) {
  int t = time(nullptr);
  printf("TimeStamp: %d\n", t);
  return t;
}
