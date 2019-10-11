#ifndef FUN_OPTION_IMPLEMENTATION_H
#define FUN_OPTION_IMPLEMENTATION_H

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
template<typename T>
auto some(T x) -> Option<typename std::remove_reference<T>::type> {
  return Option<T>(std::move(x));
}

//------------------------------------------------------------------------------
template <class T>
auto some_default() -> Option<T> { return { Option<T>(ForwardArgs{}) }; }

//------------------------------------------------------------------------------
template <class T>
auto some_ref(T& x) -> Option<T&> { return Option<T&>(OptionUnion<T&>(x)); }

//------------------------------------------------------------------------------
// template <typename T, typename ...Args>
// auto make_some(Args&& ...args) -> Option<T> {
//   return Option<T>(ForwardArgs{}, std::forward<Args>(args)...);
// }

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
match(SomeFuncT func_some, NoneFuncT func_none) && -> MatchReturn<SomeFuncT>
{
  static_assert(
    std::is_same< typename std::result_of<SomeFuncT(T)>::type
                , typename std::result_of<NoneFuncT()>::type
                >::value
    , "Some-handling and None-handling functions passed to match do not "
      "have the same return type"
    );
  return is_some() ? unvoid_call(func_some, unwrap())
                   : unvoid_call(func_none);
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
  if (is_some()) { return fun::make_ok(unwrap()); }
  else           { return fun::err(std::move(err)); }
}

//------------------------------------------------------------------------------
template <typename T>
template<typename ErrFuncT>
auto Option<T>::ok_or_else(ErrFuncT err_func) && -> Result<T, ErrorAlternative<ErrFuncT>>
{
  if (is_some()) { return fun::make_ok(unwrap()); }
  else           { return fun::make_err(unvoid_call(err_func)); }
}

//------------------------------------------------------------------------------
//!
//! Map Option<T> to Option<U> by applying the argument `func` (of type
//! U func(T) or T -> U) to the contained data if there is some.
//!
template<typename T>
template <typename F /* T -> U */>
auto Option<T>::map(F func) && -> MappedOption<F>
{
  if (is_some()) { return make_some(unvoid_call(func, unwrap())); }
  else { return {}; }
}

//------------------------------------------------------------------------------
template<typename T>
template <typename U, typename FuncT>
U Option<T>::map_or(U default_val, FuncT func) &&
{
  return is_some() ? unvoid_call(func, unwrap()) : std::move(default_val);
}

//------------------------------------------------------------------------------
template <typename T>
template <typename DefaultFunc, typename F>
auto Option<T>::
map_or_else(DefaultFunc default_func, F func) && -> MappedOption<F>
{
  return is_some() ? unvoid_call(func, unwrap()) : unvoid_call(default_func);
}

//------------------------------------------------------------------------------
//!
//! Returns a "None" variant if the Option object is "None", otherwise
//! it calls the input function with the contained value of type T and
//! returns an Option<U> with the result.
//!
template<typename T>
template <typename F /* T -> Option<U> */>
auto Option<T>::and_then(F func) && -> ValBoundOption<F>
{
  if (is_some()) { return unvoid_call(func, unwrap()); }
  else           { return {}; }
}

//------------------------------------------------------------------------------
template<typename T>
template <typename F /* () -> Option<T> */>
Option<T> Option<T>::or_else(F alt_func) &&
{
  return is_some() ? some(unwrap()) : unvoid_call(alt_func);
}

//------------------------------------------------------------------------------
template<typename T>
Option<T> Option<T>::take()
{
  return is_some() ? some(unwrap()) : Option<T>();
}

//------------------------------------------------------------------------------
template<typename T>
Option<T>& Option<T>::push(T obj) { return emplace(std::move(obj)); }

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
//!
//! Returns the "Some" value
//! @note This function has undefined behavior if it is called on a "None"
//!       Option
//!
template<typename T>
T Option<T>::unwrap() { return _inner.unwrap(); }

//------------------------------------------------------------------------------
template<typename T>
T Option<T>::expect(const char* err_msg)
{
  if (is_some()) { return unwrap(); }
  else           { throw std::runtime_error(err_msg); }
}

//------------------------------------------------------------------------------
template<typename T>
T Option<T>::unwrap_or(T alt)
{
  return is_some() ? unwrap() : std::move(alt);
}

//------------------------------------------------------------------------------
template <class T>
template <class F>
T Option<T>::unwrap_or_else(F alt_func)
{
  return is_some() ? unwrap() : unvoid_call(alt_func);
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

#endif
