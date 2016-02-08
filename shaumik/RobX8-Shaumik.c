//RobX8 anti-strat

ZRState me, other;
float temp1[3], temp2[3];
float tempMatrix[3][3];
float itemStatus[NUM_ITEMS];
int state, i;
int myZone, otherZone;
int meta_state;
int tm;

typedef enum strat_enum {
    thinking = -2, idk,
    getEP0, getEP1, getEP2, getSP3, getSP4, getSP5, getSP6, getMir7, getMir8,
    zPic, wPic 
} strat;

void init(){
	
	state = 0;
	meta_state = 99;
	
	tm = 0;
}

void loop(){

    startloop();
    
    if(meta_state!=(i=getOtherStrat()) && tm>1) { //use i as temp
        state = 0;
        meta_state = i;
    } 
    
    switch( meta_state ) {
        case getEP0:
        case getEP1:
        case getEP2:
        case getSP3:
        case getSP4:
        case getSP5:
        case getSP6:
            for(i=0; i<3; i++) {
                temp1[i] = 0.0f; 
            }
            api.setVelocityTarget(temp1);
            switch(state) { //use wPic strat (follow)
                case 0:
                    
                    mathVecSubtract(temp1,other,me,3);
                    api.setAttitudeTarget(temp1);
                    if( dtheta(temp1,me+6)<0.25 && otherZone!=-1 ) {
                        game.takePic();
                    }
                    if( game.getMemoryFilled()>0 ) state=1;
                    break;
                case 1:
                    
                    mathVecSubtract(temp1,(float*)EARTH,me,3);
                    api.setAttitudeTarget(temp1);
                    if( dtheta(temp1,me+6)<0.25 && mathVecMagnitude(me+9,3)<0.5 ) {
                        game.uploadPics();
                    }
                    if( game.getMemoryFilled()==0 ) state = 0;
                    break;
                default:
                    break;
            }
            break;
        case getMir7:
        case getMir8:
            switch(state) { //steal mir before other
                case 0:
                    //mathVecSubtract(temp2,(float*)ITEM_LOC[meta_state],me,3);
                    if( mySetPositionTarget((float*)ITEM_LOC[meta_state],MP_RADIUS,MP_SPEED) ) {
                        temp1[0] = 0.0f;
                        temp1[1] = 0.0f;
                        temp1[2] = 1.0f;
                        api.setAttRateTarget(temp1);
                    }
                    if( itemStatus[meta_state]!=-1 ) {
                        api.setPositionTarget(other); //meta-state shud take over...
                    }
                    break;
                
                default: 
                    break;
            }
            break;
        case zPic:
            switch(state) { //rush aka kamikaze
                case 0:
                    mathVecAdd(temp2,other,other+3,3);
                    mySetPositionTarget(temp2, 0.05, 0.2);
                    break;
                default:
                    break;
            }
            break;
        case wPic:
            temp2[0] = 0.0f;
            temp2[1] = game.getLightGreyBoundary() * 2.25/2.0;
            temp2[2] = -0.6f;
            switch(state) { //use zPic strat
                case 0:
                    if( mySetPositionTarget(temp2, .1, .05) ) { 
                        state = 1; 
                    }
                    mathVecSubtract(temp1, other, temp2, 3);
                    api.setAttitudeTarget(temp1);
                    game.takePic();
                    break;
                case 1:
                    api.setPositionTarget(temp2);
                    if( game.getPicPoints()>0.1 ) { 
                        game.takePic(); 
                    }
                    if(game.getMemoryFilled() == 1) {
                        state=2;
                    }
                    break;
                case 2:
                    api.setPositionTarget(temp2);
                    mathVecSubtract(temp1, (float*)EARTH, me, 3);
                    api.setAttitudeTarget(temp1);
                    if( dtheta(temp1,me+6)<0.25001f && mathVecMagnitude(me+9,3)<0.05 ) {
                        game.uploadPics();
                    }
                    if( game.getMemoryFilled() == 0 ) {
                        state=1;
                    }
                    break;
                default: 
                    //DEBUG(("Error"));
                    break;
                }
            break;
        case idk:
            if( getPack((state+3)%12) ) state++;
            break;
        case thinking:
        case 99:  //wat i do in first second
            api.setPositionTarget(other);
            break;
        default:
            DEBUG(("meta_state default"));
            break;
    }

}

