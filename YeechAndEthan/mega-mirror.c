/*******************************************************************
 * The best competitor ever created                                *
 * Maintainer:                                                     *
 *  Yicheng                                                        *
 *                                                                 *
 * TODO:                                                           *
 *  Add a way to get energy                                        *
 *  Write a better mirror logic                                    *
 ******************************************************************/

#define STOP -1
#define NORMAL 0
#define UPLOAD 1
#define GET_ITEM 2
#define GET_MIRROR 3
#define ON_TOP 4
#define TO_ORIGIN 5

/* ZR states of me and other */
ZRState me;
ZRState other;

/* Items:
 * 0 - 2 = energy
 * 3 - 6 = score <== 3 being the one on the bottom
 * 7 - 8 = mirror
 */
float items[9][3];

/* Keeps Track of who has the items */
int itemPossession[9];

/* Where to go/point at */
float target[3];
float facing[3];
int targetID;

/* temp target vector, see moveTo */
float tarVec[3];

/* Standard Locations */
float earth[3];
float origin[3];

/* Temp/Counter */
float temp[3];
int n;

/* State Vars */
int movestate;
int rotatestate;

/* Initializes everything */
void init() {
    for (n = 0 ; n < 9 ; n++) {
        game.getItemLoc(items[n], n);
    }

    updateGameState();

    origin[0] = origin[1] = origin[2] = earth[0] = earth[1] = 0.0f;
    earth[2] = 1.0f;

    movestate = GET_MIRROR;
    rotatestate = NORMAL;
    targetID = minDistEl(items, me, 7, 9);
}

/* Runs every second */
void loop() {
    updateGameState();

    // FIXME does not work that well
    if (game.posInLight(me)&&game.getOtherEnergy()>.9) {
        game.useMirror();
    }

    changeState();

    if (game.getFuelRemaining() == 0) {
        game.takePic();
    }

    /* Design concept: 2 separate state vars,
     * rotatestate governs the picture taking mechanism/rotation
     * movestate governs the item getting mechanism/movement
     */

    switch (rotatestate) {
        case NORMAL:
            if (game.posInArea(other) > 0 &&
                    game.isFacingOther() && 
                    game.getEnergy() > 1 && // don't nuke yourself
                    game.getPicPoints() > 0) { // shortcircuit eval makes sure doesn't waste energy
                game.takePic();
            }
            else {
                mathVecSubtract(facing, other, me, 3);
                api.setAttitudeTarget(facing); // always face toward the other
            }
            break;

        case UPLOAD:
            api.setAttitudeTarget(earth);
            if (game.getEnergy() > 1 && checkUpload(me)) {
                game.uploadPics();
            }
            break;
        
        case STOP:
            api.setAttRateTarget(origin);
            break;
          
    }

    switch (movestate) {
        case GET_ITEM: // item + try to point to the other sphere
            if (game.hasItem(targetID) == -1) {
                if (areWeThereYet(items[targetID], me)) {
                    api.setAttRateTarget(earth);
                }
                else {
                    moveTo(items[targetID]);
                }
            }
            else {
                moveTo(items[targetID]);
            }

            break;

        case GET_MIRROR: // get mirror
            if (game.hasItem(targetID) == -1) {
                targetID = minDistEl(items, me, 7, 9);
                if (areWeThereYet(items[targetID], me)) {
                    api.setAttRateTarget(earth);
                }
                else {
                    moveTo(items[targetID]);
                }
            }
            else {
                targetID = minDistEl(items, me, 3, 7);
                movestate = GET_ITEM;
            }
            break;

        case ON_TOP:
            getAbove(other, -0.4);
            break;

        case STOP: // if we have extremely low energy
            api.setVelocityTarget(origin);
            break;

        case: TO_ORIGIN:
              moveTo(origin);
              break;

    }

}

/* Gets above other at certain altitude
 * If other is already there, mimic across the x-axis
 * Unless out of bounds, in which case stay near 0
 */
void getAbove(float other[], float altitude) {
    memcpy(target, other, 3 * sizeof(float)); 
    target[2] = altitude;

    if (distance(target, other) < 0.5) {
        target[0] = -other[0];
    }

    if (!inBounds(other)) {
        target[2] = 0;
        target[1] = 0;
        target[0] = 0;
    }
    moveTo(target);
}

