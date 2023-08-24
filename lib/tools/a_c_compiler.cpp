#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace std::literals;
namespace fs = std::filesystem;

#include "lex.h"
#include "parse.h"

using namespace a_c_compiler;

static struct {
#define FLAG(NAME, DEFAULT_VALUE, ...) bool NAME = DEFAULT_VALUE;
#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP) TYPE NAME = DEFVAL;
#include "command_line_options.inl.h"
#undef FLAG
#undef OPTION
	std::vector<fs::path> positional_args;
} cli_opts;

int help(std::string_view exe) {
	static constexpr size_t width = 25;
	const auto help_flag = [=](std::string f_short, std::string f_long, const char* help) {
		const auto flag = f_short + (f_short.empty() ? "" : " | ") + f_long;
		std::cout << "\t" << std::left << std::setw(width) << flag << " :: " << help << "\n";
	};
	const auto help_option
	     = [=](std::string option, std::string type, std::string default_value, std::string help) {
		       std::cout << "\t" << std::left << std::setw(width) << option << " :: " << help
		                 << "\n\t" << std::setw(width + 3) << ""
		                 << " ("
		                 << "type=" << type << ", default=" << default_value << ")\n";
	       };
	std::cout << "Usage:\t" << exe << " [flags] [source files]\n\n";

	std::cout << "Flags:\n";
#define FLAG(NAME, DEFVAL, FSHORT, FLONG, HELP) help_flag(FSHORT, FLONG, HELP);
#include "command_line_options.inl.h"
#undef FLAG

	std::cout << "\nOptions:\n";
#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP) help_option(CLINAME, #TYPE, #DEFVAL, HELP);
#include "command_line_options.inl.h"
#undef OPTION

	return EXIT_FAILURE;
}

template <typename T>
T parse_option(std::string arg);

template <>
std::string parse_option<std::string>(std::string arg) {
	return arg;
}
template <>
int parse_option<int>(std::string arg) {
	return std::atoi(arg.c_str());
}

#define ARGPARSEASSERT(cond, msg)             \
	if (!(cond)) {                           \
		std::cout << "\n" << msg << "\n\n"; \
		return false;                       \
	}

bool parse_args(const std::string& exe, const std::vector<std::string>& args) {
	auto it = args.begin();
	while (it != args.end()) {
		/* parse flags */
		if (*it == "-h" or *it == "--help") {
			return false;
		}

#define FLAG(NAME, DEFVAL, FSHORT, FLONG, HELP) \
	if (*it == FSHORT or *it == FLONG) {       \
		cli_opts.NAME = true;                 \
		it++;                                 \
		continue;                             \
	}

#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP)                                        \
	if (*it == CLINAME) {                                                               \
		it++;                                                                          \
		ARGPARSEASSERT(it != args.end(), "Expected argument to follow flag -f" #NAME); \
		cli_opts.NAME = parse_option<TYPE>(*it);                                       \
		it++;                                                                          \
		continue;                                                                      \
	}

#include "command_line_options.inl.h"

#undef FLAG
#undef OPTION

		/* parse positional args */
		else {
			cli_opts.positional_args.emplace_back(*it);
		}
		it++;
	}

	/* Check validity of command line args. */

	/* Check that all positional args are files that exist */
	for (auto const& fpath : cli_opts.positional_args) {
		ARGPARSEASSERT(fs::exists(fpath), "Expected source file to exist!");
		const auto ty = fs::status(fpath).type();
		using ft      = fs::file_type;
		ARGPARSEASSERT(ty == ft::regular or ty == ft::symlink,
		     "Expected source file to be a regular file or a symlink!");
	}

	return true;
}

/* Print the full compilation configuration as YAML */
void print_cli_opts() {
	std::cout << "\nCompilation Options:\n"
	          << "  source_files: [ ";
	auto it = cli_opts.positional_args.begin();
	while (it != cli_opts.positional_args.end()) {
		std::cout << *it << (++it == cli_opts.positional_args.end() ? " " : ", ");
	}
	std::cout << "]\n";

	std::cout
#define FLAG(NAME, DEFVAL, FSHORT, FLONG, HELP) \
	<< "  " << #NAME << ":\n    kind: flag\n    value: " << cli_opts.NAME << "\n"
#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP)                            \
	<< "  " << #NAME << ":\n    kind: option\n    value: " << cli_opts.NAME \
	<< "\n    type: " << #TYPE << "\n"
#include "command_line_options.inl.h"
	     ;
#undef FLAG
#undef OPTION
}

#define VERBOSE(X)           \
	if (cli_opts.verbose) { \
		X;                 \
	}

int main(int argc, char** argv) {
	std::string exe(argv[0]);
	std::vector<std::string> args(argv + 1, argv + argc);

	if (!parse_args(exe, args)) {
		return help(exe);
	}

	if (cli_opts.verbose) {
		print_cli_opts();
	}

	for (auto const& source_file : cli_opts.positional_args) {
		if (cli_opts.verbose) {
			std::cout << "\nLexing source file " << source_file << "\n";
		}

		auto tokens = lex(source_file);

		if (cli_opts.debug_lexer) {
			dump_tokens(tokens);
		}

		auto* ast_module = parse(tokens);

		if (cli_opts.verbose) {
			ast_module->dump();
		}
	}

	return EXIT_SUCCESS;
}
