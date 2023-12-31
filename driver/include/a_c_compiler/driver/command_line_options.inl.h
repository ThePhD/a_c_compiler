// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#ifdef FLAG
/*
 * true/false flags
 *
 * (Source name, default value, short flag, long flag, feature flag?, feature flag bit?, help message)
 */
FLAG(help, false, "-h", "--help", nullopt, nullopt, "Print help message")
FLAG(verbose, false, "-v", "--verbose", nullopt, nullopt, "Display extra information from the driver")
FLAG(debug_lexer, false, "-L", "-fdebug-lexer", nullopt, nullopt, "Dump tokens after lexing phase")
FLAG(debug_parser, false, "", "-fdebug-parser", 1, 0x1, "Dump tokens after lexing phase")
#endif

#ifdef OPTION
/*
 * Options that take a value
 *
 * (Source name, type, command line name, default value, help message)
 */
OPTION(set_feature_flag, std::string, "-fset-feature-flag", "0,0x0", "Manually set a feature flag")
OPTION(stop_after_phase, std::string, "-fstop-after-phase", "",
     "Stop compilation after given phase is complete")
OPTION(output_file, std::string, "--output-file", "", "The file to write output into.")
OPTION(
     lex_output_file, std::string, "--lex-output-file", "", "The file to write lexer output into.")
OPTION(example_int_option, int, "-fexample-int-option", 123,
     "Dummy option that takes an int argument")
#endif
