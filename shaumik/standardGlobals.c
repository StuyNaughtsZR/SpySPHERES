/*
EVERONE MUST USE THESE GLOBAL VARIABLES

IF YOU NEED MORE, TRY LOCAL STATIC VARIABLES OR PASSING MORE PARAMETERS

IF YOU DONT NEED ALL THESE, JUST LEAVE THEM IN ANYWAYS; YOU CAN REMOVE THEM BEFORE FINAL SUBMIT
*/



ZRState me, other;/* use pointer arithmetic to access position, velocity,attitude,and rotational velocity (look at bottom) */
float temp1[3], temp2[3];
float tempMatrix[3][3];
float itemStatus[NUM_ITEMS];
int state, i;
int myZone, otherZone;


#define LIGHT_ZONE 1
#define GREY_ZONE 0      
#define GRAY_ZONE 0     
#define DARK_ZONE -1

#define ME_HAVE 0
#define OTHER_HAVE 1
#define NO_HAVE -1



void init(){

	state = 0;

}



void loop(){

	startloop();
    
  /*start real work here... good luck*/
}




void startloop(void) {  /*copy this function to set globals at start of loop()*/

    api.getMyZRState(me);
    api.getOtherZRState(other);
    myZone = game.posInArea(me);
    otherZone = game.posInArea(other);
	  for(i=0; i<NUM_ITEMS; i++) {
        itemStatus[i] = game.hasItem(i);
	  }
  
}


/*
If you don't know pointer arithmetic, use this:
https://www.cs.umd.edu/class/sum2003/cmsc311/Notes/BitOp/pointer.html
*/
