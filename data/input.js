function kek() {
    let i = 0;
    let sum = 0;
    while (i < 100000) {
        let arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
        let j = 0;
        while (j < 10) {
            sum = sum + arr[j];
            j = j + 1;
        }
        let a = {
            b : 1,
            c : 2
        };
        sum = sum + a.b + a.c;
        i = i + 1;
    }

    return sum;
}
