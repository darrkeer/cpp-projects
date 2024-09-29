#pragma once

#include <cassert>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <utility>

template <typename T>
class circular_buffer {
public:
  template <typename R>
  class circular_iterator {
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    using reference = R&;
    using pointer = R*;

    circular_iterator() = default;

    pointer data() const noexcept {
      return _data;
    }

    size_t offset() const noexcept {
      return _offset;
    }

    size_t capacity() const noexcept {
      return _capacity;
    }

    operator circular_iterator<const R>() const noexcept {
      return circular_iterator<const R>(_data, _offset, _capacity);
    }

    reference operator*() const {
      return *(operator->());
    }

    pointer operator->() const {
      if (offset() >= capacity()) {
        return data() + offset() - capacity();
      } else {
        return data() + offset();
      }
    }

    circular_iterator& operator++() {
      ++_offset;
      return *this;
    }

    circular_iterator operator++(int) {
      circular_iterator res(*this);
      ++(*this);
      return res;
    }

    circular_iterator& operator--() {
      --_offset;
      return *this;
    }

    circular_iterator operator--(int) {
      circular_iterator res(*this);
      --(*this);
      return res;
    }

    circular_iterator& operator+=(const difference_type& other) {
      _offset += other;
      return *this;
    }

    circular_iterator& operator-=(const difference_type& other) {
      *this += -other;
      return *this;
    }

    friend circular_iterator operator+(const circular_iterator& left, const difference_type& right) {
      circular_iterator res = left;
      res += right;
      return res;
    }

    friend circular_iterator operator+(const difference_type& left, const circular_iterator& right) {
      return right + left;
    }

    friend circular_iterator operator-(const circular_iterator& left, const difference_type& right) {
      return left + -right;
    }

    friend difference_type operator-(const circular_iterator& left, const circular_iterator& right) {
      return left.offset() - right.offset();
    }

    reference operator[](const difference_type& other) const {
      return *(*this + other);
    }

    friend bool operator<(const circular_iterator& lhs, const circular_iterator& rhs) {
      return lhs.offset() < rhs.offset();
    }

    friend bool operator==(const circular_iterator& lhs, const circular_iterator& rhs) {
      return lhs.data() == rhs.data() && lhs.offset() == rhs.offset();
    }

    friend bool operator!=(const circular_iterator& lhs, const circular_iterator& rhs) {
      return !(lhs == rhs);
    }

    friend bool operator<=(const circular_iterator& lhs, const circular_iterator& rhs) {
      return lhs < rhs || lhs == rhs;
    }

    friend bool operator>(const circular_iterator& lhs, const circular_iterator& rhs) {
      return !(lhs <= rhs);
    }

    friend bool operator>=(const circular_iterator& lhs, const circular_iterator& rhs) {
      return !(lhs < rhs);
    }

    friend circular_iterator<std::remove_const_t<R>>;
    friend circular_buffer;

  private:
    pointer _data;
    size_t _offset;
    size_t _capacity;

    circular_iterator(pointer data, size_t offset, size_t capacity)
        : _data(data)
        , _offset(offset)
        , _capacity(capacity) {}
  };

  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = circular_iterator<T>;
  using const_iterator = circular_iterator<const T>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:
  // O(1), nothrow
  circular_buffer() noexcept
      : _offset(0)
      , _size(0)
      , _capacity(0)
      , _data(nullptr) {}

  // O(n), strong
  circular_buffer(const circular_buffer& other)
      : _offset(0) // maybe change to _offset(other.offset()), but its faster
      , _size(other._size)
      , _capacity(other.capacity())
      , _data(static_cast<T*>(operator new(sizeof(T) * capacity()))) {
    try {
      std::uninitialized_copy(other.begin(), other.end(), begin());
    } catch (...) {
      operator delete(_data);
      throw;
    }
  }

  // O(1), nothrow
  circular_buffer(circular_buffer&& other) noexcept
      : _offset(other.offset())
      , _size(other.size())
      , _capacity(other.capacity())
      , _data(std::move(other._data)) {
    other._data = nullptr;
    other._size = other._capacity = other._offset = 0;
  }

