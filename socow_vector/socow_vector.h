#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

template <typename T, std::size_t SMALL_SIZE>
class socow_vector {
  struct buffer {
    std::size_t capacity;
    std::size_t refs;
    T data[0];

    buffer(std::size_t _capacity)
        : capacity(_capacity)
        , refs(1) {}
  };

public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using iterator = pointer;
  using const_iterator = const_pointer;

  socow_vector() noexcept
      : _size(0)
      , _small(true) {}

  socow_vector(const socow_vector& other)
      : _size(other.size())
      , _small(other.is_small()) {
    if (is_small()) {
      std::uninitialized_copy_n(other.cdata(), size(), cdata());
    } else {
      ++other._buffer->refs;
      _buffer = other._buffer;
    }
  }

  socow_vector(socow_vector&& other) noexcept
      : socow_vector() {
    swap(other);
  }

  socow_vector& operator=(const socow_vector& other) {
    if (&other != this) {
      socow_vector tmp(other);
      clear();
      swap(tmp);
    }
    return *this;
  }

  socow_vector& operator=(socow_vector&& other) noexcept {
    if (&other != this) {
      clear();
      swap(other);
      if (!other.is_small()) {
        operator delete(other._buffer);
        other._small = true;
      }
    }
    return *this;
  }

  void swap(socow_vector& other) noexcept {
    if (this == &other) {
      return;
    }
    socow_vector* lhs = &*this;
    socow_vector* rhs = &other;
    if ((!lhs->is_small() && rhs->is_small()) || (lhs->is_small() == rhs->is_small() && lhs->size() > rhs->size())) {
      std::swap(lhs, rhs);
    }
    // lhs <= rhs
    if (lhs->is_small() && rhs->is_small()) {
      std::swap_ranges(lhs->cdata(), lhs->cdata() + lhs->size(), rhs->cdata());
      std::uninitialized_move_n(rhs->cdata() + lhs->size(), rhs->size() - lhs->size(), lhs->cdata() + lhs->size());
      std::destroy_n(rhs->cdata() + lhs->size(), rhs->size() - lhs->size());
    }
    if (lhs->is_small() && !rhs->is_small()) {
      buffer* tmp = rhs->_buffer;
      std::uninitialized_move_n(lhs->cdata(), lhs->size(), rhs->_sdata);
      std::destroy_n(lhs->cdata(), lhs->size());
      lhs->_buffer = tmp;
    }
    if (!lhs->is_small() && !rhs->is_small()) {
      std::swap(lhs->_buffer, rhs->_buffer);
    }
    std::swap(lhs->_size, rhs->_size);
    std::swap(lhs->_small, rhs->_small);
  }

  ~socow_vector() {
    if (!is_small()) {
      --_buffer->refs;
      if (_buffer->refs == 0) {
        std::destroy_n(cdata(), size());
        operator delete(_buffer);
      }
    } else {
      std::destroy_n(cdata(), size());
    }
  }

  std::size_t size() const noexcept {
    return _size;
  }

  std::size_t capacity() const noexcept {
    return is_small() ? SMALL_SIZE : _buffer->capacity;
  }

  bool empty() const noexcept {
    return size() == 0;
  }

  reference operator[](std::size_t other) {
    unshare();
    return cdata()[other];
  }

  const_reference operator[](std::size_t other) const noexcept {
    return cdata()[other];
  }

  reference front() {
    return *data();
  }

  const_reference front() const noexcept {
    return *data();
  }

  reference back() {
    return *(data() + size() - 1);
  }

  const_reference back() const noexcept {
    return *(data() + size() - 1);
  }

  pointer data() {
    unshare();
    return cdata();
  }

  const_pointer data() const noexcept {
    return cdata();
  }

  iterator begin() {
    return data();
  }

  const_iterator begin() const noexcept {
    return data();
  }

  iterator end() {
    return data() + size();
  }

  const_iterator end() const noexcept {
    return data() + size();
  }

  void push_back(const T& value) {
    if (size() < capacity() && !shared()) {
      new (cdata() + size()) T(value);
      ++_size;
    } else {
      T x = value;
      push_back(std::move(x));
    }
  }

