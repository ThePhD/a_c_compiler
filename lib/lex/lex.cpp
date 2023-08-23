#include "lex.h"
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

/* Place lexed literals and identifiers in these vectors for the parser to
 * access later. Leave numeric literals as strings becaues it is the parser's
 * job to figure out what type the literal should be parsed to. */
static std::vector<std::string> lexed_numeric_literals;
static std::vector<std::string> lexed_string_literals;
static std::vector<std::string> lexed_ids;

namespace a_c_compiler {

/* Maps tokens to offsets into source files */
static std::vector<file_offset_info> token_source_info;

file_offset_info source_info_for_token(size_t tok_idx) { return token_source_info[tok_idx]; }

void dump_tokens(std::vector<std::tuple<token, file_offset_info>> const &toks) {
  size_t id_idx = 0, numlit_idx = 0, strlit_idx = 0;
  static constexpr size_t width = 15;
  std::cout << std::setw(width) << "line:column"
            << " | token\n";
  for (auto [t, foi] : toks) {
    std::stringstream ss;
    ss << foi.lineno << ":" << foi.column;
    std::cout << std::setw(width) << ss.str() << " | ";
    switch (t) {

#define CHAR_TOKEN(TOK, LIT)                                                                       \
  case TOK:                                                                                        \
    std::cout << #TOK;                                                                             \
    break;

#include "tokens.inl.h"
#undef CHAR_TOKEN

    case tok_block_comment:
      std::cout << "tok_block_comment";
      break;

    case tok_line_comment:
      std::cout << "tok_line_comment";
      break;

    case tok_newline:
      std::cout << "tok_newline";
      break;

    case tok_tab:
      std::cout << "tok_tab";
      break;

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

std::string lexed_numeric_literal(size_t index) { return lexed_numeric_literals[index]; }
std::string lexed_id(size_t index) { return lexed_ids[index]; }
std::string lexed_string_literal(size_t index) { return lexed_string_literals[index]; }

std::vector<std::tuple<token, file_offset_info>> lex(fs::path const &source_file) {
  std::vector<std::tuple<token, file_offset_info>> toks;
  toks.reserve(2048);

  FILE *fp = fopen(source_file.c_str(), "r");
  assert(fp && "Couldn't open file");
  char c;
  size_t lineno = 0, column = 0;

  /* Get next character while tracking source location info */
  auto getc = [&]() {
    c = fgetc(fp);
    if (c == '\n') {
      lineno++;
      column = 0;
    } else {
      column++;
    }
    return c;
  };

  c = fgetc(fp);
  while (c != EOF) {
    switch (c) {
    case ' ':
      break;

    case '\t':
      toks.push_back({tok_tab, {lineno, column}});
      break;

      /* Handle comments */
    case '/': {
      file_offset_info foi{lineno, column};
      getc();
      /* Line comment */
      if (c == '/') {
        while (c != '\n') {
          getc();
        }
        toks.push_back({tok_line_comment, foi});
        break;
      }

      /* Block comment */
      else if (c == '*') {
        while (true) {
          if (getc() == '*' && getc() == '/') {
            toks.push_back({tok_block_comment, foi});
            break;
          }
          assert(c != EOF && "unterminated block comment");
        }
        break;
      }
      else {
        toks.push_back({tok_forward_slash, foi});
      }
    }

    /* Char-like tokens */
#define CHAR_TOKEN(TOK, LIT) case LIT:
#include "tokens.inl.h"
      toks.push_back({(token)c, {lineno, column}});
      break;

    case '\n':
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
      while (std::isdigit(getc()) || c == '.') {
        lit += c;
      }

      /* numeric literal type suffixes */
      if (c == 'f' or c == 'd') {
        lit += c;
      } else {
        ungetc(c, fp);
      }

      lexed_numeric_literals.push_back(lit);
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
