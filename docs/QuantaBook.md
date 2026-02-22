# QuantaBook  
## The Quanta Programming Language — Serial Reference

**Quanta** is a general-purpose programming language built with a small set of keywords. It can be used for everyday scripting and application logic and can be **specialized for embedded systems** by relying on static (stack) allocation and bounded strings where appropriate. The compiler uses **LLVM** for code generation and ships with a **String Pool**, **auto-free** tracking for heap strings, and **fixed-size string buffers** for safe, deterministic memory on constrained targets.

This book documents the language in serial, chapter-wise form: lexical rules, types, expressions, control flow, strings (including indexing and slicing), functions, modules, memory model, and test suites. Design choices and alternatives are explained where relevant.

---

## Chapter 1 — Introduction and Design Goals

### 1.1 What is Quanta?

Quanta is a compiled, statically-typed (with optional type inference) language. Source files use the extension `.qnt`. The compiler produces an object file and links it with a small runtime (`quanta_lib.c`) to produce an executable.

### 1.2 Design Principles

- **Few keywords:** Control flow and types are expressed with a minimal reserved set (`if`, `else`, `elif`, `loop`, `return`, `void`, `import`, `var`, `string`, `int`, `float`, `bool`, `char`, plus string-function names and `in`).
- **General purpose:** Variables, arithmetic, conditionals, loops, functions, and strings support typical application and script-like programs.
- **Embedded-friendly:** Optional **static strings** (`string[N]`) and **stack-only** loop indexing and character access allow deterministic, bounded memory use without heap when desired.
- **Clear memory model:** The language distinguishes heap-allocated strings (dynamic `string`) from stack-allocated fixed buffers (`string[N]`), and uses a **String Pool** for literal reuse and **auto-free** for temporary heap strings.

### 1.3 Why These Alternatives?

| Choice | Alternative | Reason |
|--------|-------------|--------|
| LLVM for codegen | Custom bytecode / interpreter | Native performance, portability, and reuse of optimizations. |
| String Pool for literals | Allocate every literal | Fewer globals, less duplication, smaller code. |
| Explicit `string` vs `string[N]` | Only dynamic or only fixed | Programmer chooses heap vs stack; embedded can avoid heap. |
| Auto-free for string ops | Manual free / GC | No explicit free in language; reduces leaks from temporaries. |
| `loop (cond)` and `loop i in s` | Single loop form | Condition loop for general iteration; index loop for strings is stack-only and familiar. |
| Slice returns heap string | Slice as view / copy-on-write | Simpler semantics; one allocation per slice; auto-free tracks it. |

---

## Chapter 2 — Lexical Structure and Keywords

### 2.1 Comments

- **Line comments:** `@` starts a comment; the rest of the line is ignored.

### 2.2 Tokens

- **Identifiers:** Letters or `_`, then alphanumerics or `_`. Reserved words are recognized as keywords.
- **Numbers:** Integer literals (digits) and float literals (digits with optional `.` and fractional part).
- **Strings:** Double-quoted `"..."`. Escape sequences follow C-style where supported.
- **Symbols:** `+`, `-`, `*`, `/`, `%`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `(`, `)`, `{`, `}`, `[`, `]`, `,`, `;`, `:`, `.`, `*` (for `all`/import).

### 2.3 Keyword List (Serial View)

| Keyword | Role |
|---------|------|
| `print` | Built-in output |
| `if`, `elif`, `else` | Conditionals |
| `loop` | Condition loop or string-index loop |
| `in` | Used in `loop i in string` |
| `return` | Function return |
| `void` | No return value |
| `import` | Module import |
| `var` | Type-inferred declaration |
| `string` | Dynamic or fixed-size string type |
| `int`, `int8`, `int16`, … `int64` | Integer types |
| `float`, `float4`, `float8` | Float types |
| `bool` | Boolean type |
| `char` | Character type |
| `true`, `false` | Boolean literals |
| `len`, `upper`, `lower`, `reverse`, `strip`, `lstrip`, `rstrip` | String functions (1-arg) |
| `capitalize`, `title` | String functions (1-arg) |
| `isupper`, `islower`, `isalpha`, `isdigit`, `isspace`, `isalnum` | String predicates (1-arg) |
| `find`, `count`, `startswith`, `endswith` | String functions (2-arg) |
| `replace` | String function (3-arg) |
| `type`, `bytesize` | Introspection |
| `all` | Import wildcard (e.g. `import mod all`) |

