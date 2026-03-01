bool a = true
bool b = false

if (a and b) {
    print("A AND B is true")
} else {
    print("A AND B is false")
}

if (a or b) {
    print("A OR B is true")
} else {
    print("A OR B is false")
}

if (5 > 3 and 10 < 20) {
    print("Complex AND is true")
}

if (b or false or true) {
    print("Chained OR is true")
}
