#pragma once

#include "bitset-utils.h"

#include <cstddef>
#include <cstdint>

template <typename T>
class bitset_reference {
  using word_type = bitset_utils::word_type;

  static constexpr word_type max_bit = 1ULL << (bitset_utils::word_size - 1);

public:
  bitset_reference(std::size_t offset, T* word)
      : _offset(offset)
      , _word(word) {}

  operator bool() const {
    return ((*_word << _offset) & max_bit) > 0;
  }

  operator bitset_reference<const word_type>() const {
    return {_offset, _word};
  }

  bitset_reference operator=(bool x) const
    requires (!std::is_const_v<T>)
  {
    if (*this != x) {
      *_word ^= (1ULL << (bitset_utils::word_size - _offset - 1));
    }
    return *this;
  }

  bitset_reference operator&=(bool other) const
    requires (!std::is_const_v<T>)
  {
    (*this) = *this & other;
    return *this;
  }

  bitset_reference operator|=(bool other) const
    requires (!std::is_const_v<T>)
  {
    (*this) = *this | other;
    return *this;
  }

  bitset_reference operator^=(bool other) const
    requires (!std::is_const_v<T>)
  {
    (*this) = *this ^ other;
    return *this;
  }

  ~bitset_reference() = default;

  void flip() const
    requires (!std::is_const_v<T>)
  {
    *_word ^= (1ULL << (bitset_utils::word_size - _offset - 1));
  }

private:
  std::size_t _offset;
  T* _word;
};
