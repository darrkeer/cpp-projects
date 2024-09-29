#include "bitset.h"

#include <algorithm>
#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>

bitset::bitset()
    : _size(0)
    , _data(nullptr) {}

bitset::bitset(std::size_t size)
    : _size(size)
    , _data(size > 0 ? new word_type[word_cnt()]() : nullptr) {}

bitset::bitset(std::size_t size, bool value)
    : bitset(size) {
  if (value && size) {
    std::fill(_data, _data + word_cnt(), bitset_utils::npos);
  }
}

bitset::bitset(const bitset& other)
    : bitset(other.size()) {
  std::copy_n(other.data(), word_cnt(), _data);
}

bitset::bitset(std::string_view str)
    : bitset(str.size()) {
  for (int i = 0; i < str.size(); ++i) {
    (*this)[i] = str[i] - '0';
  }
}

bitset::bitset(const bitset::const_view& other)
    : bitset(other.size()) {
  for (std::size_t i = 0; i < word_cnt(); ++i) {
    _data[i] = other.get_nth_word(i);
  }
}

bitset::bitset(bitset::const_iterator first, bitset::const_iterator last)
    : bitset(last - first) {
  auto it = begin();
  while (first != last) {
    *(it++) = *(first++);
  }
}

bitset& bitset::operator=(const bitset& other) & {
  bitset temp = other;
  swap(temp);
  return *this;
}

bitset& bitset::operator=(std::string_view str) & {
  bitset temp(str);
  swap(temp);
  return *this;
}

bitset& bitset::operator=(const bitset::const_view& other) & {
  bitset tmp(other);
  swap(tmp);
  return *this;
}

bitset::~bitset() {
  delete[] _data;
}

void bitset::swap(bitset& other) {
  std::swap(_size, other._size);
  std::swap(_data, other._data);
}

std::size_t bitset::size() const {
  return _size;
}

std::size_t bitset::word_cnt() const {
  return size() / bitset_utils::word_size + (size() % bitset_utils::word_size > 0);
}

bool bitset::empty() const {
  return size() == 0;
}

bitset::pointer bitset::data() {
  return _data;
}

bitset::const_pointer bitset::data() const {
  return _data;
}

bitset::reference bitset::operator[](std::size_t index) {
  return {bitset_utils::offset(index), data() + bitset_utils::word_ind(index)};
}

bitset::const_reference bitset::operator[](std::size_t index) const {
  return {bitset_utils::offset(index), data() + bitset_utils::word_ind(index)};
}

bitset::iterator bitset::begin() {
  return {0, data()};
}

bitset::const_iterator bitset::begin() const {
  return {0, data()};
}

bitset::iterator bitset::end() {
  return {size(), data()};
}

bitset::const_iterator bitset::end() const {
  return {size(), data()};
}

bitset& bitset::operator&=(const bitset::const_view& other) & {
  bitset::view tmp(begin(), end());
  tmp &= other;
  return *this;
}

bitset& bitset::operator|=(const bitset::const_view& other) & {
  bitset::view tmp(begin(), end());
  tmp |= other;
  return *this;
}

bitset& bitset::operator^=(const bitset::const_view& other) & {
  bitset::view tmp(begin(), end());
  tmp ^= other;
  return *this;
}

bitset& bitset::operator<<=(std::size_t count) & {
  bitset tmp(size() + count, false);
  std::copy_n(data(), word_cnt(), tmp.data());
  swap(tmp);
  return *this;
}

bitset& bitset::operator>>=(std::size_t count) & {
  if (count > size()) {
    count = size();
  }
  bitset tmp(size() - count, false);
  std::copy_n(data(), tmp.word_cnt(), tmp.data());
  swap(tmp);
  return *this;
}

bitset& bitset::flip() & {
  bitset::view tmp(begin(), end());
  tmp.flip();
  return *this;
}

bitset& bitset::set() & {
  bitset::view tmp(begin(), end());
  tmp.set();
  return *this;
}

bitset& bitset::reset() & {
  bitset::view tmp(begin(), end());
  tmp.reset();
  return *this;
}

bool bitset::all() const {
  bitset::const_view tmp(begin(), end());
  return tmp.all();
}

bool bitset::any() const {
  bitset::const_view tmp(begin(), end());
  return tmp.any();
}

std::size_t bitset::count() const {
  bitset::const_view tmp(begin(), end());
  return tmp.count();
}

bitset::operator view() {
  return {begin(), end()};
}

bitset::operator const_view() const {
  return {begin(), end()};
}

bitset::view bitset::subview(std::size_t offset, std::size_t count) {
  view tmp(begin(), end());
  return tmp.subview(offset, count);
}

bitset::const_view bitset::subview(std::size_t offset, std::size_t count) const {
  const_view tmp(begin(), end());
  return tmp.subview(offset, count);
}

void swap(bitset& lhs, bitset& rhs) {
  lhs.swap(rhs);
}

bool operator==(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  if ((lhs.size()) != (rhs.size())) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.words_count(); ++i) {
    if (lhs.get_nth_word(i) != rhs.get_nth_word(i)) {
      return false;
    }
  }
  return true;
}

bool operator!=(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  return !(lhs == rhs);
}

bitset operator&(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset res(lhs);
  res &= rhs;
  return res;
}

bitset operator|(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset res(lhs);
  res |= rhs;
  return res;
}

bitset operator^(const bitset::const_view& lhs, const bitset::const_view& rhs) {
  bitset res(lhs);
  res ^= rhs;
  return res;
}

bitset operator~(const bitset::const_view& lhs) {
  bitset tmp(lhs);
  tmp.flip();
  return tmp;
}

bitset operator<<(const bitset::const_view& lhs, std::size_t count) {
  bitset tmp(lhs);
  tmp <<= count;
  return tmp;
}

bitset operator>>(const bitset::const_view& lhs, std::size_t count) {
  bitset tmp(lhs);
  tmp >>= count;
  return tmp;
}

std::string to_string(const bitset& bs) {
  bitset::const_view tmp = bs;
  return to_string(tmp);
}

std::ostream& operator<<(std::ostream& out, const bitset& bs) {
  bitset::const_view tmp = bs;
  return out << tmp;
}

std::ostream& operator<<(std::ostream& out, const bitset::const_view& v) {
  for (auto it : v) {
    out << it;
  }
  return out;
}
