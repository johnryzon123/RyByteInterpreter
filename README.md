# Ry (Ry's for You) 🌀

Ry is a lightweight, robust, and user-centric programming language designed with a focus on stability and developer experience. Whether you're building simple scripts or exploring language design, Ry is built to be helpful, colorful, and fast.

## Key Features

- **Intelligent Error Reporting**: Beautiful, color-coded error messages with caret pointers (`^~~`) to show you exactly where things went wrong.
- **Optimized Iteration**: High-level `foreach` loops and `range` expressions (`0 to 100`) designed for performance.
- **Smart REPL**: A dynamic interactive shell with auto-indentation tracking and colorized prompts.
- **Built for Stability**: A memory-conscious C++ core that respects your hardware limits.

## Performance

Ry is built to be efficient. In our internal benchmarks, Ry's native iteration outperforms standard loop structures:

| Loop Type   | Time (Lower is Better) |
| :---------- | :--------------------- |
| **foreach** | ~5.0s                  |
| **for**     | ~10.0s                 |
| **while**   | ~11.0s                 |

## Installation

Ry comes with a built-in installer for Linux and Windows systems.

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/johnryzon123/Rylang.git](https://github.com/johnryzon123/Rylang.git)
   cd Ry
   ```
2. **Build and Install:**

```bash
chmod +x scripts/install.sh
./scripts/build.sh
./scripts/install.sh
```

# Usage

**The REPL**
Simply type `ry` to enter the interactive shell.

  ```bash
  $ ry
  Ry (Ry's for you) REPL
  >> out(0 to 10)
  [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
  ```

**Running a Script**
  ```bash
  $ ry run script.ry
  ```

# Examples
```
# Range-based iteration
foreach data i in 0 to 100 {
    out(i)
}

# Error reporting example
{
    out("Hello World")
    # Missing brackets or typos will be caught with helpful red pointers!
}
```

# License
This project is licensed under the MIT License - see the `LICENSE` file for details.