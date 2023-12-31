// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#pragma once

#include <a_c_compiler/fe/lex/lex.h>
#include <a_c_compiler/fe/parse/ast_module.h>
#include <a_c_compiler/fe/reporting/diagnostic_handles.h>
#include <a_c_compiler/options/global_options.h>

namespace a_c_compiler {

	ast_module parse(token_vector const& toks, const global_options& global_opts,
	     diagnostic_handles& diag_handles) noexcept;

} /* namespace a_c_compiler */
