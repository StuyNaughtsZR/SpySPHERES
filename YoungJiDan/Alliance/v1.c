#define START 0
#define SECOND 1
#define RISE 2
#define POST 3
#define UPLOAD 4
#define STOP 5

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

float energy;

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
	
	itemNum = recalibrate(6,7);
	
	state = START;
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	getItemArray();
	energy = game.getEnergy();
	
	if((game.getCurrentTime() > 165 || game.getFuelRemaining() < 10) && game.getMemoryFilled() >= 1){
	    game.takePic();
		
	    state = UPLOAD; //game changer
	}
	
	if(game.getEnergy() < 1 && game.getCurrentTime() > 50 && !game.posInLight(me))
	    state = STOP;
	
	if(game.getFuelRemaining() == 0)
	    game.takePic();
	
	DEBUG(("STATE: %d", state));
	
	switch(state) {
		case START:
	        if( !itemBool[itemNum] ) { 
				if( itemNum == 7 || itemNum == 8 ) {
					itemNum = recalibrate(3,6);
				} else {
					state = SECOND;
				}
			}
			
			memcpy(target,items[itemNum],3*sizeof(float));
	        
	        moveTo(target);
			break;
			
	    case SECOND: //second score pack
	        if(distance(me,items[itemNum]) < 0.3 && distance(me,items[itemNum])>distance(other,items[itemNum]) )
	            itemBool[itemNum] = false;
	        if(!itemBool[itemNum]){ 
				itemNum = recalibrate(0,6);
				if(itemNum < 3)
					state = RISE;
			}
			
	        memcpy(target,items[itemNum],3*sizeof(float));
			
	        memcpy(temp, me, 3*sizeof(float));
			temp[1] -= 0.22
			
			DEBUG(("BOOL: %d", game.posInGrey(temp)||game.posInLight(temp) ));
			
			if( game.posInGrey(temp) || game.posInLight(temp) ) game.useMirror();
			
	        moveTo(target);
	            
	        mathVecSubtract(facing, other, me, 3);
	        api.setAttitudeTarget(facing);
	        
	        if(checkPhoto(me,other,facing))
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = UPLOAD;
	        break;
		
		case RISE: //go to the top
			memcpy(target,items[itemNum],3*sizeof(float));
			
			target[2] = -0.4;
			
			if(checkPhoto(me,other,facing))
	            game.takePic();
	            
	        if(game.getMemoryFilled() == 2)
	            state = UPLOAD;
			break;
			
		case POST:
			break;
		
	    case UPLOAD: //upload
	        if(energy > 2) {
	            moveTo(target);
			} else {
	            api.setVelocityTarget(origin);
			}
	        api.setAttitudeTarget(earth);
	        
	        if( energy > 1 && checkUpload(me) )
	            game.uploadPics();
	            
	        if( game.getMemoryFilled() == 0){
				if(itemNum < 3){
					state = RISE;
				} else { 
					state = SECOND;
				}
	        }
	        break;
	        
	   case STOP: //chill out
	        api.setVelocityTarget(origin);
	        api.setAttRateTarget(origin);
	        
	        if(!mathVecInner(me+6, facing, 3) > 0.9689f) api.setAttitudeTarget(facing);
	        
	        if(game.posInLight(me)){
	            if(game.getMemoryFilled() == 2) {
	                state = UPLOAD;
				} else {
	                state = SECOND;
				}
	        }
	        break;
	}
}

void moveTo(float target[]){
    int d = distance(me,target);
    int speed = 1;
    if (d > 0.5){
        displace(target,me);
        speed = 15;
        tarVec[0] = tarVec[0] * speed;
        tarVec[1] = tarVec[1] * speed;
        tarVec[2] = tarVec[2] * speed;
        api.setVelocityTarget(tarVec);
    } else if (d > 0.3){
        displace(target,me);
        speed = 9;
        tarVec[0] = tarVec[0] * speed;
        tarVec[1] = tarVec[1] * speed;
        tarVec[2] = tarVec[2] * speed;
        api.setVelocityTarget(tarVec);
    } else {
        api.setPositionTarget(target);
    }
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
    if( distance(me,other) > 0.5 && !(game.posInDark(other)) ){
        if( game.isFacingOther() && energy > 1 && game.getPicPoints() > 0)
            return true;
    }
    return false;
}

bool checkUpload( float me[] ){
    if( ( me[8] > 0.9689f ) && (mathVecMagnitude(me+9,3) < 0.05f) ) return true;
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
