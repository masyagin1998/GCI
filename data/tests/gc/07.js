function test() {
    let arr = [10, 9, 8, 7, 6, 5, 4, 3, 2, 1];

    let i = 0;
    while (i < len(arr)) {
        let j = 0;
        while (j < len(arr) - 1) {
            if (arr[j] > arr[j + 1]) {
                let tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
            j = j + 1;
        }
        i = i + 1;
    }

    i = 0;
    while (i < len(arr) - 1) {
        if (arr[i] > arr[i + 1]) {
            return 1;
        }
        i = i + 1;
    }

    return arr[0] + arr[1];
}
