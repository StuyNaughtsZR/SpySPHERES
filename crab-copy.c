/******************************************************************************\
 * Alliance Crab Nebula Copy                                                   *
 *                                                                             *
 * Authors                                                                     *
 *  Yicheng                                                                    *
 *                                                                             *
 * Description                                                                 *
 *  1. Rush bottom score item on my side                                       *
 *  2. Go to top score item on my side                                         *
 *  3. Get Mirror on my side and wait at current location                      *
 *  4. Wait for light zone to come, when it does, activate mirror and rush     *
 *  through                                                                    *
 *  5. Go back and wait at the mirror                                          *
 *                                                                             *
 \*****************************************************************************/

/* TODO
 *  TODO-List
 */

/* Dev Log
 *  Project Created: 2015-11-24 11:38 - Yicheng W.
 */
/* TODO
 *  TODO-List
 */

/* Dev Log
 *  Project Created: <Insert Date>
 */

/* INIT ***********************************************************************/
/* for pre-processing commands, declaration and initialization */

#define SCORE_1 1 // get bottom score item
#define SCORE_2 2 // get top score item
#define MIRROR 3 // get mirror
#define END_GAME 4

#define TAKE_PICTURE 5
#define UPLOAD 6

ZRState me, other;

float targetPosition[3][3];

int top_item;

int move_state;
int rotate_state;

void init() {
    api.getMyZRState(me);
    api.getOtherZRState(other);

    if (me[0] > 0) { // I am blue
        game.getItemLoc(targetPosition[0], 3);
        game.getItemLoc(targetPosition[1], 5);
        top item = (targetPosition[0][2] < targetPosition[1][2]) ? 0 : 1;
    }
    else { // I am red
        f
    }
}

/* MAIN ***********************************************************************/
/* runs every second */

void loop() {
    api.getMyZRState(me);
    api.getOtherZRState(me);
}

/* CHANGE *********************************************************************/
/* state changing functions */

/* UTILS **********************************************************************/
/* other utilities used */
