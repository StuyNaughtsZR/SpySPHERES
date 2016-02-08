#define MAIN 0
#define UPLOAD 1
#define STOP 2
#define LIGHTDARK 3

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

float tarVec[3];


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
	
	origin[0] = origin[1] = origin[2] = earth[0] = earth[1] = 0.0f;
	
	earth[2] = 1.0f;
	
	itemNum = 6;
	
	state = MAIN;
	
	DEBUG(("Stuy-Naught! Modified Againn"));
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	getItemArray();
	
	if((game.getCurrentTime() > 165 || game.getFuelRemaining() < 10) && game.getMemoryFilled() >= 1){
	    game.takePic();
	    state = UPLOAD; //game changer
	}
	
	if(game.getEnergy() < 1 && !game.posInLight(me))
	    state = STOP;
	
	if(game.getFuelRemaining() == 0)
	    game.takePic();
	
	switch(state) {
	    case MAIN: //normal case
	        if(distance(me,items[itemNum])<0.3 && distance(me,items[itemNum])>distance(other,items[itemNum]) )
	            itemBool[itemNum]=false;
	        if(!itemBool[itemNum]){
	            itemNum = recalibrate(4,6);
	        }
	        memcpy(target,items[itemNum],3*sizeof(float));
	        
	        moveTo(target);
	            
	        mathVecSubtract(facing, other, me, 3);
	        api.setAttitudeTarget(facing);
	        
	        if( checkPhoto(me,other,facing) )
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = UPLOAD;
	        break;
	        
	    case UPLOAD: //upload
	        if(game.getEnergy() > 2)
	            moveTo(target);
	        else
	            api.setVelocityTarget(origin);
	        api.setAttitudeTarget(earth);
	        
	        if( game.getEnergy() > 1 && checkUpload(me) )
	            game.uploadPics();
	            
	        if( game.getMemoryFilled() == 0){
	            state = MAIN;
	        }
	        break;
	        
	   case STOP: //chill out
	        api.setVelocityTarget(origin);
	        api.setAttRateTarget(origin);
	        
	        if(!mathVecInner(me+6,facing,3) > 0.9689f)
	            api.setAttitudeTarget(facing);
	        
	        if(game.posInLight(me)){
	            if(game.getMemoryFilled() == 2)
	                state = UPLOAD;
	            else
	                state = MAIN;
	        }
	        break;
	        
	   case LIGHTDARK: //if we are in light and they are in dark
	        itemNum = recalibrate(7,8);
	        
	        if(!itemBool[itemNum]) //if both mirrors are taken
	            itemNum = 6;
	        
	        memcpy(target,items[itemNum],3*sizeof(float));
	        api.setPositionTarget(target);
	        
	        mathVecSubtract(facing, other, me, 3);
	        api.setAttitudeTarget(facing);
	        break;
	}
}

void moveTo(float target[]){
    float d = distance(me,target);
    float speed = 1;
    if (d > 0.5*0.01*81+mathVecMagnitude(me,3)*9){
        displace(target,me);
        speed = d;
        tarVec[0] = tarVec[0] * speed;
        tarVec[1] = tarVec[1] * speed;
        tarVec[2] = tarVec[2] * speed;
        api.setVelocityTarget(tarVec);
    }
    /*else if (d > 0.3){
        displace(target,me);
        speed = 9;
        tarVec[0] = tarVec[0] * speed;
        tarVec[1] = tarVec[1] * speed;
        tarVec[2] = tarVec[2] * speed;
        api.setVelocityTarget(tarVec);
    }*/
    else {
        api.setPositionTarget(target);
    }
    DEBUG(("%f_%f", mathVecMagnitude(me+3,3), d));
}

int recalibrate(int start, int end){ //gets index of closest item in range. 
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
	        if(i == 6)
	            items[6][2] = -0.45; //aim 0.05 higher than the actual powerup
	    }
	}
}

float distance(float p1[], float p2[]){
	mathVecSubtract(temp,p1,p2,3);
	return mathVecMagnitude(temp,3);
}

void displace(float p1[], float p2[]){
     mathVecSubtract(temp,p1,p2,3);
     mathVecNormalize(temp,3);
     memcpy(tarVec,temp,3*sizeof(float));
}
