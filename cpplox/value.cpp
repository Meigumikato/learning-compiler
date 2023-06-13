#include "value.h"

#include <cstdio>
#include <cstring>
#include <type_traits>
#include <variant>

#include "chunk.h"

// helper type for the visitor #4
template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

bool ValuesEqual(Value a, Value b) {
  return std::visit(
      Overloaded{
          [](std::monostate b1, std::monostate b2) { return true; },
          [](bool b1, bool b2) { return b1 == b2; },
          [](double d1, double d2) { return d1 == d2; },
          [](Object* a, Object* b) {
            auto a_string = reinterpret_cast<String*>(a);
            auto b_string = reinterpret_cast<String*>(b);

            return a_string->GetString() == b_string->GetString();
          },
          [](auto o1, auto o2) { return false; },

      },
      a, b);
}

static void PrintObject(Object* obj) {
  switch (obj->type) {
    case ObjectType::String: {
      auto string = reinterpret_cast<String*>(obj);
      printf("%s", string->content.lock()->c_str());
      break;
    }
    case ObjectType::Function: {
      auto function = reinterpret_cast<Function*>(obj);
      if (function->name == nullptr) {
        printf("<script>");
      } else {
        printf("%s -> %d\n", function->name->content.lock()->c_str(),
               function->arity);
      }
      break;
    }
    default:
      break;
  }
}

void PrintValue(Value value) {
  std::visit(Overloaded{[](bool b) { printf(b ? "true" : "false"); },
                        [](double b) { printf("%g", b); },
                        [](std::monostate m) { printf("nil"); },
                        [](Object* o) { PrintObject(o); }},
             value);
}
