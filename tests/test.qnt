@ ==========================================
@  QUANTA COMPILER MASTER TEST SUITE
@ ==========================================



print("--- 1. Variable Assignment & Types ---");

@ Implicit Declaration (Auto-detection)
x = 100;            
print(x);           
print(type(x));     

pi = 3.14159;
print(pi);          
print(type(pi));    

msg = "Hello Quanta";
print(msg);         
print(type(msg));   

flag = true;
print(flag);        
print(type(flag));  

@ Explicit Declaration
var y = 50;
print(y);           

@ Reassignment
x = 200;
print(x);           


print("--- 2. Arithmetic Operations ---");

a = 10;
b = 3;

print(a + b);       @ Add: 13
print(a - b);       @ Sub: 7
print(a * b);       @ Mul: 30
print(a / b);       @ Div: 3 (Integer Division)

f1 = 10.5;
f2 = 2.5;
print(f1 / f2);     @ Float Div: 4.200000

@ Mixed Math (Auto-Promotion to Float)
print(10 + 2.5);    @ Expected: 12.500000


print("--- 3. Order of Operations (BODMAS) ---");

print(2 + 3 * 4);         @ Expected: 14 (Not 20)
print((2 + 3) * 4);       @ Expected: 20
print(10 - 10 / 2);       @ Expected: 5
print(5 + 5 == 10);       @ Expected: 1 (True)


print("--- 4. Logic & Comparisons ---");

val = 10;
print(val == 10);   @ Expected: 1 (True)
print(val < 20);    @ Expected: 1 (True)
print(val > 20);    @ Expected: 0 (False)


print("--- 5. Control Flow (If / Elif / Else) ---");

check = 50;

@ Case 1: Simple If
if (check == 50) {
    print("Check is 50");
}

@ Case 2: If-Else
if (check > 100) {
    print("Big");
} else {
    print("Small");      
}

@ Case 3: If-Elif-Else Chain
score = 75;

if (score > 90) {
    print("Grade A");
} elif (score > 70) {
    print("Grade B");    
} elif (score > 50) {
    print("Grade C");
} else {
    print("Fail");
}

@ Case 4: Nested If (Deep Logic)
nested_val = 10;
if (nested_val == 10) {
    print("Layer 1");
    if (nested_val > 5) {
        print("Layer 2");
    }
}


print("--- 6. Scoping (Blocks) ---");

global_var = 99;
if (1 == 1) {
    local_var = 1;
    print(global_var + local_var); @ Expected: 100
}

print("--- 7. Introspection ---");

my_int = 12345;
print(bytesize(my_int));  @ Expected: 8 (64-bit default)
print(type(my_int));      @ Expected: int

print("--- TEST COMPLETE ---");

print(11111); @ Separator

int a = 10;
print(a);     @ 10

@ Test Postfix (a++) -> Prints OLD value (10), then a becomes 11
print(a++);   @ 10 
print(a);     @ 11

@ Test Prefix (++a) -> Increments to 12, then prints NEW value (12)
print(++a);   @ 12
print(a);     @ 12

@ Test Decrement
print(a--);   @ 12
print(a);     @11
b=30

int a = 5;
int b = 4;
int result = a++ * b--;

print(result); @ Expected: 20
print(a);      @ Expected: 6
print(b);      @ Expected: 3


if(result>20){
    print("Yes")
}
else{
    print("Not")
}


print("loop Test --------------------loop Test")
int i = 0;

print(111); 

loop (i < 5) {
    print(i);
    i++;
}

print(222); 



print(1111);        
int a = 10;
int b = 20;
int result = a + b; 
print(result);



     
if (result < 50) {
    print(1);       
} else {
    print(0);       
}

if (result < 10) {
    print(0);       
} else {
    print(1);      
}



print(3333);    
int i = 0;

loop (i < 3) {
    print(8);      
    print(i);       
    i = i + 1;     
}
print(9999);      

int a=1;
a="hello"
int5 a=1



print("--- Testing Greater Than or Equal (>=) ---");

int a = 10;
int b = 10;
int c = 5;

if (a >= b) {
    print("PASS: 10 >= 10 is True"); 
} else {
    print("FAIL: 10 >= 10 should be True");
}

loop (i < 3) {
   
    print(i);       
    i = i + 1;     
}

if (a >= c) {
    print("PASS: 10 >= 5 is True");
} else {
    print("FAIL: 10 >= 5 should be True");
}

