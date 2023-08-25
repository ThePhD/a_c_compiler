#pragma once
#include <cstdint>
namespace a_c_compiler {
  bool get_feature_flag(std::size_t flag, std::size_t bit);
  void set_feature_flag(std::size_t flag, std::size_t bit);
} /* a_c_compiler */
