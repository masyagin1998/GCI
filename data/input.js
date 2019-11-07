function fibonacci(){
    let i = 0;
    let a = 1;

    while (i < 10) {
        if (i % 2 == 0) {
            i = i + 1;
            continue;
        }

        i = i + 1;
        a = a * 2;
    }

    return a;
}
