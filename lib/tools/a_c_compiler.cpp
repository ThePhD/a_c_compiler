// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>
#include <string>

using namespace std::literals;
namespace fs = std::filesystem;


#include "feature_flag.h"
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

void handle_feature_flag(std::string_view arg_str) {
	assert(arg_str.contains(',') && "expected comma separator to be in feature flag argument");
	size_t n = arg_str.find(',');
	std::size_t num_written;

	std::string_view flag_str = arg_str.substr(0, n);
	std::size_t flag_value    = std::stoul(flag_str.data(), &num_written, 10);
	assert(num_written && "failed to parse feature flag argument");

	std::string_view bit_str = arg_str.substr(n + 1);
	std::size_t bit_value    = std::stoul(bit_str.data(), &num_written, 16);
	assert(num_written && "failed to parse feature flag argument");

	set_feature_flag(flag_value, bit_value);
}

bool parse_args(const std::string& exe, const std::vector<std::string>& args) {
	auto it = args.begin();
	while (it != args.end()) {
		/* parse flags */
		if (*it == "-h" or *it == "--help") {
			return false;
		}

#define FLAG(NAME, DEFVAL, FSHORT, FLONG, HELP) \
	else if (*it == FSHORT or *it == FLONG) {  \
		cli_opts.NAME = true;                 \
	}

#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP)                                        \
	else if (*it == CLINAME) {                                                          \
		it++;                                                                          \
		ARGPARSEASSERT(it != args.end(), "Expected argument to follow flag -f" #NAME); \
		cli_opts.NAME = parse_option<TYPE>(*it);                                       \
	}

#include "command_line_options.inl.h"

#undef FLAG
#undef OPTION

		/* parse positional args */
		else {
			cli_opts.positional_args.emplace_back(*it);
		}
		if (*it == "-fset-feature-flag") {
			handle_feature_flag(cli_opts.set_feature_flag);
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

    if (cli_opts.stop_after_phase == "lex")  {
      return EXIT_SUCCESS;
    }

		auto ast_module = parse(tokens);

		if (cli_opts.verbose) {
			ast_module.dump();
		}

    if (cli_opts.stop_after_phase == "parse")  {
      return EXIT_SUCCESS;
    }
	}

	return EXIT_SUCCESS;
}