Custom integer widths: `int` plus optional digit suffix (e.g. `int8`, `int32`). Same idea for `float` (e.g. `float8`).

---

## Chapter 3 — Types and Declarations

### 3.1 Type Summary

| Type | Declaration example | Storage / notes |
|------|----------------------|----------------|
| Integer | `int x = 0;`, `int8 a = 1;` | Stack; width by suffix (default 32/64 as per impl). |
| Float | `float f = 3.14;`, `float8 d = 1.0;` | Stack. |
| Boolean | `bool b = true;` | 1 bit / byte. |
| Character | `char c = 'A';` | 1 byte. |
| Dynamic string | `string s = "hi";` | Pointer to heap or literal; content may be heap. |
| Static string | `string[N] buf = "hi";` | N bytes on stack; truncation if init longer. |
| Inferred | `var v = "hi";` | Type from initializer (here, string). |

### 3.2 Why Static Strings?

- **Embedded:** Fixed buffers avoid heap and fragmentation; size known at compile time.
- **Safety:** Assignment from a longer value is **truncated** (and null-terminated), not overrun.
- **Determinism:** No runtime allocation for the buffer itself.

### 3.3 Variable Declarations

- **Explicit:** `type name = value;` (e.g. `string s = "a";`, `string[8] b = "long";`).
- **Inferred:** `var name = value;` (e.g. `var x = 10;`, `var t = "text";`).
- **Reassignment:** Same variable can be reassigned; type must match (or be coercible).

---

## Chapter 4 — String Pool and Memory Model

### 4.1 String Pool (Introduced)

We have now **also introduced a String Pool**. Repeated string literals and constant strings (e.g. format strings for `print`) are mapped to a single global constant in the generated module. This:

- Reduces duplicate constant data.
- Keeps generated IR smaller.
- Avoids repeated runtime allocation for identical literals.

The pool is used for literals in expressions and for internal format strings (e.g. `%d`, `%s`, `%c`, `%f`).

### 4.2 Auto-Free for Heap Strings

Temporary strings created by operations (concatenation, `upper`, `lower`, `reverse`, `strip`, `replace`, slice, etc.) are allocated on the heap and registered for **auto-free**. Before a function returns (normal or via `return`), the compiler emits code to free these pointers so the program does not leak them. Returned values are excluded so that returned strings remain valid.

### 4.3 Stack vs Heap (Serial View)

| Item | Where | Notes |
|------|--------|--------|
| All variable slots | Stack | Hold value or pointer. |
| `string[N]` buffer | Stack | N bytes; pointer in variable slot. |
| String literals | Constant / pool | Not heap. |
| Result of `a + b`, `upper(s)`, slice, etc. | Heap | Tracked for auto-free. |
| Loop index (e.g. `i` in `loop i in s`) | Stack | Integer only. |
| `s[i]` (indexing) | — | Single byte load; no allocation. |

This gives a clear, serial view: stack for variables and fixed buffers; heap only for dynamic string results, with automatic cleanup.

---

## Chapter 5 — Expressions and Operators

### 5.1 Arithmetic

- `+`, `-`, `*`, `/`, `%` with usual precedence. Integer division truncates; float division when a operand is float.
- **Unary minus:** `-expr` (e.g. `-1` for negative indices).

### 5.2 Comparison and Equality

- `==`, `!=`, `<`, `>`, `<=`, `>=`. For strings: lexicographic comparison. Result is integer 0/1.

### 5.3 Increment and Decrement

- `++`, `--` (prefix and postfix). Operand must be a variable. Postfix returns value before update; prefix returns value after update.

### 5.4 String Concatenation

- `+` between two string (pointer) operands: produces a new heap string (length `len(a)+len(b)+1`), copied and null-terminated. Result is tracked for auto-free.

### 5.5 Type Introspection

- `type(variable)` — returns a string naming the type (e.g. `"int"`, `"string"`).
- `bytesize(variable)` — returns size in bytes (useful for integers and layout).

---

## Chapter 6 — Built-in and String Functions (Complete List)

### 6.1 Output

- **`print(expr1, expr2, ...)`** — Prints each expression; types supported: int (including i8 as character when formatted with `%c`), float, string (pointer), bool. Multiple arguments printed in order; newline after the last.

### 6.2 String Functions — One Argument

