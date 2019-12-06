function test() {
    let sum = 1;

    while (1) {
        sum = sum + sum * sum;

        if (sum > 1) {
            break;
        }
    }

    return sum;
}
