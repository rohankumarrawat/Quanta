@ tests/dynamic_list_test.qnt
print("--- Dynamic List Test ---");
int[] my_list = [5, 10, 15];

print("Length start:", my_list.len());
print("List[2] start:", my_list[2]);

my_list.push(50);
print("Length after push:", my_list.len());
print("List[3] should be 50:", my_list[3]);

int popped = my_list.pop();
print("Popped val:", popped);
print("Length after pop:", my_list.len());
