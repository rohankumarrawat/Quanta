<div align="center">
  <h1>The Quanta Programming Language</h1>
  <p><strong>A Memory-Safe, Intelligent, and Embedded-Ready Language</strong></p>
</div>

---

**Quanta** is a powerful modern programming language designed from the ground up to bridge the historically difficult gap between high-level scripting languages (like Python) and low-level systems languages (like C or Rust).

Built on top of a highly optimized **LLVM backend**, Quanta gives developers the flexibility to write clean, dynamic applications while allowing them to precisely lock down memory usage—making it the perfect language for both desktop software and constrained embedded devices.

## 🌟 Key Features

### 1. Dual String & Collection System
Quanta possesses a unique memory system that perfectly adapts to your execution environment:
- **Dynamic Heap Memory (`string`, `int[]`)**: Automatic memory management completely free of garbage collection pauses. Perfect for standard applications.
- **Static Stack Memory (`string[16]`, `int[5]`)**: Strictly sized, highly efficient data structures that prevent overflow and fragmentation. Essential for IoT devices and microcontrollers with tight hardware limits.

### 2. Intelligent Type Inference vs. Explicit Control
Quanta’s compiler figures things out seamlessly when you just want code to run, but stays out of your way when you require absolute control.
```quanta
@ Inferred, high-level approach (Heap allocations possible)
message = "Connecting to server..."
data_array = [10, 20, 30]

@ Explicit, embedded approach (Strict Stack memory only)
string[25] message = "Connecting to server..."
int8[3] data_array = [10, 20, 30]
```

### 3. Native Object-Oriented Standard Library
Quanta ships with a powerful native toolchain right out of the box. String and Collection manipulation feels organic with zero-overhead standard method calls.
```quanta
msg = "  system failure  "
clean = msg.strip().upper()

print(clean) @ Outputs: SYSTEM FAILURE
```

### 4. Effortless Control Flow
Quanta optimizes away bloated syntax. There is no `switch/case` complexity nor confusing iteration structures like `while` vs `do-while`.
Quanta optimizes standard nested `elif` branches intelligently behind the scenes, and provides a unified, powerful `loop` block for all iterations.

---

## 🛠️ Build & Installation

Quanta utilizes `CMake` and requires `LLVM 17+` to compile. 

### Windows (MSYS2 UCRT64)
Quanta provides a native build pipeline for Windows. Open an MSYS2 UCRT64 terminal:
```bash
# Install dependencies
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-llvm mingw-w64-ucrt-x86_64-zstd mingw-w64-ucrt-x86_64-zlib

# Build
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build .

# Run
./quanta.exe
```

### macOS (Homebrew Silicon)
```bash
# Install LLVM backend
brew install llvm@17
brew install zstd ncurses

# Build
mkdir build && cd build
cmake ..
make

# Run
./quanta
```

---

## 📖 Hello Quanta

Create a file named `hello.qnt`:

```quanta
@ Welcome to Quanta!
count = 0

loop (count < 5) {
    print("Execution cycle:", count)
    count++
}
```

Run it via the CLI:
```bash
quanta hello.qnt
```

---

## 📚 Documentation
For an exhaustive, deep-dive into the language, refer to the official [The Quanta Programming Language PDF / Markdown Guide](./docs/The_Quanta_Programming_Language.md) located in the `/docs` directory. It covers:
- Complete Language Syntax
- Strict Memory Mechanics & Variable Scoping
- Arithmetic & Casting Logic
- Embedded Software Best Practices

## 🤝 Contributing
Quanta is currently in active development. We welcome discussions on data structures, native hardware integrations, and language server protocols (LSP).

## 📄 License
Created by Rohan Kumar Rawat. All rights reserved.