  // O(n), strong
  circular_buffer& operator=(const circular_buffer& other) {
    if (&other != this) {
      circular_buffer temp(other);
      swap(temp);
    }
    return *this;
  }

  // O(1), nothrow
  circular_buffer& operator=(circular_buffer&& other) {
    swap(other);
    return *this;
  }

  // O(n), nothrow
  ~circular_buffer() {
    std::destroy(begin(), end());
    operator delete(data());
  }

  // O(1), nothrow
  size_t size() const noexcept {
    return _size;
  }

  size_t offset() const noexcept {
    return _offset;
  }

  // O(1), nothrow
  bool empty() const noexcept {
    return size() == 0;
  }

  // O(1), nothrow
  size_t capacity() const noexcept {
    return _capacity;
  }

  pointer data() noexcept {
    return _data;
  }

  const_pointer data() const noexcept {
    return _data;
  }

  // O(1), nothrow
  iterator begin() noexcept {
    return {data(), offset(), capacity()};
  }

  // O(1), nothrow
  const_iterator begin() const noexcept {
    return {data(), offset(), capacity()};
  }

  // O(1), nothrow
  iterator end() noexcept {
    return {data(), offset() + size(), capacity()};
  }

  // O(1), nothrow
  const_iterator end() const noexcept {
    return {data(), offset() + size(), capacity()};
  }

  // O(1), nothrow
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(begin() + size());
  }

  // O(1), nothrow
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(begin() + size());
  }

  // O(1), nothrow
  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  // O(1), nothrow
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  // O(1), nothrow
  T& operator[](size_t index) {
    return *(begin() + index);
  }

  // O(1), nothrow
  const T& operator[](size_t index) const {
    return *(begin() + index);
  }

  // O(1), nothrow
  T& back() {
    return *(end() - 1);
  }

  // O(1), nothrow
  const T& back() const {
    return *(end() - 1);
  }

  // O(1), nothrow
  T& front() {
    return *begin();
  }

  // O(1), nothrow
  const T& front() const {
    return *begin();
  }

  // O(1), strong
  void push_back(const T& val) {
    insert(end(), val);
  }

  // O(1), strong
  void push_back(T&& val) {
    insert(end(), std::move(val));
  }

  // O(1), strong
  void push_front(const T& val) {
    insert(begin(), val);
  }

  // O(1), strong
  void push_front(T&& val) {
    insert(begin(), std::move(val));
  }

  // O(1), nothrow
  void pop_back() {
    (*this)[size() - 1].~T();
    --_size;
  }

  // O(1), nothrow
  void pop_front() {
    (*this)[0].~T();
    --_size;
    _offset = offset() == capacity() - 1 ? 0 : offset() + 1;
  }

  // O(n), strong
  void reserve(size_t desired_capacity) {
    if (desired_capacity <= capacity()) {
      return;
    }
    circular_buffer tmp(*this, desired_capacity);
    swap(tmp);
  }

  // O(n), strong
  iterator insert(const_iterator pos, const T& val) {
    size_t pref = pos - begin();
    size_t suff = end() - pos;
    if (size() == capacity()) {
      circular_buffer tmp;
      tmp.reserve(2 * capacity() + 1);

      // push_front
      new (tmp._data + pref) T(val);
      ++tmp._size;
      tmp._offset = pref;

      for (size_t i = pref; i > 0; --i) {
        push_front_in(tmp, std::move((*this)[i - 1]));
      }
      for (size_t i = pref; i < size(); ++i) {
        push_back_in(tmp, std::move((*this)[i]));
      }

      swap(tmp);
    } else {
      if (pref < suff) {
        push_front_in(*this, val);

        size_t next = 1;
        while (next <= pref) {
          std::swap((*this)[next - 1], (*this)[next]);
          ++next;
        }
      } else {
        push_back_in(*this, val);

        size_t next = size() - 1;
        while (next > pref) {
          std::swap((*this)[next - 1], (*this)[next]);
          --next;
        }
      }
    }
    return begin() + pref;
  }

  // O(n), strong
  iterator insert(const_iterator pos, T&& val) {
    size_t pref = pos - begin();
    size_t suff = end() - pos;
    if (size() == capacity()) {
      circular_buffer tmp;
      tmp.reserve(2 * capacity() + 1);

      new (tmp._data + pref) T(std::move(val));
      ++tmp._size;
      tmp._offset = pref;

      for (size_t i = pref; i > 0; --i) {
        push_front_in(tmp, std::move((*this)[i - 1]));
      }
      for (size_t i = pref; i < size(); ++i) {
        push_back_in(tmp, std::move((*this)[i]));
      }

      swap(tmp);
    } else {
      if (pref < suff) {
        push_front_in(*this, std::move(val));

        size_t next = 1;
        while (next <= pref) {
          std::swap((*this)[next - 1], (*this)[next]);
          ++next;
        }
      } else {
        push_back_in(*this, std::move(val));

        size_t next = size() - 1;
        while (next > pref) {
          std::swap((*this)[next - 1], (*this)[next]);
          --next;
        }
      }
    }
    return begin() + pref;
  }

  // O(n), nothrow
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  // O(n), nothrow
  iterator erase(const_iterator first, const_iterator last) {
    size_t pref = first - begin();
    size_t suff = end() - last;
    size_t len = last - first;
    if (pref > suff) {
      for (size_t i = 0; i < suff; ++i) {
        std::swap((*this)[pref + i], (*this)[pref + i + len]);
      }
      for (size_t i = 0; i < len; ++i) {
        pop_back();
      }
    } else {
      for (size_t i = pref; i > 0; --i) {
        std::swap((*this)[i - 1], (*this)[len + i - 1]);
      }
      for (size_t i = 0; i < len; ++i) {
        pop_front();
      }
    }
    return begin() + pref;
  }

  // O(n), nothrow
  void clear() noexcept {
    std::destroy(begin(), end());
    _size = 0;
  }

  // O(1), nothrow
  void swap(circular_buffer& other) noexcept {
    std::swap(_offset, other._offset);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_data, other._data);
  }

  friend void swap(circular_buffer& lhs, circular_buffer& rhs) {
    lhs.swap(rhs);
  }