if (c >= a) {
    print("FAIL: 5 >= 10 should be False");
} else {
    print("PASS: 5 >= 10 is False");
}
a=5
if a%2==0{
    print("even");
}
else{
    print("odd");
}

print(a%2)


print("--- TEST START ---");

int x = 10;
int y = 20;

print("Addition:")
print(x + y)

@ @ --- 2. Control Flow (If / Else) ---
print("Logic Check:");
if (x < y) {
    print("X is smaller (Correct)");
} else {
    print("X is bigger (Wrong)");
}

@ --- 3. Loops ---
print("Counting Loop:");
int i = 0;
loop (i < 3) {
    print(i);
    i++;
}

int getInt() {
    print("Testing Int -> Int (Expect 42):");
    return 42;
}

@ 2. Valid: Implicit Cast (Int8 -> Int32)
@ Your logic handles this via CreateIntCast
int32 castSmallToBig() {
    print("Testing Int8 -> Int32 (Expect 100):");
    int8 tiny = 100;
    return tiny; 
}

@ 3. Valid: Implicit Cast (Int64 -> Int8 Truncation)
@This tests if your compiler safely chops off bits
int8 castBigToSmall() {
    print("Testing Int64 -> Int8 (Expect -56 from 200 overflow):");
    return 200; @ 200 fits in uint8, but overflows int8 (-56)
}

@ 4. Valid: Float to Int (Truncation)
@ Uses your CreateFPToSI logic
int floatToInt() {
    print("Testing Float -> Int (Expect 3):");
    return 3.14159;
}

@ 5. ERROR TEST: String to Int
@ This should TRIGGER your red error message and set HasError = true
@ int breakTheCompiler() {
@     print("Testing String -> Int (Expect ERROR):");
@     return "This should fail";
@ }

@ --- MAIN EXECUTION ---
print("--- STARTING TYPE TESTS ---");

print(getInt());
print(castSmallToBig());
print(castBigToSmall());
print(floatToInt());

@ Uncomment this line to see the compiler error!
@ print(breakTheCompiler());

print("--- TESTS FINISHED ---");


@ --- 1. Integer Tests ---
int getInt() {
    print("Testing Int -> Int (Expect 42):");
    return 42;
}

@ Implicit Cast: Small to Big
int32 castSmallToBig() {
    print("Testing Int8 -> Int32 (Expect 100):");
    int8 tiny = 100;
    return tiny; 
}

@ Implicit Cast: Truncation (Big to Small)
int8 castBigToSmall() {
    print("Testing Int64 -> Int8 (Expect -56 due to overflow):");
    return 200; 
}

@ --- 2. Float Tests ---
@ Valid: Float -> Int (Truncation)
int floatToInt() {
    print("Testing Float -> Int (Expect 3):");
    return 3.14159;
}

@ Valid: Float Return
float getFloat() {
    print("Testing Float Return (Expect 12.500000):");
    return 12.5;
}

@ --- 3. String Tests ---
@ Valid: String Return
string getGreeting() {
    print("Testing String Return:");
    return "Hello from Quanta!";
}

@ --- 4. ERROR TEST (Uncomment to break) ---
@ int breakTheCompiler() {
@     return "This should fail";
@ }

@ --- MAIN EXECUTION ---
print("--- STARTING EXTENDED TESTS ---");

print(getInt());
print(castSmallToBig());
print(castBigToSmall());
print(floatToInt());

@ Note: %f support in printf is needed for getFloat() to print nicely, 
@ but for now it might print as a raw double bits or cast to int depending on your print() logic.
print(getFloat()); 

print(getGreeting());

print("--- TESTS FINISHED ---");

print(1,2)



@ --- 1. Standard Function (Explicit Types) ---
int add(a, b) {
    
   
    return a + b;
}

@ --- 2. Function with Default Value (Implicit Types) ---
@ 'base' defaults to int. 'exp' defaults to 2 if not provided.

@ --- 3. Function with All Defaults ---
 int  getArea(width = 10, height = 20) {
    return width * height;
}

@ --- 4. String Argument Test ---


@ --- MAIN EXECUTION ---
print("--- TEST 1: Standard Args (10 + 20) ---");
print(add(10,100));


@ print("--- TEST 4: Multiple Defaults ---");
print(getArea());          @ Uses 10, 20 -> 200
print(getArea(5));         @ Uses 5, 20  -> 100
print(getArea(5, 5));      @ Uses 5, 5   -> 25

@ print("--- TEST 5: String Args ---");
string greet(name=10,rollno=20) {
    print(name,rollno);
    return "Done";
}
print(greet(rollno=30,name=40));


