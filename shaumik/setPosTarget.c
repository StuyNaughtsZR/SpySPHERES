bool mySetPositionTarget(float *target, float max_dist, float max_speed) {
    
    const float max_acc = 0.008;  //retest
    float time_allot = 4.5;
    float disp[3];
    float dist;
    int i;
    
    mathVecSubtract(disp, target, myState, 3);
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
