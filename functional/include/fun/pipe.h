#ifndef FUN_PIPE_H
#define FUN_PIPE_H

#include <utility>

namespace fun {

//------------------------------------------------------------------------------
template <class T>
auto pipe(T&& x) -> T { return std::forward<T>(x); }

template <class T, class F, class ...Args>
auto pipe(T&& x, F&& f, Args&& ...args) {
  return pipe(f(std::forward<T>(x)), std::forward<Args>(args)...);
}

//------------------------------------------------------------------------------
template <class F>
struct LiftedFunc {
  F f;

  template <template<typename> class Functor, class T>
  using Return = Functor<typename std::result_of<F(T)>::type>;

  template <template<typename> class Functor, class T>
  auto operator()(Functor<T> op) -> Return<Functor, T> {
    return std::move(op).map(f);
  }
};

template <class F>
auto lift(F&& f) -> LiftedFunc<F> {
  return { std::forward<F>(f) };
}

//------------------------------------------------------------------------------
template <class F>
struct BoundFunc {
  F f;

  template <class T>
  using Return = typename std::result_of<F(T)>::type;

  template <template<typename> class M, class T>
  auto operator()(M<T> op) -> Return<T> {
    return std::move(op).and_then(f);
  }
};

template <class F>
auto bind(F&& f) -> BoundFunc<F> {
  return { std::forward<F>(f) };
}

} // end namespace fun

#endif
