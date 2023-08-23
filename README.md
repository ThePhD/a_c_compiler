# A C Compiler

## Building

The usual CMake workflow:

```shell
$ cmake -S. -Bbuild
$ cmake --build build
```

## Testing

```shell
$ cd build
$ ctest
Internal ctest changing into directory: /home/betsy/workspace/a_c_compiler/build
Test project /home/betsy/workspace/a_c_compiler/build
    Start 1: test.lex.main
1/2 Test #1: test.lex.main ....................   Passed    0.00 sec
    Start 2: test.lex.numlit
2/2 Test #2: test.lex.numlit ..................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.00 sec
```

## Developing

See the `doc` directory for documentation on developing A C Compiler.
