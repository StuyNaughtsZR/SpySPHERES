#define getInPosForPic 0
#define upload 1
#define getItem 2
#define getEnergyState 3

ZRState me;
ZRState other;
float target[3];
float facing[3];
float brakingPt[3];
float scores[3][3];
float energy[4][3];
int state;
int targetID;
int currentEnergy;

void init() {
    int c;
    api.getMyZRState(me);
    api.getOtherZRState(other);
    if (game.posInArea(other) == 1) {
        state = getInPosForPic;
    }
    else {
        state = getItem;
    }
    scores[0][0] = 0.0f;
    scores[0][1] = 0.6f;
    scores[0][2] = 0.0f;
    scores[1][0] = 0.4f;
    scores[1][1] = 0.6f;
    scores[1][2] = 0.0f;
    scores[2][0] = -0.4f;
    scores[2][1] = 0.6f;
    scores[2][2] = 0.0f;
    
    brakingPt[0] = 50.0f;
    brakingPt[1] = 50.0f;
    brakingPt[2] = 50.0f;

    for (c = 0 ; c < 4 ; c++) {
        game.getItemLoc(energy[c], c);
    }
    for (c = 0 ; c < 9 ; c++) {
        DEBUG(("%d: %d\n", c, game.getItemType(c)));
    }
}

void mathVecMultiply(float vec[], float scalar) {
    int c;
    for (c = 0 ; c < size(vec) ; c++) {
        vec[c] = vec[c] * scalar;
    }
}

bool areWeThereYet(float target[], float me[]) {
    float velocity[3];
    memcpy(velocity, me + 3, 3 * sizeof(float));
    return (distance(target, me) < 0.05 && mathVecMagnitude(velocity, 3) < 0.05);
}

int size(float arr[]) {
    return sizeof(arr) / sizeof(arr[0]);
}

int size(float arr[][3]) {
    return sizeof(arr) / sizeof(arr[0]);
}

int minDistEl(float choices[][3], float me[]) {
    int result = 0, c = 0;
    float minDist = distance(choices[0], me);
    for (c = 0 ; c < size(choices) ; c++) {
        DEBUG(("distance to %d: %f\n", c, distance(choices[c], me)));
        if (distance(choices[c], me) < minDist) {
            minDist = distance(choices[c], me);
            result = c;
        }
    }
    return result;
}

float distance(float p1[], float p2[]){
    float temp[3];
    mathVecSubtract(temp, p1, p2, 3);
    return mathVecMagnitude(temp, 3);
}

float angle(float a[], float b[], float c[]) {
    //returns the measure of angle abc
    float side1[3], side2[3], cosine;
    
    for (int i = 0; i < 3; i++) side1[i] = a[i] - c[i];
    cosine = - mathVecMagnitude(side1,3) * mathVecMagnitude(side1,3);
    for (int i = 0; i < 3; i++) side1[i] = b[i] - a[i];
    for (int i = 0; i < 3; i++) side2[i] = c[i] - b[i];
    cosine += mathVecMagnitude(side1,3) * mathVecMagnitude(side1,3) + mathVecMagnitude(side2,3) * mathVecMagnitude(side2,3);
    if (mathVecMagnitude(side1,3) * mathVecMagnitude(side2,3) / 10 == 0) {
        DEBUG(("DIVISION BY ZERO WHILE FINDING ANGLE!"));
    }
    cosine /= 2 * mathVecMagnitude(side1,3) * mathVecMagnitude(side2,3);
    
    
    return acosf(cosine);
}

void mathVecProject(float c[], float a[], float b[], int n) {
    // finds the projection of a onto b, puts the result in c
    if (mathVecMagnitude(b,3) * mathVecMagnitude(b,3) / 10 == 0) {
        DEBUG(("DIVISION BY ZERO WHILE PROJECTING!"));
    }
    for (int i = 0; i < n; i++) {
        c[i] = (mathVecInner(a,b,3) * b[i]) / (mathVecMagnitude(b,3) * mathVecMagnitude(b,3));
    }
}

void loop() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    currentEnergy = game.getEnergy();
    if (state != upload) {
        if (currentEnergy < 1.5) {
            state = getEnergyState;
        }
        else if (game.posInArea(other) == 1) {
            state = getInPosForPic;
        }
        else {
            state = getItem;
        }
    }
    DEBUG(("%d\n", state));
    switch(state) {
        case getInPosForPic: // if other in light, point towards other
            if (game.getMemoryFilled() == 2) {
                state = upload;
            }
            else if (brakingPt[0] > 40 && brakingPt[1] > 40) {
                mathVecSubtract(facing, me, other, 3);
                mathVecNormalize(facing, 3);
                mathVecMultiply(facing, 0.5);
                mathVecAdd(brakingPt, other, facing, 3);
            }
            else {
                mathVecSubtract(facing, other, me, 3);
                mathVecNormalize(facing, 3);
                api.setPositionTarget(brakingPt);
                api.setAttitudeTarget(facing);
                if (game.isCameraOn() && game.isFacingOther()) {
                    DEBUG(("TAKING PIC"));
                    game.takePic();
                }
            }
            break;

        case upload: // upload
            game.uploadPics();
            if (game.getMemoryFilled() == 0) {
                if (game.posInArea(other) == 1) {
                    state = getInPosForPic;
                }
                else {
                    state = getItem;
                }
            }
            brakingPt[0] = 50.0f;
            brakingPt[1] = 50.0f;
            brakingPt[2] = 50.0f;
            break;
        case getItem: // if other in dark, go for the point item
            targetID = minDistEl(scores, me); // FIXME this doesn't work right
            memcpy(target, scores + targetID, 3 * sizeof(float));
            if (areWeThereYet(target, me)) {
                float tmp[3];
                tmp[0] = 0;
                tmp[1] = 0;
                tmp[2] = 1;
                api.setAttRateTarget(tmp); // supposedly will rotate
                //if (game.hasItem(targetID + 4) != -1) { // FIXME this should work but does not
                //    scores[targetID][0] = 99999;
                //    scores[targetID][1] = 99999;
                //    scores[targetID][2] = 99999;
                //}
            }
            else {
                api.setPositionTarget(target);
            }
            break;
        case getEnergyState: // if low battery, go grab energy pack
            targetID = minDistEl(energy, me);
            memcpy(target, energy + targetID, 3 * sizeof(float));
            if (areWeThereYet(target, me)) {
                float tmp[3];
                tmp[0] = 0;
                tmp[1] = 0;
                tmp[2] = 1;
                api.setAttRateTarget(tmp); // supposedly will rotate
                if (game.hasItem(targetID) != -1) {
                    energy[targetID][0] = -999;
                    energy[targetID][1] = -999;
                    energy[targetID][2] = -999;
                }
            }
            break;
    }

}

