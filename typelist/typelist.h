#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace tl {

template <typename... Types>
struct type_list {};

// Identity
template <typename Type>
struct identity {
  using type = Type;
};

template <
    template <typename...>
    typename List1,
    template <typename...>
    typename List2,
    typename... Types1,
    typename... Types2>
identity<List1<Types1..., Types2...>> operator+(identity<List1<Types1...>>, identity<List2<Types2...>>);

// Swap
template <typename List>
struct swap_impl;

template <typename A, typename B, template <typename...> typename Pair>
struct swap_impl<Pair<A, B>> {
  using type = Pair<B, A>;
};

template <typename List>
using swap = typename swap_impl<List>::type;

// Flip
template <typename List>
struct flip_impl;

template <template <typename...> typename List, typename... Types>
struct flip_impl<List<Types...>> {
  using type = List<swap<Types>...>;
};

template <typename List>
using flip = typename flip_impl<List>::type;

// Index
template <std::size_t N>
using index = std::integral_constant<std::size_t, N>;

// Map Find
template <typename... Types>
struct inherit : Types... {};

template <typename Key, typename Map>
struct map_find_impl;

template <typename Key, template <typename...> typename List, typename... Lists>
struct map_find_impl<Key, List<Lists...>> {
  template <template <typename...> typename Result, typename... Rest>
  static Result<Key, Rest...> f(identity<Result<Key, Rest...>>*);

  using derived = inherit<identity<Lists>...>;

  using type = decltype(f(static_cast<derived*>(nullptr)));
};

template <typename Key, typename Map>
using map_find = typename map_find_impl<Key, Map>::type;

// Second
template <typename List>
struct second_impl;

template <typename First, typename Second, typename... Rest, template <typename...> typename List>
struct second_impl<List<First, Second, Rest...>> {
  using type = Second;
};

template <typename List>
using second = typename second_impl<List>::type;

// Count
template <typename List>
struct count_impl;

template <template <typename...> typename List, typename... Types>
struct count_impl<List<Types...>> {
  static constexpr std::size_t value = sizeof...(Types);
};

template <typename List>
inline constexpr std::size_t count = count_impl<List>::value;

// Enumerate
template <typename List, typename Seq>
struct enumerate_impl;

template <template <typename...> typename List, typename... Types, std::size_t... Idxs>
struct enumerate_impl<List<Types...>, std::index_sequence<Idxs...>> {
  using type = List<List<index<Idxs>, Types>...>;
};

template <typename List>
using enumerate = typename enumerate_impl<List, std::make_index_sequence<count<List>>>::type;

// Concat fast
template <typename... Lists>
using concat = typename decltype((identity<Lists>{} + ... + identity<type_list<>>{}))::type;

// Apply
template <template <typename...> typename F, typename List>
struct apply_impl;

template <template <typename...> typename F, template <typename...> typename List, typename... Types>
struct apply_impl<F, List<Types...>> {
  using type = F<Types...>;
};

template <template <typename...> typename F, typename List>
using apply = typename apply_impl<F, List>::type;

// Flatten
template <typename Type>
struct flatten_impl {
  using type = type_list<Type>;
};

template <typename List>
using flatten = typename flatten_impl<List>::type;

template <template <typename...> typename List, typename... Types>
struct flatten_impl<List<Types...>> {
  using type = apply<List, concat<flatten<Types>...>>;
};

// Conditional
template <bool B>
struct conditional_impl;

template <>
struct conditional_impl<true> {
  template <typename T, typename F>
  using type = T;
};

template <>
struct conditional_impl<false> {
  template <typename T, typename F>
  using type = F;
};

template <bool B, typename T, typename F>
using conditional = typename conditional_impl<B>::template type<T, F>;

// Filter
template <template <typename...> typename Pred, typename List>
struct filter_impl;

template <template <typename...> typename Pred, typename List>
using filter = typename filter_impl<Pred, List>::type;

template <template <typename...> typename Pred, typename... Types, template <typename...> typename List>
struct filter_impl<Pred, List<Types...>> {
  template <typename Type>
  using single = conditional<Pred<Type>::value, List<Type>, List<>>;

  using type = concat<single<Types>...>;
};

template <template <typename...> typename Pred, template <typename...> typename List>
struct filter_impl<Pred, List<>> {
  using type = List<>;
};

// Contains
template <typename Type, typename List>
struct contains_impl;

template <template <typename...> typename List, typename... Types, typename Type>
struct contains_impl<Type, List<Types...>> {
  template <typename Val>
  using typed_is_same = std::is_same<Type, Val>;

  static constexpr bool value = count<filter<typed_is_same, List<Types...>>> > 0;
};

