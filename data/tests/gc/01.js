function test() {
    let arr = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    let obj = {
        a : 1,
        b : 2,
    };

    let res = len(arr);
    res = res + has_property(obj, a) + has_property(obj, b);
    res = res + len(obj) * 2;
    res = res + has_property(arr, c) * 2;

    return res;
}