/* Updates:
 * me
 * other
 * itemPossession
 */
void updateGameState() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    for (n = 0 ; n < 9 ; n++) {
        itemPossession[n] = game.hasItem(n);
    }
}

/* State changing function that changes state under a variety of
 * circumstances, more detail follows in code
 */
void changeState() {
    // first determine rotatestate
    if (game.getCurrentTime() > 170 || // if we are out of time
        game.getFuelRemaining() < 10 || // if we are out of fuel
        game.getMemoryFilled() == 2) { // or if memory is full
        rotatestate = UPLOAD;
    }
    else if (rotatestate == UPLOAD && game.getMemoryFilled() == 0) {
        rotatestate = NORMAL;
    }
    else if (rotatestate == STOP) { // reset after low-energy
        rotatestate = NORMAL;
    }

    // then movestate
    if (movestate == GET_ITEM) {
        targetID = minDistEl(items, me, 3, 7);
        if (targetID == -1) { // if there is no avaliable item
            movestate = ON_TOP;
        }
    }
    else if (movestate == STOP){
        movestate = GET_ITEM;
    }

    if (game.getCurrentTime() > 177) {
        movestate = TO_ORIGIN; //win in the end
    }

    // finally energy concerns
    if (game.getEnergy() < 1) {
        movestate = STOP;
        rotatestate = STOP;
    }
}

/* Checks if we are at target
 * Returns true if distance < 0.05 and velocity < 0.01
 */
bool areWeThereYet(float target[], float me[]) {
    memcpy(temp, me + 3, 3 * sizeof(float));
    return (distance(target, me) < 0.05 && mathVecMagnitude(temp, 3) < 0.01);
}

/* Simple distance function, returns distance between p1 and p2 */
float distance(float p1[], float p2[]){
    mathVecSubtract(temp, p1, p2, 3);
    return mathVecMagnitude(temp, 3);
}

/* Figures out the index of an element of a 2D array that is closest to me
 * Arguments: 2D array, me, beginning index (inclusive), ending index
 * (exclusive)
 *
 * Returns the index of the element in the 2D array closest to me
 * Returns -1 if no items are avaliable
 */
int minDistEl(float choices[][3], float me[], int beginning, int end) {
    int result = -1;
    float minDist = 101.0f;
    for (n = beginning ; n < end ; n++) {
        if (itemPossession[n] == -1 && distance(choices[n], me) < minDist) {
            minDist = distance(choices[n], me);
            result = n;
        }
    }
    if (minDist > 100) {
        return -1;
    }
    return result;
}

/* Figures out a pair of items such that the total distance travelled from
 * me-item1-item2 is minimized and puts it in the first variable. Other
 * parameters are the same as those for minDistEl
 */

void minDistPair(float path[2][3], float choices[][3], float me, int beginning, int end) {
    // TODO
}

/* Young's magical function that checks if we can upload in the current
 * facing
 * TODO: how does this work...
 */
bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) )
        return true;
    return false;
}

/* Joel/Young's Displacement Function
 * Takes two vectors, p1 and p2, and puts the normalized
 * difference between them itno target (the first argument)
 */
void displace(float target[], float p1[], float p2[]){
     mathVecSubtract(temp,p1,p2,3);
     mathVecNormalize(temp,3);
     memcpy(target,temp,3*sizeof(float));
}

/* bounds checking function
 * returns true if location given is within bounds
 * false otherwise
 */
bool inBounds(float loc[]) {
    return abs(loc[0]) < 0.64 &&
           abs(loc[1]) < 0.8 &&
           abs(loc[2]) < 0.64;
}

/* Shaumik's movement function
 * moves to target, but is faster than api.setPositionTarget
 */
void moveTo(float target[]){
    float d = distance(me,target);
    float speed = 1;
    if (d > 0.5*0.01*81+mathVecMagnitude(me,3)*9){
        displace(tarVec,target,me);
        speed = d;
        tarVec[0] = tarVec[0] * speed;
        tarVec[1] = tarVec[1] * speed;
        tarVec[2] = tarVec[2] * speed;
        api.setVelocityTarget(tarVec);
    }
    else {
        api.setPositionTarget(target);
    }
    DEBUG(("%f_%f", mathVecMagnitude(me+3,3), d));
}
