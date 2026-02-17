Examples for Ry interpreter

How to run an example:

./build/ry run examples/<example>.ry

Included examples:

- `hello.ry` — prints a hello message
- `fib.ry` — iterative fibonacci; prints fib(10)
- `input.ry` — reads a line via `input()` and prints it
- `while_count.ry` — prints numbers using a `while` loop
- `functions.ry` — simple function and return example

Notes:

- Use `data <name> = <expr>` for declarations. Plain assignment `x =` requires an existing declaration.
- `out(...)` is a native function for printing.
- `input()` reads a line from stdin. It accepts an optional prompt string: `input("Prompt: ")`.
