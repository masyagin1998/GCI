function test(){
    let obj = {
    };

    let obj1 = {
        a : obj,
    };

    obj.a = 1;
    obj.b = 2;
    obj.c = 3;
    obj.d = 4;

    let obj2 = {
        a : obj,
    };
    
    obj.e = 5;
    obj.f = 6;
    obj.g = 7;
    obj.h = 8;
    obj.i = 9;
    obj.j = 10;
    obj.k = 11;
    obj.l = 12;
    obj.m = 13;
    obj.n = 14;
    obj.o = 15;

    return obj1.a.a + obj1.a.g + obj1.a.o;
}
