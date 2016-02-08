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

int state, tempState;

bool setup; //false after getting mirror

int counter;

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
	
	target[0] = target[1] = target[2] = 0.0f;
	
	earth[0] = earth[1] = 0.0f;
	
	earth[2] = 1.0f;
	
	if( distance(me, items[7]) < distance(me, items[8]) )
	    itemNum = 7;
	else
	    itemNum = 8;
	
	state = 0;
	tempState = 0;
	setup = true;
	counter = 0;
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	getItemArray();
	
	if(game.getNumMirrorsHeld() >= 1 && ((game.posInDark(me) && game.getLightSwitchTime() < 2) || (game.posInLight(me) || game.posInGrey(me)))){
        if(tempState == state){
            tempState = state;
            state = 99; //deploy mirror
        }
    }
	
	if((game.getCurrentTime() > 175 || game.getFuelRemaining() < 5) && game.getMemoryFilled() >= 1){
	    state = 1; //game changer
	}
	
	if(game.getMemoryFilled() == 1)
        counter++;
        
    if(counter>10){
        state = 1;
        counter = 0;
    }
	
	switch(state) {
	    case 0: //take pictures while getting mirror
	        if(setup){
    	        memcpy(target,items[itemNum],3*sizeof(float));
    	        
    	        if( distance(me,target) > 0.5 )
    	            mySetPositionTarget(target, 0.2, 12 );
    	        else
    	            api.setPositionTarget(target);
    	            
    	        mathVecSubtract(facing, other, me, 3);
    	        mathVecNormalize(facing,3);
    	        api.setAttitudeTarget(facing);
    	        
    	        if( checkPhoto(me,other,facing) && game.getMirrorTimeRemaining() >= 0 )
    	            game.takePic();
    	            
    	        //if(game.getMemoryFilled() == 2)
    	        //    state = 1;
    	        
    	        if(game.getNumMirrorsHeld() >= 1 && ((game.posInDark(me) && game.getLightSwitchTime() < 2) || (game.posInLight(me) || game.posInGrey(me)))){
                    if(tempState == state){
                        tempState = state;
                        state = 99; //deploy mirror
                    }
	            }
	        }
	        else{
	            state = 2;
	        }
	        break;
	        
	    case 1: //upload
	        api.setAttitudeTarget(earth);
	        
	        if( game.getEnergy() > 1 && checkUpload(me) )
	            game.uploadPics();
	            
	        if( game.getMemoryFilled() == 0){
	            if (setup){
	                state = 0;
	            }
	            else{
	                state = 2;
	            }
	        }
	        
	        DEBUG(("UPLOAD"));
	        break;
	        
	    case 2: //main
	        if( distance(me, other) < 0.53)// || me[2] > -0.5)
	            state = 98; //back off
	            
	        target[0] = other[0];
	        target[1] = other[1];
	        target[2] = -0.45f;
	        
	        if( distance(me,target) > 0.3 )
	            mySetPositionTarget(target, 0.2, 12 );
	        else
	            api.setPositionTarget(target);
	            
	        mathVecSubtract(facing, other, me, 3);
	        mathVecNormalize(facing,3);
	        api.setAttitudeTarget(facing);
	        
	        if( checkPhoto(me,other,facing) )
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = 1;
	            
	        break;
	        
	    case 98: //backoff
	        if( distance(me, other) >= 0.53 )//&& me[2] > -0.5)
	            state = 2; //go back to main
	            
	        mathVecSubtract(temp, other, origin, 3);
        	mathVecNormalize(temp,3);
        	for(int i = 0; i < 2; i++){
        	    temp[i] *= -0.2; 
        	    temp[i] += other[i];
        	}
        	memcpy(target,temp,3*sizeof(float));
	        target[2] = -0.45f;
	        
	        if( distance(me,target) > 0.3 )
	            mySetPositionTarget(target, 0.2, 12 );
	        else
	            api.setPositionTarget(target);
	            
	        mathVecSubtract(facing, other, me, 3);
	        mathVecNormalize(facing,3);
	        api.setAttitudeTarget(facing);
	        
	        if( checkPhoto(me,other,facing) )
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = 1;
	            
	        DEBUG(("BACKOFF"));
	        break;
	        
	    case 99: //deploy mirror
	        game.useMirror();
	        DEBUG(("%d", game.getCurrentTime()));
	        state = tempState;
	        setup = false;
	        break;
	        
	}
}

bool checkPhoto( float me[], float other[], float facing[] ){
    if( distance(me,other) > 0.5 && game.posInLight(other) ){
        if( mathVecInner(me+6,facing,3) > 0.9689f && game.getEnergy() > 1 && game.getPicPoints() > 0)
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

bool mySetPositionTarget(float *target, float max_dist, float max_speed) { //Shaumik's code
   const float max_acc = 0.008; //retest
   float time_allot = 4.5;
   float disp[3];
   float dist;
   
   mathVecSubtract(disp, target, me, 3);
   dist = mathVecNormalize(disp, 3);
   if( dist < max_dist && mathVecMagnitude(me + 3,3) < max_speed ) {
       api.setPositionTarget(target);
       return true;
   }
   if( (dist< 0.1) || ( (0.5*max_acc*time_allot*time_allot+mathVecMagnitude(me + 3,3)*time_allot) >= dist) ) {
       api.setPositionTarget(target);
       return false;
   }
   else {
       for(i = 0; i < 3; i++) {
           disp[i] *= (dist / 2.0);
       }
       api.setVelocityTarget(disp);
       return false;
   }
}

float distance(float p1[], float p2[]){
	mathVecSubtract(temp,p1,p2,3);
	return mathVecMagnitude(temp,3);
}
