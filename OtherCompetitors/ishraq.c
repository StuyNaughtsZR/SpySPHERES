float me[12], other[12];
float target[3];
float fuel, light_boundary, dark_boundary, score, energy;
float items[9][3];
//int poss[9]; //0 is itemType, 1 is who has it
bool mirror;
int part;

void init(){
	//api.getMyZRState(me);
	//api.getOtherZRState(other);
	int i = 0;
	for(;i<9;i++){
	    game.getItemLoc(items[i], i);
	    //poss[i] = game.getItemType(i);
	}
	energy = 5.0f;
	part = 0;
	target[0] = target[1] = target[2] = 0.0f;
}

void loop(){
    api.getMyZRState(me);
    api.getOtherZRState(other);
    int targ = -1;
    switch(part){
        case 0:
            if(distance(me, items[7]) <= distance(me, items[8])){
                target[0] = items[7][0];
                target[1] = items[7][1];
                target[2] = items[7][2];
            }else{
                target[0] = items[8][0];
                target[1] = items[8][1];
                target[2] = items[8][2];
            }
            api.setPositionTarget(target);
            if(game.getNumMirrorsHeld() > 0){
                int score = getClosest(me, 1);
                if(score != -1 && game.posInLight(items[score])){
                    game.useMirror();
                    mirror = TRUE;
                    target[0] = items[score][0];
                    target[1] = items[score][1];
                    target[2] = items[score][2];
                    targ = score;
                    part = 1;
                }else if(score == -1){
                    part = 3; //I'm gonna steal all the energy
                }else{
                    target[0] = items[score][0];
                    target[1] = items[score][1];
                    target[2] = items[score][2];
                    targ = score;
                    part = 1;
                }
            }
            break;
        case 1:
            if(game.hasItem(targ) > -1){
                int score = getClosest(me, 1);
                if(score == -1){
                    part = 4;
                    break;
                }
                targ = score;
                target[0] = items[targ][0];
                target[1] = items[targ][1];
                target[2] = items[targ][2];
            }
            //api.setPositionTarget(target);
            if(mirror && game.getLightSwitchTime() - game.getMirrorTimeRemaining() > 0
                && game.posInArea(me) < 1){
                part = 4;
                break;
            }
            if(game.getMirrorTimeRemaining() <=0){
                mirror = FALSE;
            }
            if(!mirror && game.posInArea(me) == 1){
                part = 4;
            }
            if(game.getLightSwitchTime() < 7 && game.posInArea(me) == -1){
                float x[3];
                x[0] = me[0];
                x[1] = -1 * me[1];
                x[2] = me[2];
                api.setPositionTarget(x);
            }else{
                api.setPositionTarget(target);
            }
            break;
        case 3:
            game.useMirror();
            mirror = TRUE;
            targ = getClosest(me, 0);
            if(targ != -1){
                target[0] = items[targ][0];
                target[1] = items[targ][1];
                target[2] = items[targ][2];
                api.setPositionTarget(target);
            }else{
                part = 4;
            }
            if(game.getMirrorTimeRemaining()<=0){
                mirror = FALSE;
            }
            if(!mirror && game.posInArea(me) == 1){
                part = 4;
            }
            break;
        case 4:
            float x[3];
            if(game.posInArea(me) == 1){
                x[0] = me[0];
                x[1] = -1 * me[1];
                x[2] = me[2];
                api.setPositionTarget(x);
            }
            if(game.posInArea(me) == 0){
                api.setPositionTarget(x);
            }
            if(game.posInArea(me) == -1){
                int z = getClosest(me, 1);
                if(z != -1 && game.posInArea(items[z]) == -1){
                    target[0] = items[z][0];
                    target[1] = items[z][1];
                    target[2] = items[z][2];
                    api.setPositionTarget(target);
                }
            }
    }
}

int getClosest(float pos[], int ID){ //function to find closest existing item not taken yet
    int i;
    int p = 0;
    bool exists = FALSE;
    if(ID == 0){
        exists = FALSE;
        for(i=0;i<3;i++){
            if(!exists && game.hasItem(i) == -1){
                p = i;
                exists = TRUE;
            }
            if(game.hasItem(i) == -1 
                && distance(items[i], pos) <=distance(items[p], pos)){
                p = i;    
            }
        }
        if(exists){
            return p;
        }
        return -1;
    }
    if(ID == 1){
        exists = FALSE;
        for(i=0;i<4;i++){
            if(!exists && game.hasItem(i+3) == -1){
                p = i+3;
                exists = TRUE;
            }
            if(game.hasItem(i+3) == -1
            && distance(items[i+3], pos) <= distance(items[p], pos)){
                p = i+3;    
            }
        }
        if(exists){
            return p;
        }
        return -1;
    }
    if(ID==2){
        exists = FALSE;
        for(i=0;i<2;i++){
            if(!exists && game.hasItem(i+7)==-1){
                p = i+7;
                exists = TRUE;
            }
            if(game.hasItem(i+7) == -1 
                && distance(items[i+7], pos) <= distance(items[p], pos)){
                p = i+7;    
            }
        }
        if(exists){
            return p;
        }
        return -1;
    }
    return -1;
}


float velocity(float p[]){
	float d = 0;
	for(int i=3; i < 6; i++){
		d += p[i]*p[i];
	}
	return sqrtf(d);
}

float distance(float p1[], float p2[]){
	float d = 0;
	for(int i=0; i < 3; i++){
		d += (p2[i]-p1[i])*(p2[i]-p1[i]);
	}
	return sqrtf(d);
}
