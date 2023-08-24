#pragma once
#include "ast_node.h"

#include <span>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <variant>

#include <ztd/idk/assert.hpp>

namespace a_c_compiler {

	enum class type_modifier : unsigned char {
		tm_signed,
		tm_unsigned,
		tm__Atomic, // _Atomic(int)
	};
	enum class type_category : unsigned char {
		tc_void,
		tc_char,
		tc_short,
		tc_int,
		tc_long,
		tc_longlong,
		tc__BitInt,
		tc_float,
		tc_double,
		tc_longdouble,
		tc_union,
		tc_struct,
		tc_enum,
		tc_function,
		tc_array,
		tc_variable_length_array,
		tc_vla = tc_variable_length_array,
		tc_data_pointer,
		tc_function_pointer,
		tc_nullptr,
		tc_auto, // inferred type
		tc__Padding, // extension: struct foo { int meow; _Padding(16) padding; uint16_t bark; };
		tc_array_span // extension: array span
	};
	enum class qualifier : unsigned char {
		none       = 0b0000,
		q_const    = 0b0001,
		q__Atomic  = 0b0010, // e.g. _Atomic int
		q_volatile = 0b0100,
		q_restrict = 0b1000,
	};
	enum class storage_class_specifier : unsigned short {
		none             = 0b00000000000,
		scs_static       = 0b00000000001,
		scs_extern       = 0b00000000010,
		scs_constexpr    = 0b00000000100,
		scs_register     = 0b00000001000,
		scs_thread_local = 0b00000010000,
		scs_typedef      = 0b00000100000,
	};
	using sc_specifier = storage_class_specifier;
	
	struct type {
		constexpr type () noexcept = default;
		constexpr type (const type&) noexcept = default;
		constexpr type (type&&) noexcept = default;
		constexpr type& operator= (const type&) noexcept = default;
		constexpr type& operator= (type&&) noexcept = default;

		constexpr explicit type (std::size_t ref_index) noexcept : m_ref(ref_index) {}

		constexpr std::size_t index () const noexcept {
			return m_ref;
		}
	private:
		std:: uint_least32_t m_ref;
	};

	struct type_data {
		type_category category;
		qualifier qualifiers;
		storage_class_specifier specifiers;
		std::size_t bit_size; // for _BitInt and _Padding and friends
		std::vector<type> sub_types;

		type& pointee_type () {
			ZTD_ASSERT_MESSAGE("Must be a pointer type.", category == type_category::tc_data_pointer || category == type_category::tc_function_pointer);
			ZTD_ASSERT_MESSAGE("There must be at least 1 available sub-type.", sub_types.size() == 1);
			return sub_types[0];
		}

		type& element_type () {
			ZTD_ASSERT_MESSAGE("Must be an array type.", category == type_category::tc_array || category == type_category::tc_vla || category == type_category::tc_array_span);
			ZTD_ASSERT_MESSAGE("There must be at least 1 available sub-type.", sub_types.size() == 1);
			return sub_types[0];
		}

		std::span<type> member_types () {
			ZTD_ASSERT_MESSAGE("Must be a structure or union type.", category == type_category::tc_struct || category == type_category::tc_union);
			return sub_types;
		}

		type& return_type () {
			ZTD_ASSERT_MESSAGE("Must be a function type.", category == type_category::tc_function);
			ZTD_ASSERT_MESSAGE("Must have at least one type.", sub_types.size() >= 1);
			// Return Type + Parameter Layout
			// [ R | P | P | P ]
			//     ^           ^
			// [ R ]
			//     ^
			return sub_types[0];
		}

		std::span<type> parameter_types () {
			ZTD_ASSERT_MESSAGE("Must be a function type.", category == type_category::tc_function);
			ZTD_ASSERT_MESSAGE("Must have at least one type.", sub_types.size() >= 1);
			// Return Type + Parameter Layout
			// [ R | P | P | P ]
			//     ^           ^
			// [ R ]
			//     ^
			return std::span<type>(sub_types.data() + 1, sub_types.data() + sub_types.size());
		}
	};

	struct static_assert_declaration { };
	struct operator_declaration { };
	struct alias_declaration { };
	struct attribute {
		std::vector<token> tokens;
	};
	struct member_declaration {
		type t;
		std::size_t alignment;
		std::size_t bit_field_size;
		std::size_t bit_field_position; // extension
	};
	struct struct_declaration {
		type t;
		std::size_t alignment;
		std::vector<attribute> attributes;
		std::vector<member_declaration> members;
	 };
	struct parameter_declaration {
		type t;
	};
	struct function_declaration {
		std::vector<attribute> attributes;
		std::vector<parameter_declaration> members;
	};

	struct statement {
		std::vector<token> tokens;
	};

	struct compound_statement {
		std::vector<statement> statements;
	};

	struct function_definition {
		function_declaration declaration;
		compound_statement body;
	};

	using declaration = std::variant<
		function_declaration,
		struct_declaration,
		static_assert_declaration,
		operator_declaration,
		alias_declaration,
		statement
	>;

	using external_declaration = std::variant<
		function_definition,
		declaration
	>;

	struct translation_unit {
		std::vector<external_declaration> declarations;
	};

	struct ast_module {
		translation_unit top_level;

		void dump() const {
			/* */
		}
	};
} /* namespace a_c_compiler */
