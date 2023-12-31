// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#include <a_c_compiler/options/global_options.h>
#include <a_c_compiler/fe/lex/lex.h>
#include <a_c_compiler/fe/parse/parse.h>

#include <ztd/idk/assert.hpp>

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <vector>
#include <string>
#include <optional>

using namespace std::literals;
namespace fs = std::filesystem;
using std::nullopt;


using namespace a_c_compiler;

static struct {
#define FLAG(NAME, DEFAULT_VALUE, ...) bool NAME = DEFAULT_VALUE;
#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP) TYPE NAME = DEFVAL;
#include <a_c_compiler/driver/command_line_options.inl.h>
#undef FLAG
#undef OPTION
	std::vector<fs::path> positional_args;
} cli_opts;

int help_with(std::string_view exe, int return_code) noexcept {
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
#define FLAG(NAME, DEFVAL, FSHORT, FLONG, FLAG, BIT, HELP) help_flag(FSHORT, FLONG, HELP);
#include <a_c_compiler/driver/command_line_options.inl.h>
#undef FLAG

	std::cout << "\nOptions:\n";
#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP) help_option(CLINAME, #TYPE, #DEFVAL, HELP);
#include <a_c_compiler/driver/command_line_options.inl.h>
#undef OPTION

	return return_code;
}

int help(std::string_view exe) noexcept {
	return help_with(exe, EXIT_FAILURE);
}

template <typename T>
T parse_option(std::string arg) noexcept;

template <>
std::string parse_option<std::string>(std::string arg) noexcept {
	return arg;
}
template <>
int parse_option<int>(std::string arg) noexcept {
	return std::atoi(arg.c_str());
}

void handle_feature_flag(std::string_view arg_str, global_options& global_opts) {
	ZTD_ASSERT_MESSAGE(
	     "expected comma separator to be in feature flag argument", arg_str.contains(','));
	size_t n = arg_str.find(',');
	std::size_t num_written;

	std::string_view flag_str = arg_str.substr(0, n);
	std::size_t flag_value    = std::stoul(flag_str.data(), &num_written, 10);
	ZTD_ASSERT_MESSAGE("failed to parse feature flag argument", num_written);

	std::string_view bit_str = arg_str.substr(n + 1);
	std::size_t bit_value    = std::stoul(bit_str.data(), &num_written, 16);
	ZTD_ASSERT_MESSAGE("failed to parse feature flag argument", num_written);

	global_opts.set_feature_flag(flag_value, bit_value);
}

bool parse_args(const std::string& exe, const std::vector<std::string>& args,
     global_options& global_opts) noexcept {
	auto it = args.begin();
	while (it != args.end()) {
		/* parse flags */
		if (*it == "-h" or *it == "--help") {
			return false;
		}

#define FLAG(NAME, DEFVAL, FSHORT, FLONG, FLAG, BIT, HELP)              \
	else if (*it == FSHORT or *it == FLONG) {                          \
		cli_opts.NAME                     = true;                     \
		std::optional<std::uint64_t> flag = FLAG, bit = BIT;          \
		if (flag.has_value()) {                                       \
			ZTD_ASSERT(bit.has_value());                             \
			global_opts.set_feature_flag(flag.value(), bit.value()); \
		}                                                             \
	}

#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP)                                            \
	else if (*it == CLINAME) {                                                              \
		it++;                                                                              \
		ZTD_ASSERT_MESSAGE("Expected argument to follow flag -f" #NAME, it != args.end()); \
		cli_opts.NAME = parse_option<TYPE>(*it);                                           \
	}

#include <a_c_compiler/driver/command_line_options.inl.h>

#undef FLAG
#undef OPTION

		/* parse positional args */
		else {
			cli_opts.positional_args.emplace_back(*it);
		}
		if (*it == "-fset-feature-flag") {
			handle_feature_flag(cli_opts.set_feature_flag, global_opts);
		}
		it++;
	}

	/* Check validity of command line args. */

	/* Check that all positional args are files that exist */
	for (auto const& fpath : cli_opts.positional_args) {
		if (!fs::exists(fpath)) {
			std::cerr << "[error] could not find input file \"" << fpath << "\"\n";
			return EXIT_FAILURE;
		}
		const auto ty = fs::status(fpath).type();
		using ft      = fs::file_type;
		ZTD_ASSERT_MESSAGE("Expected source file to be a regular file or a symlink!",
		     ty == ft::regular or ty == ft::symlink);
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
#define FLAG(NAME, DEFVAL, FSHORT, FLONG, FLAG, BIT, HELP) \
	<< "  " << #NAME << ":\n    kind: flag\n    value: " << cli_opts.NAME << "\n"
#define OPTION(NAME, TYPE, CLINAME, DEFVAL, HELP)                            \
	<< "  " << #NAME << ":\n    kind: option\n    value: " << cli_opts.NAME \
	<< "\n    type: " << #TYPE << "\n"
#include <a_c_compiler/driver/command_line_options.inl.h>
	     ;
#undef FLAG
#undef OPTION
}

int main(int argc, char** argv) {
	std::string exe(argv[0]);
	std::vector<std::string> args(argv + 1, argv + argc);
	a_c_compiler::global_options global_opts {};
	a_c_compiler::diagnostic_handles diag_handles {};

	if (!parse_args(exe, args, global_opts)) {
		return help(exe);
	}

	if (cli_opts.verbose) {
		print_cli_opts();
	}

	bool failed_lexer_output = false;
	bool failed_parse_output = false;

	for (auto const& source_file : cli_opts.positional_args) {
		if (cli_opts.verbose) {
			std::cout << "\nLexing source file " << source_file << "\n";
		}

		auto tokens = lex(source_file, global_opts, diag_handles);

		if (cli_opts.debug_lexer) {
			const bool write_lex_to_stdout = cli_opts.lex_output_file.empty();
			if (cli_opts.verbose) {
				std::cout << "Dumping tokens to "
				          << (write_lex_to_stdout ? "standard output"
				                                  : cli_opts.lex_output_file.c_str())
				          << "\n";
			}

			if (write_lex_to_stdout) {
				dump_tokens(tokens);
			}
			else {
				std::ofstream lex_output_stream(cli_opts.lex_output_file.c_str());
				if (lex_output_stream) {
					dump_tokens_into(tokens, lex_output_stream);
				}
				else {
					std::cerr << "cannot write to lex output file \""
					          << cli_opts.lex_output_file << "\"\n";
					failed_lexer_output = true;
				}
			}
		}

		if (cli_opts.stop_after_phase == "lex") {
			return failed_lexer_output ? EXIT_FAILURE : EXIT_SUCCESS;
		}

		auto ast_module = parse(tokens, global_opts, diag_handles);

		if (cli_opts.verbose) {
			ast_module.dump();
		}

		if (cli_opts.stop_after_phase == "parse") {
			return failed_parse_output || failed_lexer_output ? EXIT_FAILURE : EXIT_SUCCESS;
		}
	}

	return EXIT_SUCCESS;
}
