function test() {
    let obj = {};

    obj.a = obj;
    obj.a.a = obj;
    obj.a.a.a = obj;
    obj.a.a.a.a = obj;
    obj.a.a.a.a.a = obj;
    obj.a.a.a.a.a.a = obj;
    obj.b = 5;

    return obj.a.a.a.a.a.a.b;
}