private:
  size_t _offset;
  size_t _size;
  size_t _capacity;
  T* _data;

private:
  static void push_back_in(circular_buffer& other, const T& val) {
    size_t ind = other.offset() + other.size();
    if (ind >= other.capacity()) {
      ind -= other.capacity();
    }
    new (other.data() + ind) T(val);
    ++other._size;
  }

  static void push_back_in(circular_buffer& other, T&& val) {
    size_t ind = other.offset() + other.size();
    if (ind >= other.capacity()) {
      ind -= other.capacity();
    }
    new (other.data() + ind) T(std::move(val));
    ++other._size;
  }

  static void push_front_in(circular_buffer& other, const T& val) {
    size_t ind = other.offset() == 0 ? other.capacity() - 1 : other.offset() - 1;
    new (other.data() + ind) T(val);
    other._offset = ind;
    ++other._size;
  }

  static void push_front_in(circular_buffer& other, T&& val) {
    size_t ind = other.offset() == 0 ? other.capacity() - 1 : other.offset() - 1;
    new (other.data() + ind) T(std::move(val));
    other._offset = ind;
    ++other._size;
  }

  circular_buffer(circular_buffer& other, size_t new_capacity)
      : _offset(0)
      , _size(other.size())
      , _capacity(new_capacity)
      , _data(static_cast<T*>(operator new(sizeof(T) * new_capacity))) {
    std::uninitialized_move(other.begin(), other.end(), begin());
  }

  circular_buffer(const circular_buffer& other, size_t new_capacity)
      : _offset(0)
      , _size(other.size())
      , _capacity(new_capacity)
      , _data(static_cast<T*>(operator new(sizeof(T) * new_capacity))) {
    std::uninitialized_copy(other.begin(), other.end(), begin());
  }
};
