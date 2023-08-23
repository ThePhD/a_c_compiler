#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

static struct {
  std::vector<fs::path> positional_args;
} cli_opts;

int help(std::string_view exe) {
  const auto help_flag = [](const char *f_short, const char *f_long, const char *help) {
    const auto flag = std::string(f_short) + "|" + f_long;
    std::cout << "\t" << std::setw(10) << flag << "\t" << help << "\n";
  };
  std::cout << "Usage:"
            << "\t" << exe << " [flags] [source files]"
            << "\n\n"
            << "Flags:\n";
  help_flag("-h", "--help", "Print this help message.");
  return EXIT_FAILURE;
}

#define ARGPARSEASSERT(cond, msg)                                                                  \
  if (!(cond)) {                                                                                   \
    std::cout << "\n" << msg << "\n\n";                                                            \
    return false;                                                                                  \
  }

bool parse_args(const std::string &exe, const std::vector<std::string> &args) {
  auto it = args.begin();
  while (it != args.end()) {
    /* parse flags */
    if (*it == "-h" or *it == "--help") {
      return false;
    }

    /* parse positional args */
    else {
      cli_opts.positional_args.emplace_back(*it);
    }
    it++;
  }

  /* Check validity of command line args. */

  /* Check that we have exatly one positional arg, the input source file. */
  ARGPARSEASSERT(cli_opts.positional_args.size() == 1, "We only handle one source file atm");

  /* Check that all positional args are files that exist */
  for (auto const &fpath : cli_opts.positional_args) {
    ARGPARSEASSERT(fs::exists(fpath), "Expected source file to exist!");
    const auto ty = fs::status(fpath).type();
    using ft = fs::file_type;
    ARGPARSEASSERT(ty == ft::regular or ty == ft::symlink,
                   "Expected source file to be a regular file or a symlink!");
  }

  return true;
}

int main(int argc, char **argv) {
  std::string exe(argv[0]);
  std::vector<std::string> args(argv + 1, argv + argc);
  if (!parse_args(exe, args)) {
    return help(exe);
  }
  return EXIT_SUCCESS;
}
