//Stanley Lin
//


#define GET_ITEM 0
#define ROTATE 1
#define UPLOAD 2
#define STAY_IN_PLACE 3
#define LIGHT_TO_DARK 4
#define SPAM_UPLOAD 5
#define DEFAULT 6


//Maybe make this into a function
//#define GOTO_TOP 1

//Item array
float items[9][3];

//Vector direction of earth
float earth[3];

//Origin if needed
float origin[3];

//General purpose vector holder
float vecBtwn[3];

//ZRState data
float me[12];
float other[12];

//Used in looping
int x;

//true is blue false is red
bool sphColor;

//Game state
short state;

short item;

float distance;

//General purpose location holder
float target[3];





void init(){
	for (x = 0; x < 9; x++) {
	    game.getItemLoc(items[x], x);
	}
	
	api.getMyZRState(me);
	if (me[0] > 0) sphColor = true;
	else sphColor = false;
	
	state = 0;
	
	origin[0] = origin[1] = origin[2] = earth[0] = earth[1] = 0.0f;
	earth[2] = 1.0f;
}

void loop(){
    DEBUG(("%d", api.getTime()));
    DEBUG(("distance btwn oppo: %f", dist(me, other)));
    
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	state = setState();
	
    switch (state) {
        case GET_ITEM:
            DEBUG(("Getting item"));
            
            game.getItemLoc(target, item);
            //moveFast(target);
            api.setPositionTarget(target);
        break;
        
        case ROTATE:
            DEBUG(("Turning to opponent"));
            
            mathVecSubtract(vecBtwn, me, other, 3);
            api.setAttitudeTarget(vecBtwn);
            
            //atan2()
        break;
        
        case UPLOAD:
            DEBUG(("Uploading"));
            api.setAttitudeTarget(earth);
            
            if (checkUpload(me) && game.getEnergy() >= 2) {
                game.uploadPics();
            }
        break;
        
        case STAY_IN_PLACE:
            DEBUG(("Staying"));
            api.setPositionTarget(me);
        break;
        
        case LIGHT_TO_DARK:
            
        break;
        
        case SPAM_UPLOAD:
            DEBUG(("Spamming"));
            
            if (game.getMemoryFilled() > 1) {
                api.setAttitudeTarget(earth);
                
                if (checkUpload(me)) {
                    game.uploadPics();
                }
            }
        break;
        
        case DEFAULT:
            DEBUG(("Default"));
            if (other[2] < 0.2) {
                target[0] = other[0];
                target[1] = other[1];
                target[2] = -0.7f;
            }
            else {
                //(0, 0, -0.8)
                
                //Get magnitude of initial vector
                distance = mathVecMagnitude(other, 3);
                
                //The unit vector is initial / magnitude
                target[0] = other[0] / distance;
                target[1] = other[1] / distance;
                target[2] = other[2] / distance;
                
                //Scale the unit vector by the length of original minus radius
                target[0] *= (mathVecMagnitude(other, 3) - 0.55);
                target[1] *= (mathVecMagnitude(other, 3) - 0.55);
                target[2] = -0.7f;
            }
            
            api.setPositionTarget(target);
        break;
	}
    if (game.getMemoryFilled() != 2) {
        DEBUG(("Turning to opponent"));
        
        mathVecSubtract(vecBtwn, other, me, 3);
        api.setAttitudeTarget(vecBtwn);
	}
	
	if (canTakePic()) {
        game.takePic();
    }
    
    if (game.getMemoryFilled() == 2) {
        api.setAttitudeTarget(earth);
        
        if (checkUpload(me) && game.getEnergy() >= 2) {
            game.uploadPics();
        }
    }
}

/*void setPositionTarget(float target[]) {
    float disp[3];
    float dist, speed;
    mathVecSubtract(disp, target, me, 3);
    dist = mathVecNormalize(disp, 3); 
    if (dist > 0.5 * 0.01 * 64 + mathVecMagnitude(me+3,3) * 8) {
        speed = dist;
        for(x = 0; x < 3; x++) {
            disp[x] *= speed;
        }
        api.setVelocityTarget(disp);
    }
    else {
    api.setPositionTarget(target);
    }
}*/

float dist(float a[], float b[]){
	mathVecSubtract(vecBtwn, a, b, 3);
	return mathVecMagnitude(vecBtwn, 3);
}

bool canTakePic() {
    return (game.posInLight(other) || game.posInGrey(other)) && (dist(me, other) > 0.5) && game.isFacingOther() && game.isCameraOn() && game.getPicPoints() > 0 && game.getEnergy() > 3;
}

//From young
bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) )
        return true;
    return false;
}

//Returns the item id of the closest item between lower and upper inclusive
short getClosestItem(float y[][3], int lower, int upper) {
    float closest = 100.0f;
    int closestItem = -1;
    for (x = lower; x < upper + 1; x++) {
        if ((dist(me, y[x]) < closest) && game.hasItem(x) == -1) {
            closest = dist(me, y[x]);
            closestItem = x;
        }
    }
    
    return closestItem;
}

void moveFast(float thing[3]) {
    if (dist(me, thing) > 0.51) {
        mathVecSubtract(vecBtwn, thing, me, 3);
	    distance = mathVecMagnitude(vecBtwn, 3);
	    api.setVelocityTarget(vecBtwn);
    }
    else api.setPositionTarget(thing);
}

int setState() {
    if (api.getTime() > 150) return SPAM_UPLOAD;
    
    if (game.getEnergy() < 1.3f) return STAY_IN_PLACE;
    
    /*if (game.getMemoryFilled() != 2 && !(game.isFacingOther())) {
        return ROTATE;
    }
    
    if (game.getMemoryFilled() == 2) {
        return UPLOAD;
    }*/
    
    //If mirror items are available
    item = getClosestItem(items, 7, 8);
    if (item != -1 && api.getTime() < 26) return GET_ITEM;
    
    //If score items are available
    item = getClosestItem(items, 3, 6);
    if (item != -1 && api.getTime() < 100) return GET_ITEM;
    
    return DEFAULT;
}
