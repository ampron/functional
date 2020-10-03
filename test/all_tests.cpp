
#include <memory>
#include <iostream>
#include <string>
#include <vector>

#include <fun/pipe.h>
#include <fun/result.h>
#include <fun/try.h>
#include <gtest/gtest.h>

#include "testing.h"

//------------------------------------------------------------------------------
class Monolith {
private:
  int _n;

public:
  using self_t = Monolith;

  ~Monolith() = default;

  Monolith(self_t&&) = delete;
  auto operator=(self_t&&) -> self_t& = delete;

  Monolith(const self_t&) = delete;
  auto operator=(const self_t&) -> self_t& = delete;

  Monolith() = delete;

  explicit Monolith(const int n) : _n(n) {}

  auto double_up() const -> int { return 2 * _n; }

  auto operator==(const self_t& rhs) const -> bool {
    return _n == rhs._n;
  }
};

//------------------------------------------------------------------------------
class CryBaby {
  int _n = 0;

public:
  ~CryBaby() {
    std::cout << "waaaaa destruction!" << std::endl;
  }

  using Self = CryBaby;
  CryBaby(const Self& other) {
    std::cout << "waaaaa copy construction!" << std::endl;
  }
  auto operator=(const Self& other) -> Self& {
    std::cout << "waaaaa copy assign!" << std::endl;
    return *this;
  }

  CryBaby(Self&& other) {
    std::cout << "waaaaa move construction!" << std::endl;
  }
  auto operator=(Self&& other) -> Self& {
    std::cout << "waaaaa move assign!" << std::endl;
    return *this;
  }

  CryBaby() = default;

  void cry() const { std::cout << "waaaaa!" << std::endl; }
};

//------------------------------------------------------------------------------
struct DestructionCounter {
  using Self = DestructionCounter;

  std::reference_wrapper<bool> _multi_dtor;
  int _destructions = 0;

  ~DestructionCounter() {
    _destructions += 1;
    if (1 < _destructions) { _multi_dtor.get() = true; }
  }

  DestructionCounter(const Self& other) : _multi_dtor(other._multi_dtor) {};
  auto operator=(const Self& other) -> Self& {
    if (this == &other) { return *this; }
    _multi_dtor = other._multi_dtor;
    _destructions = 0;
    return *this;
  }

  explicit DestructionCounter(bool& b) : _multi_dtor(std::ref(b)) {}
  explicit DestructionCounter(std::reference_wrapper<bool> b) : _multi_dtor(b) {}
};

//------------------------------------------------------------------------------
class DestructionDetector {
  using Self = DestructionDetector;
  bool* _did_destruct;
public:
  ~DestructionDetector() {
    if (_did_destruct) { *_did_destruct = true; }
  }

  DestructionDetector(): _did_destruct(nullptr) {}

  DestructionDetector(bool& b) : _did_destruct(&b) {}

  DestructionDetector(const DestructionDetector&): _did_destruct(nullptr) {}

  auto operator=(const DestructionDetector&) -> DestructionDetector& = delete;
};

//------------------------------------------------------------------------------
void do_nothing() {}

