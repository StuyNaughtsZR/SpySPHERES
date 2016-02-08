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

int itemNum; //holds item number we are going for

int i; //counter

int state;

void init(){
    api.getMyZRState(me);
	api.getOtherZRState(other);
	
	for(i = 0; i < 9; i++){
	    game.getItemLoc(items[i],i);
	    itemBool[i] = true;
	}
	
	for(i = 0; i < 3; i++){
 	    origin[i] = 0.0f;
 	}
	
	origin[0] = origin[1] = origin[2] = earth[0] = earth[1] = 0.0f;
	
	earth[2] = 1.0f;
	
	itemNum = 6;
	
	state = 0;
	
	DEBUG(("Stuy-Naught!"));
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	getItemArray();
	
	if((game.getCurrentTime() > 165 || game.getFuelRemaining() < 10) && game.getMemoryFilled() >= 1){
	    state = 1; //game changer
	    DEBUG(("GAME CHANGER"));
	}
	
	if(game.getEnergy() < 1 && !game.posInLight(me))
	    state = 2;
	
	//if(game.getScore() > game.getOtherScore() && game.posInDark(me) && game.posInDark(other) && distance(other,origin)<0.65 && distance(me,other) < 0.6)
	//    state = 3;
	
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
	            
	        if(game.getMemoryFilled() == 2)
	            state = 1;
	        break;
	        
	    case 1: //upload
	        api.setVelocityTarget(origin);
	        api.setAttitudeTarget(earth);
	        
	        if( game.getEnergy() > 1 && checkUpload(me) )
	            game.uploadPics();
	            
	        if( game.getMemoryFilled() == 0){
	            state = 0;
	        }
	        break;
	        
	   case 2: //chill out
	        api.setVelocityTarget(origin);
	            
	        mathVecSubtract(facing, other, me, 3);
	        mathVecNormalize(facing,3);
	        
	        if(!mathVecInner(me+6,facing,3) > 0.9689f)
	            api.setAttitudeTarget(facing);
	        
	        if(game.posInLight(me)){
	            if(game.getMemoryFilled() == 2)
	                state = 1;
	            else
	                state = 0;
	        }
	        break;
	        
	   case 3: //rush case throwback
	        mathVecSubtract(temp, other, origin, 3);
        	mathVecNormalize(temp,3);
        	for(int i = 0; i < 3; i++){
        	    temp[i] *= -0.1; 
        	    temp[i] += other[i];
        	}
        	memcpy(target,temp,3*sizeof(float));
        	
        	if(distance(me,other) > 0.15)
	            api.setPositionTarget(target);
	        else
	            api.setVelocityTarget(origin);
	        
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
