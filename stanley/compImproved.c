//Stanley Lin

#define GET_ITEM 0
#define GOTO_TOP 1
#define SPAM_UPLOAD 2
#define RAM 3
#define STAY_IN_PLACE 4
#define LIGHT_TO_DARK 5

float me[12];
float other[12];

float aboveOtherPos[3]; 

float earth[3];
float origin[3];

float vecBtwn[3];
float distance;

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

short state;
bool sphColor;

void init(){
    earth[0] = earth[1] = vecBtwn[0] = vecBtwn[1] = vecBtwn[2] = 0.0f;
    
    earth[2] = 1.0f;
	
	aboveOtherPos[0] = aboveOtherPos[1] = origin[0] = origin[1] = origin[2] = 0.0f;
	
	aboveOtherPos[2] = -0.6f;
	
	me[0] = me[1] = me[2] = 0.0f;
	
	game.getItemLoc(en1, 0);
	game.getItemLoc(en2, 1);
	game.getItemLoc(en3, 2);
	game.getItemLoc(score1, 3);
	game.getItemLoc(score2, 4);
	game.getItemLoc(score3, 5);
	game.getItemLoc(score4, 6);
	game.getItemLoc(mir1, 7);
	game.getItemLoc(mir2, 8);
	
	state = 0;
	distance = 0.0f;
	
	api.getMyZRState(me);
	if (me[0] > 0) sphColor = false;    //blue
	else sphColor = true;               //red
}

void loop(){
    DEBUG(("%d", api.getTime()));
    
    api.getMyZRState(me);
	api.getOtherZRState(other);
	aboveOtherPos[2] = -0.6f;
	
	aboveOtherPos[0] = other[0];
	aboveOtherPos[1] = other[1];
	
    if (game.getFuelRemaining() == 0) game.takePic();
    
    state = setState();
    
    /*if (game.getNumMirrorsHeld() > 0) {
        game.useMirror();
    }*/
    
	//bunch of states
    switch (state) {
        case GET_ITEM:
            DEBUG(("State 0"));
            
            if (!(sphColor)) {
                if (game.hasItem(4) == -1) {
                    api.setPositionTarget(score2);
                }
                else if (game.hasItem(8) == -1) {
                    api.setPositionTarget(mir2);
                    
                }
                /*else if (game.hasItem(4) == -1) {
                    api.setPositionTarget(score2);
                }*/
            }
            
            else {
                if (game.hasItem(5) == -1) {
                        api.setPositionTarget(score3);
                }
                else if (game.hasItem(7) == -1) {
                        api.setPositionTarget(mir1);
                }
                /*else if (game.hasItem(5) == -1) {
                        api.setPositionTarget(score3);
                }*/
            }
        break;
        
        case GOTO_TOP:
            DEBUG(("State 1"));
            
            //Already at top
            if (other[2] <= -0.5){
                DEBUG(("Opponent at top"));
                
                if (other[1] > 0) {
                    aboveOtherPos[0] = 0.0f;
                    aboveOtherPos[1] = 0.2f;
                }
                else {
                    aboveOtherPos[0] = 0.0f;
                    aboveOtherPos[1] = -0.2f;
                }
                
                aboveOtherPos[0] = aboveOtherPos[1] = 0.0f;
                api.setPositionTarget(aboveOtherPos);
            }
            else if (dist(me, other) < 0.5) {
                DEBUG(("Too close to opponent"));
                
                if (other[1] > 0) {
                    aboveOtherPos[0] = 0.0f;
                    aboveOtherPos[1] = 0.2f;
                    if (dist(other, aboveOtherPos) < 0.5) {
                        aboveOtherPos[0] = 0.55f;
                        if (dist(other, aboveOtherPos) < 0.5) {
                            aboveOtherPos[0] = -0.55f;
                        }
                    }
                }
                else {
                    aboveOtherPos[0] = 0.0f;
                    aboveOtherPos[1] = -0.2f;
                    if (dist(other, aboveOtherPos) < 0.5) {
                        aboveOtherPos[0] = 0.55f;
                        if (dist(other, aboveOtherPos) < 0.5) {
                            aboveOtherPos[0] = -0.55f;
                        }
                    }
                }
                
                api.setPositionTarget(aboveOtherPos);
            }
            else {
                DEBUG(("Normal"));
                
                api.setPositionTarget(aboveOtherPos);
            }
        break;
        
        case SPAM_UPLOAD:
            DEBUG(("State 2"));
            
            if (api.getTime() > 170) game.uploadPics();
            
            if (game.getMemoryFilled() != 0 && game.getEnergy() > 2) {
                game.uploadPics();
            }
        break;
        
        /*case RAM:
            DEBUG(("Case 3"));
            
            moveFast(otherPos);
            
        break;*/
        
        case STAY_IN_PLACE:
            DEBUG(("State 4"));
            
            //If close to light zone, charge for it
            if (abs(me[1] - game.getLightGreyBoundary()) < 0.2 && !game.posInDark(other)) {
                DEBUG(("Moving Forward"));
                
                if (game.getLightGreyBoundary() > 0) {
                    aboveOtherPos[0] = me[0];
                    aboveOtherPos[1] = me[1] + 0.2f;
                    aboveOtherPos[2] = me[2];
                    moveFast(aboveOtherPos);
                }
                else {
                    aboveOtherPos[0] = me[0];
                    aboveOtherPos[1] = me[1] - 0.2f;
                    aboveOtherPos[2] = me[2];
                    moveFast(aboveOtherPos);
                }
            }
            
            api.setPositionTarget(me);
            break;
        
        case LIGHT_TO_DARK:
            DEBUG(("State 5"));
            
            game.useMirror();
            
            if (game.getLightGreyBoundary() > 0) {
                api.setPositionTarget(me);
            }
            
            break;
        
    }
    
    if (game.getMemoryFilled() != 2 /*&& game.getEnergy() >= 3*/) {
	    DEBUG(("Turning to opponent"));
	    
	    mathVecSubtract(vecBtwn, other, me, 3);
	    api.setAttitudeTarget(vecBtwn);
	}
	
	if (canTakePic()) {
        game.takePic();
    }
    
    if (game.getMemoryFilled() == 2) {
        api.setAttitudeTarget(earth);
        
        if (matrixEqual(me+6, earth) && game.getEnergy() >= 2) {
            game.uploadPics();
        }
    }
}

