#define upload 1
#define getItem 2
#define tisShinyCptn 3
#define stay_in_dark 4

ZRState me;
ZRState other;
float target[3];
float facing[3];
float rotateZ[3];
float temp[3];
float items[9][3];
int state;
int targetID;
int c;

void init() {
    updateGameState();

    for (c = 0 ; c < 9 ; c++) {
        game.getItemLoc(items[c], c);
    }

    state = getItem;
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
        case upload:
            api.setPositionTarget(me);
            if (game.getMemoryFilled() == 0) {
                state = getItem;
            }
            else {
                api.setAttitudeTarget(rotateZ);
                if (checkUpload(me)) {
                    game.uploadPics();
                }
            }
            break;

        case getItem: // item + try to point to the other sphere
            if (game.hasItem(targetID) == -1) {
                if (areWeThereYet(items[targetID], me)) {
                    api.setAttRateTarget(rotateZ);
                }
                else {
                    api.setPositionTarget(items[targetID]);
                }
            }
            else {
                targetID = minDistEl(items, me, 3, 7);
                api.setPositionTarget(items[targetID]);
            }

            if (game.posInArea(other) > 0 &&
                    game.isFacingOther() && 
                    game.getEnergy() > 1 &&
                    game.getPicPoints() > 0) {
                game.takePic();
            }
            else {
                mathVecSubtract(facing, other, me, 3);
                api.setAttitudeTarget(facing);
            }
            break;
        case tisShinyCptn:
            if (game.hasItem(targetID) == -1) {
                targetID = minDistEl(items, me, 7, 9);
                if (areWeThereYet(items[targetID], me)) {
                    api.setAttRateTarget(rotateZ);
                }
                else {
                    api.setPositionTarget(items[targetID]);
                }
            }
            else {
                targetID = minDistEl(items, me, 3, 7);
                state = getItem;
            }
            break;

    }
}

void updateGameState() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    for (c = 0 ; c < 9 ; c++) {
        if (game.hasItem(c) != -1) { // if someone has the item
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
    if (game.getCurrentTime() > 175 || game.getFuelRemaining() < 5) {
        state = upload;
    }
    if (game.getMemoryFilled() == 2 || (state == upload && game.getMemoryFilled() != 0)) {
        state = upload;
    }
}

bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) )
        return true;
    return false;
}
