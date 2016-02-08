/*******************************************************************************
 * SHA-2468 Alliance Competitor -- Advanced Rush Bot                           *
 *                                                                             *
 * Authors                                                                     *
 *  Yicheng Wang + Young Kim                                                   *
 *                                                                             *
 * Description                                                                 *
 *  Grab 2 items and rush the opponent!                                        *
 *                                                                             *
 ******************************************************************************/

/* TODO
 *  Better checkPhoto, everything regarding when to spam and when not to spam is
 *   stored there -- curently very duct-tapy
 *  general improvements
 *  Testing!
 */

/* Dev Log
 *  Project Created: 2015-11-26 21:45 - Yicheng W.
 */

/* INIT ***********************************************************************/
/* MOVE STATES */
#define STOP -1
#define GET_SCORE_PACKS 0
#define END_GAME 1
#define TO_ORIGIN 2

/* ATT STATES */
#define FACE_OTHER 3
#define UPLOAD 4

#define GIVE_UP_THRESHOLD 0.3f
#define ENERGY_SHUTDOWN_THRESHOLD 0.5f

ZRState me;
ZRState other;

float items[9][3];
int itemBool[9];

float target[3];

float temp[3];

float origin[3];
float facing[3];

int itemID;

int i;

int move_state;
int att_state;

void init() {
    for (int i = 0 ; i < 9 ; i++) {
        game.getItemLoc(items[i], i);
    }
    updateGameState();
    
    origin[0] = origin[1] = origin[2] = 0.0f;

    move_state = GET_SCORE_PACKS;
    att_state = FACE_OTHER;
    itemID = recalibrate(3, 6);

    DEBUG(("Hello from SHA-2468!"));
}

/* MAIN ***********************************************************************/
void loop() {
    blah();
    updateGameState();
    changeState();
    DEBUG(("%d, %d\n", att_state, move_state));

    if (game.getEnergy() < ENERGY_SHUTDOWN_THRESHOLD) {
        api.setVelocityTarget(origin);
        return;
    }

    switch (att_state) {
        case FACE_OTHER:
            if (distance(me, other) > 0.4f) {
                mathVecSubtract(temp, other, me, 3);
                api.setAttitudeTarget(temp);
            }
            if (checkPhoto()) {
                game.takePic();
            }
            break;

        case UPLOAD:
            temp[0] = 0.0f;
            temp[1] = 0.0f;
            temp[2] = 1.0f;
            api.setAttitudeTarget(temp);
            if (checkUpload()) {
                game.uploadPics();
            }
            break;
    }

    switch (move_state) {
        case GET_SCORE_PACKS:
            if (game.getScore() == 0) {
                mathVecAdd(temp, me, me + 3, 3); // where we will be
                if (!game.posInDark(temp)) {
                    api.setVelocityTarget(origin); // steady...
                }
                else {
                    temp[0] = game.getCurrentTime();
                    if (temp[0] < 5 || temp[0] > 25) {
                        api.setPositionTarget(items[itemID]);
                    }
                }
            }
            else {
                api.setPositionTarget(items[itemID]);
            }
            break;

        case END_GAME:
            memcpy(temp, other, 3 * sizeof(float));
            mathVecNormalize(temp, 3);
            mathVecMultiply(temp, -0.3f, 3);
            mathVecAdd(target, temp, other, 3);
            api.setPositionTarget(target);            
            break;

        case TO_ORIGIN:
            api.setPositionTarget(origin);
            break;
    }
}
/* CHANGE *********************************************************************/
void changeState() {
    if (game.getMemoryFilled() == 2) {
        att_state = UPLOAD;
    }

    if (att_state == UPLOAD && game.getMemoryFilled() == 0) {
        att_state = FACE_OTHER;
    }

    if (game.getCurrentTime() > 167 && game.getMemoryFilled() > 0) {
        att_state = UPLOAD;
    }

    if (move_state == GET_SCORE_PACKS) {
        if (game.getScore() > 3) {
            itemID = recalibrate(3,6);
            if (itemID == -1 || distance(me, items[itemID]) - distance(other, items[itemID]) > GIVE_UP_THRESHOLD) {
                move_state = END_GAME;
            }
        }
        else if (itemBool[itemID] != -1) {
            itemID = recalibrate(3, 6);
            if (itemID == -1) {
                move_state = END_GAME;
            }
        }
    }
    else {
        if (!inBounds(other)) {
            move_state = TO_ORIGIN;
        }
        else {
            move_state = END_GAME;
        }
    }
}

