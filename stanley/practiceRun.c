//GetZRState 
float me[12];
float other[12];

float myPos[3];
float otherPos[3];

float myAtt[3];

//Item locations
float en1[3];
float en2[3];
float en3[3];
float score1[3];
float score2[3];
float score3[3];
float score4[3];
float mir1[3];
float mir2[3];

float earth[3];

//For holding vector calculations
float vecBtwn[3];
float vecBtwnSph[3];
float vecBtwnEarth[3];

int counter;
int sphColor;
bool initSwitch;

//For setVelocityTarget
float distance;

void init(){
	for (int x = 0; x < 12; x++) {
	    me[x] = 0.0f;
	    other[x] = 0.0f;
	}
	
	//Assign zero to all arrays with 3 elements
	for (int x = 0; x < 3; x++) {
	    myPos[x] = 0.0f;
	    otherPos[x] = 0.0f;
	    vecBtwn[x] = 0.0f;
	    vecBtwnSph[x] = 0.0f;
	    vecBtwnEarth[x] = 0.0f;
	    myAtt[x] = 0.0f;
	}
	
	game.getItemLoc(en1, 0);
	game.getItemLoc(en2, 1);
	game.getItemLoc(en3, 2);
	game.getItemLoc(score1, 3);
	game.getItemLoc(score2, 4);
	game.getItemLoc(score3, 5);
	game.getItemLoc(score4, 6);
	game.getItemLoc(mir1, 7);
	game.getItemLoc(mir2, 8);
	
	counter = 0;
	
	distance = 0.0f;
	
	//Zero is blue, one is red
	api.getMyZRState(me);
	if (me[0] > 0) {
	    sphColor = 0;
	} else {
	    sphColor = 1;
	}
	
	earth[0] = 0.0f;
	earth[1] = 0.0f;
	earth[2] = 1.0f;
	initSwitch = false;
}

void loop(){
	DEBUG(("%d", api.getTime()));
	
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	myPos[0] = me[0];
	myPos[1] = me[1];
	myPos[2] = me[2];
	
	otherPos[0] = other[0];
	otherPos[1] = other[1];
	otherPos[2] = other[2];
	
	myAtt[0] = me[6];
	myAtt[1] = me[7];
	myAtt[2] = me[8];
	/*EVERYTHING ABOVE HERE MUST BE FIRST*/
	
	
	
	
	//Always face other opponent
	if (game.getMemoryFilled() != 2) {
	    mathVecSubtract(vecBtwnSph, otherPos, myPos, 3);
	    mathVecNormalize(vecBtwnSph, 3);
	    api.setAttitudeTarget(vecBtwnSph);
	}
	
	if (game.getEnergy() > 3 && canTakePic()) {
	    game.takePic();
	}
	
	//WIP should sttempt to upload once attitude is within a certain amount
	if (game.getMemoryFilled() == 2) {
	    //mathVecSubtract(vecBtwnEarth, earth, myPos, 3);
	    //mathVecNormalize(vecBtwnEarth, 3);
	    //api.setAttitudeTarget(vecBtwnEarth);
	    api.setAttitudeTarget(earth);
	    
	    mathVecSubtract(vecBtwnEarth, earth, myAtt, 3);
	    
	    if (attCloseToEqual(vecBtwnEarth) && game.getEnergy() >= 2) {
	        game.uploadPics();
	    }
	}
	
	if (counter == 0 && (!(game.posInDark(myPos)) || initSwitch)) {
	    initSwitch = true;
	    if (sphColor == 0) {
	        mathVecSubtract(vecBtwn, mir2, myPos, 3);
	        distance = mathVecMagnitude(vecBtwn, 3);
	        
	        if (distance > 0.47) {
	            api.setVelocityTarget(vecBtwn);
	        }
	        else {
	            api.setPositionTarget(mir2);
	        }
	        
	        if (game.hasItem(8) != -1) {
	            initSwitch = false;
	            counter++;
	        }
	    }
	    else {
	        mathVecSubtract(vecBtwn, mir1, myPos, 3);
	        distance = mathVecMagnitude(vecBtwn, 3);
	        
	        if (distance > 0.47) {
	            api.setVelocityTarget(vecBtwn);
	        }
	        else {
	            api.setPositionTarget(mir1);
	        }
	        
	        if (game.hasItem(7) != -1) {
	            initSwitch = false;
	            counter++;
	        }
	    }
	}
	
	if (counter == 1) {
	    //If blue
	    if (sphColor == 0) {
	        //Mir2 (red side) not picked up
	        if (shouldIGetItem(7)) {
	        mathVecSubtract(vecBtwn, mir1, myPos, 3);
	        distance = mathVecMagnitude(vecBtwn, 3);
	        
	            if (distance > 0.47) {
	                api.setVelocityTarget(vecBtwn);
	            }
	            else {
	                api.setPositionTarget(mir1);
	            }
	            if (game.hasItem(7) != -1) counter++;
	        }
	        //If picked up, skip this block
	        else {
	            counter++;
	        }
	    }
	    //If red
	    else {
	        //Get mir1 (blue side)
	        if (shouldIGetItem(8)) {
	        mathVecSubtract(vecBtwn, mir2, myPos, 3);
	        distance = mathVecMagnitude(vecBtwn, 3);
	        
	            if (distance > 0.47) {
	                api.setVelocityTarget(vecBtwn);
	            }
	            else {
	                api.setPositionTarget(mir2);
	            }
	            if (game.hasItem(8) != -1) counter++;
	        }
	        //If picked up, skip
	        else {
	            counter++;
	        }
	    }//End red
	}//End counter 1
	
	if (counter == 2) {
	    DEBUG(("Hello"));
	    //Top center score+
	    if (shouldIGetItem(6)) {
	        mathVecSubtract(vecBtwn, score4, myPos, 3);
	        distance = mathVecMagnitude(vecBtwn, 3);
	        
	        if (distance > 0.47) {
	            api.setVelocityTarget(vecBtwn);
	        }
	        else {
	            api.setPositionTarget(score4);
	        }
	    }
	    else {
	        counter++;
	    }
	    
	    if (game.hasItem(6) != -1) counter++;
	}
	
	if (counter == 2) {
	    DEBUG(("Hi"));
	}
	
}

float dist(float a[], float b[]) {
    return(sqrtf (powf( ( b[0] - a[0]), 2) + powf( ( b[1] - a[1]), 2)));
}

//Tests if can take pic DOES NOT CHECK FOR MIRROR
bool canTakePic() {
    return (game.posInLight(otherPos) || game.posInGrey(otherPos)) && (dist(myPos, otherPos) > 0.5) && game.isFacingOther() && game.isCameraOn(); //&& (game.getMirrorTimeRemaining() == 0);
}

//Tests if item has been picked up or not and if it is safe (opponent in light zone)
bool shouldIGetItem(int id) {
    return ((game.hasItem(id) == -1) && game.posInLight(otherPos));
}

//Returns attitude from Earth
float attitudeFromEarth(float att[3]) {
    
}

bool attCloseToEqual(float a[3]) {
    return (a[0] <= 0.03 && a[1] <= 0.03 && a[2] <= 0.03);
}