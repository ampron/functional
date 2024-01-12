#pragma once

#include <type_traits>
#include <functional>

namespace fun {

//------------------------------------------------------------------------------
struct Unit {
  using Self = Unit;

  constexpr bool operator==(const Self&) const { return true; }
  constexpr bool operator!=(const Self&) const { return false; }
};

//------------------------------------------------------------------------------
struct ForwardArgs {};

//------------------------------------------------------------------------------
template <class T> struct Sized;

template <class T>
struct Sized<T&> {
  std::reference_wrapper<T> _val;

  Sized(T& ref) : _val(ref) {}
  Sized(const std::reference_wrapper<T> ref) : _val(ref) {}

  Sized(const Sized<T&>& other) : _val(other._val) {}

  auto val() -> T& { return _val; }
  auto val() const -> const T& { return _val; }

  auto unwrap() && -> T& { return _val; }
};

template <class T>
struct Sized {
  T _val;

  template <class ...Args>
  Sized(Args&&... args) : _val(std::forward<Args>(args)...)
  {}

  auto val() -> T& { return _val; }
  auto val() const -> const T& { return _val; }

  auto unwrap() && -> T { return std::move(_val); }
};

//------------------------------------------------------------------------------
template <class T>
using Unvoid_t = std::conditional_t<std::is_same_v<T, void>, Unit, T>;

//------------------------------------------------------------------------------
template <class F, class ...Args>
using InvokeResult_t = Unvoid_t<std::invoke_result_t<F, Args...>>;

//------------------------------------------------------------------------------
template <class F, class ...Args>
auto unvoid_call(F&& f, Args&& ...args) -> InvokeResult_t<F, Args...> {
  if constexpr (std::is_same_v<std::invoke_result_t<F, Args...>, void>) {
    std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    return Unit{};
  } else {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
  }
}

//------------------------------------------------------------------------------
template <class T, class... Args>
auto construct_at(T* location, Args&&... args) -> T& {
  ::new (static_cast<void*>(location)) T(std::forward<Args>(args)...);
  return *location;
}

}
