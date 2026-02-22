@ tests/string_index_assign_test.qnt

@ Heap allocated string created via a method call natively supports mutation
string str = "hello".upper();
print("Before:", str);

str[0] = 'h';
print("After modifying start:", str);

str[-1] = 'o';
print("After modifying end:", str);
