/******************************************************************************\
 * SHA-2468 Alliance Competitior -- MIRROR ITEM BOT                            *
 *                                                                             *
 * Authors                                                                     *
 *  Written by Young Kim, Improved/Edited by Yicheng Wang                      *
 *                                                                             *
 * Description                                                                 *
 *  // TODO Improve  END_GAME                                                  *
 *                                                                             *
 \*****************************************************************************/

/* TODO
 *
 */

/* Dev Log
 *  Project Created: 2015-11-22 21:59 - Yicheng W.
 */

/* INIT ***********************************************************************/


/*----------------MOVE STATES----------------*/
#define GET_MIRROR 0
#define GET_SCORE_PACKS 1
#define END_GAME 2
#define STOP 3

/*----------------ATTITUDE STATES----------------*/
#define FACE_ENEMY 4
#define UPLOAD 5

/* ETA SYSTEM */
#define CONE_ANGLE_COSINE 0.997f
#define MAX_INT 9999
#define MY_SPEED 0.027 // average estimate, subject to change


ZRState me;
ZRState other;

float items[9][3];
//items 0-2: Energy
//3-6: Score
//7-8: Mirror

int itemBool[9];
//more efficient than calling 'game.hasItem()' every second // no it's not

float target[3];
//temporary target

float temp[3];
//for setPositionTarget

float origin[3];
//origin target

float earth[3];

float energy;

int itemNum; //holds item number we are going for

int i; //counter

int time;

int move_state;
int att_state;

void init(){
    for (int i = 0 ; i < 9 ; i++) {
        game.getItemLoc(items[i], i);
    }
	updateGameState();
	
	origin[0] = origin[1] = origin[2] = earth[0] = earth[1] = 0.0f;
	
	earth[2] = 1.0f;
	
	itemNum = recalibrate(3,6);
	memcpy(target,items[itemNum],3*sizeof(float)); //do this here so we don't have to do it in GET_MIRROR
	
	move_state = GET_SCORE_PACKS;
	att_state = FACE_ENEMY;
	
	DEBUG(("Hello from SHA-2468!"));
}

/* MAIN ***********************************************************************/

void loop(){
    blah();
    /*----------------PRE-SWITCH FUNCTIONS----------------*/
	updateGameState();
	changeState();

	//DEBUG(("move state: %d\n", move_state));
	//DEBUG(("MY SPEED: %f\n", mathVecMagnitude(me + 3, 3)));

	if(game.getFuelRemaining() == 0)
	    game.takePic();
	
	/*
	temp position to estimate in advance
	*/
    memcpy(temp, me, 3*sizeof(float));
	temp[1] -= 0.17;
	
	/*
	If time > 130 and we have memory and substantial energy
		and we are not going to be uploading while in dark 
	THEN upload
	*/
	if(time > 130 &&
	   game.getMemoryFilled() > 0 &&
	   energy > 2 &&
	   game.posInDark(temp) &&
	   !game.posInLight(me))
	{
	    att_state = UPLOAD;
	}
	
	/*
	If we have a mirror and our temp position is not in dark 
		If we are far enough to take a picture and our y coordinate is lower and we have enough energy
		OR
		We have taken two pictures
	THEN use the mirror
	*/
	if(game.getNumMirrorsHeld() > 0 &&
	   !game.posInDark(temp) && 
	   ((distance(me,other) > 0.45 &&
	     me[1] < other[1] &&
	     game.getOtherEnergy() > 1) ||
	   game.getMemoryFilled() == 2)){

	   game.useMirror(); 
	   if(game.getMemoryFilled() > 0) att_state = UPLOAD; //if we have a picture then upload while mirror
	} 
	
	/*----------------MOVE STATES----------------*/
	switch(move_state) {
			
	    case GET_SCORE_PACKS: //second score pack
			/*
			Set target to the item we are going for, and face the enemy
			*/
	        memcpy(target,items[itemNum],3*sizeof(float));
			if(time < 4 || time > 17) api.setPositionTarget(target); //save fuel and energy
	        break;
			
		case GET_MIRROR:
		    /*
		    Go for mirror, then switch to GET_SCORE_PACKS if we have it.
		    */	        
			memcpy(target,items[itemNum], 3*sizeof(float));
			api.setPositionTarget(target);
	        
			break;
		
		case END_GAME: 
		    /*
		    If we are not above a certain threshold, try and reach it.
		    Else:
		        If we are in dark, move towards the -Y slowly.
		        If we are in grey/light, move towards the -Y quickly.
		    */
		    if( me[2] > -0.475 ){
                if(me[0] > 0){
                    target[0] = 0.53;
                } else {
                    target[0] = -0.53;
                }
    			target[1] = 0.4;
    			target[2] = -0.5;
    			api.setPositionTarget(target);
		    }
		    else{
		        if( me[1] > -0.5 ){
					
					/* 
					Make sure our x coordinate is in the bounds of the game
					*/
		            if(fabsf(me[0]) > 0.53) me[0] = me[0]/fabsf(me[0]) * 0.53;
					
					/*
					If we are in dark, move slowly
					Else move fast
					*/
    		        if( game.posInDark(me) ){
    		            memcpy( target, me, 3*sizeof(float) );
    		            target[1] -= 0.04;
    		        }
    		        else{
    		            memcpy( target, me, 3*sizeof(float) );
    		            target[1] -= 0.25;
    		        }
					
					/*
					Keep our y-coor greater than or equal to 0.125
					*/
					if( target[1] < 0.125 ) target[1] = 0.125;
					
    		        api.setPositionTarget(target);
		        }
		        else{
		            api.setPositionTarget(me);
		        }
		    }
			break;
	        
	   case STOP:
            api.setVelocityTarget(origin);	        
	        break;
	}
	/*----------------ATTITUDE STATES----------------*/
	switch(att_state){

	    case FACE_ENEMY:
	        mathVecSubtract(temp, other, me, 3);
	        api.setAttitudeTarget(temp);
	        if(checkPhoto(me,other))
	            game.takePic();
	        break;
	        
	    case UPLOAD:			
	        /*
	        Face Earth
	        If we have enough energy and are facing correctly, upload.
	        */
	        api.setAttitudeTarget(earth);
	        
	        if( energy > 1 && checkUpload(me) )
	            game.uploadPics();
	        break;
	}
}

