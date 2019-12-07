function test() {
    let sum = 0;
    let a = 5;

    sum = sum + a;

    a = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    let i = 0;
    while (i < len(a)) {
        sum = sum + a[i];
        i = i + 1;
    }

    a = {
        a : 1,
        b : 2,
        c : 3,
    };

    sum = sum + a.a + a.b + a.c;

    return sum;
}
