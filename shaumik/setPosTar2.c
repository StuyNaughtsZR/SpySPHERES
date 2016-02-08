/*
Better setPositionTarget function!
NOTE: THIS IS INTENDED TO BE USED GOING TO A STATIC POINT, USING IT TO MOVE TO A CONSTANTLY CHANGING POINT MAY MESS IT UP!!!!

Assumes globals: me, i

I used this algorithm in the JOEL2SA competitor move function, beats JOEL2 in speed
*/

void setPositionTarget(float target[]) {
    
    float disp[3];
    float dist, speed;
    
    mathVecSubtract(disp, target, me, 3);
    dist = mathVecNormalize(disp, 3); //disp normalized to unit vector, holds direction of desired velocity
    
    /* Condition here is based off of equation: d = (1/2)at^2 + vt
       a = 0.01, which is the approx maximum acceleration I have experimentally found of a SPHERE
       I assume t = 8s, which works and is faster then api.setPositionTarget
       You can change t if there is an overshoot
    */
    if (dist > 0.5*0.01*64+mathVecMagnitude(me+3,3)*8) {

        speed = dist; 

        for(i=0; i<3; i++) {  //scale velocity (disp) to speed
          disp[i] *= speed;
        }
        
        api.setVelocityTarget(disp);
    
    }
    else {
        api.setPositionTarget(target);
    }
    
}
