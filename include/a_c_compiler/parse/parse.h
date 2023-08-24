#pragma once

#include "lex.h"
#include "ast_module.h"

namespace a_c_compiler {

	ast_module parse(token_vector const& toks) noexcept;

} /* namespace a_c_compiler */
