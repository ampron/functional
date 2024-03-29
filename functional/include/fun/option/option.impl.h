#pragma once

//!
//! @author Alex Pronschinske
//! @copyright MIT License
//!

#include <fun/option/option.declare.h>

#include <fun/result/result.declare.h>

namespace fun {

//==============================================================================
// Option-related function definitions
//------------------------------------------------------------------------------
inline auto some() -> Option<Unit> { return some(Unit{}); }

//------------------------------------------------------------------------------
template<typename Arg>
auto some(Arg&& x) -> Option<std::decay_t<Arg>> {
  return Option<std::decay_t<Arg>>(ForwardArgs{}, std::forward<Arg>(x));
}

//------------------------------------------------------------------------------
template <class T>
auto some_default() -> Option<T> { return Option<T>(ForwardArgs{}); }

//------------------------------------------------------------------------------
template <class T>
auto some_ref(T& x) -> Option<T&> { return Option<T&>(OptionUnion<T&>(x)); }

//==============================================================================
// Option definitions
//------------------------------------------------------------------------------
//!
//! The match method take a function for handling either variant and
//! dispatches the appropriate function and returns the common type.
//!
template<typename T>
template <typename SomeFuncT, typename NoneFuncT>
auto Option<T>::
match(SomeFuncT&& func_some, NoneFuncT&& func_none) && -> MatchReturn<SomeFuncT>
{
  static_assert(
    std::is_same_v<std::invoke_result_t<SomeFuncT, T>, std::invoke_result_t<NoneFuncT>>
    , "Some-handling and None-handling functions passed to match do not "
      "have the same return type"
    );
  if (is_some()) { return unvoid_call(std::forward<SomeFuncT>(func_some), std::move(*this).unwrap()); }
  else           { return unvoid_call(std::forward<NoneFuncT>(func_none)); }
}

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::begin() -> Iter { return Iter(as_ptr()); }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::begin() const -> ConstIter { return ConstIter(as_ptr()); }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::cbegin() const -> ConstIter { return begin(); }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::end() -> Iter { return Iter(); }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::end() const -> ConstIter { return ConstIter(); }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::cend() const -> ConstIter { return ConstIter(); }

//------------------------------------------------------------------------------
template <typename T>
template <typename E, typename ... Args>
auto Option<T>::ok_or(E err) && -> Result<T, E>
{
  if (is_some()) { return fun::make_ok(std::move(*this).unwrap()); }
  else           { return fun::make_err(std::forward<E>(err)); }
}

//------------------------------------------------------------------------------
template <typename T>
template<typename ErrFuncT>
auto Option<T>::ok_or_else(ErrFuncT&& err_func) && -> Result<T, ErrorAlternative<ErrFuncT>>
{
  if (is_some()) { return fun::make_ok(std::move(*this).unwrap()); }
  else           { return fun::make_err(unvoid_call(std::forward<ErrFuncT>(err_func))); }
}

//------------------------------------------------------------------------------
//!
//! Map Option<T> to Option<U> by applying the argument `func` (of type
//! U func(T) or T -> U) to the contained data if there is some.
//!
template<typename T>
template <typename F /* T -> U */>
auto Option<T>::map(F&& func) && -> MappedOption<F>
{
  if (is_some()) { return fun::make_some(unvoid_call(std::forward<F>(func), std::move(*this).unwrap())); }
  else { return {}; }
}

//------------------------------------------------------------------------------
template<typename T>
template <typename U, typename FuncT>
U Option<T>::map_or(U default_val, FuncT&& func) &&
{
  if (is_some()) { return unvoid_call(std::forward<FuncT>(func), std::move(*this).unwrap()); }
  else           { return std::forward<U>(default_val); }
}

//------------------------------------------------------------------------------
template <typename T>
template <typename DefaultFunc, typename F>
auto Option<T>::
map_or_else(DefaultFunc&& default_func, F&& func) && -> MappedOption<F>
{
  if (is_some()) { return unvoid_call(std::forward<F>(func), std::move(*this).unwrap()); }
  else           { return unvoid_call(std::forward<DefaultFunc>(default_func)); }
}

//------------------------------------------------------------------------------
template <typename T>
template <typename U>
auto Option<T>::
zip(Option<U> other) && -> Option<std::pair<T, U>>
{
  if (this->is_some() && other.is_some()) {
    return fun::make_some(std::move(*this).unwrap(), std::move(other).unwrap());
  } else {
    return {};
  }
}

//------------------------------------------------------------------------------
//!
//! Returns a "None" variant if the Option object is "None", otherwise
//! it calls the input function with the contained value of type T and
//! returns an Option<U> with the result.
//!
template<typename T>
template <typename F /* T -> Option<U> */>
auto Option<T>::and_then(F&& func) && -> ValBoundOption<F>
{
  if (is_some()) { return unvoid_call(std::forward<F>(func), std::move(*this).unwrap()); }
  else           { return {}; }
}

//------------------------------------------------------------------------------
template<typename T>
template <typename F /* () -> Option<T> */>
Option<T> Option<T>::or_else(F&& alt_func) &&
{
  if (is_some()) { return fun::make_some(std::move(*this).unwrap()); }
  else           { return unvoid_call(std::forward<F>(alt_func)); }
}

//------------------------------------------------------------------------------
template<typename T>
Option<T> Option<T>::take()
{
  if (is_some()) { return Option<T>(ForwardArgs(), _inner.dump()); }
  else           { return Option<T>(); }
}

//------------------------------------------------------------------------------
template<typename T>
Option<T>& Option<T>::push(T obj) { return emplace(std::forward<T>(obj)); }

//------------------------------------------------------------------------------
template <typename T>
template <typename ...Args>
auto Option<T>::emplace(Args&& ...args) -> self_t&
{
  _inner.emplace(std::forward<Args>(args)...);
  return *this;
}

template <class T>
auto Option<T>::cloned() const -> Option<value_t>
{
  if (is_some()) { return Option<value_t>(clone().unwrap()); }
  else           { return {}; }
}

//------------------------------------------------------------------------------
template<typename T>
T Option<T>::unwrap() && { return _inner.dump(); }

//------------------------------------------------------------------------------
template<typename T>
T Option<T>::expect(const char* err_msg) &&
{
  if (is_some()) { return std::move(*this).unwrap(); }
  else           { throw std::runtime_error(err_msg); }
}

//------------------------------------------------------------------------------
template <class T>
template <class F>
T Option<T>::unwrap_or_else(F&& alt_func) &&
{
  static_assert(
    !std::is_reference_v<T> || is_safe_reference_convertible_v<InvokeResult_t<F>, T>,
    "The callback passed to Option<T&>::unwrap_or_else must return a compatible reference"
  );

  if (is_some()) { return std::move(*this).unwrap(); }
  else           { return unvoid_call(std::forward<F>(alt_func)); };
}

//------------------------------------------------------------------------------
template<typename T>
Option<T>::ConstIter::ConstIter(const value_t* ptr) : _ptr(ptr) {}

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::ConstIter::operator*() const -> const value_t& { return *_ptr; }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::ConstIter::operator->() const -> const value_t* { return _ptr; }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::ConstIter::operator++() -> ConstIter& {
  if (_ptr != nullptr) { _ptr = nullptr; }
  return *this;
}

//------------------------------------------------------------------------------
template<typename T>
bool Option<T>::ConstIter::operator==(const ConstIter& other) const {
  return _ptr == other._ptr;
}

//------------------------------------------------------------------------------
template<typename T>
bool Option<T>::ConstIter::operator!=(const ConstIter& other) const {
  return !(*this == other);
}

//------------------------------------------------------------------------------
template<typename T>
Option<T>::Iter::Iter(value_t* ptr) : _ptr(ptr) {}

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::Iter::operator*() const -> value_t& { return *_ptr; }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::Iter::operator->() const -> value_t* { return _ptr; }

//------------------------------------------------------------------------------
template<typename T>
auto Option<T>::Iter::operator++() -> Iter& {
  if (_ptr != nullptr) { _ptr = nullptr; }
  return *this;
}

//------------------------------------------------------------------------------
template<typename T>
bool Option<T>::Iter::operator==(const Iter& other) const {
  return _ptr == other._ptr;
}

//------------------------------------------------------------------------------
template<typename T>
bool Option<T>::Iter::operator!=(const Iter& other) const {
  return !(*this == other);
}

}

//------------------------------------------------------------------------------
template <class T>
std::ostream& operator<<(std::ostream& os, const fun::Option<T>& op) {
  return op.as_ref().match(
    [&](const T& x) -> std::ostream& {
      os << "Some(" << x << ")";
      return os;
    },
    [&]() -> std::ostream& {
      os << "Nothing";
      return os;
    }
  );
}
