function test() {
    let arr = [
        1,
        [1],
        2,
        [1, 2],
        3,
        [1, 2, 3],
        4,
        [1, 2, 3, 4]
    ];

    let sum = 0;
    let i = 0;

    while (i < len(arr)) {
        let l = len(arr[i]);
        if (l == -1) {
            sum = sum + arr[i];
        } else {
            let j = 0;
            while (j < l) {
                sum = sum + arr[i][j];
                j = j + 1;
            }
        }
        i = i + 1;
    }

    return sum;
}
