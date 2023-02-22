#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <variant>

namespace std {

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;

template <typename T> using Option = std::optional<T>;

template <typename T, typename D = std::default_delete<T>>
using Box = std::unique_ptr<T, D>;

template <typename T> using Rc = std::shared_ptr<T>;

using Unit = std::tuple<>;

template <typename... Ts> using Enum = std::variant<Ts...>;

template <typename... Ts> struct Matcher : Ts... {
  using Ts::operator()...;
};
template <typename... Ts> Matcher(Ts...) -> Matcher<Ts...>;

} // namespace std
