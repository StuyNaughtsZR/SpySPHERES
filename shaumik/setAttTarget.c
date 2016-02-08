//This is the best I have right now, but its still worse then api.setAttitudeTarget
//its slower and consumes more feel than api.setAttitudeTarget
//i think the main problem is tat it goes to far off the plane of rotation 
//does any one have any ideas?

bool mySetAttitudeTarget(float target[3], float tolerance) {
	/* parameters:
		target - point to which satelite will face
		tolerance - tolerance angle, faces target if true angle difference less than tolerance
	  return value:
	  	true if facing target
		false if not
	  NOTE: does not check angular velocity in facing target check!!!
	*/

	float att[3], dir[3], attRate[3]; //my attitude, target direction, angulaur velocity
	float thetha;                     //current angle difference between att and dir
	float time_alloc = 4.5;           //adjust this to optimize
	float alpha = 0.0;                //OPTIMIZABLE set alpha to acceleration per sec value...

	//find and normalize vectors: att, dir
	mathVecSubtract(dir, target, me, 3);
	mathVecNormalize(dir, 3);
	
	memcpy(att, me+6, sizeof(float)*3);
	mathVecNormalize(att, 3); 
	
	//get attRate as unit vector, adjust magnitude later
	mathVecCross(attRate, dir, att);    //this shud determine plane of rotation
	mathVecNormalize(attRate, 3);
	
	//find thetha
	thetha = acosf( mathVecInner(att, dir, 3) ); //magnitudes are 1, so no need to divide
	
	if( thetha < 0.1 || 
		thetha < tolerance ||
		0.5*alpha*time_alloc*time_alloc+mathVecMagnitude(me+9, 3)*time_alloc > thetha
	  ) {
		api.setAttitudeTarget(dir);
	}
	else {
		//set magnitude of attRate
		for(i=0; i<3; i++) {
			attRate[i] *= ( thetha / 3.0 );  //OPTIMIZE HERE
		}
		api.setAttRateTarget(attRate);
	}
	
	return( thetha < tolerance ? 1==1 : 1==0 );  //felt like using this operator for once

} 
