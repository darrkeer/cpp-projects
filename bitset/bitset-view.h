#pragma once

#include "bitset-iterator.h"
#include "bitset-utils.h"

#include <bit>
#include <functional>
#include <string>

template <typename T>
class bitset_view {
public:
  using word_type = bitset_utils::word_type;
  using value_type = bool;
  using reference = bitset_reference<T>;
  using const_reference = bitset_reference<const T>;
  using iterator = bitset_iterator<T>;
  using const_iterator = bitset_iterator<const T>;
  using view = bitset_view<T>;
  using const_view = bitset_view<const T>;

  bitset_view() = default;

  bitset_view(iterator first, iterator last)
      : _l_offset(first.offset())
      , _r_offset(last.offset())
      , _data(first.data()) {}

  bitset_view(std::size_t l_offset, std::size_t r_offset, T* data)
      : _l_offset(l_offset)
      , _r_offset(r_offset)
      , _data(data) {}

  bitset_view(const bitset_view& other) = default;

  bitset_view& operator=(const bitset_view& other) = default;

  operator const_view() const {
    return {_l_offset, _r_offset, _data};
  }

  iterator begin() const {
    return {_l_offset, _data};
  }

  iterator end() const {
    return {_r_offset, _data};
  }

  reference operator[](std::size_t other) const {
    return begin()[other];
  }

  bool all() const {
    for (std::size_t i = 0; i < words_count(); ++i) {
      std::size_t sz = std::min(bitset_utils::word_size, size() - i * bitset_utils::word_size);
      if (get_nth_word(i) != submask(bitset_utils::mask, 0, sz)) {
        return false;
      }
    }
    return true;
  }

  bool any() const {
    for (std::size_t i = 0; i < words_count(); ++i) {
      if (get_nth_word(i) > 0) {
        return true;
      }
    }
    return false;
  }

  std::size_t count() const {
    std::size_t res = 0;
    for (std::size_t i = 0; i < words_count(); ++i) {
      res += std::popcount(get_nth_word(i));
    }
    return res;
  }

  void swap(bitset_view& other) {
    std::swap(_l_offset, other._l_offset);
    std::swap(_r_offset, other._r_offset);
    std::swap(_data, other._data);
  }

  friend void swap(bitset_view& lhs, bitset_view& rhs) {
    lhs.swap(rhs);
  }

  std::size_t size() const {
    return _r_offset - _l_offset;
  }

  bool empty() const {
    return !size();
  }

  bitset_view subview(std::size_t offset = 0, std::size_t count = bitset_utils::npos) const {
    if (offset >= size()) {
      return {end(), end()};
    } else if (offset + count < size() && offset < size() && count < size()) {
      return {begin() + offset, begin() + offset + count};
    }
    return {begin() + offset, end()};
  }

  bitset_view operator&=(const const_view& other) const
    requires (!std::is_const_v<T>)
  {
    transform(other, std::bit_and<>());
    return *this;
  }

  bitset_view operator|=(const const_view& other) const
    requires (!std::is_const_v<T>)
  {
    transform(other, std::bit_or<>());
    return *this;
  }

  bitset_view operator^=(const const_view& other) const
    requires (!std::is_const_v<T>)
  {
    transform(other, std::bit_xor<>());
    return *this;
  }

  friend std::string to_string(const bitset_view& other) {
    std::string tmp;
    tmp.reserve(other.size());
    for (auto it : other) {
      tmp.push_back('0' + it);
    }
    return tmp;
  }

  bitset_view flip() const
    requires (!std::is_const_v<T>)
  {
    transform(std::bit_not<>());
    return *this;
  }

  bitset_view set() const
    requires (!std::is_const_v<T>)
  {
    fill_words(bitset_utils::mask);
    return *this;
  }

  bitset_view reset() const
    requires (!std::is_const_v<T>)
  {
    fill_words(0);
    return *this;
  }

  std::size_t words_count() const {
    return size() / bitset_utils::word_size + (size() % bitset_utils::word_size > 0);
  }

  word_type get_nth_word(std::size_t ind) const {
    size_t read = std::min(bitset_utils::word_size, size() - ind * bitset_utils::word_size);
    size_t l_wi = bitset_utils::word_ind(_l_offset);
    size_t l = bitset_utils::offset(_l_offset);
    size_t r = l + read;
    if (r <= bitset_utils::word_size) {
      return submask(_data[l_wi + ind], l, r) << l;
    } else {
      r -= bitset_utils::word_size;
      return (submask(_data[l_wi + ind], l, bitset_utils::word_size) << l) |
             (submask(_data[l_wi + ind + 1], 0, r) >> (bitset_utils::word_size - l));
    }
  }

  void set_nth_word(std::size_t ind, word_type word) const
    requires (!std::is_const_v<T>)
  {
    size_t write = std::min(bitset_utils::word_size, size() - ind * bitset_utils::word_size);
    size_t l_wi = bitset_utils::word_ind(_l_offset);
    size_t l = bitset_utils::offset(_l_offset);
    size_t r = l + write;
    if (r <= bitset_utils::word_size) {
      // 1 word
      _data[l_wi + ind] &= ~mask(l, r);
      _data[l_wi + ind] |= submask(word, 0, r - l) >> l;
    } else {
      // 2 words
      r -= bitset_utils::word_size;
      _data[l_wi + ind] &= ~mask(l, bitset_utils::word_size);
      _data[l_wi + ind] |= word >> l;
      word <<= bitset_utils::word_size - l;
      _data[l_wi + ind + 1] &= ~mask(0, r);
      _data[l_wi + ind + 1] |= submask(word, 0, r);
    }
  }

private:
  std::size_t _l_offset;
  std::size_t _r_offset;
  T* _data;

  static word_type mask(std::size_t l, std::size_t r) {
    return (((bitset_utils::mask << l) >> l) >> (bitset_utils::word_size - r) << (bitset_utils::word_size - r));
  }

  static word_type submask(word_type x, std::size_t l, std::size_t r) {
    return x & mask(l, r);
  }

  template <typename F>
  void transform(const const_view& other, F f) const {
    for (std::size_t i = 0; i < words_count(); ++i) {
      set_nth_word(i, f(get_nth_word(i), other.get_nth_word(i)));
    }
  }

  template <typename F>
  void transform(F f) const {
    for (std::size_t i = 0; i < words_count(); ++i) {
      set_nth_word(i, f(get_nth_word(i)));
    }
  }

  void fill_words(word_type val) const {
    for (std::size_t i = 0; i < words_count(); ++i) {
      set_nth_word(i, val);
    }
  }
};
