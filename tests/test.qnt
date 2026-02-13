@ ==========================================
@ QUANTA COMPILER - COMPREHENSIVE TEST SUITE
@ ==========================================

'''
This is a multi-line comment.
It tests that the lexer correctly skips blocks of text.
'''

print("--- 1. PRIMITIVE TYPES & DECLARATIONS ---");
var i = 42;             @ Integer
var f = 3.14;           @ Float
var b = true;           @ Bool (prints 1)
var c = 'A';            @ Char
var s = "Hello Quanta"; @ String

print(i);
print(f);
print(b);
print(c);
print(s);



print("--- 2. ARITHMETIC & MIXED MATH ---");
x = 10;
y = 20;
print(x + y);       @ Addition (30)
print(x - y);       @ Subtraction (-10)
print(y * 2);       @ Multiplication (40)
print(y / x);       @ Division (2)






@ Float Math (Mixed types should auto-cast to float)
f_res = 10 + 5.5;
print(f_res);       @ 15.500000




print("--- 3. INCREMENT & DECREMENT (NEW) ---");
count = 10;

@ Postfix: Use value THEN update
print(count++);     @ Should print 10
print(count);       @ Should print 11




@ Prefix: Update THEN use value
print(++count);     @ Should print 12
print(count);       @ Should print 12




@ Decrement
print(count--);     @ Should print 12
print(count);       @ Should print 11
print(--count);     @ Should print 10



print("--- 4. LOGIC & COMPARISONS ---");
print(10 > 5);      @ 1 (True)
print(10 < 5);      @ 0 (False)
print(10 == 10);    @ 1 (True)
print(5.5 == 5.5);  @ 1 (True - Float comparison)






print("--- 5. INTROSPECTION (sizeof / type) ---");
print(type(i));     @ int

print(type(f));     @ float
print(type(s));     @ string




print("--- 6. CONTROL FLOW ---");
val = 50;

if (val > 100) {
    print("Big");
} elif (val > 10) {
    print("Medium"); @ This should print
} else {
    print("Small");
}

print("--- 7. OPTIMIZER TEST (AUTO-SWITCH) ---");
@ The backend should detect this dense chain and 
@ convert it into a Jump Table or Constant Fold it.

opt_val = 3;

if (opt_val == 1) {
    print("Case 1");
} elif (opt_val == 2) {
    print("Case 2");
} elif (opt_val == 3) {
    print("Case 3: Optimized!"); @ Expected
} elif (opt_val == 4) {
    print("Case 4");
} else {
    print("Default");
}

print("--- TEST COMPLETE ---");

a = 2;
b = 10;

@ Step-by-Step Execution:
@ 1. ++a: 'a' becomes 3. Value used is 3.
@ 2. b--: Value used is 10. 'b' becomes 9.
@ 3. Multiply: 3 * 10 = 30.

result = a++ * b--;

print(result); @ Should be 30
print(a);      @ Should be 3
print(b);      @ Should be 9
