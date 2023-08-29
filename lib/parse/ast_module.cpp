#include "ast_module.h"
namespace a_c_compiler {
	namespace {
		std::vector<type_data> type_data_table;
	}
	type_data& type::data() const noexcept {
		return type_data_table[m_ref];
	}
} // namespace a_c_compiler
