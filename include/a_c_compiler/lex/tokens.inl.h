#ifdef CHAR_TOKEN
CHAR_TOKEN(tok_l_paren, '(')
CHAR_TOKEN(tok_r_paren, ')')
CHAR_TOKEN(tok_l_curly_bracket, '{')
CHAR_TOKEN(tok_r_curly_bracket, '}')
CHAR_TOKEN(tok_l_square_bracket, '[')
CHAR_TOKEN(tok_r_square_bracket, ']')
CHAR_TOKEN(tok_semicolon, ';')
CHAR_TOKEN(tok_asterisk, '*')
CHAR_TOKEN(tok_colon, ':')
CHAR_TOKEN(tok_comma, ',')
#endif

#ifdef WHITESPACE_TOKEN
WHITESPACE_TOKEN(tok_newline, '\n')
WHITESPACE_TOKEN(tok_tab, '\t')
#endif

#ifdef TOKEN
TOKEN(tok_id, -1)
TOKEN(tok_num_literal, -2)
TOKEN(tok_str_literal, -3)
#endif
