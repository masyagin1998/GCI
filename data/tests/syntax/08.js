function test() {
    let n = 100;

    let i = 2;

    let count = 0;

    while (i <= n) {
        let j = 2;
        let flag = 0;
        while (j < i) {
            if (i % j == 0) {
                flag = 1;
                break;
            }
            j = j + 1;
        }
        if (flag == 0) {
            count = count + 1;
        }
        i = i + 1;
    }

    return count;
}
