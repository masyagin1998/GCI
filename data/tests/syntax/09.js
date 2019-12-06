function test() {
    let sum = 0;
    let a = 1;
    if (1) {
        let a = 2;
        if (1) {
            let a = 3;
            if (1) {
                let a = 4;
                if (1) {
                    let a = 5;
                    sum = sum + a;
                }
                sum = sum + a;
            }
            sum = sum + a;
        }
        sum = sum + a;
    }
    sum = sum + a;

    return sum;
}