/* UTILS **********************************************************************/
void updateGameState() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    for (i = 0 ; i < 9 ; i++) {
        itemBool[i] = game.hasItem(i);
    }
}

int recalibrate(int start, int end){ //gets index of closest item in item range[start:end] 
    float shortdist = 100.0f;
    int index = -1;
    for(int i = start; i <= end; i++){
        if(itemBool[i] == -1 &&
           distance(me,items[i]) < shortdist &&
           (game.getScore() < 3 || distance(me, items[i]) < distance(other, items[i]))){
            shortdist = distance(me,items[i]);
            index = i;
        }
    }
    return index;
}
float distance(float p1[], float p2[]){
    mathVecSubtract(temp,p1,p2,3);
    return mathVecMagnitude(temp,3);
}
void mathVecMultiply(float vec[], float multipler, int dimensions) {
    for (i = 0 ; i < dimensions ; i++) {
        vec[i] = vec[i] * multipler;
    }
}

bool checkPhoto() {
    if (game.getCurrentTime() > 170 && game.getMemoryFilled() == 0) {
        return true;
    }
    float energy = game.getEnergy();
    if (game.getScore() > 1.5f || game.posInLight(me)) {
        if (energy > 2 || game.posInLight(me)){
            if (distance(me, other) < 0.5f ||
                (game.posInArea(other) >= 0 &&
                 game.isCameraOn() &&
                 game.isFacingOther() &&
                 ((game.hasItem(7) != 1 && game.hasItem(8) != 1) || game.getPicPoints() > 0))) {
                return true;
            }
        }

        else if (energy > 1.5f || game.posInLight(me)) {
            if ((move_state == END_GAME && distance(me, other) < 0.5f) ||
                (game.posInArea(other) >= 0 &&
                 game.isCameraOn() &&
                 game.isFacingOther() &&
                 ((game.hasItem(7) != 1 && game.hasItem(8) != 1) || game.getPicPoints() > 0))) {
                return true;
            }
        }
    }
    else if (game.posInArea(other) >= 0 && // in the beginning, they cannot have mirror!
             energy > 3 &&
             game.isFacingOther()) {
        return true;
    }
    return false;

}

bool checkUpload(){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) ) return true;
    return false;
}

bool inBounds(float loc[]) {
    return (abs(loc[0]) < 0.64f &&
            abs(loc[1]) < 0.8f &&
            abs(loc[2]) < 0.64f);
}

void blah() {
    DEBUG(("[FIB, 0, TIME, %d]\n", time));
DEBUG(("[FIB, 1, FRED, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f]\n", 
            me[0] *  -17 ,
            me[1] *  2   ,
            me[2] *  9   ,
            me[3] *  19  ,
            me[4] *  -27 ,
            me[5] *  -8  ,
            me[6] *  29  ,
            me[7] *  3   ,
            me[8] *  -13 ,
            me[9] *  -21 ,
            me[10] *  4   ,
            me[11] *  12));
DEBUG(("[FIB, 2, GEORGE, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f]\n",
            other[0] *  17 ,
            other[1] *  -2   ,
            other[2] *  -9   ,
            other[3] *  -19  ,
            other[4] *  27 ,
            other[5] *  8  ,
            other[6] *  -29  ,
            other[7] *  -3   ,
            other[8] *  13 ,
            other[9] *  21 ,
            other[10] *  -4   ,
            other[11] *  -12));
}


