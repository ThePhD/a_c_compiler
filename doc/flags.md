# Compiler Flags

To add a compiler flag, you may add an entry to `lib/tools/command_line_options.inl.h`.
There are two kinds of entries here: flags and options.
Flags are either true or false and are enabled when they are found in the command line arguments and otherwise disabled.
Options take a value.
