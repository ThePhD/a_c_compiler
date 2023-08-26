#pragma once

#include <cstdlib>

namespace a_c_compiler {
	bool get_feature_flag(std::size_t flag, std::size_t bit);
	void set_feature_flag(std::size_t flag, std::size_t bit);
} // namespace a_c_compiler
