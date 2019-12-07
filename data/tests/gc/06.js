function test() {
    let a = {};
    let i = 0;
    while (i < 10) {
        a.a = a;
        i = i + 1;
    }

    a.b = 12345;

    return a.a.a.a.a.a.a.a.a.a.b;
}