| Function | Returns | Description |
|----------|--------|-------------|
| `len(s)` | int | Length of string (excluding null). |
| `upper(s)` | string | Uppercase copy (heap, auto-free). |
| `lower(s)` | string | Lowercase copy (heap, auto-free). |
| `reverse(s)` | string | Reversed copy (heap, auto-free). |
| `strip(s)` | string | Trim whitespace both ends (heap, auto-free). |
| `lstrip(s)` | string | Trim left whitespace. |
| `rstrip(s)` | string | Trim right whitespace. |
| `capitalize(s)` | string | First character upper, rest lower. |
| `title(s)` | string | Title case. |
| `isupper(s)` | int 0/1 | 1 if all alphabetic chars are uppercase. |
| `islower(s)` | int 0/1 | 1 if all alphabetic chars are lowercase. |
| `isalpha(s)` | int 0/1 | 1 if all chars are letters. |
| `isdigit(s)` | int 0/1 | 1 if all chars are digits. |
| `isspace(s)` | int 0/1 | 1 if all chars are whitespace. |
| `isalnum(s)` | int 0/1 | 1 if all chars are alphanumeric. |

### 6.3 String Functions — Two Arguments

| Function | Returns | Description |
|----------|--------|-------------|
| `find(s, sub)` | int | Index of first occurrence of `sub` in `s`, or -1. |
| `count(s, sub)` | int | Number of non-overlapping occurrences of `sub`. |
| `startswith(s, prefix)` | int 0/1 | 1 if `s` starts with `prefix`. |
| `endswith(s, suffix)` | int 0/1 | 1 if `s` ends with `suffix`. |

### 6.4 String Functions — Three Arguments

| Function | Returns | Description |
|----------|--------|-------------|
| `replace(s, old, new)` | string | New string with every `old` replaced by `new` (heap, auto-free). |

All string functions accept both dynamic and static (`string[N]`) and `var` strings; they operate on the underlying pointer (null-terminated buffer).

---

## Chapter 7 — Control Flow

### 7.1 Conditionals

- **`if (condition) { block }`**
- **`if (condition) { block } else { block }`**
- **`if (condition) { block } elif (condition) { block } else { block }`**

Condition is an expression; non-zero is true. Blocks are braced statement lists.

### 7.2 Condition Loop

- **`loop (condition) { block }`** — While-loop. Condition checked before each iteration; block runs while condition is true (non-zero).

### 7.3 String-Index Loop (Embedded-Friendly)

- **`loop index_var in string_expr { block }`** — Index variable runs from 0 to length−1. No per-character allocation; index is an integer on the stack. Suitable for embedded and for using with indexing: `s[i]`.

---

## Chapter 8 — String Indexing and Slicing

### 8.1 Indexing

- **`s[i]`** — Single character at index `i`. Result is a **char** (one byte). **Negative index:** `-1` is last character, `-2` second to last, etc. (implemented as `i + len(s)` when `i < 0`). No allocation; stack-only load.

### 8.2 Slicing

- **`s[start:end]`** — Substring from `start` to `end` (end exclusive). Indices 0-based. One heap allocation per slice; auto-free tracks it.
- **`s[start:end:step]`** — Same range, step. `step` positive: forward; negative: reverse order. Default step 1.

### 8.3 Negative Indices in Slices

- In slices, negative `start` or `end` mean “from end”: `-1` = last character position, `-2` = second to last, etc. Normalized to valid range before use.

### 8.4 Negative Step (Reverse Slice)

- **`s[high:low:-1]`** — Characters from index `high` down to (but not including) `low`. Example: full reverse with `s[len(s)-1:-1:-1]`.

---

## Chapter 9 — Functions and Modules

### 9.1 Function Definition

- **Return type**, **name**, **parameter list**, **body.**  
  Example: `string greet(string name) { return "Hi " + name; }`  
  Body is a sequence of statements; `return expr;` returns a value (or `return;` in void functions).

### 9.2 Parameters and Defaults

- Parameters can have default values (e.g. `int getArea(width = 10, height = 20)`). Call sites can pass positional or keyword arguments.

### 9.3 Import

- **`import module_name;`** or **`import module_name all;`** — Loads a `.qnt` module and registers its functions. Calls use `module_name.function_name(...)` or, with `all`, the function name alone if unique.

### 9.4 Main

- Execution starts at a function named **`main`**. The compiler may wrap top-level statements in an implicit `main` where applicable.

---

## Chapter 10 — Test Suites and What They Cover

The following test files exist and should be run to validate the language. Serial view of purpose and coverage:

### 10.1 `test.qnt`

