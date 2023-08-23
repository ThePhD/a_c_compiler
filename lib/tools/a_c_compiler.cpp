#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

static struct {
  std::vector<std::string> positional_args;
} cli_opts;

bool parse_args(const std::vector<std::string> &args) {
  auto it = args.begin();
  while (it != args.end()) {
    /* parse flags */
    if (*it == "-h" or *it == "--help") {
      return false;
    }

    /* parse positional args */
    else {
      cli_opts.positional_args.push_back(*it);
    }
    it++;
  }

  /* Check validity of command line args. At this point, we only check that
   * we have exatly one positional arg, the input source file. */
  if (cli_opts.positional_args.size() != 1) {
    return false;
  }
  return true;
}

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

int main(int argc, char **argv) {
  std::string exe(argv[0]);
  std::vector<std::string> args(argv + 1, argv + argc);
  if (!parse_args(args)) {
    return help(exe);
  }
  return EXIT_SUCCESS;
}
