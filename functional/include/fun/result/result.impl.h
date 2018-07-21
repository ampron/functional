#ifndef FUN_RESULT_IMPLEMENTATION_H
#define FUN_RESULT_IMPLEMENTATION_H

//!
//! @author Alex Pronschinske
//! @copyright MIT LIcense
//!

#include <fun/result/result.declare.h>
#include <fun/option/option.impl.h>

namespace fun {

//==============================================================================
// Result-releated function definitions
//------------------------------------------------------------------------------
template <class T>
auto return_ok(T val) -> MakeOkResult<T> {
  return { std::move(val) };
}

//------------------------------------------------------------------------------
template <class E, class T>
auto return_ok(T val) -> Result<T, E> {
  return return_ok(std::move(val));
}

//------------------------------------------------------------------------------
template <class T>
auto return_ok_cref(const T& val) -> MakeOkResult<const T&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class T>
auto return_ok_ref(T& val) -> MakeOkResult<T&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class E, class T>
auto return_ok_ref(T& val) -> Result<T&, E> {
  return return_ok_ref(val);
}

//------------------------------------------------------------------------------
template <class E>
auto return_err(E val) -> MakeErrResult<E> {
  return { std::move(val) };
}

//------------------------------------------------------------------------------
template <class T, class E>
auto return_err(E val) -> Result<T, E> {
  return return_err(std::move(val));
}

//------------------------------------------------------------------------------
template <class E>
auto return_err_cref(const E& val) -> MakeErrResult<const E&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class E>
auto return_err_ref(E& val) -> MakeErrResult<E&> {
  return { val };
}

//------------------------------------------------------------------------------
template <class T, class E>
auto return_err_ref(E& val) -> Result<T, E&> {
  return return_err_ref(val);
}

//==============================================================================
// Option definitions
//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::dump_ok() -> T {
  assert(_variant == Ok);
	auto temp = std::move(_ok._val);
	_ok.~Sized<T>();
	return temp;
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::dump_err() -> E {
  assert(_variant == Err);
	auto temp = std::move(_err._val);
	_err.~Sized<E>();
	return temp;
}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::~Result() {
  if (_variant == Ok) { _ok.~Sized<T>(); }
  else                { _err.~Sized<E>(); }
}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(self_t&& other)
  : _variant(other._variant)
{
  if (_variant == Ok) { new (&_ok) Sized<T>{std::move(other._ok)}; }
  else                { new (&_err) Sized<E>{std::move(other._err)}; }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator=(self_t&& other) -> self_t& {
  if (this != &other) { new (this) self_t(std::move(other)); }
  return *this;
}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(const self_t& other)
  : _variant(other._variant)
{
  if (_variant == Ok) { new (&_ok) Sized<T>{other._ok}; }
  else                { new (&_err) Sized<E>{other._err}; }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator=(const self_t& other) -> self_t& {
  if (this != &other) { new (this) self_t(other); }
  return *this;
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::clone() const -> self_t { return self_t(*this); }

//------------------------------------------------------------------------------
template <class T, class E>
template <typename ...Args>
Result<T, E>::Result(OkTag, ForwardArgs, Args&& ...args)
  : _variant(Ok), _ok(Sized<T>{T(std::forward<Args>(args)...)})
{}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename ...Args>
Result<T, E>::Result(ErrTag, ForwardArgs, Args&& ...args)
  : _variant(Err), _err(Sized<E>{E(std::forward<Args>(args)...)})
{}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(MakeOkResult<T> ok)
  : Result(OkTag{}, ForwardArgs{}, std::move(ok.val))
{}

//------------------------------------------------------------------------------
template <class T, class E>
Result<T, E>::Result(MakeErrResult<E> err)
  : Result(ErrTag{}, ForwardArgs{}, std::move(err.val))
{}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::is_ok() const -> bool { return _variant == Ok; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::is_err() const -> bool { return !is_ok(); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_ptr() -> ok_value_t* { return is_ok() ? &_ok : nullptr; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_err_ptr() -> err_value_t* { return is_err() ? &_err : nullptr; }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator==(const self_t& other) const -> bool {
  if (is_ok()) {
    return other.is_ok() ? (_ok._val == other._ok._val) : false;
  } else {
    return other.is_err() ? (_err._val == other._err._val) : false;
  }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::operator!=(const self_t& other) const -> bool { return !(*this == other); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::unwrap() -> T { return dump_ok(); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::unwrap_err() -> E { return dump_err(); }

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_ref() -> Result<ok_value_t&, err_value_t&> {
  if (is_ok()) { return return_ok_ref(_ok._val); }
  else         { return return_err_ref(_err._val); }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_ref() const -> Result<const ok_value_t&, const err_value_t&> {
  if (is_ok()) { return return_ok_ref(this->_ok._val); }
  else         { return return_err_ref(this->_err._val); }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::as_cref() const -> Result<const ok_value_t&, const err_value_t&> {
  return as_ref();
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::ok() && -> Option<T> {
  if (is_ok()) { return some(dump_ok()); }
  else         { return {}; }
}

//------------------------------------------------------------------------------
template <class T, class E>
auto Result<T, E>::err() && -> Option<E> {
  if (is_err()) { return some(dump_err()); }
  else         { return {}; }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename OkFunc, typename ErrFunc>
auto Result<T, E>::match(OkFunc func_ok, ErrFunc func_err) && -> MatchReturn<OkFunc> {
  static_assert(
    std::is_same< typename std::result_of<OkFunc(T)>::type
                , typename std::result_of<ErrFunc(E)>::type
                >::value
    , "Ok-handling and Err-handling functions passed to match do not "
      "have the same return type"
    );
  return is_ok() ? unvoid_call(func_ok, unwrap())
                 : unvoid_call(func_err, unwrap_err());
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F>
auto Result<T, E>::map(F func) && -> MapReturn<F> {
  if (is_ok()) { return return_ok(unvoid_call(func, unwrap())); }
  else         { return return_err(unwrap_err()); }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F>
auto Result<T, E>::map_err(F func) && -> ErrMapReturn<F> {
    if (is_err()) { return return_err(unvoid_call(func, unwrap_err())); }
    else          { return return_ok(unwrap()); }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F /* T -> Result<U, E> */>
auto Result<T, E>::and_then(F func) && -> AndThenReturn<F> {
    if (is_ok()) { return unvoid_call(func, unwrap()); }
    else         { return return_err(unwrap_err()); }
}

//------------------------------------------------------------------------------
template <class T, class E>
template <typename F>
auto Result<T, E>::or_else(F alt_func) && -> OrElseReturn<F> {
    if (is_ok()) { return return_ok(unwrap()); }
    else         { return unvoid_call(alt_func, unwrap_err()); }
}

}

//------------------------------------------------------------------------------
template <class T, class E>
std::ostream& operator<<(std::ostream& os, const fun::Result<T, E>& res) {
  return res.as_ref().match(
    [&](const T& x) -> std::ostream& {
      os << "Ok(" << x << ")";
      return os;
    },
    [&](const E& y) -> std::ostream& {
      os << "Err(" << y << ")";
      return os;
    }
  );
}

#endif
