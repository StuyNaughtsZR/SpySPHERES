#define getInPosForPic 0
#define upload 1
#define getItem 2
#define getEnergyState 3
#define tisShinyCptn 4

ZRState me;
ZRState other;
float target[3];
float facing[3];
float rotateZ[3];
float temp[3];
float items[9][3];
float itemPossession[9];
int state;
int targetID;
int c;

void init() {
    updateGameState();

    for (c = 0 ; c < 9 ; c++) {
        game.getItemLoc(items[c], c);
    }

    state = tisShinyCptn;
    targetID = minDistEl(items, me, 7, 9);

    rotateZ[0] = 0.0f;
    rotateZ[1] = 0.0f;
    rotateZ[2] = 1.0f;
}

void loop() {
    updateGameState();
    if (game.posInArea(me) > 0 && game.getLightSwitchTime() > 2) {
        game.useMirror();
    }
    changeState();
    DEBUG(("STATE: %d\n", state));

    switch (state) {
        case getInPosForPic:
            if (game.posInArea(other) > 0 &&
                    game.isFacingOther() && 
                    game.getEnergy() > 1 &&
                    game.getPicPoints() > 0) {
                game.takePic();
            }
            else {
                getAbove(other, 1);
            }
            break;
        case upload:
            api.setPositionTarget(me);
            if (game.getMemoryFilled() == 0) {
                state = getInPosForPic;
            }
            else {
                api.setAttitudeTarget(rotateZ);
                game.uploadPics();
            }
            break;

        case getItem: // we only get this 1 item to prevent going out of bounds
            if (itemPossession[6] == -1) {
                if (areWeThereYet(items[6], me)) {
                    api.setAttRateTarget(rotateZ);
                }
                else {
                    api.setPositionTarget(items[6]);
                }
            }
            else {
                state = getInPosForPic;
            }

            break;

        case tisShinyCptn:
            if (itemPossession[targetID] == -1) {
                targetID = minDistEl(items, me, 7, 9);
                if (areWeThereYet(items[targetID], me)) {
                    api.setAttRateTarget(rotateZ);
                }
                else {
                    api.setPositionTarget(items[targetID]);
                }
            }
            else {
                state = getItem;
            }

            break;
    }
}

void getAbove(float other[], float n) {
    memcpy(target, other, 3 * sizeof(float));
    memcpy(temp, other + 3, 3 * sizeof(float));
    // crude approx. of pos + (n * velocity)
    target[0] = target[0] + (n * temp[0]);
    target[1] = target[1] + (n * temp[1]);
    target[2] = target[2] + (n * temp[2]);
    mathVecSubtract(facing, target, me, 3);
    api.setAttitudeTarget(facing);
    target[2] = -0.6;
    api.setPositionTarget(target);
}

void updateGameState() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    for (c = 0 ; c < 9 ; c++) {
        itemPossession[c] = game.hasItem(c);
        if (itemPossession[c] != -1) { // if someone has the item
            // make the item effectively unreachable
            items[c][0] = 9999;
            items[c][1] = 9999;
            items[c][2] = 9999;
        }
    }
}

int size(float arr[]) {
    return sizeof(arr) / sizeof(arr[0]);
}

void mathVecMultiply(float vec[], float scalar) {
    for (c = 0 ; c < size(vec) ; c++) {
        vec[c] = vec[c] * scalar;
    }
}

bool areWeThereYet(float target[], float me[]) {
    memcpy(temp, me + 3, 3 * sizeof(float));
    return (distance(target, me) < 0.05 && mathVecMagnitude(temp, 3) < 0.01);
}

// @input: list of items, me, beginning index of considered items (inclusive), end index of considered items (exclusive)
// @returns: index of the item that is closest to me
int minDistEl(float choices[][3], float me[], int beginning, int end) {
    int result = beginning;
    float minDist = distance(choices[beginning], me);
    for (c = beginning ; c < end ; c++) {
        DEBUG(("distance to %d: %f\n", c, distance(choices[c], me)));
        if (distance(choices[c], me) < minDist) {
            minDist = distance(choices[c], me);
            result = c;
        }
    }
    if (minDist > 100) {
        return -1;
    }
    return result;
}

float distance(float p1[], float p2[]){
    mathVecSubtract(temp, p1, p2, 3);
    return mathVecMagnitude(temp, 3);
}

void changeState() {
    if (game.getMemoryFilled() == 2 || (state == upload && game.getMemoryFilled() != 0)) {
        state = upload;
    }
    else if (state != tisShinyCptn && state != getItem) {
        state = getInPosForPic;
    }
}
