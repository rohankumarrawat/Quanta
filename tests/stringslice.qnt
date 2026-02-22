@ ==========================================
@  QUANTA STRING SLICE TEST - stringslice.qnt
@  s[start:end], s[start:end:step]
@  Negative indices (-1 = last) and negative step (reverse)
@ ==========================================

print("========== 1. Basic slice s[start:end] ==========");
string a = "Hello";
print("a[0:2]:", a[0:2]);
print("a[1:4]:", a[1:4]);
print("a[0:5]:", a[0:5]);
print("a[0:1]:", a[0:1]);
print("a[4:5]:", a[4:5]);

print("========== 2. Negative indexing s[i] ==========");
string b = "Quanta";
print("b[-1] (last):", b[-1]);
print("b[-2] (2nd last):", b[-2]);
print("b[-6] (first):", b[-6]);

print("========== 3. Negative slice (from end) ==========");
string c = "Hello";
print("c[0:-1] (all but last):", c[0:-1]);
print("c[0:-2] (all but last 2):", c[0:-2]);
print("c[-4:-1] (3 chars from end):", c[-4:-1]);
print("c[-5:5] (full):", c[-5:5]);

print("========== 4. Negative step (reverse slice) ==========");
string d = "01234";
print("d[4:0:-1] (reverse):", d[4:0:-1]);
print("d[4:-1:-1] (reverse full):", d[4:-1:-1]);
string e = "abcde";
print("e[4:1:-2]:", e[4:1:-2]);

print("========== 5. Full reverse via slice ==========");
string f = "Quanta";
int L = len(f);
print("f reversed:", f[L-1:-1:-1]);

print("========== 6. Empty and single-char slice ==========");
print("a[2:2]:", a[2:2]);
print("a[3:3]:", a[3:3]);
string single = "X";
print("single[0:1]:", single[0:1]);
print("single[-1]:", single[-1]);

print("========== 7. Slice with positive step ==========");
string g = "0123456789";
print("g[0:10:2]:", g[0:10:2]);
print("g[1:9:3]:", g[1:9:3]);

print("========== 8. Static string + negative ==========");
string[16] fix = "QuantaLang";
print("fix[0:6]:", fix[0:6]);
print("fix[-4:10]:", fix[-4:10]);
print("fix[-1]:", fix[-1]);

print("========== 9. var string slice ==========");
var v = "Embedded";
print("v[0:3]:", v[0:3]);
print("v[-6:8]:", v[-6:8]);

print("========== 10. Slice then use ==========");
string orig = "abcdef";
string part = orig[2:5];
print("orig[2:5]:", part);
print("upper(part):", upper(part));

print("========== 11. Index vs slice ==========");
string w = "ABC";
print("w[0] (char):", w[0]);
print("w[-1] (char):", w[-1]);
print("w[0:1]:", w[0:1]);

print("========== ALL STRING SLICE TESTS DONE =========");
