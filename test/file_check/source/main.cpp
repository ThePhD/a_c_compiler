// =============================================================================
// a_c_compiler
//
// © Asher Mancinelli & JeanHeyd "ThePhD" Meneide
// All rights reserved.
// ============================================================================ //

#include <ctre.hpp>

#include <ztd/idk/unreachable.hpp>

#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <string_view>

enum class check_style : unsigned char { first = 0, next = 1 };

std::string_view to_string_view(check_style style) {
	switch (style) {
	case check_style::first:
		return "CHECK";
	case check_style::next:
		return "CHECK-NEXT";
	default:
		ZTD_UNREACHABLE();
	}
}

struct check_result {
	std::string_view check_text;
	std::string_view remaining_file;
	bool successful;
	check_style style;
};
using check_action = std::function<check_result(std::string_view, std::string_view)>;
using check_chain  = std::vector<check_action>;

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr
		     << "[error] file_check requires at least 1 argument, which is the source match_file "
		        "present."
		     << std::endl;
		return 127;
	}
	std::string_view match_file_name = argv[1];
	int argument_index               = 2;
	std::string input_file_name      = "";
	std::string match_file           = "";
	std::string input                = "";
	bool verbose                     = false;

	constexpr std::string_view verbose_flag_name    = "--verbose";
	constexpr std::string_view input_file_flag_name = "--input-file";
	constexpr std::string_view end_of_flags_name    = "--";
	constexpr std::size_t input_file_flag_size      = input_file_flag_name.size();

	for (; argument_index < argc; ++argument_index) {
		std::string_view current_arg = argv[argument_index];
		if (current_arg.starts_with(input_file_flag_name)) {
			// is it `=` style?
			if (current_arg.size() > input_file_flag_size
			     && current_arg[input_file_flag_size] == '=') {
				// `=` style
				input_file_name = current_arg.substr(input_file_flag_size + 1);
			}
			else {
				// non `=` style
				if (current_arg != input_file_flag_name) { }
				// after verifying, take from second arg
				if (argument_index + 1 >= argc) {
					// not enough arguments; bail
					std::cerr << "[error] file_check `--input-file` flag requires a second "
					             "argument with the input file"
					          << std::endl;
					return 125;
				}
				input_file_name = argv[argument_index + 1];
				// increment current index to ensure we do not double back
				++argument_index;
			}
		}
		else if (current_arg == verbose_flag_name) {
			verbose = true;
		}
		else if (current_arg == end_of_flags_name) {
			// stop processing, exactly where we are
			++argument_index;
			break;
		}
		else {
			// all unrecognized data is passed through...
			break;
		}
	}

	{
		std::ifstream match_file_stream(match_file_name.data(), std::ios::binary);
		if (match_file_stream) {
			match_file_stream >> std::noskipws;
			std::istreambuf_iterator<char> file_stream_first(match_file_stream);
			std::istreambuf_iterator<char> file_stream_last {};
			match_file.append(file_stream_first, file_stream_last);
		}
		else {
			std::cerr << "[error] file_check could not read the match file \"" << match_file_name
			          << "\"" << std::endl;
			return 63;
		}
	}

	if (input_file_name.empty()) {
		const int first_argument_index = argument_index;
		for (int i = argument_index; i < argc; ++i) {
			std::string_view current_arg(argv[i]);
			if (i > first_argument_index) {
				// emulate some kind of whitespace?
				// TODO: command-line accurate simulation of given whitespace during
				// argument dump...
				input += " ";
			}
			input += current_arg;
		}
	}
	else {
		std::ifstream input_file_stream(input_file_name.data(), std::ios::binary);
		if (input_file_stream) {
			input_file_stream >> std::noskipws;
			std::istreambuf_iterator<char> file_stream_first(input_file_stream);
			std::istreambuf_iterator<char> file_stream_last {};
			input_file_name.append(file_stream_first, file_stream_last);
		}
		else {
			std::cerr << "[error] file_check could not read the input file \"" << input_file_name
			          << "\"" << std::endl;
			return 62;
		}
	}

	if (verbose) {
		std::cout << "[info] using input data consumed from\n\t";
		if (input_file_name.empty()) {
			std::cout << "standard output";
		}
		else {
			std::cout << "file \"" << input_file_name << "\"";
		}
		std::cout << "\nthat will be checked with directives found within file\n\t\""
		          << match_file_name << "\"" << std::endl;
	}

	// match file checks can be singular in nature, e.g.
	//
	// CHECK: bark
	// CHECK: woof
	//
	// Or they can be chained, where it depends on the last check done and expect things to
	// progress in a linear order, e.g.
	//
	// CHECK: meow
	// CHECK-NEXT: purr
	//
	// Each `CHECK` creates a new check_chain, with the first entry in the chain being what's in
	// the `CHECK`. Each `CHECK-NEXT` creates a new `check_action` within the pre-existing
	// `check_chain`, meaning there is an error if there's already not a check chain in there.

	// Run CTRE to get all the targets we need to search for
	constexpr const auto check_regex = ctre::range<R"(//\h*+(?:CHECK(-NEXT)?):\h*+([^\v]++)\v++)">;

	std::size_t potential_checks = 0;
	std::vector<check_chain> check_chains {};
	for (auto [whole_match, first_group, expected] : check_regex(match_file)) {
		++potential_checks;
		check_style style = first_group ? check_style::next : check_style::first;
		if (first_group) {
			if (check_chains.empty() || check_chains.back().empty()) {
				// FAILURE: there was a CHECK-NEXT without a CHECK, cannot be chained
				std::cerr << "[error] there was a `CHECK-NEXT` that was not preceeded by a "
				             "`CHECK`:\n"
				          << whole_match << std::endl;
				return 61;
			}
		}
		else {
			check_chains.emplace_back();
		}
		auto& current_chain         = check_chains.back();
		std::string_view check_text = expected.view();
		current_chain.emplace_back(
		     [style, check_text](std::string_view, std::string_view remaining_file) {
			     auto find_it = remaining_file.find(check_text);
			     if (find_it == std::string::npos) {
				     return check_result {
					     check_text,
					     remaining_file,
					     false,
					     style,
				     };
			     }
			     return check_result {
				     check_text,
				     remaining_file.substr(find_it + check_text.size()),
				     true,
				     style,
			     };
		     });
	}


	if (verbose) {
		std::cout << "[info] produced " << potential_checks
		          << " matches from the match file to check!" << std::endl;
	}


	for (std::size_t current_check_chain_index = 0;
	     current_check_chain_index < check_chains.size(); ++current_check_chain_index) {
		const auto& current_check_chain = check_chains[current_check_chain_index];
		// check a specific chain, subset match_file as we go
		std::string_view remaining_file = match_file;
		for (std::size_t current_check_index = 0;
		     current_check_index < current_check_chain.size(); ++current_check_index) {
			const auto& current_check = current_check_chain[current_check_index];
			const auto check_result   = current_check(match_file, remaining_file);
			if (!check_result.successful) {
				std::cerr << "[fail] ❌ check failed\n"
				          << to_string_view(check_result.style) << ": "
				          << check_result.check_text << std::endl;
				return 1;
			}
			else if (verbose) {
				std::cout << "[pass] ✅ check passed\n"
				          << to_string_view(check_result.style) << ": "
				          << check_result.check_text << std::endl;
			}
			remaining_file = check_result.remaining_file;
		}
		// when the loop ends and we go back to the top, we reset `remaining_file`,
		// which means we effectively check the whole match_file again to find the match we want.
		// this should keep our checks running normally and nominally.
	}

	return 0;
}
