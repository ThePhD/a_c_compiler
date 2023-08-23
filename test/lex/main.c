int main(int argc, char** argv) {
  return 0;
}
// CHECK: tok_id: int
// CHECK-NEXT: tok_id: main
// CHECK-NEXT: tok_l_paren
// CHECK-NEXT: tok_id: int
// CHECK-NEXT: tok_id: argc
// CHECK-NEXT: tok_comma
// CHECK-NEXT: tok_id: char
// CHECK-NEXT: tok_asterisk
// CHECK-NEXT: tok_asterisk
// CHECK-NEXT: tok_id: argv
// CHECK-NEXT: tok_r_paren
// CHECK-NEXT: tok_l_curly_bracket
// CHECK-NEXT: tok_newline
// CHECK-NEXT: tok_id: return
// CHECK-NEXT: tok_num_literal: 0
// CHECK-NEXT: tok_semicolon
// CHECK-NEXT: tok_newline
// CHECK-NEXT: tok_r_curly_bracket
// CHECK-NEXT: tok_newline
