#include <cstdio>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "value.h"
#include "vm.h"

class Allocator {
 public:
  template <typename T>
    requires std::is_base_of_v<Object, T>
  T* AllocatorObject() {}

 private:
};

class GrabageCollector {
 public:
  void Collect();

  void MarkRoots() {
    auto vm = VM::GetInstance();
    for (auto slot = vm->stack.begin(); slot < vm->stack_top; ++slot) {
      MarkValue(*slot);
    }

    for (auto& frame : vm->frames) {
      MarkObject(frame.closure);
    }

    for (auto upvallue = vm->open_upvalues; upvallue != nullptr; upvallue = upvallue->next) {
      MarkObject(upvallue);
    }

    MarkCompilerRoots();

    MarkTable(vm->globals);
  }

  void MarkCompilerRoots() {}

  void MarkValue(Value value) {
    if (std::holds_alternative<Object*>(value)) {
      MarkObject(std::get<Object*>(value));
    }

    return;
  }

  void MarkObject(Object* obj) {
    if (obj == nullptr) return;
    obj->is_marked = true;

    // debug message
    printf("%p mark ", (void*)obj);
    PrintValue(obj);
    printf("\n");
  }

  void MarkTable(std::unordered_map<std::size_t, Value>& table) {
    for (auto& [hash, value] : table) {
      MarkValue(value);
    }
  }
};
