/*
 * Ensure all keywords are lexed as keywords and not passed to parser as
 * identifiers.
 *
 * TODO: make use of all keywords
 */
// CHECK: tok_keyword_int
int main() {
// CHECK: tok_keyword_typedef
  typedef int int;
// CHECK: tok_keyword_register
  register int foo;
// CHECK: tok_keyword_float
  { float var; }
// CHECK: tok_keyword_double
  { double var; }
// CHECK: tok_keyword_long
  { long var; }
// CHECK: tok_keyword_unsigned
  { unsigned var; }
// CHECK: tok_keyword_char
  { char var; }
// CHECK: tok_keyword_short
  { short var; }
}
