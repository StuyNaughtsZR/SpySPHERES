/*
written by yk with dan in spirit
comments are for my own sanity
*/
#define GET_MIRROR 0
#define GET_SCORE_PACKS 1
#define GO_THROUGH_LIGHT 2
#define STAY_IN_DARK 3
#define UPLOAD 4
#define STOP 5

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

int state;

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
	
	state = GET_MIRROR;
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	getItemArray();
	energy = game.getEnergy();
	time = game.getCurrentTime();
	
	/*
	if the end of the world is approaching
	*/
	if((time > 165 || game.getFuelRemaining() < 10) && game.getMemoryFilled() >= 1){
	    game.takePic();
	    state = UPLOAD; 
	}
	
	if(game.getEnergy() < 1.3 && game.getMemoryFilled() > 0 && !game.posInLight(me))
	    state = STOP;
	
	if(game.getFuelRemaining() == 0)
	    game.takePic();
	
	/*
	This is a check to see when we should use the mirror
	*/
    memcpy(temp, me, 3*sizeof(float));
	temp[1] -= 0.18;
	
	if(!game.posInDark(temp) && ((distance(me,other) > 0.45 && me[1] < other[1] && game.getOtherEnergy() > 1) 
	    || 
	    game.getMemoryFilled() == 2)){
	   game.useMirror(); 
	   if(game.getMemoryFilled() > 0) state = UPLOAD; //if we have a picture then upload while mirror
	} 
	
	mathVecSubtract(facing, other, me, 3);
	api.setAttitudeTarget(facing);
	
	DEBUG(("STATE: %d", state));
	
	switch(state) {
		case GET_MIRROR:
		    /*
		    Go for mirror, then switch to GET_SCORE_PACKS if we have it.
		    */
	        if( !itemBool[itemNum] ) { 
	            itemNum = recalibrate(3,6);
				state = GET_SCORE_PACKS;
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
				itemNum = recalibrate(3,6);
				if(itemNum == 3 && time > 100)
					state = GO_THROUGH_LIGHT;
			}
			
			/*
			Set target to the item we are going for, and face the enemy
			*/
	        memcpy(target,items[itemNum],3*sizeof(float));
	        api.setPositionTarget(target);
	        
	        /*
	        If we are facing them, take photo
	        If we have full memory go to UPLOAD
	        */
	        if(checkPhoto(me,other,facing))
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = UPLOAD;
	        break;
		
		case GO_THROUGH_LIGHT: 
		    /*
		    Start off by setting our target to where we
		    */
			memcpy(target,me,3*sizeof(float));
			target[2] = -0.45;
			
			/*
			game.getDarkGreyBoundary() returns the closest dark/grey boundary
			To deal with this, I found at what time one of the light zones fully went off the screen.
			*/
			if(time > 104 && game.getScore() > game.getOtherScore())
			    target[1] = game.getDarkGreyBoundary() - 0.05;
			else
			    target[1] = game.getDarkGreyBoundary() - 0.5;
			
			if(target[1] < -0.4) target[1] = -0.4;
			
			api.setPositionTarget(target);
			
			/*
			More of the same
			*/
			if(checkPhoto(me,other,facing))
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = UPLOAD;
	        
	        /*
	        If we're near the end, just try and stay in the dark zone. 
	        */
	        if(time > 164)
	            state = STAY_IN_DARK;
			break;
			
		case STAY_IN_DARK:
		    /*
		    Stay in the dark zone during the beginning/mid game
		    */
		    if(time < 164 && time > 113) state = GO_THROUGH_LIGHT;
		    
		    memcpy(target,me,3*sizeof(float));
			
			target[1] = 0.65;
			target[2] = -0.45;
			
			api.setPositionTarget(target);
	        
	        /*
	        More of the same
	        */
	        if(checkPhoto(me,other,facing))
	            game.takePic();
	        
	        if(game.getMemoryFilled() == 2)
	            state = UPLOAD;
			break;
		
	    case UPLOAD: 
	        if(energy > 2) {
	            api.setPositionTarget(target);
			} else {
	            api.setVelocityTarget(origin);
			}
	        api.setAttitudeTarget(earth);
	        
	        /*
	        If we have enough energy and are facing correctly
	        */
	        if( energy > 1 && checkUpload(me) )
	            game.uploadPics();
	            
	        if( game.getMemoryFilled() == 0){
	            /*
    	        Decide which case to go back to
    	        */
	            if(time > 164){
	                state = STAY_IN_DARK;
	            } else if(itemNum < 3){
					state = GO_THROUGH_LIGHT;
				} else { 
					state = GET_SCORE_PACKS;
				}
				
	        }
	        break;
	        
	   case STOP:
	        /*
	        Basically if we are in the beginning of the game and we don't have energy
	        There are 2 possibilities
	            1) We took a picture 
	            2) Someone else went for our score pack
	        To deal with this, I set our velocity back towards the start to make sure that
	            we stay in bounds. 
	        
	        Else, try and stay still.
	        */
            target[0] = target[2] = 0.0f;
            target[1] = -0.5f;
            if(time < 100){
                target[1] = -1.0f;
            } else if(time > 130){
                target[1] = 0.0f;
            }
            api.setVelocityTarget(target);
	        
	        if(!mathVecInner(me+6, facing, 3) > 0.9689f) api.setAttitudeTarget(facing);
	        
	        if(game.posInLight(me)){
	            if(game.getMemoryFilled() == 2) {
	                state = UPLOAD;
				} else if (itemNum < 3) {
				    state = GET_SCORE_PACKS;
				} else {
	                state = GO_THROUGH_LIGHT;
				}
	        }
	        break;
	}
}

int recalibrate(int start, int end){ //gets index of closest item in item range[start:end] 
    float shortdist = 100.0f;
    int index = end;
    for(int i = start; i < end + 1; i++){
        if(itemBool[i] && distance(me,items[i]) < shortdist){
            shortdist = distance(me,items[i]);
            index = i;
        }
    }
    return index;
}

bool checkPhoto( float me[], float other[], float facing[] ){
    if( distance(me,other) > 0.5 && !(game.posInDark(other)) ){
        if( game.isFacingOther() && energy > 1 && game.getPicPoints() > 0)
            return true;
    }
    return false;
}

bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) ) return true;
    return false;
}

void getItemArray(){
	for(i = 0; i < 9; i++){
	    if(itemBool[i] && game.hasItem(i) != -1){
	        itemBool[i] = 0;
	        if(i == 6)
	            items[6][2] = -0.45; //aim 0.05 higher than the actual powerup
	    }
	}
}

float distance(float p1[], float p2[]){
	mathVecSubtract(temp,p1,p2,3);
	return mathVecMagnitude(temp,3);
}
