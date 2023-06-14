#pragma once

#include <assert.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

template <typename T>
  requires std::is_enum_v<T>
constexpr auto operator+(T a) noexcept -> std::underlying_type_t<T> {
  return static_cast<std::underlying_type_t<T>>(a);
}
