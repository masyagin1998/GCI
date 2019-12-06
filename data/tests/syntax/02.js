function test() {
    let i = 1;
    let res = 0;

    while (i <= 100000) {
        i = i + 1;
        if (i == 40) {
            break;
        }
        if (i % 2 == 0) {
            continue;
        }
        if (i < 10) {
            if (i >= 5) {
                res = res + 1;
            } else {
                res = res + 2;
            }
        } else if (i < 20) {
            if (i >= 15) {
                res = res + 3;
            } else {
                res = res + 4;
            }
        } else {
            res = res + 5;
        }
    }

    return res;
}
