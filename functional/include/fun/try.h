#ifndef FUN_TRY_MACRO_H
#define FUN_TRY_MACRO_H

#include <fun/option.h>
#include <fun/result.h>

#define FUN_TRY_CHECK_DIVERGE(tmp_id, expr)                                    \
  auto tmp_id = (expr);                                                        \
  if (!tmp_id) { return ::fun::try_detail::diverge(::std::move(tmp_id)); }

#define FUN_TRY_DECLARE_IMPL(tmp_id, dst_id, expr)                             \
  FUN_TRY_CHECK_DIVERGE(tmp_id, expr);                                         \
  decltype(auto) dst_id = std::move(tmp_id).unwrap();

#define FUN_TRY_ASSIGN_IMPL(tmp_id, dst_id, expr)                              \
  FUN_TRY_CHECK_DIVERGE(tmp_id, expr);                                         \
  dst_id = std::move(tmp_id).unwrap();

#define FUN_TRY_DECLARE(dst_id, expr)                                          \
  FUN_TRY_DECLARE_IMPL(dst_id##_fun_try_declare_tmp, dst_id, expr)

#define FUN_TRY_ASSIGN(dst_id, expr)                                           \
  FUN_TRY_ASSIGN_IMPL(dst_id##_fun_try_assign_tmp, dst_id, expr)

namespace fun {
namespace try_detail {

template <class T>
auto diverge(fun::Option<T>&&) { return fun::nothing(); }

template <class T, class E>
auto diverge(fun::Result<T, E>&& res) { return fun::err(std::move(res).unwrap_err()); }

} // end namespace try_detail
} // end namespace fun

#endif