/* CHANGE *********************************************************************/
void changeState() {

    if ((time > 165 || game.getFuelRemaining() < 10) && // end game stage
        game.getMemoryFilled() > 0) {
        att_state = UPLOAD;
    }

    if (game.getMemoryFilled() == 2) {
        att_state = UPLOAD;
    }

    if (game.getMemoryFilled() == 0) {
        att_state = FACE_ENEMY;
    }

    if (move_state == STOP) {
        move_state = GET_SCORE_PACKS;
    }

    if (move_state == GET_SCORE_PACKS || move_state == GET_MIRROR) {
        if (itemBool[itemNum] != -1) {
            itemNum = recalibrate(3,6);
            if (itemNum == -1) {
				itemNum = recalibrate(7,8);
				if(itemNum != -1) move_state = GET_MIRROR;
				else move_state = END_GAME;
            }
            else {
                move_state = GET_SCORE_PACKS;
            }
        }
    }

    if (energy < 1.3 && game.posInDark(me)) {
        move_state = STOP;
    }
}

/* UTILS **********************************************************************/
void updateGameState() {
    api.getMyZRState(me);
    api.getOtherZRState(other);
    for (i = 0 ; i < 9 ; i++) {
        itemBool[i] = game.hasItem(i);
    }
	energy = game.getEnergy();
	time = game.getCurrentTime();
}

int recalibrate(int start, int end){ //gets index of closest item in item range[start:end] 
    float shortdist = 100.0f;
    int index = -1;
    for(int i = start; i < end + 1; i++){
        if (itemBool[i] == -1) {
            temp[0] = distance(items[i], me) / MY_SPEED;
	    if (temp[0] * MY_SPEED < shortdist &&
                temp[0] < getETA(other, items[i]) &&
                (distance(me, items[i]) > 0.3 ||
                 distance(me, items[i]) < distance(other, items[i]))) {
                shortdist = distance(me, items[i]);
                index = i;
            }
        }
    }
    return index;
}

bool checkPhoto( float me[], float other[] ){
    return (distance(me, other) > 0.5 &&
            !(game.posInDark(other)) &&
            energy > 1.1 &&
            game.isFacingOther() &&
            game.getPicPoints() > 0);
}

bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) ) return true;
    return false;
}

float distance(float p1[], float p2[]){
    mathVecSubtract(temp,p1,p2,3);
    return mathVecMagnitude(temp,3);
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
    float cosine = mathVecInner(temp, sphere + 3, 3) / (distance * speed);
    if (cosine > CONE_ANGLE_COSINE) { // means that the angle is within the boundary
        return distance / speed;
    }
    return MAX_INT;
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

