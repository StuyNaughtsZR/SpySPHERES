ZRState me;
ZRState other;

float items[9][3];
//items 0-2: Energy
//3-6: Score
//7-8: Mirror 
//7 is red side, 8 is blue side

float retreat[2][3];
//if we're winning stay in dark zone

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

int itemNum; //holds item number we are going for

int i; //counter

int state;

bool color;

void init(){
    api.getMyZRState(me);
	api.getOtherZRState(other);
	
	for(i = 0; i < 9; i++){
	    game.getItemLoc(items[i],i);
	    itemBool[i] = true;
	}
	
	origin[0] = origin[1] = origin[2] = earth[0] = earth[1] = 0.0f;
	
	earth[2] = 1.0f;
	
	itemNum = recalibrate(0,2);
	
	state = 0;
	
	if(me[0] > 0){ //blue
	    memcpy(retreat[0],items[8],3*sizeof(float));
	    memcpy(retreat[1],items[8],3*sizeof(float));
	    retreat[1][1] *= -1;
	}
	else{ //red
	    memcpy(retreat[0],items[7],3*sizeof(float));
	    memcpy(retreat[1],items[8],3*sizeof(float));
	    retreat[1][1] *= -1;
	}
	
	DEBUG(("Stuy-Naught!"));
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	getItemArray();
	
	if(game.getFuelRemaining() == 0){
	    game.takePic();
	}
	
	if((game.getCurrentTime() > 165 || game.getFuelRemaining() < 10) && game.getMemoryFilled() >= 1){
	    state = 1; //game changer
	}
	
	if(game.getEnergy() < 0.6 && !game.posInLight(me))
	    state = 2;
	
	DEBUG(("%d",state));
	
	switch(state) {
	    case 0: //do the stuffs
	        if(!itemBool[itemNum]){
	            itemNum = recalibrate(4,6);
	        }
	        memcpy(target,items[itemNum],3*sizeof(float));
	        
	        api.setPositionTarget(target);
	            
	        mathVecSubtract(facing, other, me, 3);
	        api.setAttitudeTarget(facing);
	        
	        if( checkPhoto(me,other,facing) )
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2 && game.getCurrentTime() > 21)
	            state = 1;
	            
	        if(game.getScore() > game.getOtherScore() && game.getCurrentTime() > 21)
	            state = 3;
	        break;
	        
	    case 1: //upload
	        api.setVelocityTarget(origin);
	        api.setAttitudeTarget(earth);
	        
	        if( game.getEnergy() > 1 && checkUpload(me) )
	            game.uploadPics();
	            
	        if( game.getMemoryFilled() == 0){
	            if(game.getScore() < game.getOtherScore())
	                state = 0;
	            else
	                state = 3;
	        }
	        break;
	        
	   case 2: //chill out
	        api.setVelocityTarget(origin);
	        
	        if(game.posInLight(retreat[0]))
    	        api.setPositionTarget(retreat[1]);
    	    else
    	        api.setPositionTarget(retreat[0]);
	        
	        if(game.posInLight(me)){
	            if(game.getMemoryFilled() == 2)
	                state = 1;
	            else if(game.getScore() < game.getOtherScore())
	                state = 0;
	            else
	                state = 3;
	        }
	        break;
	        
	   case 3:
	        mathVecSubtract(facing, other, me, 3);
	        api.setAttitudeTarget(facing);
	        
	        if( checkPhoto(me,other,facing) && game.getMemoryFilled() < 2)
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = 1;
	        
	        if(game.getLightSwitchTime() < 2 && game.getNumMirrorsHeld() > 0 && game.posInDark(me))
	            game.useMirror();
	            
	        if(game.getMirrorTimeRemaining() > 2)
	            api.setPositionTarget(retreat[0]);
	        else{
    	        if(game.posInLight(retreat[0])){
    	            api.setPositionTarget(retreat[1]);
    	        }
    	        else
    	            api.setPositionTarget(retreat[0]);
	        }
	        
	        if(game.getScore() < game.getOtherScore())
	            state = 0;
	        break;
	}
}

int recalibrate(int start, int end){
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
    if( distance(me,other) > 0.5 && game.posInLight(other) ){
        if( game.isFacingOther() && game.getEnergy() > 1 && game.getPicPoints() > 0)
            return true;
    }
    return false;
}

bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) )
        return true;
    return false;
}

void getItemArray(){
	for(i = 0; i < 9; i++){
	    if(itemBool[i] && game.hasItem(i) != -1){
	        itemBool[i] = 0;
	    }
	}
}

float distance(float p1[], float p2[]){
	mathVecSubtract(temp,p1,p2,3);
	return mathVecMagnitude(temp,3);
}
