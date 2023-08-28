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
		std::cerr << "[error] file_check requires at least 1 argument, which is the source file "
		             "present."
		          << std::endl;
		return 127;
	}
	std::string_view file_name = argv[1];
	std::string file           = "";
	std::string search_space   = "";
	{
		std::ifstream file_stream(file_name.data(), std::ios::binary);
		if (file_stream) {
			file_stream >> std::noskipws;
			std::istreambuf_iterator<char> file_stream_first(file_stream);
			std::istreambuf_iterator<char> file_stream_last {};
			file.append(file_stream_first, file_stream_last);
		}
		else {
			std::cerr << "[error] file_check could not read the file \"" << file_name << "\""
			          << std::endl;
			return 126;
		}
	}

	if (argc > 2) {
		for (int i = 2; i < argc; ++i) {
			std::string_view current_arg(argv[i]);
			if (i > 2) {
				// emulate some kind of whitespace?
				// TODO: command-line accurate simulation of given whitespace during argument
				// dump...
				search_space += " ";
			}
			search_space += current_arg;
		}
	}

	// File checks can be singular in nature, e.g.
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
	auto matches                     = check_regex(file);

	std::vector<check_chain> check_chains {};
	for (auto [whole_match, first_group, expected] : matches) {
		check_style style = first_group ? check_style::next : check_style::first;
		if (first_group) {
			if (check_chains.empty() || check_chains.back().empty()) {
				// FAILURE: there was a CHECK-NEXT without a CHECK, cannot be chained
				std::cerr << "[error] there was a `CHECK-NEXT` that was not preceeded by a "
				             "`CHECK`:\n"
				          << whole_match << std::endl;
				return 125;
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

	for (std::size_t current_check_chain_index = 0;
	     current_check_chain_index < check_chains.size(); ++current_check_chain_index) {
		const auto& current_check_chain = check_chains[current_check_chain_index];
		// check a specific chain, subset file as we go
		std::string_view remaining_file = file;
		for (std::size_t current_check_index = 0;
		     current_check_index < current_check_chain.size(); ++current_check_index) {
			const auto& current_check = current_check_chain[current_check_index];
			const auto check_result   = current_check(file, remaining_file);
			if (!check_result.successful) {
				std::cerr << "[fail] ❌ check failed\n"
				          << to_string_view(check_result.style) << ": "
				          << check_result.check_text << std::endl;
				return 1;
			}
			remaining_file = check_result.remaining_file;
		}
		// when the loop ends and we go back to the top, we reset `remaining_file`,
		// which means we effectively check the whole file again to find the match we want.
		// this should keep our checks running normally and nominally.
	}

	return 0;
}
