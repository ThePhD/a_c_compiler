#include "lex.h"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>

/* Place lexed literals and identifiers in these vectors for the parser to
 * access later */
static std::vector<int> lexed_numeric_literals;
static std::vector<std::string> lexed_string_literals;
static std::vector<std::string> lexed_ids;

namespace a_c_compiler {

/* Maps tokens to offsets into source files */
static std::vector<file_offset_info> token_source_info;

file_offset_info source_info_for_token(size_t tok_idx) {
  return token_source_info[tok_idx];
}

void dump_tokens(std::vector<std::tuple<token, file_offset_info>> const& toks) {
  size_t id_idx = 0, numlit_idx = 0, strlit_idx = 0;
  static constexpr size_t width = 15;
  std::cout << std::setw(width) << "line:column" << " | token\n";
  for (auto [t, foi] : toks) {
    std::stringstream ss;
    ss << foi.lineno << ":" << foi.column;
    std::cout << std::setw(width) << ss.str() << " | ";
    switch (t) {

#define WHITESPACE_TOKEN(TOK, LIT)                                                                 \
  case TOK:                                                                                        \
    std::cout << #TOK;                                                                             \
    break;

#define CHAR_TOKEN(TOK, LIT)                                                                       \
  case TOK:                                                                                        \
    std::cout << #TOK << ": '" << LIT << "'";                                                      \
    break;

#include "tokens.inl.h"
#undef CHAR_TOKEN
#undef WHITESPACE_TOKEN

    case tok_id:
      std::cout << "tok_id: " << lexed_id(id_idx++);
      break;

    case tok_num_literal:
      std::cout << "tok_num_literal: " << lexed_numeric_literal(numlit_idx++);
      break;

    case tok_str_literal:
      std::cout << "str_literal: " << lexed_string_literal(strlit_idx++);
      break;

    default:
      assert(false && "Got invalid token");
    }
    std::cout << "\n";
  }
}

int lexed_numeric_literal(size_t index) { return lexed_numeric_literals[index]; }
std::string lexed_id(size_t index) { return lexed_ids[index]; }
std::string lexed_string_literal(size_t index) { return lexed_string_literals[index]; }

std::vector<std::tuple<token, file_offset_info>> lex(fs::path const &source_file) {
  std::vector<std::tuple<token, file_offset_info>> toks;
  toks.reserve(1024);

  FILE *fp = fopen(source_file.c_str(), "r");
  assert(fp && "Couldn't open file");
  char c;
  size_t lineno = 0, column = 0;

  /* Get next character while tracking source location info */
  auto getc = [&] () {
    c = fgetc(fp);
    column++;
    while (c == ' ') {
      c = fgetc(fp);
      column++;
    }
    return c;
  };

  c = fgetc(fp);
  while (c != EOF) {
    switch (c) {
    case ' ':
      continue;

      /* Char-like tokens */
    case '\t':
#define CHAR_TOKEN(TOK, LIT)       case LIT:
#include "tokens.inl.h"
      toks.push_back({(token)c, {lineno, column}});
      break;

    case '\n':
      lineno++;
      column = 0;
      toks.push_back({tok_newline, {lineno, column}});
      break;

      /* Numeric literals */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '.': {
      file_offset_info foi{lineno, column};
      std::string lit = "";
      lit += c;
      while (std::isdigit(c = fgetc(fp))) {
        lit += c;
      }

      /* numeric literal type suffixes */
      if (c == 'f' or c == 'd') {
        lit += c;
      } else {
        ungetc(c, fp);
      }

      lexed_numeric_literals.push_back(std::atoi(lit.c_str()));
      toks.push_back({tok_num_literal, foi});
      break;
    }
    }

    /* Identifier */
    if (std::isalpha(c) or c == '_') {
      file_offset_info foi{lineno, column};
      std::string lit = "";
      lit += c;

      while (std::isalnum(c = fgetc(fp))) {
        lit += c;
      }
      ungetc(c, fp);

      lexed_ids.push_back(lit);
      toks.push_back({tok_id, foi});
    }
    getc();
  }
  return toks;
}

} // namespace a_c_compiler