  void push_back(T&& value) {
    if (size() < capacity() && !shared()) {
      new (cdata() + size()) T(std::move(value));
      ++_size;
    } else {
      socow_vector tmp(size() == capacity() ? 2 * capacity() + 1 : capacity());
      if (!shared()) {
        new (tmp.cdata() + size()) T(std::move(value));
        tmp._size = 1;
        std::uninitialized_move_n(cdata(), size(), tmp.cdata());
      } else {
        std::uninitialized_copy_n(cdata(), size(), tmp.cdata());
        tmp._size = size();
        new (tmp.cdata() + size()) T(std::move(value));
      }
      tmp._size = size() + 1;
      clear();
      swap(tmp);
    }
  }

  iterator insert(const_iterator pos, const T& value) {
    T x = value;
    return insert(pos, std::move(x));
  }

  iterator insert(const_iterator pos, T&& value) {
    size_t start = pos - cdata();
    if (size() != capacity() && !shared()) {
      push_back(std::move(value));
      for (size_t i = start + 1; i < size(); ++i) {
        std::swap((*this)[start], (*this)[i]);
      }
    } else {
      socow_vector tmp(size() == capacity() ? 2 * capacity() + 1 : capacity());
      if (shared()) {
        std::uninitialized_copy_n(cdata(), start, tmp.cdata());
        tmp._size = start;
        std::uninitialized_copy_n(cdata() + start, size() - start, tmp.cdata() + start + 1);
      } else {
        std::uninitialized_move_n(cdata(), start, tmp.cdata());
        tmp._size = start;
        std::uninitialized_move_n(cdata() + start, size() - start, tmp.cdata() + start + 1);
      }
      new (tmp.cdata() + start) T(std::move(value));
      tmp._size = size() + 1;
      swap(tmp);
    }
    return cdata() + start;
  }

  void pop_back() {
    if (shared()) {
      socow_vector tmp(capacity());
      for (std::size_t i = 0; i < size() - 1; ++i) {
        tmp.push_back(std::as_const(*this)[i]);
      }
      swap(tmp);
    } else {
      back().~T();
      --_size;
    }
  }

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    size_t tail = last - first;
    size_t start = first - cdata();
    size_t res = start;
    if (shared()) {
      socow_vector tmp(capacity());
      for (std::size_t i = 0; i < start; ++i) {
        tmp.push_back(std::as_const(*this)[i]);
      }
      for (std::size_t i = start + tail; i < size(); ++i) {
        tmp.push_back(std::as_const(*this)[i]);
      }
      swap(tmp);
    } else {
      while (start + tail != size()) {
        std::swap((*this)[start], (*this)[start + tail]);
        start++;
      }
      while (tail--) {
        pop_back();
      }
    }
    return begin() + res;
  }

  void clear() {
    if (shared()) {
      socow_vector tmp;
      swap(tmp);
    } else {
      while (size()) {
        pop_back();
      }
    }
  }

  void reserve(std::size_t new_capacity) {
    if (new_capacity > capacity() || (new_capacity > size() && shared())) {
      expand(new_capacity);
    }
  }

  void shrink_to_fit() {
    if (is_small() || size() == capacity()) {
      return;
    }
    expand(size());
  }

  bool is_small() const noexcept {
    return _small;
  }

private:
  std::size_t _size;
  bool _small;

  union {
    T _sdata[SMALL_SIZE];
    buffer* _buffer;
  };

  pointer cdata() noexcept {
    return is_small() ? _sdata : _buffer->data;
  }

  const_pointer cdata() const noexcept {
    return is_small() ? _sdata : _buffer->data;
  }

  void expand(std::size_t new_capacity) {
    socow_vector tmp(new_capacity);
    for (std::size_t i = 0; i < size(); ++i) {
      if (shared()) {
        tmp.push_back(std::as_const(*this)[i]);
      } else {
        tmp.push_back(std::move((*this)[i]));
      }
    }
    clear();
    swap(tmp);
  }

  socow_vector(std::size_t capacity)
      : _size(0)
      , _small(capacity <= SMALL_SIZE) {
    if (!is_small()) {
      void* buf = operator new(sizeof(buffer) + sizeof(T) * capacity);
      _buffer = new (buf) buffer(capacity);
    }
  }

  bool shared() const noexcept {
    return !is_small() && _buffer->refs > 1;
  }

  void unshare() {
    if (!shared()) {
      return;
    }
    expand(capacity());
  }
};
