function test() {
    let a = 123456789;
    let sum = 0;
    while (a != 0) {
        sum = sum + a % 10;
        a = a / 10;
    }

    return sum;
}
