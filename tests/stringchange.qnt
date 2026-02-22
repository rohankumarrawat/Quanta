@ tests/stringchange.qnt

@ ==============================================================================
@ QUANTA STRING MEMORY & MUTABILITY GUIDE
@ ==============================================================================
@ In Quanta, strings are natively represented as C-style character pointers (i8*).
@ Because Quanta compiles directly to native machine code via LLVM, it inherits
@ the strict memory safety rules of the operating system.
@
@ This file demonstrates the 3 ways strings are stored, bounded, and modified.
@ ==============================================================================

print("--- 1. STACK STRINGS (MUTABLE) ---");
@ Syntax: string[SIZE] name = "text"
@ Memory: Stack Frame (Automatic allocation)
@ Behavior: Quanta executes a hardware-level memory copy to clone the string 
@           literal into a fixed-size, safe buffer right on your processor's stack.
@ Lifespan: Automatically destroyed when the function/block ends. No memory leaks!

string[15] stack_str = "hello stack";
print("Before:", stack_str);

@ Because it's on the stack, we own the memory, so we can freely mutate characters:
stack_str[0] = 'H';
stack_str[-1] = 'K'; 
print("After :", stack_str);
print("");


print("--- 2. HEAP STRINGS (MUTABLE) ---");
@ Syntax: string name = function_returning_string()
@ Memory: Heap (Dynamic allocation via malloc)
@ Behavior: When you call modifiers like .upper() or .replace(), Quanta dynamically 
@           requests raw memory from the OS to build the new string.
@ Lifespan: AutoFree detects heap allocations and manages the memory cleanup.

string heap_str = "hello heap".upper();
print("Before:", heap_str);

@ Since this memory was dynamically allocated for us, we can safely overwrite it:
heap_str[0] = 'h';
heap_str[-1] = 'P';
print("After :", heap_str);
print("");


print("--- 3. RAW LITERALS (IMMUTABLE, READ-ONLY) ---");
@ Syntax: string name = "text"
@ Memory: .rodata (Read-Only Data segment of the compiled binary)
@ Behavior: This simply creates a pointer to the hardcoded text baked directly
@           into the executable file itself.
@ Lifespan: Exists for the entire duration of the program.

string readonly_str = "hello static";
print("Pointer points to:", readonly_str);

@ 🚨 CRITICAL ERROR PREVIEW 🚨
@ If you uncomment the line below and run this program, it will CRASH.
@ Why? Because the OS strictly forbids writing to the read-only section of 
@ the compiled application. Modifying `.rodata` will trigger a hardware-level 
@ "Bus error" (Signal 10).
@
readonly_str[0] = 'H'; 
print(readonly_str) 
