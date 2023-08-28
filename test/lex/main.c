int main(int argc, char** argv) {
	return 0;
}
// CHECK: tok_keyword_int
// CHECK: tok_id: main
// CHECK: tok_l_paren
// CHECK: tok_keyword_int
// CHECK: tok_id: argc
// CHECK: tok_comma
// CHECK: tok_keyword_char
// CHECK: tok_asterisk
// CHECK: tok_asterisk
// CHECK: tok_id: argv
// CHECK: tok_r_paren
// CHECK: tok_l_curly_bracket
// CHECK: tok_keyword_return
// CHECK: tok_num_literal: 0
// CHECK: tok_semicolon
// CHECK: tok_r_curly_bracket
