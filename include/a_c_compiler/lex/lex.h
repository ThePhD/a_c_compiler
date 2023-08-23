#pragma once
#include <vector>
#include <filesystem>

namespace a_c_compiler {

namespace fs = std::filesystem;

enum token {

};

std::vector<token> lex(fs::path const &source_file);

}
