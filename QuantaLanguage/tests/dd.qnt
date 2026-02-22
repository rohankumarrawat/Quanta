@ @ ==========================================
@ @  QUANTA STRING & MEMORY TEST SUITE
@ @ ==========================================

@ print("--- 1. Basic Concatenation ---");
@ string first = "Hello";
@ string second = "World";

@ @ Test concatenating multiple strings (triggers malloc)
@ string greeting = first + " " + second + "!";
@ print(greeting);  @ Expected: Hello World!


@ print("--- 2. String Comparison (Equality) ---");
@ if (first == "Hello") {
@     print("PASS: 'Hello' == 'Hello'");
@ } else {
@     print("FAIL: Equality check broken.");
@ }

@ if (first != second) {
@     print("PASS: 'Hello' != 'World'");
@ } else {
@     print("FAIL: Inequality check broken.");
@ }


@ print("--- 3. Lexicographical (Alphabetical) Comparison ---");
@ string a = "Apple";
@ string b = "apple";

@ if (a < b) {
@     print("PASS: Apple comes before Banana nono");
@ }
@ else{
@     print("lol")
@ }

@ if (b > a) {
@     print("PASS: Banana comes after Apple");
@ }


@ print("--- 4. Auto-Free & Variable Reassignment ---");
@ @ This tests if the compiler handles loops and reassignment 
@ @ without crashing or leaking memory.
@ int i = 0;
@ string builder = "Start-";

@ loop (i < 5) {
@     @ Every time this loops, it mallocs a new string.
@     @ When the main function ends, your AutoFreeList should clean all of them up!
@     builder = builder + "x";
@     i++;
@ }
@ print(builder); @ Expected: Start-xxxxx


@ print("--- 5. Function Return (The Auto-Free Exception) ---");

@ string getDynamicString() {
@     string p1 = "Quanta ";
@     string p2 = "Language";
    
@     @ We allocate "Quanta Language" on the heap.
@     @ Your ReturnAST logic should ensure this specific string is NOT freed!
@     return p1 + p2; 
@ }

@ print("Result from function:");
@ print(getDynamicString()); @ Expected: Quanta Language

@ print("--- ALL STRING TESTS FINISHED ---");

@ ==========================================
@  QUANTA FINAL STRING & MEMORY TEST SUITE
@ ==========================================



@ @ print("--- 2. Boolean Functions Test ---");
@ @ string shout = "HELLO";
@ @ string whisper = "world";

@ @ if (isupper(shout)) {
@ @     print("PASS: isupper() detected uppercase.");
@ @ }

@ @ if (islower(whisper)) {
@ @     print("PASS: islower() detected lowercase.");
@ @ }

@ @ if (isupper("MixEd")) {
@ @     print("FAIL: isupper() broke.");
@ @ } else {
@ @     print("PASS: isupper() correctly rejected mixed case.");
@ @ }


@ @ print("--- 3. Concatenation & Auto-Free Test ---");
@ @ string part1 = "Smart";
@ @ string part2 = "Compiler";
@ @ @ This uses the Heap and the AutoFree tracker!
@ @ string combined = part1 + " " + part2;
@ @ print("Combined string:", combined);  @ Expected: Smart Compiler


@ @ print("--- 4. Heavy Loop Memory Test ---");
@ @ int i = 0;
@ @ string builder = "Start-";

@ @ This loop generates temporary strings. 
@ @ Your compiler will automatically free them behind the scenes!
@ loop (i < 5) {
@     builder = builder + "X";
@     i = i + 1;
@ }
@ print("Loop result:", builder);       @ Expected: Start-XXXXX


@ print("--- 5. Safe Return Memory Test ---");
@ string createMessage() {
@     string a = "Memory";
@     string b = "Safe";
    
@     @ Your ReturnAST handles this so it DOES NOT delete the returned string!
@     return a + " " + b;
@ }

@ print("Function returned:", createMessage()); @ Expected: Memory Safe

@ print("--- ALL TESTS FINISHED SUCCESSFULLY! ---");



@ ==========================================
@  QUANTA EMBEDDED STRING TEST SUITE
@ ==========================================

print("--- 1. Native Keywords Test ---");
string text = "Quanta";

@ Notice the comma separating the string literal and the function!
print("Original:", text);
print("Length:", len(text));          @ Expected: 6
print("Uppercase:", upper(text));     @ Expected: QUANTA
print("Lowercase:", lower(text));     @ Expected: quanta
print("Reversed:", reverse(text));    @ Expected: atnauQ

lol=reverse(text)
print("lol:",lol)


print("--- 1. Safe Stack Allocation ---");
@ This allocates exactly 16 bytes on the CPU Stack (no malloc!)
string[16] deviceName = "Sensor_A1";
print("Device:", deviceName);          @ Expected: Sensor_A1


print("--- 2. Overflow Protection Test ---");
@ The buffer is 8 bytes. The text is 15 bytes.
@ On a normal C compiler, this would cause a fatal SegFault.
@ Quanta will safely truncate it to 7 chars + 1 hidden null terminator!
string[8] smallBuffer = "SuperLongString";
print("Truncated:", smallBuffer);      @ Expected: SuperLo


print("--- 3. Coexisting with Dynamic Strings ---");
@ Proving that your old dynamic strings still work perfectly alongside them.
string heapString = "I am on the Heap";
string[20] stackString = "I am on the Stack";


print("Dynamic:", heapString);
print("Fixed:", stackString);
stackString[0]='3';
print(stackString[0])
print("--- ALL EMBEDDED TESTS COMPLETED ---");