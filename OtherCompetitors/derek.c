// Derek Lin

float myState[12];
float otherState[12];
int myColor;
// 0 is blue, 1 is red.
int phase;
// marks which phase of the plan I am in.

float position[3];
float preset1[3];
float preset2[3];

void init(){
    //!WARNING! This does not work because myState[0] does not return a value that corresponds with the actual location of the sphere.
    //As a result, the complete process will be 1 second slower where the first second will be used to determine the color of the sphere.
    //Find out what color sphere I am.
    //if ( myState[0] > 0.0f ){
	//    myColor = 0;
	//}
	//else{
	//   myColor = 1;
	//}
	phase = 0;
	preset1[0] = 0.0f;
	preset1[1] = 0.2f;
	preset1[2] = 0.0f;
	preset2[0] = 0.0f;
	preset2[1] = -0.2f;
	preset2[2] = 0.0f;
}

void loop(){
	api.getMyZRState(myState);
	api.getOtherZRState(otherState);
	position[0] = myState[0];
	position[1] = myState[1];
	position[2] = myState[2];
	//At all points in time, if the enemy is facing me and I have a mirror, activate it.
	
	//Prepare:
	//Find out what color I am
	
	if( phase == 0 ){
	    if( myState[0] > 0.0f ){
	        myColor = 0;
	        phase = 1;
	    }
	    else{
	        myColor = 1;
	        phase = 1;
	    }
	}
	
	//PHASE 1: Mirror collection
	//Collect the closest mirror.
	
	if( phase == 1 ){
    	if( myColor == 0 ){
    	    game.getItemLoc(position, 8);
    	    if( game.hasItem(8) == 0 ){
    	        phase = 2;
    	    }
	    }
	    if( myColor == 1 ){
	        game.getItemLoc(position, 7);
	        if( game.hasItem(7) == 0 ){
	            phase = 2;
	        }
	    }
	    if( game.posInLight(position) || game.posInGrey(position) ){
	        game.useMirror();
	    }
	    api.setPositionTarget(position);
	}
	
	//PHASE 2: Second Mirror
	//Collect the other mirror. (If possible)
	
	// !WARNING!: It the enemy uses the mirror right away, we will be stuck in Phase 2 forever.
	
	if( phase == 2 ){
	    if( myColor == 0 ){
	        if( game.hasItem(7) == -1 ){
	            game.getItemLoc(position, 7);
	        }
	        else{
	            phase = 3;
	        }
    	}
    	if( myColor == 1 ){
    	    if( game.hasItem(8) == -1 ){
	            game.getItemLoc(position, 8);
	        }
	        else{
	            phase = 3;
	        }
	    }
	    api.setPositionTarget(position);
	}
	
	//PHASE 3: Score
	//Go collect score items.
	
	if( phase == 3 ){
	    if( myColor == 0 ){
	        if( game.hasItem(6) == -1 ){
	            game.getItemLoc(position, 6);
	        }
	        else if( game.hasItem(5) == -1 ){
	            game.getItemLoc(position, 5);
	        }
	        //if( game.hasItem(5) == 1 ){
	        //    game.getItemLoc(position, 4);
	        //}
	        else if( game.hasItem(3) == -1 ){
	            game.getItemLoc(position, 3);
	        }
	        else{
	            phase = 4;
	        }
	    }
	    if( myColor == 1 ){
	        if( game.hasItem(6) == -1 ){
	            game.getItemLoc(position, 6);
	        }
	        else if( game.hasItem(4) == -1 ){
	            game.getItemLoc(position, 4);
	        }
	        //if( game.hasItem(4) == 1 ){
	        //    game.getItemLoc(position, 5);
	        //}
	        else if( game.hasItem(3) == -1 ){
	            game.getItemLoc(position, 3);
	        }
	        else{
	            phase = 4;
	        }
	    }
	    if( game.posInLight(position) || game.posInGrey(position) ){
	        game.useMirror();
	    }
	    api.setPositionTarget(position);
	}
	
	//PHASE 4: Be annoying
	//Stall out the game if ahead.
	if( phase == 4 ){
	    if( game.posInLight(preset1) ){
	        api.setPositionTarget(preset2);
	    }
	    else{
	        api.setPositionTarget(preset1);
	    }
	}
	
	
	//Things to work on: 
	//Collision avoidance, 
	//Taking a few pics while collecting items if possible (face the opponent), 
	//Fuel efficiency.
	//Staying in the dark for as long as possible during item collection (adjust speed?).
	//Be less predictable.
	DEBUG(("Phase %d", phase));
}
