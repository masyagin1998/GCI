function fib() {
    let a = 1;
    let b = 1;
    let i = 3;
    while (i <= n) {
        let c = a + b;
        a = b;
        b = c;
        i = i + 1;
    }
    
    return b;
}