//Eats up 32% of codespace
/*float dist(float a[3], float b[3]) {
    return(sqrtf(powf((b[0] - a[0]), 2) + powf((b[1] - a[1]), 2) + powf((b[2] - a[2]), 2)));
}*/

float dist(float a[], float b[]){
	mathVecSubtract(vecBtwn, a, b, 3);
	return mathVecMagnitude(vecBtwn, 3);
}

//If the two matrices are within 0.05 of each other, they're equal
bool matrixEqual(float a[3], float b[3]) {
    return (abs(a[0] - b[0]) < 0.05) && (abs(a[1] - b[1]) < 0.05) && (abs(a[2] - b[2]) < 0.05);
}

void moveFast(float target[3]) {
    if (dist(me, target) > 0.5) {
        mathVecSubtract(vecBtwn, target, me, 3);
	    distance = mathVecMagnitude(vecBtwn, 3);
	    api.setVelocityTarget(vecBtwn);
    }
    else api.setPositionTarget(target);
}

int setState() {
    if (api.getTime() > 150) return SPAM_UPLOAD;
    
    if (game.getEnergy() < 1.3f) return STAY_IN_PLACE;
    
    if (game.posInLight(me) && game.posInDark(other)) return LIGHT_TO_DARK;
    
    if (!(sphColor)) {
        if ((game.hasItem(4) == -1) || (game.hasItem(8) == -1)) return GET_ITEM;
    }
    else {
        if ((game.hasItem(5) == -1) || (game.hasItem(7) == -1)) return GET_ITEM;
    }
    
    /*if ((game.getScore() - game.getOtherScore()) > 5) {
        return RAM;
    }*/
    
    return GOTO_TOP;
}

bool canTakePic() {
    return (game.posInLight(other) || game.posInGrey(other)) && (dist(me, other) > 0.5) && game.isFacingOther() && game.isCameraOn() && game.getPicPoints() > 0 && game.getEnergy() > 3;
}
