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
CHAR_TOKEN(tok_equals_sign, '=')

CHAR_TOKEN(tok_plus, '+')
CHAR_TOKEN(tok_minus, '-')
CHAR_TOKEN(tok_ampersand, '&')
CHAR_TOKEN(tok_percent, '%')
#endif

#ifdef TOKEN
TOKEN(tok_id, -1)
TOKEN(tok_num_literal, -2)
TOKEN(tok_str_literal, -3)
TOKEN(tok_line_comment, -4)
TOKEN(tok_block_comment, -5)
TOKEN(tok_newline, -6)
TOKEN(tok_tab, -7)
TOKEN(tok_forward_slash, -8)
#endif
