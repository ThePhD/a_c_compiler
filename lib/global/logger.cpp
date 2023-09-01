#include "logger.h"
namespace {
	static std::size_t indent_level = 0;
}
namespace a_c_compiler {
	void logger::indent() noexcept {
		for (std::size_t i = 0; i < indent_level; i++)
			for (std::size_t j = 0; j < indent_width; j++)
				fprintf(stderr, "| ");
	}
	void logger::incr_indent() noexcept {
		indent_level++;
	}
	void logger::decr_indent() noexcept {
		indent_level--;
	}
} // namespace a_c_compiler
