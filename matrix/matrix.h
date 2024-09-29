#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>

template <class T>
class matrix {
private:
  template <class S>
  struct base_col_iterator {
  public:
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = S&;
    using pointer = S*;
    using iterator_category = std::random_access_iterator_tag;

    base_col_iterator() = default;

    operator base_col_iterator<const S>() const {
      return {_current, _cols, _col};
    }

    reference operator*() const {
      return _current[_col];
    }

    pointer operator->() const {
      return _current + _col;
    }

    base_col_iterator& operator++() {
      _current += _cols;
      return *this;
    }

    base_col_iterator operator++(int) {
      base_col_iterator res = *this;
      ++*this;
      return res;
    }

    base_col_iterator& operator--() {
      _current -= _cols;
      return *this;
    }

    base_col_iterator operator--(int) {
      base_col_iterator res = *this;
      --*this;
      return res;
    }

    base_col_iterator& operator+=(const difference_type& other) {
      _current += other * static_cast<difference_type>(_cols);
      return *this;
    }

    base_col_iterator& operator-=(const difference_type& other) {
      _current -= other * static_cast<difference_type>(_cols);
      return *this;
    }

    friend base_col_iterator operator+(const base_col_iterator& left, const difference_type& right) {
      base_col_iterator res = left;
      res += right;
      return res;
    }

    friend base_col_iterator operator+(const difference_type& left, const base_col_iterator& right) {
      return right + left;
    }

    friend base_col_iterator operator-(const base_col_iterator& left, const difference_type& right) {
      base_col_iterator res = left;
      res -= right;
      return res;
    }

    friend difference_type operator-(const base_col_iterator& left, const base_col_iterator& right) {
      return (left._current - right._current) / static_cast<difference_type>(left._cols);
    }

    reference operator[](difference_type other) const {
      return *(*this + other);
    }

    friend bool operator==(const base_col_iterator& lhs, const base_col_iterator& rhs) {
      return lhs._current == rhs._current;
    }

    friend bool operator!=(const base_col_iterator& lhs, const base_col_iterator& rhs) {
      return !(lhs == rhs);
    }

    friend bool operator<(const base_col_iterator& lhs, const base_col_iterator& rhs) {
      return lhs._current < rhs._current;
    }

    friend bool operator<=(const base_col_iterator& lhs, const base_col_iterator& rhs) {
      return lhs < rhs || lhs == rhs;
    }

    friend bool operator>(const base_col_iterator& lhs, const base_col_iterator& rhs) {
      return !(lhs <= rhs);
    }

    friend bool operator>=(const base_col_iterator& lhs, const base_col_iterator& rhs) {
      return !(lhs < rhs);
    }

  private:
    pointer _current;
    size_t _cols;
    size_t _col;

    friend matrix;

    base_col_iterator(pointer current, size_t cols, size_t col) : _current(current), _cols(cols), _col(col) {}
  };

public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using pointer = T*;
  using const_pointer = const T*;

  using iterator = pointer;
  using const_iterator = const_pointer;

  using row_iterator = pointer;
  using const_row_iterator = const_pointer;

  using col_iterator = base_col_iterator<T>;
  using const_col_iterator = base_col_iterator<const T>;

  matrix() : _rows(0), _cols(0), _data(nullptr) {}

  matrix(size_t rows, size_t cols)
      : _rows(rows * cols == 0 ? 0 : rows),
        _cols(rows * cols == 0 ? 0 : cols),
        _data(empty() ? nullptr : new T[size()]()) {}

  template <size_t Rows, size_t Cols>
  matrix(const T (&init)[Rows][Cols]) : _rows(Rows),
                                        _cols(Cols),
                                        _data(new T[size()]) {
    for (size_t row = 0; row < _rows; ++row) {
      std::copy_n(init[row], _cols, _data + _cols * row);
    }
  }

  matrix(const matrix& other) : _rows(other._rows), _cols(other._cols), _data(empty() ? nullptr : new T[size()]) {
    std::copy(other.begin(), other.end(), begin());
  }

  matrix& operator=(const matrix& other) {
    if (this == &other) {
      return *this;
    }
    matrix copy = other;
    swap(*this, copy);
    return *this;
  }

