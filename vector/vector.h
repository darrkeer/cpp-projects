#pragma once

#include <cstddef>
#include <utility>

template <typename T>
class vector {
public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

private:
  size_t _size;
  size_t _capacity;
  pointer _data;

  vector(size_t capacity)
      : _size(0)
      , _capacity(capacity)
      , _data(static_cast<pointer>(operator new(sizeof(T) * capacity))) {}

  static void clear_raw(pointer start, size_t size) noexcept {
    while (size) {
      (start + size-- - 1)->~T();
    }
  }

  vector copy_reserve(size_t new_capacity) {
    vector tmp(new_capacity);
    for (size_t i = 0; i < size(); ++i) {
      tmp.push_back((*this)[i]);
    }
    return tmp;
  }

public:
  // O(1) nothrow
  vector() noexcept
      : _size(0)
      , _capacity(0)
      , _data(nullptr) {}

  // O(N) strong
  vector(const vector& other) {
    if (other.empty()) {
      _size = _capacity = 0;
      _data = nullptr;
      return;
    }
    pointer new_data = static_cast<pointer>(operator new(sizeof(T) * other.size()));
    size_t i = 0;
    try {
      for (; i < other.size(); ++i) {
        new (new_data + i) T(other.data()[i]);
      }
    } catch (...) {
      clear_raw(new_data, i);
      operator delete(new_data);
      throw;
    }
    _size = _capacity = other.size();
    _data = new_data;
  }

  // O(1) strong
  vector(vector&& other)
      : _size(other.size())
      , _capacity(other.capacity())
      , _data(std::move(other._data)) {
    other._data = nullptr;
    other._capacity = other._size = 0;
  }

  // O(N) strong
  vector& operator=(const vector& other) {
    if (&other != this) {
      vector temp(other);
      swap(temp);
    }
    return *this;
  }

  // O(1) strong
  vector& operator=(vector&& other) {
    swap(other);
    return *this;
  }

  // O(N) nothrow
  ~vector() noexcept {
    clear();
    operator delete(_data);
  }

  // O(1) nothrow
  reference operator[](size_t index) {
    return *(data() + index);
  }

  // O(1) nothrow
  const_reference operator[](size_t index) const {
    return *(data() + index);
    ;
  }

  // O(1) nothrow
  pointer data() noexcept {
    return _data;
  }

  // O(1) nothrow
  const_pointer data() const noexcept {
    return _data;
  }

  // O(1) nothrow
  size_t size() const noexcept {
    return _size;
  }

  // O(1) nothrow
  reference front() {
    return *begin();
  }

  // O(1) nothrow
  const_reference front() const {
    return *begin();
  }

  // O(1) nothrow
  reference back() {
    return *(begin() + size() - 1);
  }

  // O(1) nothrow
  const_reference back() const {
    return *(begin() + size() - 1);
  }

  // O(1)* strong
  void push_back(const T& value) {
    if (size() < capacity()) {
      new (_data + size()) T(value);
      ++_size;
      return;
    }
    size_t new_capacity = 2 * capacity() + 1;
    vector tmp = copy_reserve(new_capacity);
    tmp.push_back(value);
    swap(tmp);
  }

  // O(1) nothrow
  void pop_back() {
    (*this)[size() - 1].~T();
    --_size;
  }

  // O(1) nothrow
  bool empty() const noexcept {
    return !size();
  }

  // O(1) nothrow
  size_t capacity() const noexcept {
    return _capacity;
  }

  // O(N) strong
  void reserve(size_t new_capacity) {
    if (new_capacity <= capacity()) {
      return;
    }
    vector tmp = copy_reserve(new_capacity);
    swap(tmp);
  }

  // O(N) strong
  void shrink_to_fit() {
    _data = static_cast<pointer>(operator new(sizeof(T) * size(), _data));
    _capacity = size();
  }

  // O(N) nothrow
  void clear() noexcept {
    clear_raw(begin(), size());
    _size = 0;
  }

  // O(1) nothrow
  void swap(vector& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  // O(1) nothrow
  iterator begin() noexcept {
    return data();
  }

  // O(1) nothrow
  iterator end() noexcept {
    return begin() + size();
  }

  // O(1) nothrow
  const_iterator begin() const noexcept {
    return data();
  }

  // O(1) nothrow
  const_iterator end() const noexcept {
    return begin() + size();
  }

  // O(N) strong
  iterator insert(const_iterator pos, const T& value) {
    size_t new_capacity = size() == capacity() ? 2 * capacity() + 1 : capacity();
    vector tmp = copy_reserve(new_capacity);
    tmp.push_back(value);
    size_t start = pos - begin();
    for (size_t i = start + 1; i < tmp.size(); ++i) {
      std::swap(tmp[start], tmp[i]);
    }
    swap(tmp);
    return begin() + start;
  }

  // O(N) nothrow(swap)
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  // O(N) nothrow(swap)
  iterator erase(const_iterator first, const_iterator last) {
    size_t tail = last - first;
    size_t start = first - begin();
    iterator res = begin() + start;
    while (start + tail != size()) {
      std::swap((*this)[start], (*this)[start + tail]);
      start++;
    }
    while (tail--) {
      pop_back();
    }
    return res;
  }
};
