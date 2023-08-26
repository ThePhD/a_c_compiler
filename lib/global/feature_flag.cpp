#include "feature_flag.h"

#include <cstdint>

namespace {
	static constexpr std::size_t num_feature_flags = 32;
	static std::uint64_t feature_flags[num_feature_flags];
} // namespace

namespace a_c_compiler {
	bool get_feature_flag(std::size_t flag, std::size_t bit) {
		return feature_flags[flag] & (1 << bit);
	}
	void set_feature_flag(std::size_t flag, std::size_t bit) {
		feature_flags[flag] |= (1 << bit);
	}
} // namespace a_c_compiler
