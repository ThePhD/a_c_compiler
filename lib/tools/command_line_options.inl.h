#ifdef FLAG
/*
 * true/false flags
 *
 * (Source name, default value, short flag, long flag, help message)
 */
FLAG(help, false, "-h", "--help", "Print help message")
FLAG(verbose, false, "-v", "--verbose", "Display extra information from the driver")
FLAG(debug_lexer, false, "-L", "-fdebug-lexer", "Dump tokens after lexing phase")
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
OPTION(example_int_option, int, "-fexample-int-option", 123,
     "Dummy option that takes an int argument")
#endif
