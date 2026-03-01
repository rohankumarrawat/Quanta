@ tests/string_array_test.qnt
@ Tests for static and dynamic string arrays

print("--- Fixed String Array ---");
string[3] names = ["Alice", "Bob", "Charlie"];
print(names[0]);   @ Alice
print(names[1]);   @ Bob
print(names[2]);   @ Charlie

@ Mutation
names[1] = "Dave";
print(names[1]);   @ Dave

print("--- Dynamic String List ---");
string[] words = ["hello", "world"];
print(words[0]);           @ hello
print(words[1]);           @ world
print(words.len());        @ 2

words.push("quanta");
print(words.len());        @ 3
print(words[2]);           @ quanta

string popped = words.pop();
print(popped);             @ quanta
print(words.len());        @ 2

print("--- Fixed char buffer unchanged ---");
string[50] buf = "original";
print(buf);                @ original
