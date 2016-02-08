ZRState me;
ZRState other;

float items[9][3];
int itemBool[9];

float facing[3];

float target[3];

float temp[3];

int state;

void init(){
    api.getMyZRState(me);
	api.getOtherZRState(other);
	
	for(int i = 0; i < 9; i++){
	    game.getItemLoc(items[i],i);
	    itemBool[i] = true;
	}
	
	memcpy(target,items[5],3*sizeof(float));
	
	state = 0;
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	for(int i = 0; i < 9; i++){
	    if(itemBool[i] && game.hasItem(i) != -1){
	        itemBool[i] = 0;
	    }
	}
	
	switch(state) {
	    case 0:
	        mathVecSubtract(facing,other,me,3);
	        mathVecNormalize(facing,3);
	        api.setAttitudeTarget(facing);
	        //api.setPositionTarget(target);
	        if(game.isCameraOn() && game.posInLight(other) && game.isFacingOther()){
	            DEBUG(("TAKING PICTURES"));
	            game.takePic();
	        }
	        if(game.getMemoryFilled() == 2)
	            state = 1;
	        break;
	        
	    case 1:
	        game.uploadPics();
	            
	        if(game.getMemoryFilled() == 0)
	            state = 0;
	        break;
	   
	   case 2:
	        mathVecSubtract(facing,target,other,3);
	        mathVecNormalize(facing,2);
	        api.setAttitudeTarget(facing);
	        
	        setPositionTarget(target,3);
	        
	        if(distance(me,target) < 0.1){
	            setPositionTarget(target,8);
	        }
	        
	        if(distance(me,target) < 0.03){
	            state = 0;
	            target[0] = 0.0;
	            target[1] = 0.6;
	            target[2] = 0.0;
	        }
	        break;
	}
}

float minDistanceFromOrigin(float target[3]) {
	float cos;
	float targetMag = mathVecMagnitude(target,3);
	float meMag = mathVecMagnitude(me,3);
	mathVecSubtract(temp,target,me,3);
	float tempMag = mathVecMagnitude(temp,3);
	cos = (targetMag*targetMag - meMag*meMag - tempMag*tempMag) / (-2 * meMag * tempMag);
	if (cos < 0) {
		return meMag; // Shortest at endpoint
	}
	cos = (meMag*meMag - targetMag*targetMag - tempMag*tempMag) / (-2 * targetMag * tempMag);
	if (cos < 0) {
		return targetMag; // Shortest at endpoint
	}
	else {
		mathVecProject(temp,me,temp,3);
		mathVecSubtract(temp,me,temp,3);
		
		return mathVecMagnitude(temp,3);
	}

}

void setPositionTarget(float target[3], float multiplier) {
	api.getMyZRState(me);
	
	float myPos[3],meMag;
	
	memcpy(myPos, me, 3*sizeof(float));
	
	meMag = mathVecMagnitude(myPos,3);
	
	if (minDistanceFromOrigin(target) > 0.32) {
		//if (distance(me, target) < 0.6) { // Save braking distance
		if(distance(me,target)<0.15){
		    haulAssTowardTarget(target,2.5);
		}
		else if(distance(me,target)<0.2){
		    haulAssTowardTarget(target,1.8);
		}
		haulAssTowardTarget(target,1.3);
		//}

		//else { // Or haul ass towards target
		//	float temp[3];

		//	mathVecSubtract(temp,target,me,3);
			
		//	for (int i = 0 ; i < 3 ; i++) {
		//		temp[i] = me[i] + temp[i] * multiplier;
		//	}

		//	api.setPositionTarget(temp);
		//}

		DEBUG(("GOING STRAIGHT\n"));
	}
	
	else if (meMag >= 0.22 && meMag <= 0.315) {
		for (int i = 0; i < 3; i++) {
			myPos[i] = myPos[i] * 2;
		}
		
		api.setPositionTarget(myPos);
		DEBUG(("TOO CLOSE\n"));
	}
	
	else {
		float opposite[3], perpendicular[3], mePrep[3], path[3], temp[3];
		
		mathVecProject(opposite,target,myPos,3);
		mathVecSubtract(perpendicular,target,opposite,3);
		
		for (int i = 0; i < 3; i++) {
		    mePrep[i] = perpendicular[i] / mathVecMagnitude(perpendicular,3);
		}
		
		for (int i = 0; i < 3; i++) {
			mePrep[i] = (mePrep[i] * 0.325 * meMag) / (sqrtf(meMag*meMag - 0.315*0.315));
		}
		
		mathVecSubtract(path,mePrep,myPos,3);
		
		for (int i = 0; i < 3; i++) {
			path[i] = path[i] * multiplier;
		}
		
		mathVecAdd(temp,myPos,path,3);

		api.setPositionTarget(temp);
		
		DEBUG(("TAKING THE TANGENT\n"));
	}
}

void haulAssTowardTarget(float target[], float scalar) {
    // makes you go in the direction of target, but scalar times faster
    float scaledTarget[3];
    for (int i = 0; i < 3; i++) scaledTarget[i] = me[i] + scalar * (target[i] - me[i]);
    api.setPositionTarget(scaledTarget);
}

void mathVecProject(float c[], float a[], float b[], int n) {
    // finds the projection of a onto b, puts the result in c
    if (mathVecMagnitude(b,3) * mathVecMagnitude(b,3) / 10 == 0) {
        DEBUG(("DIVISION BY ZERO WHILE PROJECTING!"));
    }
    for (int i = 0; i < n; i++) {
        c[i] = (mathVecInner(a,b,3) * b[i]) / (mathVecMagnitude(b,3) * mathVecMagnitude(b,3));
    }
}
float distance(float p1[], float p2[]){
	// Thanks Cornel
	mathVecSubtract(temp,p1,p2,3);
	return mathVecMagnitude(temp,3);
}
