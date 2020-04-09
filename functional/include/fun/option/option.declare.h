#ifndef FUN_OPTION_TEMPLATE_DECLARATIONS_H
#define FUN_OPTION_TEMPLATE_DECLARATIONS_H

//!
//! @author Alex Pronschinske
//! @copyright MIT LIcense
//!

#include <functional>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <cassert>
#include <ostream>

#include <fun/option/option_inner.h>

namespace fun {

template <typename T> class Option;

struct NothingTag{};

inline auto nothing() -> NothingTag { return {}; }

auto some() -> Option<Unit>;

template<typename T>
auto some(T x) -> Option<std::remove_reference_t<T>>;

template <class T>
auto some_default() -> Option<T>;

template <class T>
auto some_ref(T& x) -> Option<T&>;

// template <typename T, typename ...Args>
// auto make_some(Args&& ...args) -> Option<T>;

template <typename T, typename E> class Result;

struct SomeTag{};

template <class ...Args>
struct MakeOptionArgs { std::tuple<Args...> tup; };

template <class ...Args>
auto make_some(Args&& ...args) -> MakeOptionArgs<Args&&...>
{
  return { std::forward_as_tuple(std::forward<Args>(args)...) };
}

//!
//! Option type
//!
//! This type is a tagged union that may or may not contain the templated
//! type T. This type is excellent for modeling the return type of functions
//! that can fail. This implementation is inspired by the following:
//!   * Rust's `Option` type: https://doc.rust-lang.org/std/option/index.html
//!   * Boost's `optional` class:
//!     http://www.boost.org/doc/libs/1_62_0/libs/optional/doc/html/index.html
//!   * C++17's `optional` class:
//!     http://en.cppreference.com/w/cpp/experimental/optional
//!   * F# `optional`
//!   * Haskell `Maybe`
//!
//! The key idea with this class is that you can either deal with the type-
//! level uncertainty it models by checking (`is_some`) and "unwrapping" or
//! by dealing with it monadically and passing a function to the `map` method
//! to operate on the potentially contained type.
//!
template <typename T>
class Option {
public:
  using self_t = Option<T>;
  using Inner = T;
  using value_t = typename std::remove_reference<T>::type;

  class ConstIter;
  class Iter;

private:
  // Data members
  OptionUnion<T> _inner;

public:
  ~Option() = default;

  Option(self_t&&) = default;
  auto operator=(self_t&&) -> self_t& = default;

  Option(const self_t&) = default;
  auto operator=(const self_t&) -> self_t& = default;

  auto clone() const -> self_t { return *this; }

  explicit Option(OptionUnion<T> mem) : _inner(std::move(mem)) {}

  // Constructors
  //!
  //! Default constructor is to the None value
  //!
  Option() = default;

  Option(NothingTag) : Option() {}

  explicit Option(T x) : _inner(std::move(x)) {}

  template <typename ...Args>
  explicit Option(ForwardArgs, Args&& ... args)
    : _inner(ForwardArgs{}, std::forward<Args>(args)...)
  {}

  template <class ...Args, size_t ...Indices>
  Option(SomeTag, std::tuple<Args...>& args, std::integer_sequence<size_t, Indices...>)
    : Option(ForwardArgs{}, std::forward<Args>(std::get<Indices>(args))...)
  {}

  template <class ...Args>
  Option(MakeOptionArgs<Args...>&& make_args)
    : Option(SomeTag{}, make_args.tup, std::index_sequence_for<Args...>{})
  {}

    // variant testing
  bool is_some() const { return _inner.is_some(); }
  bool is_none() const { return !is_some(); }
  explicit operator bool() const { return is_some(); }

  value_t* as_ptr() { return _inner.as_ptr(); }
  const value_t* as_ptr() const {
    return const_cast<OptionUnion<T>&>(_inner).as_ptr();
  }
  const value_t* as_const_ptr() const { return as_ptr(); }

  auto as_ref() -> Option<value_t&> {
    if (is_some()) { return some_ref(*as_ptr()); }
    else           { return {}; }
  }
  auto as_ref() const -> Option<const value_t&> {
    if (is_some()) { return some_ref(*as_ptr()); }
    else           { return {}; }
  }
  auto as_const_ref() const -> Option<const value_t&> { return as_ref(); }

