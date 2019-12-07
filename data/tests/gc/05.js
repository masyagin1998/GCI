function test() {
    let a = {};
    let b = {};
    let c = {};

    a.h = 1;
    b.h = 1;
    c.h = 1;

    let d = {};

    return a.h + b.h + c.h;
}
