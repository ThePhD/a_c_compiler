#include "ast_module.h"
namespace a_c_compiler {
	namespace {
		std::vector<type_data> type_data_table;
	}
  type type_data::get_new_type() {
    std::size_t index = type_data_table.size();
    type_data_table.push_back(type_data{});
    return type{index};
  }
	type_data& type::data() const noexcept {
		return type_data_table[m_ref];
	}
} // namespace a_c_compiler