  bool operator==(const Option<T>& other) const {
    return _inner == other._inner;
  }
  bool operator!=(const Option<T>& other) const {
    return !(*this == other);
  }

  template <class F>
  using MatchReturn = ResultOf_t<F(T)>;

  //!
  //! The match method take a function for handling either variant and
  //! dispatches the appropriate function and returns the common type.
  //!
  template <typename SomeFuncT, typename NoneFuncT>
  auto match(SomeFuncT, NoneFuncT) && -> MatchReturn<SomeFuncT>;

  // Iterator creation
  Iter begin();
  ConstIter begin() const;
  ConstIter cbegin() const;
  Iter end();
  ConstIter end() const;
  ConstIter cend() const;

  template<typename E, typename ... Args>
  auto ok_or(E err) && -> Result<T, E>;

  template <class F>
  using ErrorAlternative = ResultOf_t<F()>;

  template<typename ErrFuncT>
  auto ok_or_else(ErrFuncT err_func) && -> Result<T, ErrorAlternative<ErrFuncT>>;

  template <class F>
  using MappedOption = Option<ResultOf_t<F(T)>>;

  //!
  //! Map Option<T> to Option<U> by applying the argument `func` (of type
  //! U func(T) or T -> U) to the contained data if there is some.
  //!
  template <typename F /* T -> U */>
  auto map(F func) && -> MappedOption<F>;

  template <typename U, typename FuncT>
  U map_or(U default_val, FuncT func) &&;

  template <typename DefaultFunc, typename F>
  auto map_or_else(DefaultFunc, F) && -> MappedOption<F>;

  template <class F>
  using ValBoundOption = Option<typename ResultOf_t<F(T)>::Inner>;

  //!
  //! Returns a "None" variant if the Option object is "None", otherwise
  //! it calls the input function with the contained value of type T and
  //! returns an Option<U> with the result.
  //!
  template <typename F /* T -> Option<U> */>
  auto and_then(F func) && -> ValBoundOption<F>;

  template <typename F /* () -> Option<T> */>
  Option<T> or_else(F alt_func) &&;

  template <class F /* const T& -> bool */>
  auto filter(F&& predicate) && -> Option<T>
  {
    return std::move(*this).and_then(
      [&](auto&& obj) -> Option<T> {
        if (predicate(obj)) { return fun::Option<T>(std::forward<decltype(obj)>(obj)); }
        else                { return {}; }
      }
    );
  }

  Option<T> take();

  auto push(T obj) -> self_t&;

  template <typename ...Args>
  auto emplace(Args&& ...args) -> self_t&;

  auto cloned() const -> Option<value_t>;

  //!
  //! Returns the "Some" value
  //!
  //! @note This function has undefined behavior if it is called on a "None"
  //!       Option
  //!
  //! @note This function leaves the instance on which it is called in an
  //!       unspecified but valid state. After this function has returned, the
  //!       behavior of methods other than the destructor and assignment
  //!       operators is unspecified.
  //!
  T unwrap() &&;

  T expect(const char* err_msg) &&;

  T unwrap_or(T alt) &&;

  template <class F>
  T unwrap_or_else(F alt_func) &&;

  auto unwrap_or_default() && -> T {
    return std::move(*this).unwrap_or_else([]() -> T { return {}; });
  }

  // Iterators
  //----------
  class ConstIter {
  protected:
    const value_t* _ptr = nullptr;
  public:
    ConstIter() = default;
    explicit ConstIter(const value_t* ptr);

    // overload *, ->, and ++_
    const value_t& operator*() const;
    const value_t* operator->() const;
    ConstIter& operator++();
    bool operator==(const ConstIter& other) const;
    bool operator!=(const ConstIter& other) const;
  };

  class Iter {
  protected:
    value_t* _ptr = nullptr;
  public:
    Iter() = default;
    explicit Iter(value_t* ptr);

    // overload *, ->, and ++_
    value_t& operator*() const;
    value_t* operator->() const;
    Iter& operator++();
    bool operator==(const Iter& other) const;
    bool operator!=(const Iter& other) const;
  };
};

}

template <class T>
std::ostream& operator<<(std::ostream& os, const fun::Option<T>& op);

#endif
