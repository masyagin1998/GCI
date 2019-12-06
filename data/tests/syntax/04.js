function test() {
    let n = 10;
    let fact = 1;
    let i = 1;

    while (i <= n) {
        fact = fact * i;
        i = i + 1;
    }

    return fact;
}