//------------------------------------------------------------------------------
std::vector<double> example_vector() {
  return std::vector<double>({1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
}

//------------------------------------------------------------------------------
auto example_unique_one() -> std::unique_ptr<int> {
  return std::unique_ptr<int>(new int(1));
}

//------------------------------------------------------------------------------
TEST(OptionTest, none_constructor) {
  fun::Option<int> op;
  ASSERT_TRUE(op.is_none());
}

//------------------------------------------------------------------------------
TEST(OptionTest, some_constructor) {
  const fun::Option<int> op(3);
  ASSERT_TRUE(op.is_some());
}

//------------------------------------------------------------------------------
TEST(OptionTest, destroy_once) {
  auto any_multi_dtor = false;
  {
    auto y = fun::Option<DestructionCounter>(fun::make_some(any_multi_dtor)).unwrap();
    ASSERT_FALSE(any_multi_dtor);
  }
  ASSERT_FALSE(any_multi_dtor);
}

//------------------------------------------------------------------------------
TEST(OptionTest, move_assignment_with_cry_baby) {
  const auto cry_baby = std::make_shared<CryBaby>();
  ASSERT_TRUE(cry_baby.unique());
  {
    auto maybe_baby = fun::Option<std::shared_ptr<CryBaby>>();
    ASSERT_TRUE(maybe_baby.is_none());

    maybe_baby = fun::Option<std::shared_ptr<CryBaby>>(cry_baby);
    ASSERT_TRUE(maybe_baby.is_some());
    ASSERT_FALSE(cry_baby.unique());

    maybe_baby = fun::Option<std::shared_ptr<CryBaby>>(std::make_shared<CryBaby>());
    ASSERT_TRUE(maybe_baby.is_some());
    ASSERT_TRUE(cry_baby.unique());
  }

  ASSERT_TRUE(cry_baby.unique());
  cry_baby->cry();
}

//------------------------------------------------------------------------------
TEST(OptionTest, copy_assignment_with_cry_baby) {
  const auto cry_baby = std::make_shared<CryBaby>();
  ASSERT_TRUE(cry_baby.unique());
  {
    fun::Option<std::shared_ptr<CryBaby>> maybe_baby{};
    ASSERT_TRUE(maybe_baby.is_none());

    maybe_baby = fun::Option<std::shared_ptr<CryBaby>>(cry_baby);
    ASSERT_TRUE(maybe_baby.is_some());
    ASSERT_FALSE(cry_baby.unique());

    const fun::Option<std::shared_ptr<CryBaby>> other_maybe_baby{std::make_shared<CryBaby>()};
    maybe_baby = other_maybe_baby;
    ASSERT_TRUE(maybe_baby.is_some());
    ASSERT_TRUE(cry_baby.unique());
  }

  ASSERT_TRUE(cry_baby.unique());
  cry_baby->cry();
}

//------------------------------------------------------------------------------
TEST(OptionTest, some_reference_constructor) {
  const auto x = 3;
  const fun::Option<const int&> op(x);
  ASSERT_TRUE(op.is_some());
}

//------------------------------------------------------------------------------
TEST(OptionTest, some_function) {
    const auto x = fun::some(3);
    ASSERT_TRUE(x.is_some());
}

//------------------------------------------------------------------------------
TEST(OptionTest, some_ref_function) {
    const auto x = 3;
    const auto op = fun::some_ref(x);
    ASSERT_TRUE(op.is_some());
}

//------------------------------------------------------------------------------
TEST(OptionTest, forwarding_constructor) {
  const fun::Option<std::vector<int>> op = fun::make_some(3, 0);
  ASSERT_TRUE(op.is_some());
}

//------------------------------------------------------------------------------
TEST(OptionTest, emplace) {
  auto op = fun::Option<std::vector<int>>();
  ASSERT_TRUE(op.is_none());

  op.emplace(5, 0);
  ASSERT_TRUE(op.is_some());
}

//------------------------------------------------------------------------------
TEST(OptionTest, equality_operator) {
  const auto a = fun::some(2.0);
  const auto b = fun::some(2.0);
  const auto c = fun::some(1.0);

  ASSERT_TRUE(a == b);
  ASSERT_TRUE(b != c);
}

//------------------------------------------------------------------------------
TEST(OptionTest, as_ref) {
  auto x = fun::some(5);
  const auto x_ref = x.as_ref();
  ASSERT_TRUE(x_ref.is_some());
  ASSERT_TRUE(x_ref.clone().unwrap() == 5);
}

//------------------------------------------------------------------------------
TEST(OptionTest, as_const_ref) {
  const auto x = fun::some(5);
  const auto x_ref = x.as_ref();
  ASSERT_TRUE(x_ref);
  ASSERT_TRUE(x_ref.clone().unwrap() == 5);
}

//------------------------------------------------------------------------------
TEST(OptionTest, iterators) {
  {
    auto op = fun::Option<double>(1.5);
    size_t n = 0;
    for (const auto& x : op) {
      ++n;
      EXPECT_EQ(x, 1.5);
    }
    EXPECT_EQ(n, size_t(1));
  }
  {
    auto op = fun::Option<double>();
    size_t n = 0;
    for (const auto& x : op) {
      ++n;
    }
    EXPECT_EQ(n, size_t(0));
  }
}

//------------------------------------------------------------------------------
TEST(OptionTest, implicit_bool_conversion) {
  {
    size_t n = 0;
    if (auto op = fun::Option<double>(1.5)) {
      ++n;
      EXPECT_EQ(op.clone().unwrap(), 1.5);
    }
    EXPECT_EQ(n, size_t(1));
  }
  {
    size_t n = 0;
    if (auto op = fun::Option<double>()) {
      ++n;
    }
    EXPECT_EQ(n, size_t(0));
  }
}

//------------------------------------------------------------------------------
TEST(OptionTest, match) {
  using std::vector;

  const auto y = 10.0;
  {
    auto op = fun::some(example_vector());
    const auto xs = std::move(op).match(
      [&](vector<double> vec){
        vec.push_back(y); return vec;
      },
      [](){
        return vector<double>();
      }
    );
    ASSERT_EQ(xs.size(), example_vector().size() + 1);
  }
  {
    const auto op = fun::some(example_vector());
    const auto x = op.as_ref().match(
      [&](const vector<double>& vec) { return vec.size() + y; },
      [&]() { return -y; }
    );
    ASSERT_EQ(x, example_vector().size() + y);
  }
}

//------------------------------------------------------------------------------
TEST(OptionTest, match_void) {
  using std::vector;

  fun::some(example_vector()).match(
    [&](vector<double> vec) {
      ASSERT_EQ(vec.size(), example_vector().size());
    },
    [&]() {
      ASSERT_TRUE(false);
    }
  );

  const auto op = fun::some(example_vector());
  op.as_ref().match(
    [&](const vector<double>& vec) {
      ASSERT_EQ(vec.size(), size_t(6));
    },
    [&]() {
      ASSERT_TRUE(false);
    }
  );
}

//------------------------------------------------------------------------------
TEST(OptionTest, map) {
  using std::vector;

  const auto xs = fun::some(example_vector())
    .map(
      [](vector<double> vec) {
        vec.push_back(7.0);
        return vec;
      }
    )
    .unwrap();

  ASSERT_EQ(xs.size(), example_vector().size() + 1);
}

//------------------------------------------------------------------------------
TEST(OptionTest, map_ref) {
  auto xs = example_vector();
  auto& xs_ref =
    fun::some_ref(xs).map([](auto& vec) -> decltype(vec)& {
      vec.push_back(7.0);
      return vec;
    })
    .unwrap();

  ASSERT_EQ(xs.size(), example_vector().size() + 1);
  ASSERT_EQ(xs, xs_ref);
}

//------------------------------------------------------------------------------
TEST(OptionTest, map_void) {
  using std::vector;

  auto xs = example_vector();
  const auto unit = fun::some_ref(xs).map(
    [](vector<double>& vec) -> void { vec.push_back(7.0); }
  );

  ASSERT_EQ(xs.size(), example_vector().size() + 1);

  const auto ys = example_vector();
  size_t n = 0;
  fun::some(ys).map(
    [&](vector<double> vec) -> void {
      vec.push_back(7.0);
      n = vec.size();
    }
  );

  ASSERT_EQ(ys.size() + 1, n);
}

//------------------------------------------------------------------------------
TEST(OptionTest, unvoid) {
  const auto unit = fun::unvoid_call(do_nothing);
  ASSERT_EQ(unit, fun::Unit{});
}

//------------------------------------------------------------------------------
TEST(OptionTest, map_or) {
  using std::vector;

  const auto xs = fun::some(example_vector()).map_or(vector<double>(),
    [](vector<double> vec) {
      vec.push_back(7.0);
      return vec;
    }
  );

  ASSERT_EQ(xs.size(), example_vector().size() + 1);
}

//------------------------------------------------------------------------------
TEST(OptionTest, zip) {
  const auto sum_pair = [](auto xy) { return xy.first + xy.second; };
  {
    const auto sum =
      fun::some(1).zip(fun::some(1.)).map(sum_pair).unwrap_or(0);
    ASSERT_EQ(sum, 2);
  }
  {
    const auto sum =
      fun::some(1).zip(fun::Option<double>()).map(sum_pair).unwrap_or(0);
    ASSERT_EQ(sum, 0);
  }
  {
    const auto sum =
      fun::Option<int>().zip(fun::some(1.)).map(sum_pair).unwrap_or(0);
    ASSERT_EQ(sum, 0);
  }
  {
    const auto sum =
      fun::Option<int>().zip(fun::Option<double>()).map(sum_pair).unwrap_or(0);
    ASSERT_EQ(sum, 0);
  }
}

//------------------------------------------------------------------------------
TEST(OptionTest, bind) {
  using std::vector;

  auto xs = example_vector();
  auto last = fun::some_ref(xs).and_then(
    [](vector<double>& vec) {
      vec.push_back(7.0);
      return fun::some(7.0);
    }
  );

  ASSERT_EQ(xs.size(), example_vector().size() + 1);
  ASSERT_EQ(last.clone().unwrap(), 7.);
}

//------------------------------------------------------------------------------
TEST(OptionTest, filter) {
  auto maybe_xs = fun::some(example_vector());

  auto maybe_ys = maybe_xs.clone().filter([](const auto& xs) { return 1 < xs.size(); });
  ASSERT_TRUE(maybe_ys.is_some());

  auto maybe_zs = maybe_xs.clone().filter([](const auto& xs) { return 100 < xs.size(); });
  ASSERT_TRUE(maybe_zs.is_none());
}

//------------------------------------------------------------------------------
TEST(OptionTest, iteration) {
  using std::vector;

  auto xs = example_vector();
  auto maybe_xs = fun::some_ref(xs);
  for (auto& xs : maybe_xs) {
      xs.push_back(7.0);
  }

  ASSERT_EQ(xs.size(), example_vector().size() + 1);

  auto maybe_ys = fun::some(example_vector());
  for (auto& ys : maybe_ys) {
      ys.push_back(7.0);
  }
  const auto p_ys = maybe_ys.as_ptr();
  const auto n_ys = p_ys ? p_ys->size() : 0;

  ASSERT_EQ(n_ys, example_vector().size() + 1);
}

//------------------------------------------------------------------------------
TEST(OptionTest, unwrap_or) {
  const auto x = fun::some(2).unwrap_or(0);
  ASSERT_TRUE(x == 2);
}

//------------------------------------------------------------------------------
TEST(OptionTest, unwrap_or_default) {
  auto empty_str = fun::Option<std::string>().unwrap_or_default();
  ASSERT_TRUE(empty_str.empty());
}

//------------------------------------------------------------------------------
TEST(OptionTest, equality) {
  const auto x = fun::some(2);
  const auto y = fun::some(2);
  const auto z = fun::Option<int>();
  EXPECT_EQ(x, y);
  EXPECT_NE(z, x);
}

//------------------------------------------------------------------------------
TEST(OptionTest, expect) {
  const auto x = fun::Option<int>();
  ASSERT_THROW(x.clone().expect("error message"), std::runtime_error);
}

//------------------------------------------------------------------------------
TEST(OptionTest, emplace_unit) {
  auto x = fun::Option<fun::Unit>();
  EXPECT_TRUE(x.is_none());
  x.emplace();
  EXPECT_TRUE(x.is_some());
  x.emplace(fun::Unit());
  EXPECT_TRUE(x.is_some());
}

//------------------------------------------------------------------------------
TEST(OptionTest, emplace_reference) {
  auto x = fun::Option<int&>();
  auto y = 5;
  EXPECT_TRUE(x.is_none());
  x.emplace(y);
  EXPECT_TRUE(x.is_some());
  EXPECT_EQ(std::move(x).unwrap(), y);
}

//------------------------------------------------------------------------------
TEST(OptionTest, take) {
  auto x = fun::some(5);
  EXPECT_TRUE(x.is_some());
  EXPECT_EQ(x.take().unwrap(), 5);
  EXPECT_TRUE(x.is_none());

  auto y = fun::some(fun::Unit());
  EXPECT_TRUE(y.is_some());
  y.take();
  EXPECT_TRUE(y.is_none());

  auto z = fun::some_ref(x);
  EXPECT_TRUE(z.is_some());
  EXPECT_EQ(z.take().unwrap(), x);
  EXPECT_TRUE(z.is_none());
}

//------------------------------------------------------------------------------
TEST(OptionTest, destruct_after_move_construct) {
  auto x_did_destruct = false;
  auto y = [&]() {
    auto x = fun::Option<DestructionDetector>(fun::ForwardArgs(), x_did_destruct);
    return std::move(x);
  }();
  EXPECT_TRUE(x_did_destruct);
}

//------------------------------------------------------------------------------
TEST(OptionTest, destruct_after_move_assign) {
  auto x_did_destruct = false;
  auto y = fun::Option<DestructionDetector>();
  {
    auto x = fun::Option<DestructionDetector>(fun::ForwardArgs(), x_did_destruct);
    y = std::move(x);
  }
  EXPECT_TRUE(x_did_destruct);
}

//------------------------------------------------------------------------------
TEST(OptionTest, destruct_after_unwrap) {
  auto did_destruct = false;
  fun::Option<DestructionDetector>(fun::ForwardArgs(), did_destruct).unwrap();
  EXPECT_TRUE(did_destruct);
}

//------------------------------------------------------------------------------
TEST(OptionTest, destruct_after_take) {
  auto did_destruct = false;
  fun::Option<DestructionDetector>(fun::ForwardArgs(), did_destruct).take();
  EXPECT_TRUE(did_destruct);
}

//------------------------------------------------------------------------------
TEST(OptionTest, empty_base) {
  auto op1 = fun::some(fun::Unit());
  EXPECT_TRUE(op1.is_some());
  fun::Option<fun::Unit> op2 {};
  EXPECT_TRUE(op2.is_none());
  op2 = std::move(op1);
  EXPECT_TRUE(op2.is_some());
  EXPECT_TRUE(op1.is_none());
  op1.emplace(fun::Unit());
  EXPECT_TRUE(op1 == op2);
}

//------------------------------------------------------------------------------
TEST(ResultTest, construction) {
  const auto x = fun::ok<std::string>(3);
  ASSERT_EQ(x.clone().unwrap(), 3);
  const auto y = fun::err<int>(std::string("fail"));
  ASSERT_EQ(y.clone().unwrap_err().size(), size_t(4));
  auto z = fun::Result<std::vector<double>, std::string>(
    fun::OkTag{}, fun::ForwardArgs{}, 5, 2.0
  );
  ASSERT_EQ(std::move(z).unwrap().size(), size_t(5));
  const auto xs = example_vector();
  const auto xs_ref = fun::ok_ref<std::string>(xs);
  ASSERT_EQ(xs_ref.clone().unwrap().size(), xs.size());
  const auto xs_err_ref = fun::err_ref<std::string>(xs);
  ASSERT_EQ(xs_err_ref.clone().unwrap_err().size(), xs.size());
}

//------------------------------------------------------------------------------
TEST(ResultTest, unwrap) {
  auto p = fun::ok<std::string>(example_unique_one()).unwrap();
  ASSERT_TRUE(p.get());
}

//------------------------------------------------------------------------------
TEST(ResultTest, unwrap_or_default) {
  auto empty_str = fun::err<std::string>(0).unwrap_or_default();
  ASSERT_TRUE(empty_str.empty());
}

//------------------------------------------------------------------------------
TEST(ResultTest, unwrap_ref) {
  fun::Result<const int&, std::string> res = fun::make_err("blah");
  {
    const auto n = 5;
    res = fun::ok_ref(n);
    ASSERT_TRUE(res.is_ok());
    const auto n_ = res.clone().unwrap();
    ASSERT_EQ(n_, n);
  }
  {
    const auto n = 3;
    res = fun::ok_ref(n);
    ASSERT_TRUE(res.is_ok());
    const auto n_ = res.clone().unwrap();
    ASSERT_EQ(n_, n);
  }
}

//------------------------------------------------------------------------------
TEST(ResultTest, destroy_once) {
  auto any_multi_dtor = false;
  {
    auto y = fun::Result<DestructionCounter, fun::Unit>(fun::make_ok(any_multi_dtor)).unwrap();
    ASSERT_FALSE(any_multi_dtor);
  }
  ASSERT_FALSE(any_multi_dtor);
}

//------------------------------------------------------------------------------
TEST(ResultTest, destroy_err_once) {
  auto any_multi_dtor = false;
  {
    auto y = fun::Result<fun::Unit, DestructionCounter>(fun::make_err(any_multi_dtor)).unwrap_err();
    ASSERT_FALSE(any_multi_dtor);
  }
  ASSERT_FALSE(any_multi_dtor);
}

//------------------------------------------------------------------------------
TEST(ResultTest, equality) {
  using std::string;

  const auto a = fun::ok<string>(5.0);
  const auto b = fun::ok<string>(5.0);
  const auto c = fun::ok<string>(4.0);
  const auto d = fun::err<double>(string("not a number"));
  const auto e = fun::err<double>(string("not a number"));

  ASSERT_EQ(a, b);
  ASSERT_NE(a, c);
  ASSERT_NE(a, d);
  ASSERT_EQ(d, e);
}

//------------------------------------------------------------------------------
TEST(ResultTest, map) {
  using std::vector;
  using error_t = int;

  const auto xs = fun::ok<error_t>(example_vector())
    .map(
      [](vector<double> vec) {
        vec.push_back(7.0);
        return vec;
      }
    )
    .unwrap();

  ASSERT_EQ(xs.size(), example_vector().size() + 1);
}

//------------------------------------------------------------------------------
TEST(ResultTest, zip) {
  using E = std::string;
  const auto sum_pair = [](auto xy) { return xy.first + xy.second; };
  {
    const auto sum =
      fun::ok<E>(1).zip(fun::ok<E>(1.)).map(sum_pair).unwrap_or(0);
    ASSERT_EQ(sum, 2);
  }
  {
    const auto e =
      fun::ok<E>(1).zip(fun::err<double, E>("a")).map(sum_pair)
      .err().unwrap_or("");
    ASSERT_EQ(e, "a");
  }
  {
    const auto e =
      fun::err<int, E>("a").zip(fun::ok<E>(1.)).map(sum_pair)
      .err().unwrap_or("");
    ASSERT_EQ(e, "a");
  }
  {
    const auto e =
      fun::err<int, E>("a").zip(fun::err<double, E>("b")).map(sum_pair)
      .err().unwrap_or("");
    ASSERT_EQ(e, "a");
  }
}

//------------------------------------------------------------------------------
TEST(ResultTest, and_then) {
  using std::vector;
  using error_t = int;

  auto xs = example_vector();
  auto last = fun::ok_ref<error_t>(xs).and_then(
    [](vector<double>& vec) {
      vec.push_back(7.0);
      return fun::ok<error_t>(7.0);
    }
  );

  ASSERT_EQ(xs.size(), example_vector().size() + 1);
  ASSERT_EQ(last.clone().unwrap(), 7.);
}

//------------------------------------------------------------------------------
TEST(ResultTest, or_else) {
  using ok_t = double;
  using error_t = std::pair<int, std::string>;

  fun::Result<ok_t, error_t> x = fun::err(error_t(-1, "fail"));
  auto last = x.clone().or_else(
    [](error_t e) -> fun::Result<ok_t, error_t> {
      if (e.first == -1) { return fun::ok(100.); }
      else               { return fun::err(e);   }
    }
  );

  ASSERT_TRUE(last.is_ok());
  ASSERT_EQ(last.clone().unwrap(), 100.);
}

//------------------------------------------------------------------------------
TEST(ResultTest, into_option) {
  using std::string;
  using std::unique_ptr;

  const auto a = fun::ok<string>(example_unique_one()).ok();
  const auto n = a.as_ref().map([](const unique_ptr<int>& p) -> int { return *p; })
                  .unwrap_or(0);
  ASSERT_EQ(n, 1);

  const auto b = fun::err<string>(example_unique_one()).err();
  const auto m = b.as_ref().map([](const unique_ptr<int>& p) -> int { return *p; })
                  .unwrap_or(0);
  ASSERT_EQ(m, 1);
}

//------------------------------------------------------------------------------
TEST(ResultTest, variant_type_equality) {
  const fun::Result<int, std::string> x = fun::ok(3);
  ASSERT_TRUE(x == fun::ok(3));
  ASSERT_TRUE(fun::ok(3) == x);
  ASSERT_TRUE(x != fun::err(std::string()));
  ASSERT_TRUE(fun::err(std::string()) != x);
}

//------------------------------------------------------------------------------
TEST(ResultTest, reference_mapping_is_copy_and_move_free) {
  using std::vector;

  Monolith obj(1);

  const auto n =
    fun::ok<std::string>(1)
    .map([&](auto&&) -> Monolith& { return obj; });

  ASSERT_TRUE(n.is_ok());
}

//------------------------------------------------------------------------------
TEST(ResultTest, result_reference_convertion_into_option) {
  const auto msg = std::string("DEADBEEF");

  const auto bad_result = fun::err<int>(msg);
  ASSERT_TRUE(bad_result.is_err());

  ASSERT_EQ(bad_result.as_ref().err().unwrap(), msg);

  Monolith obj(1);

  const auto ok_result = fun::ok_ref<int>(obj);
  ASSERT_TRUE(ok_result.is_ok());

  ASSERT_EQ(ok_result.as_ref().ok().unwrap(), obj);
}

//------------------------------------------------------------------------------
TEST(TryTest, try_declare_option) {
  const auto int_to_float =
    [](fun::Option<int> opt) -> fun::Option<float> {
      FUN_TRY_DECLARE(int_val, std::move(opt));
      return fun::make_some(int_val);
    };

  EXPECT_EQ(int_to_float(fun::some(3)), fun::some(3.0f));
  EXPECT_EQ(int_to_float(fun::Option<int>()), fun::Option<float>());
}

//------------------------------------------------------------------------------
TEST(TryTest, try_assign_option) {
  const auto int_to_float =
    [](fun::Option<int> opt) -> fun::Option<float> {
      auto int_val = 0;
      FUN_TRY_ASSIGN(int_val, std::move(opt));
      return fun::make_some(int_val);
    };

  EXPECT_EQ(int_to_float(fun::some(3)), fun::some(3.0f));
  EXPECT_EQ(int_to_float(fun::Option<int>()), fun::Option<float>());
}

//------------------------------------------------------------------------------
TEST(TryTest, try_declare_result) {
  const auto int_to_float =
    [](fun::Result<int, std::string> res) -> fun::Result<float, std::string> {
      FUN_TRY_DECLARE(int_val, std::move(res));
      return fun::make_ok(int_val);
    };

  EXPECT_EQ(int_to_float(fun::make_ok(3)), fun::ok<std::string>(3.f));
  EXPECT_EQ(int_to_float(fun::make_err("error")), fun::err<float>(std::string("error")));
}

//------------------------------------------------------------------------------
TEST(TryTest, try_assign_result) {
  const auto int_to_float =
    [](fun::Result<int, std::string> res) -> fun::Result<float, std::string> {
      auto int_val = 0;
      FUN_TRY_ASSIGN(int_val, std::move(res));
      return fun::make_ok(int_val);
    };

  EXPECT_EQ(int_to_float(fun::make_ok(3)), fun::ok<std::string>(3.f));
  EXPECT_EQ(int_to_float(fun::make_err("error")), fun::err<float>(std::string("error")));
}

//------------------------------------------------------------------------------
TEST(LayoutTest, option_sizes) {
  EXPECT_EQ(sizeof(fun::Option<fun::Unit>), 1);
  EXPECT_EQ(sizeof(fun::Option<std::is_empty<void>>), 1);
  EXPECT_EQ(sizeof(fun::Option<std::uint8_t>),  sizeof(std::pair<std::uint8_t, std::uint8_t>));
  EXPECT_EQ(sizeof(fun::Option<std::uint16_t>), sizeof(std::pair<std::uint8_t, std::uint16_t>));
  EXPECT_EQ(sizeof(fun::Option<std::uint32_t>), sizeof(std::pair<std::uint8_t, std::uint32_t>));
  EXPECT_EQ(sizeof(fun::Option<std::uint64_t>), sizeof(std::pair<std::uint8_t, std::uint64_t>));
}

//------------------------------------------------------------------------------
class Foo {
public:
  Foo() = delete;
  Foo(int, double) {}
};

auto foobar(int n) -> fun::Result<fun::Option<Foo>, int> {
  const double x = 2 * n;
  // return MAKE_OK(MAKE_SOME(2 * n));
  return { fun::OkTag{}, fun::ForwardArgs{}, fun::ForwardArgs{}, n, x };
}

auto good_int(const int n) -> fun::Result<Foo, int>
{
  const auto x = 2. * static_cast<double>(n);
  if (n % 2 == 0) {
    return fun::make_ok(n, x);
  }
  else {
    // return fun::err(n);
    return fun::make_err(n);
  }
}

auto even_baby(const int n) -> fun::Option<CryBaby> {
  if (n % 2 == 0) { return fun::make_some(); }
  else            { return {}; }
}

int main(int nargs, char** vargs) {
  ::testing::InitGoogleTest(&nargs, vargs);
  const auto gtest_return_code = RUN_ALL_TESTS();

  const auto safe_cstr = [](std::string s) -> fun::Option<std::string> {
    if (s.empty()) { return {}; }
    else           { return fun::some(std::move(s)); }
  };
  const auto small_str = [](std::string s) -> fun::Option<std::string> {
    if (s.size() < 3) { return {}; }
    else              { return fun::some(std::move(s)); }
  };
  const auto y = fun::pipe( fun::some(std::string("345"))
                          , fun::bind(safe_cstr)
                          , fun::bind(small_str)
                          ).unwrap_or(std::string("failure"));
  std::cout << y << std::endl;

  // const auto y_ = fun::compose(
  //   fun::bind_op(safe_cstr),
  //   fun::bind_op(small_str),
  //   [](auto&& op) { return std::forward<decltype(op)>(op).unwrap_or(std::string("failure")); }
  // );
  // std::cout << y_(fun::some(std::string("345*"))) << std::endl;

  const auto x = fun::make_ok(y, 6., "hal:");
  const auto baby = even_baby(nargs * 2);
  // baby.as_ref().map([](const auto& b) { b.cry(); });
  // const auto baby2 = baby;
  // baby2.as_ref().map([](const auto& b) { b.cry(); });

  std::cout << "finished" << std::endl;

  return gtest_return_code;
}
