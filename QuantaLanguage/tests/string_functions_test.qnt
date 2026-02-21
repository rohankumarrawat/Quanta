@ Test all new string functions
print("--- strip/lstrip/rstrip ---");
string s = "  hello world  ";
print("strip:", strip(s));
print("lstrip:", lstrip(s));
print("rstrip:", rstrip(s));

print("--- capitalize / title ---");
print("capitalize:", capitalize("hello"));
print("title:", title("hello world"));

print("--- isalpha, isdigit, isspace, isalnum ---");
print("isalpha(abc):", isalpha("abc"));
print("isalpha(a1):", isalpha("a1"));
print("isdigit(123):", isdigit("123"));
print("isdigit(12a):", isdigit("12a"));
print("isspace(   ):", isspace("   "));
print("isalnum(ab1):", isalnum("ab1"));

print("--- find, count, startswith, endswith ---");
string t = "foo bar foo baz foo";
print("find(foo):", find(t, "foo"));
print("count(foo):", count(t, "foo"));
print("startswith(foo):", startswith(t, "foo"));
print("endswith(foo):", endswith(t, "foo"));
print("endswith(baz):", endswith(t, "baz"));

print("--- replace ---");
print("replace:", replace("hello world", "world", "quanta"));

print("--- ALL STRING FUNCTION TESTS DONE ---");
