// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#ifdef DIAGNOSTIC
DIAGNOSTIC(out_of_tokens, "out of tokens")
DIAGNOSTIC(unrecognized_token, "unrecognized token '{}'")
DIAGNOSTIC(unimplemented_keyword, "unimplemented keyword '{}'")
DIAGNOSTIC(expected_attribute_identifier,
     "expected an identifier, or a double colon (`::`)-joined set of identifiers")
DIAGNOSTIC(unbalanced_token_sequence,
     "expected a balanced set of parentheses, square brackets, or curly brackets, but received an "
     "unexpected {}")
#endif
