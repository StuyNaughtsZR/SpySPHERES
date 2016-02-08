/* Known bugs:
 * tries to pick up both mirrors
 * does not face the opponent correctly
 * spins indefinitely while trying to pick up debris
 */

#define getInPosForPic 0
#define upload 1
#define getItem 2
#define getEnergyState 3
#define tisShinyCptn 4

typedef struct {
    ZRState me;
    ZRState other;
    float target[3];
    float facing[3];
    float items[9][3];
    float itemPossession[9];
    int state;
} gameState;

gameState current;

float rotateZ[3]; // to rotate up
float temp[3]; // temp array
int targetID; // index for target
int c; // counter

void init() {
    updateGameState();
    for (c = 0 ; c < 9 ; c++) {
        game.getItemLoc(current.items[c], c);
    }

    current.state = tisShinyCptn;
    
    rotateZ[0] = 0.0f;
    rotateZ[1] = 0.0f;
    rotateZ[2] = 1.0f;
}

void loop() {
    DEBUG(("UNTIL SWITCH: %f", game.getLightSwitchTime()));
    updateGameState();
    if (game.posInArea(current.me) > 0 && game.getLightSwitchTime() > 2) {
        game.useMirror();
    }
    changeState();
    DEBUG(("STATE: %d\n", current.state));
    DEBUG(("DIST TO TARGET: %f", distance(current.target, current.me)));
    DEBUG(("%d\n", current.state));
    switch(current.state) {
        case getInPosForPic: // if other in light, point towards other
            // facing here is the vector that points from the opponent to me
            mathVecSubtract(current.facing, current.me, current.other, 3);
            mathVecNormalize(current.facing, 3);

            // facing here is the vector that points from the opponent to me
            // with 0.55 magnitude
            mathVecMultiply(current.facing, 0.55);

            // target here is the vector from the origin to 0.55 away from the
            // opponent on the line connecting me to him
            mathVecAdd(current.target, current.other, current.facing, 3);
            mathVecNormalize(current.facing, 3);

            // facing now points towards the opponent
            mathVecMultiply(current.facing, -1);

            // now we move
            api.setPositionTarget(current.target);
            api.setAttitudeTarget(current.facing);

            // if we can, take a pic
            if (game.isCameraOn() && game.isFacingOther()) {
                DEBUG(("TAKING PIC"));
                game.takePic();
            }
            break;

        case upload: // upload
            // stay still
            api.setPositionTarget(current.me);

            if (checkUpload(current.me)) {
                memcpy(current.facing, current.me + 6, 3 * sizeof(float));
                api.setAttitudeTarget(current.facing);
            }
            else {
                api.setAttitudeTarget(rotateZ);
            }
            game.uploadPics();
            if (game.getMemoryFilled() == 0) {
                changeState();
            }
            break;

        case getItem: // if other in dark, go for the debris
            // debris is items 3-6

            targetID = minDistEl(current.items, current.me, 3, 7);
            //if (targetID == -1) {
            //    current.state = getEnergyState;
            //    break;
            //}

            memcpy(current.target, current.items + targetID, 3 * sizeof(float));
            if (areWeThereYet(current.target, current.me)) {
                api.setAttRateTarget(rotateZ); // supposedly will rotate
            }
            else {
                api.setPositionTarget(current.target);
            }
            break;

        case tisShinyCptn: // first thing --> grab mirrors
            targetID = minDistEl(current.items, current.me, 7, 9);
            memcpy(current.target, current.items + targetID, 3 * sizeof(float));

            if (game.hasItem(targetID) != -1) {
                if (game.posInArea(current.other) == 1) {
                    current.state = getInPosForPic;
                }
                else {
                    current.state = getItem;
                }
                break;
            }
            else if (areWeThereYet(current.target, current.me)) {
                api.setAttRateTarget(rotateZ);
            }
            else {
                api.setPositionTarget(current.target);
            }
            break;
    }

}

void updateGameState() {
    api.getMyZRState(current.me);
    api.getOtherZRState(current.other);
    for (c = 0 ; c < 9 ; c++) {
        current.itemPossession[c] = game.hasItem(c);
        if (current.itemPossession[c] != -1) { // if someone has the item
            // make the item effectively unreachable
            current.items[c][0] = 9999;
            current.items[c][1] = 9999;
            current.items[c][2] = 9999;
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
    float velocity[3];
    memcpy(velocity, me + 3, 3 * sizeof(float));
    return (distance(target, me) < 0.05 && mathVecMagnitude(velocity, 3) < 0.01);
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

// THANKS YOUNG
bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) ) {
        return true;
    }
    return false;
}

bool checkPhoto( float me[], float other[], float facing[] ){
    if( distance(me,other) > 0.5 && game.posInLight(other) ){
        if( mathVecInner(me+6,facing,3) > 0.9689f && game.getEnergy() > 1 && game.getPicPoints() > 0) {
            return true;
        }
    }
    return false;
}

float distance(float p1[], float p2[]){
    DEBUG(("distance function called on [%f, %f, %f] [%f, %f, %f]\n", p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]));
    mathVecSubtract(temp, p1, p2, 3);
    DEBUG(("result of vec subtract: [%f, %f, %f]\n", temp[0], temp[1], temp[2]));
    return mathVecMagnitude(temp, 3);
}

void changeState() {
    if (game.getMemoryFilled() == 2 || (current.state == upload && game.getMemoryFilled() != 0)) {
        current.state = upload;
    }
    else if (current.state != tisShinyCptn && distance(current.target, current.me) > 0.3) {
        if (game.posInArea(current.other) == 1) {
            current.state = getInPosForPic;
        }
        else {
            current.state = getItem;
        }
    }
}
