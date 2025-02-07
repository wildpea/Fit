#include <fit/construct.h>
#include "test.h"

#include <fit/conditional.h>
#include <fit/by.h>
#include <fit/placeholders.h>

#include <tuple>
#include <type_traits>
#include <vector>

template<class T>
struct ac
{
    T value;
    constexpr ac(T i) : value(i)
    {}
};

template<class... Ts>
struct tuple_meta
{
    typedef std::tuple<Ts...> type;
};

struct tuple_meta_class
{
    template<class... Ts>
    struct apply
    {
        typedef std::tuple<Ts...> type;
    };
};

FIT_TEST_CASE()
{
    auto v = fit::construct<std::vector<int>>()(5, 5);
    FIT_TEST_CHECK(v.size() == 5);
    FIT_TEST_CHECK(v == std::vector<int>{5, 5, 5, 5, 5});
}

FIT_TEST_CASE()
{
    auto make = fit::construct<std::vector<int>>().by(fit::_1 * fit::_1);
    auto v = make(3, 3);
    FIT_TEST_CHECK(v.size() == 9);
    FIT_TEST_CHECK(v == std::vector<int>{9, 9, 9, 9, 9, 9, 9, 9, 9});
}

FIT_TEST_CASE()
{
    auto v = fit::construct<std::vector<int>>()({5, 5, 5, 5, 5});
    FIT_TEST_CHECK(v.size() == 5);
    FIT_TEST_CHECK(v == std::vector<int>{5, 5, 5, 5, 5});
}

FIT_TEST_CASE()
{
    auto t = fit::construct<std::tuple>()(1, 2, 3);
    static_assert(std::is_same<std::tuple<int, int, int>, decltype(t)>::value, "");
    FIT_TEST_CHECK(t == std::make_tuple(1, 2, 3));
// GCC 4.7 doesn't have fully constexpr tuple
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ > 7
    FIT_STATIC_TEST_CHECK(std::make_tuple(1, 2, 3) == fit::construct<std::tuple>()(1, 2, 3));
#endif
}

FIT_TEST_CASE()
{
    auto t = fit::construct<std::pair>()(1, 2);
    static_assert(std::is_same<std::pair<int, int>, decltype(t)>::value, "");
    FIT_TEST_CHECK(t == std::make_pair(1, 2));
// GCC 4.7 doesn't have fully constexpr pair
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ > 7
    FIT_STATIC_TEST_CHECK(std::make_pair(1, 2) == fit::construct<std::pair>()(1, 2));
#endif
}

FIT_TEST_CASE()
{
    auto make = fit::construct<std::tuple>().by(fit::_1 * fit::_1);
    auto t = make(1, 2, 3);
    static_assert(std::is_same<std::tuple<int, int, int>, decltype(t)>::value, "");
    FIT_TEST_CHECK(t == std::make_tuple(1, 4, 9));
}

FIT_TEST_CASE()
{
    auto f = fit::conditional(fit::construct<std::pair>(), fit::identity);
    FIT_TEST_CHECK(f(1, 2) == std::make_pair(1, 2));
    FIT_TEST_CHECK(f(1) == 1);
}

FIT_TEST_CASE()
{
    auto x = fit::construct<ac>()(1);
    static_assert(std::is_same<ac<int>, decltype(x)>::value, "");
    FIT_TEST_CHECK(x.value == ac<int>(1).value);
    FIT_STATIC_TEST_CHECK(ac<int>(1).value == fit::construct<ac>()(1).value);
}

FIT_TEST_CASE()
{
    auto t = fit::construct_meta<tuple_meta>()(1, 2, 3);
    static_assert(std::is_same<std::tuple<int, int, int>, decltype(t)>::value, "");
    FIT_TEST_CHECK(t == std::make_tuple(1, 2, 3));
// GCC 4.7 doesn't have fully constexpr tuple
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ > 7
    FIT_STATIC_TEST_CHECK(std::make_tuple(1, 2, 3) == fit::construct_meta<tuple_meta>()(1, 2, 3));
#endif
}

FIT_TEST_CASE()
{
    auto t = fit::construct_meta<tuple_meta_class>()(1, 2, 3);
    static_assert(std::is_same<std::tuple<int, int, int>, decltype(t)>::value, "");
    FIT_TEST_CHECK(t == std::make_tuple(1, 2, 3));
// GCC 4.7 doesn't have fully constexpr tuple
#if defined(__GNUC__) && !defined (__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ > 7
    FIT_STATIC_TEST_CHECK(std::make_tuple(1, 2, 3) == fit::construct_meta<tuple_meta_class>()(1, 2, 3));
#endif
}

FIT_TEST_CASE()
{
    auto make = fit::construct_meta<tuple_meta>().by(fit::_1 * fit::_1);
    auto t = make(1, 2, 3);
    static_assert(std::is_same<std::tuple<int, int, int>, decltype(t)>::value, "");
    FIT_TEST_CHECK(t == std::make_tuple(1, 4, 9));
}

FIT_TEST_CASE()
{
    auto make = fit::construct_meta<tuple_meta_class>().by(fit::_1 * fit::_1);
    auto t = make(1, 2, 3);
    static_assert(std::is_same<std::tuple<int, int, int>, decltype(t)>::value, "");
    FIT_TEST_CHECK(t == std::make_tuple(1, 4, 9));
}

