struct s {
  int field;
};

typedef struct {
  bool field;
} var;
// CHECK: unimplemented keyword 'struct'