template <typename Type, typename List>
constexpr bool contains = contains_impl<Type, List>::value;

// Get value from pair
template <typename Pair>
struct get_index_impl;

template <template <typename...> typename Pair, std::size_t N, typename Val>
struct get_index_impl<Pair<index<N>, Val>> {
  static constexpr std::size_t value = N;
};

template <typename Pair>
constexpr std::size_t get_index = get_index_impl<Pair>::value;

// Transform
template <template <typename...> typename F, typename List>
struct transform_impl;

template <template <typename...> typename F, typename... Types, template <typename...> typename List>
struct transform_impl<F, List<Types...>> {
  using type = List<F<Types>...>;
};

template <template <typename...> typename F, typename List>
using transform = typename transform_impl<F, List>::type;

// Slice
template <typename List, std::size_t Start, std::size_t Len>
struct slice_impl {
  template <typename Pair>
  struct comp {
    static constexpr bool value = Start <= get_index<Pair> && get_index<Pair> < Start + Len;
  };

  using type = transform<second, filter<comp, enumerate<List>>>;
};

template <typename List, std::size_t Start, std::size_t End>
using slice = typename slice_impl<List, Start, End>::type;

// Merge
template <typename List1, typename List2, template <typename...> typename Compare>
struct merge_impl;

template <bool B, typename List1, typename List2, template <typename...> typename Compare>
struct s;

template <
    template <typename...>
    typename List,
    typename First1,
    typename... Rest1,
    typename First2,
    typename... Rest2,
    template <typename...>
    typename Compare>
struct s<true, List<First1, Rest1...>, List<First2, Rest2...>, Compare> {
  using res = concat<List<First1>, typename merge_impl<List<Rest1...>, List<First2, Rest2...>, Compare>::type>;
};

template <
    template <typename...>
    typename List,
    typename First1,
    typename... Rest1,
    typename First2,
    typename... Rest2,
    template <typename...>
    typename Compare>
struct s<false, List<First1, Rest1...>, List<First2, Rest2...>, Compare> {
  using res = concat<List<First2>, typename merge_impl<List<First1, Rest1...>, List<Rest2...>, Compare>::type>;
};

template <
    template <typename...>
    typename List,
    typename First1,
    typename... Rest1,
    typename First2,
    typename... Rest2,
    template <typename...>
    typename Compare>
struct merge_impl<List<First1, Rest1...>, List<First2, Rest2...>, Compare> {
  static constexpr bool comp = Compare<First1, First2>::value;
  using type = typename s<comp, List<First1, Rest1...>, List<First2, Rest2...>, Compare>::res;
};

template <template <typename...> typename List, typename... Types, template <typename...> typename Compare>
struct merge_impl<List<Types...>, List<>, Compare> {
  using type = List<Types...>;
};

template <template <typename...> typename List, typename... Types, template <typename...> typename Compare>
struct merge_impl<List<>, List<Types...>, Compare> {
  using type = List<Types...>;
};

template <template <typename...> typename List, template <typename...> typename Compare>
struct merge_impl<List<>, List<>, Compare> {
  using type = List<>;
};

template <typename List1, typename List2, template <typename...> typename Compare>
using merge = typename merge_impl<List1, List2, Compare>::type;

// Merge-Sort
template <typename List, template <typename...> typename Compare>
struct merge_sort_impl;

template <typename List, template <typename...> typename Compare>
using merge_sort = typename merge_sort_impl<List, Compare>::type;

template <template <typename...> typename List, typename... Types, template <typename...> typename Compare>
struct merge_sort_impl<List<Types...>, Compare> {
  static constexpr std::size_t len1 = count<List<Types...>> / 2;
  static constexpr std::size_t len2 = count<List<Types...>> - len1;

  using sorted1 = merge_sort<slice<List<Types...>, 0, len1>, Compare>;
  using sorted2 = merge_sort<slice<List<Types...>, len1, len2>, Compare>;

  using type = merge<sorted1, sorted2, Compare>;
};

template <template <typename...> typename List, template <typename...> typename Compare, typename A>
struct merge_sort_impl<List<A>, Compare> {
  using type = List<A>;
};

template <template <typename...> typename List, template <typename...> typename Compare, typename A, typename B>
struct merge_sort_impl<List<A, B>, Compare> {
  using type = conditional<Compare<A, B>::value, List<A, B>, List<B, A>>;
};

template <template <typename...> typename List, template <typename...> typename Compare>
struct merge_sort_impl<List<>, Compare> {
  using type = List<>;
};

// Index of unique
template <typename Type, typename List>
constexpr std::size_t index_of_unique = get_index<swap<map_find<Type, flip<enumerate<List>>>>>;
} // namespace tl
