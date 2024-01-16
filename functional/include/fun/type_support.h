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
/**
 * Tests whether `From` and `To` are lvalue reference types _and_ that a conversion from `From` to `To` will result in
 * trivial "direct binding" (i.e. simple pointer coercion, without any temporary materialization or user-defined
 * conversions).
 */
template <class From, class To>
constexpr bool is_safe_reference_convertible_v =
  std::is_lvalue_reference_v<From> &&
  std::is_lvalue_reference_v<To> &&
  std::is_convertible_v<std::remove_reference_t<From>*, std::remove_reference_t<To>*>;

//------------------------------------------------------------------------------
/**
 * An alias template that evaluates to its first type argument, and ignores the second. This is useful to establish a
 * "false dependency" on another type variable in order to force the compiler to defer its analysis of a signature.
 */
template <class T, class U>
using first_t = std::conditional_t<true, T, U>;

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
