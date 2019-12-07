# GCI

`GCI` is a tiny JavaScript-like object-oriented language

It's main features are:
  - Strong dynamic typing;
  - Objects and arrays;

### Tech

This is `GCI` interpreter.
It consists of:
  - Object-oriented lexer;
  - Top-down parser;
  - Bytecode generator;
  - Stack-based VM;
  - Cheney's GC;

### Installation

`GCI` requires only C89-compatible compiler and `MAKE` utility to run.

```sh
$ git clone https://github.com/masyagin1998/GCI.git
$ cd GCI
$ make
$ make tests
$ ./bin/interpreter -i data/input.js
```

