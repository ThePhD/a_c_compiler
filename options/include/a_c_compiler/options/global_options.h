// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include <cstdlib>
#include <cstddef>
#include <cstdint>

namespace a_c_compiler {

	struct global_options {
		constexpr global_options() : m_feature_flags() {
		}

		[[nodiscard]] constexpr bool get_feature_flag(
		     std::size_t flag, std::size_t bit) const noexcept {
			return m_feature_flags[flag] & (1 << bit);
		}

		constexpr void set_feature_flag(std::size_t flag, std::size_t bit) noexcept {
			m_feature_flags[flag] |= (1 << bit);
		}

	private:
		inline static constexpr const std::size_t num_feature_flags = 32;
		std::uint64_t m_feature_flags[num_feature_flags];
	};
} // namespace a_c_compiler
