//Stanley Lin

//I removed your pos, enemy pos, your attitude
//It's at 99% right now but try and cut down on variables and only have those that are necessary

float me[12];
float other[12];

float aboveOtherPos[3]; 

float earth[3];
float origin[3];

float vecBtwnSph[3];
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

int state;
bool sphColor;

void init(){
	earth[0] = earth[1] = 0.0f;
	earth[2] = 1.0f;
	
	vecBtwnSph[0] = vecBtwnSph[1] = vecBtwnSph[2] = vecBtwn[0] = vecBtwn[1] = vecBtwn[2] = 0.0f;
	
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
	
	aboveOtherPos[0] = other[0];
	aboveOtherPos[1] = other[1];
	
	if (game.getMemoryFilled() != 2 /*&& game.getEnergy() >= 3*/) {
	    mathVecSubtract(vecBtwnSph, other, me, 3);
	    mathVecNormalize(vecBtwnSph, 3);
	    api.setAttitudeTarget(vecBtwnSph);
	}
	
    if ((game.getEnergy() > 3) && (game.getPicPoints() > 0) && canTakePic()) {
        game.takePic();
    }
    
    if (game.getMemoryFilled() == 2) {
        api.setAttitudeTarget(earth);
        
        if (matrixEqual(me+6, earth) && game.getEnergy() >= 3) {
            game.uploadPics();
        }
    }
    
    state = setState();
    
    if (game.getNumMirrorsHeld() > 0) game.useMirror();
    
	//bunch of states
    switch (state) {
        case 0://Get items
            DEBUG(("State 0"));
            
            if (!(sphColor)) {
                if (game.hasItem(8) == -1) {
                    moveFast(mir2);
                }
                else if (game.hasItem(4) == -1) {
                    if (game.getEnergy() > 3) moveFast(score2);
                    else api.setPositionTarget(me);
                }
            }
            
            else {
                if (game.hasItem(7) == -1) {
                    moveFast(mir1);
                }
                else if (game.hasItem(5) == -1) {
                    if (game.getEnergy() > 3) moveFast(score3);
                    else api.setPositionTarget(me);
                }
            }
            
            //Not worth it to get the other mirror
            /*else if (game.hasItem(7) == -1 && !(sphColor)) {
                moveFast(mir1);
            }
            else if (game.hasItem(8) && sphColor) {
                moveFast(mir2);
            }*/
            
            //Bottom score a waste of energy?
            /*else if (game.hasItem(3) == -1) {
                if (game.getEnergy() > 3) moveFast(score1);
                else api.setPositionTarget(me);
            }*/
            
            //Top score object
            /*else if (game.hasItem(6) == -1) {
                if (game.getEnergy() > 3) moveFast(score4);
                else api.setPositionTarget(me);
            }*/
        break;
        
        case 1://Stay at top
            DEBUG(("State 1"));
            
            if (dist(me, other) > 0.5) {
                api.setPositionTarget(aboveOtherPos);
            }
            else {
                api.setPositionTarget(me);
            }
        break;
        
        case 2://Spam upload
            DEBUG(("State 2"));
            
            if (game.getMemoryFilled() != 0 && game.getEnergy() > 2) game.uploadPics();
        break;
        
        /*case 3://Try to ram
            DEBUG(("Case 3"));
            
            moveFast(otherPos);
            
        break;*/
    }
}

float dist(float a[3], float b[3]) {
    return(sqrtf(powf((b[0] - a[0]), 2) + powf((b[1] - a[1]), 2) + powf((b[2] - a[2]), 2)));
}

bool matrixEqual(float a[3], float b[3]) {
    return (abs(a[0] - b[0]) < 0.05) && (abs(a[1] - b[1]) < 0.05) && (abs(a[2] - b[2]) < 0.05);
}

void moveFast(float target[3]) {
    if (dist(me, target) > 0.49) {
        mathVecSubtract(vecBtwn, target, me, 3);
	    distance = mathVecMagnitude(vecBtwn, 3);
	    api.setVelocityTarget(vecBtwn);
    }
    else {
        api.setPositionTarget(target);
    }
}

int setState() {
    if (!(sphColor)) {
        if ((game.hasItem(4) == -1) || (game.hasItem(8) == -1)) return 0;
    }
    else {
        if ((game.hasItem(5) == -1) || (game.hasItem(7) == -1)) return 0;
    }
    
    if (api.getTime() > 150) return 2;//spam upload
    
    /*if ((game.getScore() - game.getOtherScore()) > 5) {
        return 3;
    }*/
    
    return 1;//Stay at top
}

bool canTakePic() {
    return (game.posInLight(other) || game.posInGrey(other)) && (dist(me, other) > 0.5) && game.isFacingOther() && game.isCameraOn();
}
