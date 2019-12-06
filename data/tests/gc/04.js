function test() {
    let a = {
        b : {
            c : {
                d : {
                    e : {
                        f : [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                    }
                }
            }
        }
    };

    a.b.c.d.e.g = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

    let i = 0;

    let sum = 0;

    while (i < len(a.b.c.d.e.g)) {
        sum = sum + a.b.c.d.e.g[i] * a.b.c.d.e.f[len(a.b.c.d.e.f) - i - 1];
        i = i + 1;
    }

    return sum;
}
