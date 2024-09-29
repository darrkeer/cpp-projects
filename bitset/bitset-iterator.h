#pragma once

#include "bitset-reference.h"
#include "bitset-utils.h"

#include <iterator>

template <typename T>
class bitset_iterator {
public:
  using word_type = bitset_utils::word_type;
  using value_type = bool;
  using reference = bitset_reference<T>;
  using const_reference = bitset_reference<const T>;
  using iterator = bitset_iterator<T>;
  using const_iterator = bitset_iterator<const T>;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::random_access_iterator_tag;

  bitset_iterator() = default;

  bitset_iterator(std::size_t offset, T* data)
      : _offset(offset)
      , _data(data) {}

  bitset_iterator(const bitset_iterator& other) = default;

  reference operator*() const {
    return {bitset_utils::offset(_offset), _data + bitset_utils::word_ind(_offset)};
  }

  bitset_iterator& operator=(const bitset_iterator& other) = default;

  operator const_iterator() const {
    return {_offset, _data};
  }

  bitset_iterator& operator++() {
    ++_offset;
    return *this;
  }

  bitset_iterator operator++(int) {
    bitset_iterator res(*this);
    ++(*this);
    return res;
  }

  bitset_iterator& operator--() {
    --_offset;
    return *this;
  }

  bitset_iterator operator--(int) {
    bitset_iterator res(*this);
    --(*this);
    return res;
  }

  bitset_iterator& operator+=(const difference_type& other) {
    _offset += other;
    return *this;
  }

  bitset_iterator& operator-=(const difference_type& other) {
    *this += -other;
    return *this;
  }

  friend bitset_iterator operator+(const bitset_iterator& left, const difference_type& right) {
    bitset_iterator res = left;
    res += right;
    return res;
  }

  friend bitset_iterator operator+(const difference_type& left, const bitset_iterator& right) {
    return right + left;
  }

  friend bitset_iterator operator-(const bitset_iterator& left, const difference_type& right) {
    return left + -right;
  }

  friend difference_type operator-(const bitset_iterator& left, const bitset_iterator& right) {
    return left.offset() - right.offset();
  }

  reference operator[](const difference_type& other) const {
    return *(*this + other);
  }

  friend bool operator<(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return lhs.offset() < rhs.offset();
  }

  friend bool operator==(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return lhs.data() == rhs.data() && lhs.offset() == rhs.offset();
  }

  friend bool operator!=(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<=(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return lhs < rhs || lhs == rhs;
  }

  friend bool operator>(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return !(lhs <= rhs);
  }

  friend bool operator>=(const bitset_iterator& lhs, const bitset_iterator& rhs) {
    return !(lhs < rhs);
  }

  std::size_t offset() const {
    return _offset;
  }

  T* data() const {
    return _data;
  }

private:
  std::size_t _offset;
  T* _data;
};