  ~matrix() {
    delete[] _data;
  }

  friend void swap(matrix& lhs, matrix& rhs) {
    std::swap(lhs._cols, rhs._cols);
    std::swap(lhs._rows, rhs._rows);
    std::swap(lhs._data, rhs._data);
  }

  // Iterators

  iterator begin() {
    return data();
  }

  const_iterator begin() const {
    return data();
  }

  iterator end() {
    return begin() + size();
  }

  const_iterator end() const {
    return begin() + size();
  }

  row_iterator row_begin(size_t row) {
    assert(row < _rows);
    return data() + row * _cols;
  }

  const_row_iterator row_begin(size_t row) const {
    assert(row < _rows);
    return data() + row * _cols;
  }

  row_iterator row_end(size_t row) {
    return row_begin(row) + cols();
  }

  const_row_iterator row_end(size_t row) const {
    return row_begin(row) + cols();
  }

  col_iterator col_begin(size_t col) {
    assert(col < _cols);
    return {begin(), _cols, col};
  }

  const_col_iterator col_begin(size_t col) const {
    assert(col < _cols);
    return {begin(), _cols, col};
  }

  col_iterator col_end(size_t col) {
    return col_begin(col) + _rows;
  }

  const_col_iterator col_end(size_t col) const {
    return col_begin(col) + _rows;
  }

  // Size

  size_t rows() const {
    return _rows;
  }

  size_t cols() const {
    return _cols;
  }

  size_t size() const {
    return _rows * _cols;
  }

  bool empty() const {
    return size() == 0;
  }

  // Elements access

  reference operator()(size_t row, size_t col) {
    assert(row < _rows && col < _cols);
    return _data[row * _cols + col];
  }

  const_reference operator()(size_t row, size_t col) const {
    assert(row < _rows && col < _cols);
    return _data[row * _cols + col];
  }

  pointer data() {
    return _data;
  }

  const_pointer data() const {
    return _data;
  }

  // Comparison

  friend bool operator==(const matrix& left, const matrix& right) {
    if (left.size() == 0 && right.size() == 0) {
      return true;
    }
    if (left._cols == right._cols && left._rows == right._rows) {
      return std::equal(left.begin(), left.end(), right.begin());
    }
    return false;
  }

  friend bool operator!=(const matrix& left, const matrix& right) {
    return !(left == right);
  }

  // Arithmetic operations

  matrix& operator+=(const matrix& other) {
    assert(_cols == other.cols() && _rows == other.rows());
    std::transform(begin(), end(), other.begin(), begin(), std::plus<T>());
    return *this;
  }

  matrix& operator-=(const matrix& other) {
    assert(_cols == other.cols() && _rows == other.rows());
    std::transform(begin(), end(), other.begin(), begin(), std::minus<T>());
    return *this;
  }

  matrix& operator*=(const matrix& other) {
    matrix tmp = *this * other;
    swap(*this, tmp);
    return *this;
  }

  matrix& operator*=(const_reference factor) {
    std::transform(begin(), end(), begin(), [&factor](reference a) { return a * factor; });
    return *this;
  }

  friend matrix operator+(const matrix& left, const matrix& right) {
    matrix res = left;
    res += right;
    return res;
  }

  friend matrix operator-(const matrix& left, const matrix& right) {
    matrix res = left;
    res -= right;
    return res;
  }

  friend matrix operator*(const matrix& left, const matrix& right) {
    assert(left.cols() == right.rows());
    matrix res(left.rows(), right.cols());
    for (size_t row = 0; row < left.rows(); ++row) {
      for (size_t col = 0; col < right.cols(); ++col) {
        res(row, col) = std::inner_product(left.row_begin(row), left.row_end(row), right.col_begin(col), T());
      }
    }
    return res;
  }

  friend matrix operator*(const matrix& left, value_type right) {
    matrix res = left;
    res *= right;
    return res;
  }

  friend matrix operator*(value_type left, const matrix& right) {
    matrix res = right;
    res *= left;
    return res;
  }

private:
  size_t _rows;
  size_t _cols;
  pointer _data;
};
