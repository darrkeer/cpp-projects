#include "typelist.h"

#include <tuple>
#include <type_traits>

// Flip
static_assert(std::is_same_v<tl::swap<tl::type_list<double, int>>, tl::type_list<int, double>>);

static_assert(std::is_same_v<
              tl::flip<tl::type_list<tl::type_list<int, double>, tl::type_list<float, int>>>,
              tl::type_list<tl::type_list<double, int>, tl::type_list<int, float>>>);

// Index of unique
static_assert(tl::index_of_unique<int, tl::type_list<double, int>> == 1);
static_assert(tl::index_of_unique<int, tl::type_list<int, double, float>> == 0);

// Flatten
static_assert(std::is_same_v<
              tl::flatten<tl::type_list<tl::type_list<double>, tl::type_list<tl::type_list<int, long>>>>,
              tl::type_list<double, int, long>>);

static_assert(std::is_same_v<
              tl::flatten<std::tuple<tl::type_list<double, int>, tl::type_list<int, long>>>,
              std::tuple<double, int, int, long>>);

// Contains
static_assert(tl::contains<tl::type_list<int, int>, std::tuple<int, tl::type_list<int, int>>>);

// Get index
static_assert(tl::get_index<tl::type_list<tl::index<2>, int>> == 2);

// Slice
static_assert(std::is_same_v<tl::slice<tl::type_list<int, long, double, float>, 1, 1>, tl::type_list<long>>);

// Types comparing
template <typename A, typename B>
struct compare_impl;

// int < float < double < long
template <>
struct compare_impl<int, int> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<int, float> {
  static constexpr bool value = true;
};

template <>
struct compare_impl<int, double> {
  static constexpr bool value = true;
};

template <>
struct compare_impl<int, long> {
  static constexpr bool value = true;
};

template <>
struct compare_impl<float, int> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<float, float> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<float, double> {
  static constexpr bool value = true;
};

template <>
struct compare_impl<float, long> {
  static constexpr bool value = true;
};

template <>
struct compare_impl<double, int> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<double, float> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<double, double> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<double, long> {
  static constexpr bool value = true;
};

template <>
struct compare_impl<long, int> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<long, float> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<long, double> {
  static constexpr bool value = false;
};

template <>
struct compare_impl<long, long> {
  static constexpr bool value = false;
};

// Merge
static_assert(std::is_same_v<
              tl::merge<tl::type_list<int, long>, tl::type_list<double, long>, compare_impl>,
              tl::type_list<int, double, long, long>>);

static_assert(std::is_same_v<tl::merge<tl::type_list<>, tl::type_list<>, compare_impl>, tl::type_list<>>);

// Merge-sort
static_assert(std::is_same_v<
              tl::merge_sort<tl::type_list<long, double, int, long>, compare_impl>,
              tl::type_list<int, double, long, long>>);
