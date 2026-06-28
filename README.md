# FCC

<!--toc:start-->

- [FCC](#fcc)
  - [Try it out on your machine](#try-it-out-on-your-machine)
  - [Implemented Optimization Passes](#implemented-optimization-passes)
  - [Currently Lowered C Features](#currently-lowered-c-features)
    - [Functions](#functions)
    - [Variables](#variables)
    - [Expressions](#expressions)
      - [Binary arithmetic](#binary-arithmetic)
      - [Comparison operators](#comparison-operators)
      - [Unary operators](#unary-operators)
      - [Logical operators](#logical-operators)
    - [Control Flow](#control-flow)
  - [References](#references)
  <!--toc:end-->

FCC is a optimizing compiler for a (growing) C99 subset.

## Try it out on your machine

Build:

```
make
```

Run tests:

```
python tests.py
```

Cleanup:

```
make clean
```

Use:

```
./fcc test.c
```

Requirements:

- make
- clang
- mold (optional)
- clang development libraries
- python

## Implemented Optimization Passes

- Constant Folding
- Dead Branch Elimination
- Dead Block Elimination
- Dead Definition Elimination

## Currently Lowered C Features

### Functions

- Function declarations and definitions
- Function calls

### Variables

- Variable declarations
- Variable assignment
- SSA variable reads / writes with phi insertion

### Expressions

#### Binary arithmetic

- Addition (`+`)
- Subtraction (`-`)
- Multiplication (`*`)
- Division (`/`)
- Modulus / Remainder (`%`)

#### Comparison operators

- Less than (`<`)
- Less than or equal (`<=`)
- Greater than (`>`)
- Greater than or equal (`>=`)
- Equal (`==`)
- Not equal (`!=`)

#### Unary operators

- Unary plus (`+x`)
- Unary minus (`-x`)
- Logical NOT (`!x`)

#### Logical operators

- Logical AND (`&&`)
- Logical OR (`||`)

### Control Flow

- `if`
- `if-else`
- `while`
- `for`
- `break`
- `continue`

## References

- [libclang: C Interface to Clang](https://clang.llvm.org/doxygen/group__CINDEX.html)
- [Simple and Efficient Construction of Static Single Assignment Form by Braun et al.](https://c9x.me/compile/bib/braun13cc.pdf)
