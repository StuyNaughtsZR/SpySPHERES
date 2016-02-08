/*
written by yk with dan in spirit
comments are for my own sanity
*/

/*----------------MOVE STATES----------------*/
#define GET_MIRROR 0
#define GET_SCORE_PACKS 1
#define END_GAME 2
#define STOP 3

/*----------------ATTITUDE STATES----------------*/
#define FREEZE 4
#define FACE_ENEMY 5
#define UPLOAD 6

ZRState me;
ZRState other;

float items[9][3];
//items 0-2: Energy
//3-6: Score
//7-8: Mirror

int itemBool[9];
//more efficient than calling 'game.hasItem()' every second

float facing[3];
//where you are facing; attitude

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
    api.getMyZRState(me);
	api.getOtherZRState(other);
	
	for(i = 0; i < 9; i++){
	    game.getItemLoc(items[i],i);
	    itemBool[i] = true;
	}
	
	origin[0] = origin[1] = origin[2] = earth[0] = earth[1] = 0.0f;
	
	earth[2] = 1.0f;
	
	itemNum = recalibrate(6,7);
	memcpy(target,items[itemNum],3*sizeof(float)); //do this here so we don't have to do it in GET_MIRROR
	
	move_state = GET_MIRROR;
	att_state = FACE_ENEMY;
	
	DEBUG(("Hello from SHA-2468!"));
}

void loop(){
    /*----------------PRE-SWITCH FUNCTIONS----------------*/
	api.getMyZRState(me);
	api.getOtherZRState(other);
	getItemArray();
	energy = game.getEnergy();
	time = game.getCurrentTime();
	
	/*
	if the end of the world is approaching
	*/
	if((time > 165 || game.getFuelRemaining() < 10) && game.getMemoryFilled() > 0){
	    game.takePic();
	    att_state = UPLOAD; 
	}
	
	if(energy < 1.3 && !game.posInLight(me))
	    move_state = STOP;
	
	if(game.getFuelRemaining() == 0)
	    game.takePic();
	
	/*
	This is a check to see when we should use the mirror
	*/
    memcpy(temp, me, 3*sizeof(float));
	temp[1] -= 0.17;
	
	if(time > 130 && game.getMemoryFilled() > 0 
	    && energy > 2 && game.posInDark(temp) && !game.posInLight(me))
	{
	    att_state = UPLOAD;
	}
	
	if(game.getNumMirrorsHeld() > 0 && !game.posInDark(temp) 
	    && 
	    ((distance(me,other) > 0.45 && me[1] < other[1] && game.getOtherEnergy() > 1) 
	        || game.getMemoryFilled() == 2)){
	   game.useMirror(); 
	   if(game.getMemoryFilled() > 0) att_state = UPLOAD; //if we have a picture then upload while mirror
	} 
	
	/*----------------MOVE STATES----------------*/
	switch(move_state) {
		case GET_MIRROR:
		    /*
		    Go for mirror, then switch to GET_SCORE_PACKS if we have it.
		    */
	        if( !itemBool[itemNum] ) { 
	            itemNum = recalibrate(3,6);
				move_state = GET_SCORE_PACKS;
			}
	        
	        if(time < 4 || time > 17) api.setPositionTarget(target); //save fuel and energy
			break;
			
	    case GET_SCORE_PACKS: //second score pack
	        /*
	        First line written by Joel - avoids dances
	        If all score packs are gone, then go to STAY_IN_DARK
	        */
	        if(distance(me,items[itemNum]) < 0.3 && distance(me,items[itemNum]) > distance(other,items[itemNum]) )
	            itemBool[itemNum] = false;
	        if(!itemBool[itemNum]){ 
	            if(itemNum == 6)
					move_state = END_GAME;
				itemNum = recalibrate(3,6);
			}
			
			/*
			Set target to the item we are going for, and face the enemy
			*/
	        memcpy(target,items[itemNum],3*sizeof(float));
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
		            if(fabsf(me[0]) > 0.53) me[0] = me[0]/fabsf(me[0]) * 0.53;
		            
    		        if( game.posInDark(me) ){
    		            memcpy( target, me, 3*sizeof(float) );
    		            target[1] -= 0.04;
    		            api.setPositionTarget(target);
    		        }
    		        else{
    		            memcpy( target, me, 3*sizeof(float) );
    		            target[1] -= 0.25;
    		            api.setPositionTarget(target);
    		        }
		        }
		        else{
		            api.setPositionTarget(me);
		        }
		    }
			break;
	        
	   case STOP:
	        att_state = FREEZE;
            api.setVelocityTarget(origin);
	        
	        if(game.posInLight(me)){
	            if(game.getMemoryFilled() == 2) {
	                att_state = UPLOAD;
				} else if( itemNum == 6 && itemBool[6] == 0 ){
					move_state = END_GAME;
				} else { 
					move_state = GET_SCORE_PACKS;
				}
	        }
	        break;
	}
	/*----------------ATTITUDE STATES----------------*/
	switch(att_state){
	    case FREEZE:
	        if(time > 120){
    	        mathVecSubtract(facing, other, me, 3);
    	        api.setAttitudeTarget(facing);
	        }
	        if(move_state != STOP){
	            if(game.getMemoryFilled() == 2)
	                att_state = UPLOAD;
	            else
	                att_state = FACE_ENEMY;
	        }
	        break;
	        
	    case FACE_ENEMY:
	        mathVecSubtract(facing, other, me, 3);
	        api.setAttitudeTarget(facing);
	        if(checkPhoto(me,other,facing))
	            game.takePic();
	        if(game.getMemoryFilled() == 2)
	            att_state = UPLOAD;
	        break;
	        
	    case UPLOAD:
	        /*
	        @Override
	        if not enough energy, then stop. 
	        */
	        if(energy < 2){
	            api.setVelocityTarget(origin);
			}
			
	        /*
	        Face Earth
	        If we have enough energy and are facing correctly, upload.
	        */
	        api.setAttitudeTarget(earth);
	        
	        if( energy > 1 && checkUpload(me) )
	            game.uploadPics();
	            
	        if( game.getMemoryFilled() == 0)
	            att_state = FACE_ENEMY;
	        break;
	}
}
