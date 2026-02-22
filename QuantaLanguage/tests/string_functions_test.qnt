@ tests/string_functions_test.qnt

print("--- Testing String Modification Methods ---");
string base = "  Hello Quanta Framework  ";
print("Original:", base);
print(".len():", base.len());
print(".upper():", base.upper());
print(".lower():", base.lower());
print(".strip():", base.strip());
print(".lstrip():", base.lstrip());
print(".rstrip():", base.rstrip());
print(".reverse():", base.reverse());

print("\n--- Testing String Case & Formatting ---");
string word = "quanta language";
print("Word:", word);
print(".capitalize():", word.capitalize());
print(".title():", word.title());

print("\n--- Testing String Inspection & Boolean Methods ---");
print("'HELLO'.isupper():", "HELLO".isupper());
print("'hello'.islower():", "hello".islower());
print("'abc'.isalpha():", "abc".isalpha());
print("'123'.isdigit():", "123".isdigit());
print("'   '.isspace():", "   ".isspace());
print("'A1b2'.isalnum():", "A1b2".isalnum());

print("\n--- Testing Search & Replace Methods ---");
string sentence = "The quick brown fox jumps over the lazy dog";
print("Sentence:", sentence);
print(".find('brown'):", sentence.find("brown"));
print(".count('o'):", sentence.count("o"));
print(".startswith('The'):", sentence.startswith("The"));
print(".endswith('cat'):", sentence.endswith("cat"));
print(".replace('fox', 'cat'):", sentence.replace("fox", "cat"));

print("\n--- Testing Method Chaining ---");
print("'   hello world   '.strip().upper().replace('WORLD', 'QUANTA'):", "   hello world   ".strip().upper().replace("WORLD", "QUANTA"));
