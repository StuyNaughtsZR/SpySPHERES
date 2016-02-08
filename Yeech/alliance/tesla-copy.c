/*******************************************************************************
 * SHA-2468 Code -- Copy of Team Tesla's Strategy                              *
 *                                                                             *
 * Authors                                                                     *
 *  Yicheng Wang                                                               *
 *                                                                             *
 * Description                                                                 *
 *  1 item, shootout, get mirror, rush back in light, rush to origin/get E     *
 *                                                                             *
 ******************************************************************************/

/* TODO
 *  TODO-List
 */

/* Dev Log
 *  Project Created: 2015-12-02 11:51 - Yicheng W.
 */

/* INIT ***********************************************************************/
/* move_states */
#define GET_ITEM 0
#define GET_MIRROR 1
#define RUSH_BACK_IN_LIGHT 2
#define GET_ENERGY 3
#define END_GAME 4

/* rotate_states */
#define FACE_OTHER 5
#define UPLOAD 6

#define PIC_ENERGY_THRESHOLD 1
#define START_ROTATION_TIME 17
#define START_DRIFTING_TIME 5
#define STOP_DRIFTING_TIME 25

ZRState me, other;

int targetID;
float target[3];

float temp[3];

float origin[3];

int i;

int time;

int move_state, rotate_state;

bool am_i_blue;

void init() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    
    if (me[0] > 0) { // i am blue
        am_i_blue = true;
        game.getItemLoc(temp, 3);
        game.getItemLoc(target, 5);
        targetID = (temp[2] < target[2]) ? 3 : 5; // set targetID to higher target
    }
    else { // i am red
        am_i_blue = false;
        game.getItemLoc(temp, 4);
        game.getItemLoc(target, 6);
        targetID = (temp[2] < target[2]) ? 4 : 6; // set targetID to higher target
    }

    game.getItemLoc(target, targetID);

    move_state = GET_ITEM;
    rotate_state = -1; // will change at t=17
}

/* MAIN ***********************************************************************/
void loop() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    time = game.getCurrentTime();

    change_state();
    activate_mirror();

    switch (rotate_state) {
        case FACE_OTHER:
            if (time > START_ROTATION_TIME) {
                mathVecSubtract(temp, other, me, 3);
                api.setAttitudeTarget(temp);
                if (checkPhoto()) {
                    game.takePic();
                }
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
        case GET_ITEM:
            if (time < START_DRIFTING_TIME || time > STOP_DRIFTING_TIME) {
                api.setPositionTarget(target);
            }
            break;

        case GET_MIRROR:
            api.setPositionTarget(target);
            break;

        case RUSH_BACK_IN_LIGHT:
            break;
    }
}
/* CHANGE *********************************************************************/
void change_state() {
    if (game.getMemoryFilled() == 2) {
        rotate_state = UPLOAD;
    }

    if (rotate_state == UPLOAD && game.getMemoryFilled() == 0) {
        rotate_state = FACE_OTHER;
    }

    if (move_state == GET_ITEM) {
        if (game.hasItem(targetID) != -1) {
            if (am_i_blue) {
                targetID = 7;
            }
            else {
                targetID = 8;
            }
            game.getItemLoc(target, targetID);
            move_state = GET_MIRROR;
        }
    }

    if (move_state == GET_MIRROR) {
        if (game.hasItem(targetID) == -1) {
            move_state = RUSH_BACK_IN_LIGHT;
        }
    }

}
/* UTILS **********************************************************************/
/* activate_mirror: handles mirror activation logic
 * TODO written by young/joel, please explain
 */
void activate_mirror (){
    // TODO
}

/* checkPhoto: checks if we can take a photo
 * TODO long description
 */
bool checkPhoto (){
    return (game.getEnergy() > PIC_ENERGY_THRESHOLD &&
            game.isCameraOn() &&
            game.posInArea(other) > -1 &&
            game.isFacingOther() &&
            game.getPicPoints() > 0);
}

bool checkUpload(){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) ) return true;
    return false;
}

float distance(float p1[], float p2[]){
    mathVecSubtract(temp,p1,p2,3);
    return mathVecMagnitude(temp,3);
}

