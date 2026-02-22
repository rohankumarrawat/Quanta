@ ==========================================
@  QUANTA COMPREHENSIVE STRING TEST SUITE
@  Dynamic (string), Static (string[N]), and var typing
@ ==========================================

print("========== 1. DYNAMIC STRING (string) - Basics ==========");
string d1 = "Quanta";
string d2 = "Language";
print("d1:", d1);
print("d2:", d2);
print("len(d1):", len(d1));
print("len(d2):", len(d2));

print("========== 2. DYNAMIC STRING - All 1-arg functions ==========");
string dyn = "  HeLLo WoRLd  ";
print("Original:", dyn);
print("upper:", upper(dyn));
print("lower:", lower(dyn));
print("reverse:", reverse(dyn));
print("strip:", strip(dyn));
print("lstrip:", lstrip(dyn));
print("rstrip:", rstrip(dyn));
print("capitalize:", capitalize(lower(dyn)));
print("title:", title(dyn));
print("isupper(HELLO):", isupper("HELLO"));
print("islower(hello):", islower("hello"));
print("isalpha(abc):", isalpha("abc"));
print("isdigit(123):", isdigit("123"));
print("isspace(   ):", isspace("   "));
print("isalnum(a1):", isalnum("a1"));

print("========== 3. DYNAMIC STRING - 2-arg and 3-arg ==========");
string text = "foo bar foo baz foo";
print("find(foo):", find(text, "foo"));
print("count(foo):", count(text, "foo"));
print("startswith(foo):", startswith(text, "foo"));
print("endswith(foo):", endswith(text, "foo"));
print("replace:", replace(text, "foo", "x"));

print("========== 4. STATIC STRING (string[N]) - Basics ==========");
string[16] s1 = "Sensor_A1";
string[8] s2 = "Hi";
string[32] s3 = "Embedded";
print("s1:", s1);
print("s2:", s2);
print("s3:", s3);
print("len(s1):", len(s1));
print("len(s2):", len(s2));
print("len(s3):", len(s3));

print("========== 5. STATIC STRING - Overflow/Truncation ==========");
string[6] small = "ABCDEFGH";
print("Truncated (6):", small);
string[4] tiny = "ab";
print("Short (4):", tiny);

print("========== 6. STATIC STRING - All 1-arg functions ==========");
string[20] st = "  QUANTA  ";
print("static upper:", upper(st));
print("static lower:", lower(st));
print("static strip:", strip(st));
print("static len:", len(st));
print("static reverse:", reverse(st));

print("========== 7. STATIC STRING - 2-arg and 3-arg ==========");
string[24] buf = "hello world hello";
print("static find(world):", find(buf, "world"));
print("static count(hello):", count(buf, "hello"));
print("static startswith(hello):", startswith(buf, "hello"));
print("static replace:", replace(buf, "hello", "hi"));

print("========== 8. VAR TYPING - Inferred string ==========");
var v1 = "Quanta";
var v2 = "Var";
print("var v1:", v1);
print("var v2:", v2);
print("len(v1):", len(v1));
print("upper(v1):", upper(v1));
print("lower(v2):", lower(v2));
print("strip(var):", strip("  var  "));

print("========== 9. VAR TYPING - All functions on var ==========");
var vx = "  TeST  ";
print("var strip:", strip(vx));
print("var capitalize:", capitalize(lower(vx)));
print("var title:", title("one two"));
var vy = "aa bb aa";
print("var find(aa):", find(vy, "aa"));
print("var count(aa):", count(vy, "aa"));
print("var replace:", replace(vy, "aa", "z"));

print("========== 10. CONCATENATION - Dynamic + Dynamic ==========");
string a = "Hello";
string b = "World";
string c = a + " " + b;
print("a+b:", c);
print("len(c):", len(c));

print("========== 11. CONCATENATION - Static + Dynamic ==========");
string[8] fix = "AB";
string dynPart = "CD";
string concat1 = fix + dynPart;
print("static+dynamic:", concat1);
string concat2 = "X" + fix;
print("literal+static:", concat2);

print("========== 12. CONCATENATION - var + string ==========");
var vc1 = "Prefix";
string vc2 = "Suffix";
string vc3 = vc1 + "_" + vc2;
print("var+string:", vc3);

print("========== 13. COMPARISON - Dynamic ==========");
string x = "Apple";
string y = "Banana";
print("Apple==Apple:", x == "Apple");
print("Apple!=Banana:", x != y);
print("Apple<Banana:", x < y);
print("Banana>Apple:", y > x);

print("========== 14. COMPARISON - Static ==========");
string[10] sx = "AA";
string[10] sy = "BB";
print("static AA==AA:", sx == "AA");
print("static AA!=BB:", sx != sy);

print("========== 15. COMPARISON - var string ==========");
var cmp1 = "same";
var cmp2 = "same";
print("var same==same:", cmp1 == cmp2);
print("var same!=diff:", cmp1 != "diff");

print("========== 16. EDGE - Empty string ==========");
string empty = "";
print("len(empty):", len(empty));
print("empty strip:", strip(""));
print("find(empty in x):", find("abc", ""));
print("count(x, empty):", count("ab", ""));

print("========== 17. EDGE - Single character ==========");
string single = "X";
print("single:", single);
print("upper(single):", upper(single));
print("len(single):", len(single));
print("find(X in XXX):", find("XXX", "X"));

print("========== 18. EDGE - Static exact fit ==========");
string[5] exact = "four";
print("exact (5):", exact);
print("len(exact):", len(exact));

print("========== 19. MIXED - Function return on static input ==========");
string[12] mix = "  trim me  ";
string mixedResult = strip(mix);
print("strip(static):", mixedResult);
print("replace(static, ...):", replace(mix, "me", "you"));

print("========== 20. LOOP + Dynamic string ==========");
int i = 0;
string builder = "";
loop (i < 3) {
    builder = builder + "x";
    i = i + 1;
}
print("loop builder:", builder);

print("========== 21. isupper/islower on static and var ==========");
string[10] up = "HELLO";
var lo = "world";
print("isupper(static HELLO):", isupper(up));
print("islower(var world):", islower(lo));
print("isalpha(static):", isalpha(up));
print("isdigit(123):", isdigit("123"));

print("========== 22. startswith/endswith on all types ==========");
string[20] sw = "QuantaLang";
print("startswith(Quanta):", startswith(sw, "Quanta"));
print("endswith(Lang):", endswith(sw, "Lang"));
var vsw = "prefix_suffix";
print("var startswith(prefix):", startswith(vsw, "prefix"));
print("var endswith(suffix):", endswith(vsw, "suffix"));

print("========== ALL STRING TESTS COMPLETED =========");
