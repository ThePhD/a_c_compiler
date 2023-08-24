void foo() {
	int i   = 123;
	float j = 12.3;
	float k = 234324.f;
	float l = 0.0f;
}
// CHECK: tok_num_literal: 123
// CHECK: tok_num_literal: 12.3
// CHECK: tok_num_literal: 234324.f
// CHECK: tok_num_literal: 0.0f
