function test() {
    let stack = {
        l : 0,
        cap : 10,
        arr : [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    };

    stack.arr[stack.l] = 1;
    stack.l = stack.l + 1;

    stack.arr[stack.l] = 2;
    stack.l = stack.l + 1;

    stack.arr[stack.l] = 3;
    stack.l = stack.l + 1;

    stack.arr[stack.l] = 4;
    stack.l = stack.l + 1;

    stack.l = stack.l - 1;

    stack.l = stack.l - 1;

    return stack.arr[stack.l - 1];
}
