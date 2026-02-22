# Quanta Language — Update: Strings, Pool, Loops & Slicing

This document showcases the latest string and memory features in Quanta, including the **String Pool**, dynamic/static strings, built-in string functions, loop-over-string, indexing, and slicing (including negative indices and reverse).

---

## 1. String Pool (Introduced)

We have now **also introduced a String Pool** for efficient handling of string literals. Repeated use of the same literal reuses the same pooled pointer, reducing redundant allocations and keeping generated code lean. The pool is used internally for constant string expressions and print format strings.

---

## 2. Dynamic vs Static Strings

Quanta supports both **dynamic** and **static** (fixed-size) strings, so you can choose the right trade-off for embedded and general use.

| Type           | Syntax       | Storage   | Use case                          |
|----------------|--------------|-----------|-----------------------------------|
| Dynamic string | `string x = "..."` | Heap      | Unknown or variable length        |
| Static string | `string[N] x = "..."` | Stack (N bytes) | Fixed max length, no heap, truncation-safe |

**Examples:**

```quanta
string dyn = "Hello World";       @ heap
string[16] dev = "Sensor_A1";     @ stack, truncates if longer
string[8] buf = "SuperLong";      @ stores "SuperLo" (7 chars + null)
```

Static strings are **truncated** safely when the initializer is longer than the buffer (no overflow). Both types work with all string functions and operators below.

---

## 3. Built-in String Functions

### Single-argument (return string or int)

| Function        | Returns | Description                    |
|----------------|---------|--------------------------------|
| `len(s)`       | int     | Length of string               |
| `upper(s)`     | string  | Uppercase copy                 |
| `lower(s)`     | string  | Lowercase copy                 |
| `reverse(s)`   | string  | Reversed copy                  |
| `strip(s)`     | string  | Trim whitespace both ends      |
| `lstrip(s)`    | string  | Trim left whitespace           |
| `rstrip(s)`    | string  | Trim right whitespace          |
| `capitalize(s)`| string  | First char upper, rest lower   |
| `title(s)`     | string  | Title case                     |
| `isupper(s)`   | int 0/1 | All alphabetic chars uppercase?|
| `islower(s)`   | int 0/1 | All alphabetic chars lowercase?|
| `isalpha(s)`   | int 0/1 | All chars letters?             |
| `isdigit(s)`   | int 0/1 | All chars digits?              |
| `isspace(s)`   | int 0/1 | All chars whitespace?          |
| `isalnum(s)`   | int 0/1 | All chars alphanumeric?         |

### Two-argument

| Function              | Returns | Description                          |
|-----------------------|--------|--------------------------------------|
| `find(s, sub)`        | int    | First index of `sub` in `s`, else -1 |
| `count(s, sub)`       | int    | Number of non-overlapping `sub`      |
| `startswith(s, prefix)` | int 0/1 | `s` starts with `prefix`?          |
| `endswith(s, suffix)` | int 0/1 | `s` ends with `suffix`?            |

### Three-argument

| Function                | Returns | Description                    |
|-------------------------|--------|--------------------------------|
| `replace(s, old, new)`  | string | Replace all `old` with `new`   |

**Examples:**

```quanta
string a = "  Quanta  ";
print(strip(a));           @ "Quanta"
print(find("hello", "ll")); @ 2
print(replace("a-b-c", "-", "_")); @ "a_b_c"
```

All of these work with **dynamic**, **static**, and **var** strings.

---

## 4. Loop Over String

You can iterate over a string by **index** with a dedicated loop form. The loop variable is an integer index (0 to length−1); no extra string allocations.

**Syntax:**

```quanta
loop index_var in string_expr { body }
```

**Examples:**

```quanta
string s = "Hi";
loop i in s {
    print(i);        @ prints 0 then 1
}

string word = "Quanta";
loop i in word {
    print(word[i]);  @ prints each character: Q u a n t a
}

string[10] fix = "AB";
loop i in fix {
    print(fix[i]);   @ A then B
}
```

Works with dynamic `string`, static `string[N]`, and `var` strings.

---

## 5. String Indexing

Access a single character by index. Result is a **char** (one byte); no new string is allocated.

**Syntax:** `s[index]`

**Negative indexing:** `-1` is the last character, `-2` the second to last, and so on (same as common “from end” semantics).

**Examples:**

```quanta
string t = "abc";
print(t[0]);   @ 'a'
print(t[-1]);  @ 'c'
print(t[-2]);  @ 'b'
```

`print(s[i])` prints the character (using `%c` for single-byte values).

---

## 6. String Slicing

Slice a string with **start**, **end** (exclusive), and optional **step**. One allocation per slice (heap), tracked for auto-free.

**Syntax:**

- `s[start:end]`        — step 1
- `s[start:end:step]`   — step can be negative for reverse

**Negative indices:** `-1` = last character, `-2` = second to last, etc.

**Negative step:** reverse order. Use `s[len(s)-1:-1:-1]` for full reverse.

**Examples:**

```quanta
string u = "Hello";
print(u[0:2]);    @ "He"
print(u[0:-1]);   @ "Hell" (all but last)
print(u[-4:-1]);  @ "ell"

string v = "01234";
print(v[4:0:-1]);    @ "4321"
print(v[4:-1:-1]);   @ "43210" (full reverse)

string f = "Quanta";
int L = len(f);
print(f[L-1:-1:-1]); @ "atnauQ"
```

---

## 7. Dynamic Typing (`var`) with Strings

You can use **var** with string initializers; the type is inferred as string and all string operations (functions, loop, index, slice) apply.

**Examples:**

```quanta
var v = "Quanta";
print(upper(v));
print(len(v));
loop i in v { print(v[i]); }
print(v[0:3]);
print(v[-1]);
```

---

## 8. Concatenation & Comparison

- **Concatenation:** `+` — e.g. `a + " " + b`. Result is a new heap string (auto-freed).
- **Comparison:** `==`, `!=`, `<`, `>`, `<=`, `>=` — work with both dynamic and static strings.

---

## 9. Memory Summary

| Feature              | Where it lives | Notes                          |
|----------------------|----------------|--------------------------------|
| String Pool          | Constant data  | Reuse of literal pointers      |
| Static `string[N]`   | Stack          | Fixed buffer, truncation-safe  |
| Dynamic string value | Heap           | Auto-freed when no longer used |
| Loop index `i`       | Stack          | Integer, no allocation         |
| `s[i]`               | —              | Single byte load, no allocation|
| Slice result         | Heap           | One allocation per slice       |

---

## 10. Test Files

- **string.qnt** — Dynamic/static/var strings, all functions, concat, comparison.
- **stringloop.qnt** — Loop over string, indexing, slicing (positive indices and step).
- **stringslice.qnt** — Slicing with negative indices, negative step (reverse), full reverse.

Run examples:

```bash
./quanta tests/string.qnt
./quanta tests/stringloop.qnt
./quanta tests/stringslice.qnt
```

---

*Quanta: strings, String Pool, static/dynamic choice, and Python-like indexing and slicing for embedded and general use.*
