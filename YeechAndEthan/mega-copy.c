/*******************************************************************************
 * ZR alliance competitor -- copycat                                           *
 *                                                                             *
 * Authors                                                                     *
 *  Written by Yicheng, Optimized and designed by Ethan, With code from        *
 *  Shaumik and Young                                                          *
 *                                                                             *
 * Description                                                                 *
 *  Copy everything the opponent does across the x-axis before t=177,          *
 *  Then rush to the origin and win base off of that                           *
 *                                                                             *
 ******************************************************************************/

/* TODO
 *  Write the code
 */

/* Dev Log
 *  Project Created: 2015-11-16 01:51
 */

#define FULLSTOP 0      // Stop movement and rotation completely
#define SNIPER 1        // Stop movement, but continue rotating and taking pictures
#define MIMIC 2         // Mimic opponent movement
#define TO_ORIGIN 3     // Move towards origin
#define NORMAL 4
#define UPLOAD 5

/* ZR states of me and other */
ZRState me;
ZRState other;

/* Items:
 * 0 - 2 = energy
 * 3 - 6 = score <== 3 being the one on the bottom
 * 7 - 8 = mirror
 */
float items[9][3];

/* Keeps track of who has the items */
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

    movestate = MIMIC;
    rotatestate = NORMAL;
}

/* Runs every second */
void loop() {
    updateGameState();

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
        
        case FULLSTOP:
            api.setAttRateTarget(origin);
            break;
          
    }
    switch (movestate) {
        case FULLSTOP:
            break;
        case SNIPER:
            if (!inBounds(me)) {
                api.setPositionTarget(origin);
            }
            else {
                api.setVelocityTarget(origin);
            }
            break;
        case MIMIC:
            mimic();
            break;
        case TO_ORIGIN:
            moveTo(origin);
            break;
    }
}

/* Mimics the opponent competitor across the x-axis  (mirror image)
 */
void mimic() {

    // Mirror the opponent competitor's position

    target[0] = -1 * (1.5 * other[0]) - 2 *other[3];
    target[1] = 1.5 * other[1] + 2 * other[4];
    target[2] = 1.5 * other[2] + 2 * other[5];

    // Mirror the opponent competitor's attitude

    facing[0] = other[6];
    facing[1] = other[7];
    facing[2] = -other[8];

    api.setAttitudeTarget(facing);

    // Go to the center if the opponent is out of bounds
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
    else if (rotatestate == FULLSTOP) { // reset after low-energy
        rotatestate = NORMAL;
    }

    // Should be mimicking unti 177s
    movestate = MIMIC;

    if (movestate == MIMIC && !inBounds(other)) { // don't follow the other out of bounds
        movestate = SNIPER;
    }

    if (game.getCurrentTime() > 177) {
        movestate = TO_ORIGIN; //win in the end
    }

    // finally energy concerns
    if (game.getEnergy() < 1) {
        movestate = FULLSTOP;
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

/* bounds checking function
 * returns true if location given is within bounds
 * false otherwise
 */
bool inBounds(float loc[]) {
    return abs(loc[0]) < 0.64 &&
           abs(loc[1]) < 0.8 &&
           abs(loc[2]) < 0.64;
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

