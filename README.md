# Arithmetic Expression Evaluator

A DSA assignment that takes an infix expression, converts it to postfix, and evaluates it.

## How to compile

```
g++ -o expr expr.cpp
```

## How to run

```
./dsaAsisn1
```

Then type your expression and press Enter. The program will ask you to enter a value for each variable.

## Example

```
a + b * (c + 2)
Enter value for a: 3
Enter value for b: 5
Enter value for c: 2
a b c 2 + * +
23
```

## What it supports

- Operators: `+`, `-`, `*`, `/`
- Brackets: `()`, `[]`, `{}`
- Integer and decimal numbers (e.g. `3.14`)
- Variables (any name that follows C++ rules like `x`, `myVar`, `a1`)
- Unary minus (e.g. `-a` or `a * -b`)

## Error handling

- Syntax errors (wrong expression) → exits with code 1
- Runtime errors (division by zero) → exits with code 2
- Logical errors (malformed postfix) → exits with code 3
