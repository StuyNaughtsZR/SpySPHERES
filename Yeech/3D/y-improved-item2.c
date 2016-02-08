#define stop -1
#define normal 0
#define upload 1
#define getItem 2
#define tisShinyCptn 3
#define chillout 4
#define on_top 5

ZRState me;
ZRState other;
float target[3];
float facing[3];
float rotateZ[3];
float temp[3];
float zero[3];
float items[9][3];
int movestate;
int rotatestate;
int targetID;
int c;
bool winning;

void init() {
    updateGameState();

    for (c = 0 ; c < 9 ; c++) {
        game.getItemLoc(items[c], c);
    }

    zero[0] = 0.0f;
    zero[1] = 0.0f;
    zero[2] = 0.0f;

    items[3][0] = 9999;
    items[3][1] = 9999;
    items[3][2] = 9999;

    movestate = tisShinyCptn;
    rotatestate = normal;
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
    DEBUG(("STATE: %d\n", movestate));
    
    // Design concept: 2 separate state vars,
    // rotatestate governs the picture taking mechanism
    // movestate governs the item getting mechanism

    // Note that rotatestate does not govern movement, but rather governs rotation

    switch (rotatestate) {
        case normal:
            if (game.posInArea(other) > 0 &&
                    game.isFacingOther() && 
                    game.getEnergy() > 1.5 &&
                    game.getPicPoints() > 0) {
                game.takePic();
            }
            else {
                mathVecSubtract(facing, other, me, 3);
                api.setAttitudeTarget(facing);
            }
            break;

        case upload:
            if (game.getMemoryFilled() == 0) {
                movestate = getItem;
            }
            else {
                api.setAttitudeTarget(rotateZ);
                if (game.getEnergy() > 1 && checkUpload(me)) {
                    game.uploadPics();
                }
            }
            break;
        
        case stop:
            api.setAttRateTarget(zero);
            break;
          
    }

    switch (movestate) {
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
                api.setPositionTarget(items[targetID]);
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
                movestate = getItem;
            }
            break;

        case on_top:
            getAbove(other, -0.4);
            //temp[0] = other[0];
            //temp[1] = other[1];
            //temp[2] = other[2];
            //if (!winning) {
            //    if (game.posInLight(temp)) {
            //        temp[1] = -temp[1] / fabsf(temp[1]) * 0.12;
            //    }
            //    else {
            //        temp[1] = temp[1] / fabsf(temp[1]) * 0.12;
            //    }
            //}
            //else {
            //    temp[1] = temp[1] * 0.12;
            //}
            //DEBUG(("CURRENT LOC: %f, %f, %f\n", me[0], me[1], me[2]));
            //DEBUG(("GOING TO: %f, %f, %f\n", temp[0], temp[1], temp[2]));
            //api.setPositionTarget(temp);
            break;

        case stop: // if we have extremely low energy
            api.setVelocityTarget(zero);
            break;

    }
}
void getAbove(float other[], float altitude) {
    memcpy(target, other, 3 * sizeof(float)); 
    target[2] = altitude;

    if (distance(target, other) < 0.5) {
        target[0] = -other[0];
    }

    api.setPositionTarget(target);
}
void updateGameState() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    winning = game.getScore() > game.getOtherScore();
    for (c = 0 ; c < 9 ; c++) {
        if (c != 3) { // c == 3 -> death zone
            if (game.hasItem(c) != -1) { // if someone has the item
                // make the item effectively unreachable
                items[c][0] = 9999;
                items[c][1] = 9999;
                items[c][2] = 9999;
            }
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
    // first determine rotatestate
    if (game.getCurrentTime() > 175 || 
        game.getFuelRemaining() < 5 ||
        game.getMemoryFilled() == 2) {
        rotatestate = upload;
    }
    else if (rotatestate == upload && game.getMemoryFilled() == 0) {
        rotatestate = normal;
    }
    else if (rotatestate == stop) {
        rotatestate = normal;
    }

    // then movestate
    if (movestate == getItem) {
        targetID = minDistEl(items, me, 3, 7);
        if (targetID == -1) {
            movestate = on_top;
        }
    }
    else if (movestate == stop){
        movestate = getItem;
    }

    // finally energy concerns
    if (game.getEnergy() < 1) {
        movestate = stop;
        rotatestate = stop;
    }
}

bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) )
        return true;
    return false;
}