- **Variables and types:** Implicit and explicit declarations, `var`, reassignment, `type()`, `bytesize()`.
- **Arithmetic and order of operations:** +, -, *, /, mixed int/float.
- **Comparisons and logic:** ==, <, >, >=, %, if/else.
- **Control flow:** if/elif/else, nested if, blocks.
- **Loops:** `loop (i < n)` with increment.
- **Increment/decrement:** Prefix and postfix `++`, `--`.
- **Functions:** Return types (int, float, string), parameter passing, default arguments, keyword arguments.
- **Type casts and overflow:** int8, int32, int64, float-to-int, optional error test (string to int).

### 10.2 `dd.qnt`

- **String concatenation and comparison:** ==, !=, lexicographic <, >.
- **Loop and reassignment:** Building string in loop; no leak.
- **Function return of string:** Returned string not freed before use.
- **Static (embedded) strings:** `string[16]`, `string[8]` with truncation (“SuperLongString” → “SuperLo”), coexistence with dynamic strings.

### 10.3 `string.qnt`

- **Dynamic strings:** Declare, len, all 1-arg and 2-arg and 3-arg string functions.
- **Static strings:** Same functions on `string[N]`; truncation; concat with dynamic.
- **var strings:** Inferred string type and all string operations.
- **Concatenation:** Dynamic+dynamic, static+dynamic, var+string.
- **Comparison:** Dynamic, static, var.
- **Edge cases:** Empty string, single char, exact-fit buffer, loop building string.

### 10.4 `stringloop.qnt`

- **`loop i in string`:** Index and character printing (dynamic, static, var).
- **Indexing:** `s[i]`, `print(s[i])` (char output).
- **Slicing:** `s[start:end]`, `s[start:end:step]` (positive only in this file).
- **Slice then loop:** Slice assigned to variable, then loop over it.

### 10.5 `stringslice.qnt`

- **Negative indexing:** `s[-1]`, `s[-2]`, etc.
- **Negative slice bounds:** `s[0:-1]`, `s[-4:-1]`, etc.
- **Negative step:** `s[4:0:-1]`, `s[4:-1:-1]`, full reverse `s[L-1:-1:-1]`.
- **Static and var:** Same slice/index semantics.

### 10.6 `string_functions_test.qnt`

- Focused run of strip, capitalize, title, isalpha, isdigit, isspace, isalnum, find, count, startswith, endswith, replace.

---

## Chapter 11 — Embedded Specialization (Summary)

Quanta can be **specialized for embedded systems** by how you use it:

1. **Prefer static strings:** Use `string[N]` for device IDs, fixed messages, and any bounded text. No heap for those buffers; truncation avoids overrun.
2. **Loop over string by index:** Use `loop i in s` and `s[i]` so that only an integer index is on the stack and no per-character strings are allocated.
3. **Indexing:** Use `s[i]` (and `s[-1]` etc.) for single-character access without allocation.
4. **Limit dynamic operations:** Minimize concatenation and slice operations in hot paths if heap is undesirable; use fixed buffers and indexing where possible.
5. **Same syntax everywhere:** The same small keyword set and the same string functions work in both general-purpose and embedded-style code; only the choice of types and constructs changes.

---

## Chapter 12 — Reference Tables

### 12.1 All Keywords (Alphabetical)

`all`, `bool`, `capitalize`, `char`, `count`, `else`, `elif`, `endswith`, `false`, `find`, `float`, `if`, `import`, `in`, `int`, `isalnum`, `isalpha`, `isdigit`, `islower`, `isspace`, `isupper`, `len`, `loop`, `lower`, `lstrip`, `print`, `replace`, `return`, `reverse`, `rstrip`, `startswith`, `string`, `strip`, `title`, `true`, `upper`, `var`, `void`.

### 12.2 Operators by Category

- **Arithmetic:** `+` `-` `*` `/` `%` (unary `-` for negative).
- **Comparison:** `==` `!=` `<` `>` `<=` `>=`.
- **Update:** `++` `--` (prefix and postfix).
- **Subscript:** `[ ]`, `[ : ]`, `[ : : ]`.

### 12.3 File and Build

- **Source extension:** `.qnt`
- **Compile:** `quanta <file.qnt>` (produces `output.o`, links with `quanta_lib.c`, runs `my_quanta_app` by default).
- **Runtime:** `quanta_lib.c` provides string helpers (upper, lower, reverse, strip, find, count, replace, slice, etc.) and is linked into the final executable.

---

*End of QuantaBook. Quanta: general-purpose language, few keywords, String Pool, static and dynamic strings, and embedded-friendly options.*
