@ ==========================================
@  QUANTA STRING LOOP, INDEXING & SLICING
@  loop i in string, s[i], s[start:end], s[start:end:step]
@ ==========================================

print("========== 1. loop i in string (index) ==========");
string s = "Hi";
print("Indices of Hi:");
loop i in s {
    print(i);
}

print("========== 2. loop i in string + s[i] (character) ==========");
string word = "Quanta";
print("Characters of Quanta:");
loop i in word {
    print(word[i]);
}

print("========== 3. Static string loop ==========");
string[10] fix = "AB";
print("Indices of static AB:");
loop i in fix {
    print(i);
}
print("Chars of static AB:");
loop i in fix {
    print(fix[i]);
}

print("========== 4. Indexing s[i] ==========");
string t = "abc";
print("t[0]:", t[0]);
print("t[1]:", t[1]);
print("t[2]:", t[2]);

print("========== 5. Slicing s[start:end] ==========");
string u = "Hello";
print("u[0:2]:", u[0:2]);
print("u[1:4]:", u[1:4]);
print("u[0:5]:", u[0:5]);
print("u[2:2]:", u[2:2]);

print("========== 6. Slicing with step s[start:end:step] ==========");
string v = "0123456789";
print("v[0:10:2]:", v[0:10:2]);
print("v[1:8:3]:", v[1:8:3]);

print("========== 7. var string loop ==========");
var vs = "xyz";
print("loop over var xyz:");
loop i in vs {
    print(vs[i]);
}

print("========== 8. Index + slice mixed ==========");
string w = "Embedded";
print("w[0:3]:", w[0:3]);
print("w[3:8]:", w[3:8]);
print("len(w):", len(w));
loop i in w {
    print(w[i]);
}

print("========== 9. Empty and short ==========");
string short = "X";
print("short[0]:", short[0]);
print("short[0:1]:", short[0:1]);
loop i in short {
    print(i);
    print(short[i]);
}

print("========== 10. Slice then loop ==========");
string orig = "QuantaLang";
string part = orig[0:6];
print("part (orig[0:6]):", part);
print("loop over part:");
loop i in part {
    print(part[i]);
}

print("========== ALL STRING LOOP TESTS DONE =========");
