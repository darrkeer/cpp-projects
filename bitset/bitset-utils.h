#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>

namespace bitset_utils {
using word_type = uint64_t;

static constexpr size_t word_size = std::numeric_limits<word_type>::digits;
static constexpr std::size_t npos = -1;
static constexpr word_type mask = -1;

inline static std::size_t word_ind(std::size_t bit) {
  return bit / word_size;
}

inline static std::size_t offset(std::size_t bit) {
  return bit % word_size;
}
} // namespace bitset_utils
