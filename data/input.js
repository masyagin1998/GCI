function kek() {
    let a = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    a[0] = 0;
    a[9] = 0
    let i = -1;
    let sum = 0;
    while (i + 1 < 10) {
        sum = sum + a[i + 1];
        i = i + 1;
    }
    return sum;
}