void startloop(void) {  

    api.getMyZRState(me);
    api.getOtherZRState(other);
    myZone = game.posInArea(me);
	otherZone = game.posInArea(other);
    for(i=0; i<NUM_ITEMS; i++) {
        itemStatus[i] = game.hasItem(i);
    }
    tm = api.getTime();
    
}

//anti-strat
strat getOtherStrat(void) {

    static ZRState past;
    ZRState diff;
    static int past_tm;
    
    if( tm==0 ) {
        past_tm = 0;
        memcpy(past,other,12*sizeof(float) );
        return( thinking );
    }
    
    mathVecSubtract(diff, other, past, 12);
    
    for(i=0; i<NUM_ITEMS; i++) {
        mathVecSubtract(temp1, other, (float*)ITEM_LOC[i], 3);
        if( dtheta(temp1, diff+3) < 0.05    /*!!!*/
            && mathVecMagnitude(diff+3,3) > 0.1 /*!!!*/
            ) {
        DEBUG(("aekta %i id:%i choori korba\n", ITEM_TYPES[i], i));
        return( (strat)i );
        }
    }
    
    temp1[0] = 0.0f;
    temp1[1] = 0.0f;
    temp1[2] = -0.6f;
    
    if( dtheta(temp1, diff+3) < 0.05 /*!!!*/ ) {
        DEBUG(("shob matha upore jiye chobi neba\n"));
        return( zPic );
    }
    
    mathVecSubtract(temp1, me, other, 3);
    if( mathVecMagnitude(diff+9, 3) > 0.05 /*!!!*/ 
        && dtheta(temp1,other+6) < dtheta(temp1,past+6)
        ) {
        DEBUG(("dhariye teke amar chobi neba\n"));
        return( wPic );
    } 
    
    memcpy(past, other, 12*sizeof(float));
    past_tm = tm;
    
    return( idk );
}

float dtheta(float a[], float b[]) {
    //returns angle between vectors a, b (assumes 3 elements)
    
    mathVecNormalize(a, 3);
    mathVecNormalize(b, 3);
    
    return( acosf( mathVecInner(a,b,3) ) );
    
}

bool mySetPositionTarget(float *target, float max_dist, float max_speed) {
    
    const float max_acc = 0.008;  //retest
    float time_allot = 5.5;
    float disp[3];
    float dist, speed=mathVecMagnitude(me+3,3);
    
    mathVecSubtract(disp, target, me, 3);
    dist = mathVecNormalize(disp, 3);
    
    if( dist<max_dist && speed<max_speed ) {
        api.setPositionTarget(target);
        return( 1==1 );
    }
    
    if( (dist<0.1) || 
        ((0.5*max_acc*time_allot*time_allot+speed*time_allot)>=dist) ) {
            
        api.setPositionTarget(target);
        return( 0==1 );
    }
    else {
        for(i=0; i<3; i++) {
            disp[i] *= (dist/2.0);
        }
        api.setVelocityTarget(disp);
        return( 0==1 );
    }
    
}

bool getPack(int targetPackId) {
            
        if( mySetPositionTarget( (float*)ITEM_LOC[targetPackId],MP_RADIUS,MP_SPEED ) ) {
            temp1[0] = 0.0f;
            temp1[1] = 0.0f;
            temp1[2] = 1.0f;
            api.setAttRateTarget(temp1);
        }
        else ;
        
        if( game.hasItem(targetPackId)!=-1 ) {
            return(true);
        }
        else return(false);
    
}

