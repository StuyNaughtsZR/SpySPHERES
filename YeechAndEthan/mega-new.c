/*******************************************************************************
 * Stuy_Naught competitor -- really advanced item bot                          *
 *                                                                             *
 * Authors                                                                     *
 *  Written by Yicheng, Strategy by Jared                                      *
 *                                                                             *
 * Description                                                                 *
 *  Pick items based on ETA of the other guy and us, also based on the         *
 *  placement of items given the game. If they are close together we rush and  *
 *  try to get everything. If the other is going for mirror first, then we go  *
 *  for their top energy. If the energy items are far apart, we go for the     *
 *  enegies on top and nuke 'em when they try to grab the ones on the bottom   *
 *                                                                             *
 ******************************************************************************/

/* TODO
 *  finish ETA system
 */

/* Dev Log
 *  Project Created: 2015-11-20 11:11 - Yicheng W.
 */

/*** INIT *********************************************************************/

#define STOP -1
#define NORMAL 0
#define UPLOAD 1
#define GET_ITEM 2
#define ON_TOP 4
#define TO_ORIGIN 5

#define CONE_ANGLE_COSINE 0.997f
#define MAX_INT 9999
#define MY_SPEED 0.035 // average estimate, subject to change

/* ZR states of me and other */
ZRState me;
ZRState other;

/* Items:
 * 0 - 2 = energy
 * 3 - 6 = score
 * 7 - 8 = mirror
 */
float items[9][3];

/* Keeps Track of who has the items */
int itemPossession[9];

/* Where to go/point at */
float target[3];
float facing[3];
int targetID;

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

    movestate = GET_ITEM;
    rotatestate = NORMAL;

    targetID = minDistEl(items, me, 3, 7);
    if (targetID == -1) { // if there is no avaliable item
        movestate = ON_TOP;
    }

}

/*** MAIN *********************************************************************/
void loop() {
    updateGameState();

    changeState();

    if (game.getFuelRemaining() == 0) {
        game.takePic();
    }

    //DEBUG(("%d\n", inBounds(other)));

    /* Design concept: 2 separate state vars,
     * rotatestate governs the picture taking mechanism/rotation
     * movestate governs the item getting mechanism/movement
     */

    switch (rotatestate) {
        case NORMAL:
            if (game.posInArea(other) >= 0 &&
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
            if (game.getEnergy() > 1 && checkUpload(me)) {
                game.uploadPics();
            }
            else {
                api.setAttitudeTarget(earth);
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

        case ON_TOP:
            getAbove(other, -0.4);
            break;

        case STOP: // if we have extremely low energy
            api.setVelocityTarget(origin);
            break;

        case TO_ORIGIN:
            moveTo(origin);
            break;

    }

}

/*** CHANGE *******************************************************************/
/* State changing function that changes state under a variety of
 * circumstances, more detail follows in code
 */
void changeState() {
    // first determine rotatestate
    if (((game.getCurrentTime() > 170 || // if we are out of time
          game.getFuelRemaining() < 10) && game.getMemoryFilled() != 0) || // if we are out of fuel
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

    else if (movestate == ON_TOP && !inBounds(other)) { // don't follow the other out of bounds
        movestate = STOP;
    }


    if (game.getCurrentTime() > 177) {
        movestate = TO_ORIGIN; //win in the end
    }

    // finally energy concerns
    if (game.getEnergy() < 0.75) {
        movestate = STOP;
        rotatestate = STOP;
    }
}

/*** UTILS ********************************************************************/
/* Gets above other at certain altitude
 * If other is already there, mimic across the x-axis
 * Unless out of bounds, in which case stay near 0
 */
void getAbove(float other[], float altitude) {
    memcpy(target, other, 3 * sizeof(float)); 
    target[2] = altitude;

    if (distance(target, other) < 0.5) {
        target[0] = -other[0];
        target[2] = other[2];
    }

    if (!inBounds(other)) {
        target[2] = 0;
        target[1] = 0;
        target[0] = 0;
    }
    moveTo(target);
}

/* getETA: get estimated time of arrival upon item based on the sphere, 
 * finds the angle between th other's velocity vector and the vector from the
 * other that points towards the location. If that is more than
 * CONE_ANGLE_COSINE as defined in INIT, the function will return MAX_INT,
 * otherwise the function returns the distance divided by the other's speed.
 */
float getETA (float sphere[], float location[]){
    mathVecSubtract(temp, location, sphere, 3);
    float distance = mathVecMagnitude(temp, 3);
    float speed = mathVecMagnitude(sphere + 3, 3);
    float cosine = mathVecInner(temp, sphere + 3, 3) / (distance, speed);
    if (cosine > CONE_ANGLE_COSINE) { // means that the angle is within the boundary
        return distance / speed;
    }
    return MAX_INT;
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
        if (itemPossession[n] == -1) {
            temp[0] = distance(choices[n], me) / MY_SPEED;
            if (temp[0] * MY_SPEED < minDist && temp[0] < getETA(other, items[n])) {
                minDist = distance(choices[n], me);
                result = n;
            }
        }
    }
    if (minDist > 100) {
        return -1;
    }
    return result;
}

/* Figures out a pair of items, 
*/

void minDistPair(float path[2][3], float choices[][3]) {
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

/* bounds checking function
 * returns true if location given is within bounds
 * false otherwise
 */
bool inBounds(float loc[]) {
    //DEBUG(("%f, %f, %f\n", fabsf(loc[0]), fabsf(loc[1]), fabsf(loc[2])));
    return (fabsf(loc[0]) < 0.64 &&
        fabsf(loc[1]) < 0.8 &&
        fabsf(loc[2]) < 0.64);
}

/* Shaumik's better setPositionTarget function
 * NOTE: THIS IS INTENDED TO BE USED GOING TO A STATIC POINT
 * USING IT TO MOVE TO A CONSTANTLY CHANGING POINT MAY MESS IT UP!!!!
 */

void moveTo(float target[]) {

    float disp[3];
    float dist, speed;

    mathVecSubtract(disp, target, me, 3);
    dist = mathVecNormalize(disp, 3); //disp normalized to unit vector, holds direction of desired velocity

    /* Condition here is based off of equation: d = (1/2)at^2 + vt
     * a = 0.01, which is the approx maximum acceleration I have experimentally found of a SPHERE
     * I assume t = 8s, which works and is faster then api.setPositionTarget
     * You can change t if there is an overshoot
     */
    if (dist > 0.5*0.01*64+mathVecMagnitude(me+3,3)*8) {

        speed = dist; 

        for(n = 0; n < 3; n++) {  //scale velocity (disp) to speed
            disp[n] *= speed;
        }

        api.setVelocityTarget(disp);

    }
    else {
        api.setPositionTarget(target);
    }

}

/* isFacingOther -- creds to Herder Berlin
 * returns if we are facing other without using energy
 */
//bool isFacingOther() {
//    mathVecSubtract(temp, me, other, 3);
//    mathVecNormalize(temp, 3);
//    return (distance(temp, &other[6])<= 0.35f);
//}
