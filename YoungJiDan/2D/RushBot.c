ZRState me;
ZRState other;

float items[9][3];
//items 0-3: Energy
//4-6: Score
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

bool legitPic;

int state;

void init(){
    api.getMyZRState(me);
	api.getOtherZRState(other);
	
	for(int i = 0; i < 9; i++){
	    game.getItemLoc(items[i],i);
	    itemBool[i] = true;
	}
	
	for(int i = 0; i < 3; i++){
 	    origin[i] = 0.0f;
 	}
	
	legitPic = false;
	
	target[0] = target[1] = target[2] = 0.0f;
	
	state = 0;
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	getItemArray();
	
	checkBounds();
	
	handInPics();
	
	if(distance(other, origin) < 0.2){
	    state = 1;
	}
	else{
	    state = 0;
	}
	
	switch(state) {
	    case 0: //follow and take pictures when opponent is not near origin
	        mathVecSubtract(temp, other, origin, 3);
        	mathVecNormalize(temp,3);
        	for(int i = 0; i < 3; i++){
        	    temp[i] *= -0.38; //needs function to do this
        	    temp[i] += other[i];
        	}
        	memcpy(target,temp,3*sizeof(float));
        	
        	DEBUG(("Target: %f, %f, %f",target[0],target[1],target[2]));
        	DEBUG(("Enemy Position: %f, %f, %f", other[1], other[2], other[3]));
        	
	        mySetPositionTarget(target, 0.47, 10);
	        
	        mathVecSubtract(facing, other, me, 3);
	        mathVecNormalize(facing,3);
	        api.setAttitudeTarget(facing);
	        
	        if(game.isCameraOn() && game.posInLight(me)){// && game.isFacingOther() && game.posInLight(other)){
	            DEBUG(("TAKING PICTURES"));
	            if(game.getPicPoints()>=2){
	                legitPic = true;
	            }
	            else{
	                legitPic = false;
	            }
	            
	            game.takePic();
	        }
	        break;
	        
	    case 1: //follow and take pictures when opponent is near origin
	        mathVecSubtract(temp, other, me, 3);
        	mathVecNormalize(temp,3);
        	for(int i = 0; i < 3; i++){
        	    temp[i] *= -0.05; //needs function to do this
        	    temp[i] += other[i];
        	}
        	memcpy(target,temp,3*sizeof(float));
        	
        	DEBUG(("Target: %f, %f, %f",target[0],target[1],target[2]));
        	DEBUG(("Enemy Position: %f, %f, %f", other[1], other[2], other[3]));
        	
	        mySetPositionTarget(target, 0.47, 10);
	        
	        mathVecSubtract(facing, other, me, 3);
	        mathVecNormalize(facing,3);
	        api.setAttitudeTarget(facing);
	        
	        if(game.isCameraOn() && game.posInLight(me)){// && game.isFacingOther() && game.posInLight(other)){
	            DEBUG(("TAKING PICTURES"));
	            if(game.getPicPoints()>=2){
	                legitPic = true;
	            }
	            else{
	                legitPic = false;
	            }
	            
	            game.takePic();
	        }
	        break;
	  
	    case 10: //recover if too far away
	        api.setPositionTarget(origin);
	        DEBUG(("COMING BACK"));
	        break;
	        
	}
}

void getItemArray(){
	for(int i = 0; i < 9; i++){
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
   int i;
   
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

void handInPics(){ //upload pictures
    if(game.getMemoryFilled() > 0){
        game.uploadPics();
    }
}

void checkBounds(){
        if(me[0]>=.75 || me[0]<=-.75 || me[1]>=.6 || me[1]<=-.6 || me[2]>=.6 || me[2]<=-.6){
            state = 10;
        }
        else{
            state = 0;
        }
}

float distance(float p1[], float p2[]){
	// Thanks Cornel
	mathVecSubtract(temp,p1,p2,3);
	return mathVecMagnitude(temp,3);
}
