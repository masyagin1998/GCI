function test() {
    let i = 0;
    
    let army0 = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    i = 0;
    while (i < len(army0)) {
        army0[i] = {
            health    : 7,
            damage    : 5,
            avoidance : 3,
        };
        i = i + 1;
    }

    let army1 = [0, 0, 0, 0, 0];
    i = 0;
    while (i < len(army1)) {
        army1[i] = {
            health    : 5,
            damage    : 4,
            avoidance : 9,
        };
        i = i + 1;
    }

    i = 0;
    while (1) {
        let attack  = army0;
        let protect = army1;
        if (i % 2 == 1) {
            let tmp = attack;
            attack = protect;
            protect = tmp;
        }

        let attackind = 0;
        while (attackind < len(attack)) {
            let attackunit = attack[attackind];
            if (attackunit.health > 0) {
                break;
            }

            attackind = attackind + 1;
        }
        if (attackind == len(attack)) {
            if (i % 2 == 1) {
                return 0;
            } else {
                return 1;
            }
        }        
        
        let protectind = 0;
        while (protectind < len(protect)) {
            let protectunit = protect[protectind];
            if (protectunit.health > 0) {
                break;
            }

            protectind = protectind + 1;
        }
        if (protectind == len(protect)) {
            if (i % 2 == 1) {
                return 1;
            } else {
                return 0;
            }
        }

        if (i % 10 > protect[protectind].avoidance) {
            protect[protectind].health = protect[protectind].health - attack[attackind].damage;
        }

        i = i + 1;
        if (i == 10000) {
            break;
        }
    }

    return -1;
}
