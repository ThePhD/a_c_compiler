#ifdef FLAG
/* 
 * true/false flags
 *
 * (Source name, default value, short flag, long flag, help message)
 */
FLAG(help, false, "-h", "--help", "Print help message")
FLAG(verbose, false, "-v", "--verbose", "Display extra information from the driver")
FLAG(debug_lexer, false, "-L", "--debug-lexer", "Dump tokens after lexing phase")
#endif

#ifdef OPTION
/* 
 * Options that take a value
 *
 * (Source name, type, command line name, default value, help message)
 */
OPTION(example_str_option, std::string, "-fexample-string-option", "example string value", "Dummy option that takes a string argument")
OPTION(example_int_option, int, "-fexample-int-option", 123, "Dummy option that takes an int argument")
#endif
