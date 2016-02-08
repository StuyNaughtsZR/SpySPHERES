//Declare any variables shared between functions here
/*
Charles Zhang
10/20/15
*/




#include <ZRGame.h>

float me[12],other[12],velocity[3],attrate[3],force[3];
float otherpos[3],mypos[3],uploadpos[3],mirrorpos[3],myatt[3];
float angle,tookmirror,stage;
float mirror1[3],mirror2[3],score1[3],score2[3],score3[3],score4[3],energy1[3],energy2[3],energy3[3];
float mydistance[3],otherdistance[3],direction[3];

void init(){
	mirror1[0] = -0.4f;
	mirror1[1] = 0.15f;
	mirror1[2] = -0.4f;
	
	mirror2[0] = 0.4f;
	mirror2[1] = 0.15f;
	mirror2[2] = -0.4f;
	
	score1[0] = 0.0f;
	score1[1] = 0.6f;
	score1[2] = 0.4f;
	
	score2[0] = 0.4f;
	score2[1] = 0.6f;
	score2[2] = 0.0f;
	
	score3[0] = -0.4f;
	score3[1] = 0.6f;
	score3[2] = 0.0f;
	
	score4[0] = 0.0f;
	score4[1] = 0.6f;
	score4[2] = -0.4f;
	
	energy1[0] = 0.3f;
	energy1[1] = -0.2f;
	energy1[2] = 0.3f;
	
    energy2[0] = -0.3f;
	energy2[1] = -0.2f;
	energy2[2] = 0.3f;
	
	energy3[0] = 0.0f;
	energy3[1] = -0.0f;
	energy3[2] = 0.3f;
	
	tookmirror=0;
	stage = 0;
	
	uploadpos[0] = 0.0f;
	uploadpos[1] = 0.0f;
	uploadpos[2] = 0.1f;
	
}

void loop(){
    api.getMyZRState(me);
    api.getOtherZRState(other);
    
    for(int i = 0;i < 3;i++)
    {
        direction[i] = other[i]-me[i];
        otherpos[i] = other[i];
        mypos[i] = me[i];
        myatt[i] = me[i+6];
        velocity[i] = me[i+3];
    }
    
    angle = acosf(mathVecInner(myatt,uploadpos,3)/(mathVecMagnitude(myatt,3)*mathVecMagnitude(uploadpos,3)));
    DEBUG(("%f",angle));
    
     if (game.getMemoryFilled() != 2)
     {
        api.setAttitudeTarget(direction);
     }
     else
     {
         if (angle > 0.25)
         {
             direction[0]=direction[1]=0;
             direction[2]=0.1f;
            api.setAttitudeTarget(direction);
         }
         else
         {
             attrate[0]=attrate[1]=attrate[2]=0;
             api.setAttRateTarget(attrate);
             game.uploadPics();
         }
     }
    if (game.getPicPoints() > 0)
    {
        if (game.getMemoryFilled() != 2)
        {
            game.takePic();
        }
    }
    //DEBUG(("%d",game.hasItem(ITEM_TYPE_MIRROR)));

    if (tookmirror == 0)
       {
             api.setPositionTarget(mirror2);
             //velocity[0]=velocity[2] = 0.0f;
             //velocity[1] =  1.0f;
             //api.setVelocityTarget(velocity);
            mathVecSubtract(mydistance,mirror2,mypos,3);
            
            if (mathVecMagnitude(mydistance,3) <= 0.01)
            {
                velocity[0]=velocity[1]=velocity[2]=0;
                api.setVelocityTarget(velocity);
            
                tookmirror = 1;
            }
       }
        
       if ( (tookmirror==1) && (stage==0))
       {
                move(score2);


        }
        
        if (stage == 1)
        {
               move(score4);
        }
       
        
        if (stage == 2)
        {
               move(score3);
        }
        
        if (stage == 3)
        {
            move(score1);
        }
        
        
        if (stage == 4)
        {
            
                api.setPositionTarget(energy2);
                mathVecSubtract(mydistance,energy2,mypos,3);
                
            if (mathVecMagnitude(mydistance,3) <= 0.01)
            {
                velocity[0]=velocity[1]=velocity[2]=0;
                api.setVelocityTarget(velocity);
                stage = 1;
            }
            
            mathVecSubtract(otherdistance,energy2,otherpos,3);
            if (mathVecMagnitude(otherdistance,3) < mathVecMagnitude(mydistance,3))
            {
                if (mathVecMagnitude(otherdistance,3) < 0.1)
                {
                    stage = 1;
                }
                
            }
            
            
            
            
        }
        
        
        if (game.posInLight(mypos) && (game.getNumMirrorsHeld() != 0))
        {
            game.useMirror();
        }
        
        if (mypos[0] >= 0.6 || mypos[0] <= -0.6|| mypos[1] >= 0.75|| mypos[1] <= -0.75|| mypos[2] >= 0.6|| mypos[2] <= -0.6)
        {
            force[0] = -2*velocity[0];
            force[1] = -2*velocity[1];
            force[2] = -2*velocity[2];
            api.setForces(force);
            
        }
        
       /*
        if (! (game.posInLight(mypos) && game.posInLight(otherpos)))
        {
            if (game.posInLight(mypos) && game.getMirrorTimeRemaining() == 0)
            {
                stage = -1;
            }
        }
        
        
        
        if (stage == -1)
        {
            mypos[1] = game.getDarkGreyBoundary();
            api.setPositionTarget(mypos);
        }
        */
}

void move(float dest[3])
{
     api.setPositionTarget(dest);
                mathVecSubtract(mydistance,dest,mypos,3);
                
            if (mathVecMagnitude(mydistance,3) <= 0.01)
            {
                //api.setPositionTarget(score1);
                velocity[0]=velocity[1]=velocity[2]=0;
                api.setVelocityTarget(velocity);
                stage++;
            }
            
            mathVecSubtract(otherdistance,dest,otherpos,3);
            if (mathVecMagnitude(otherdistance,3) < mathVecMagnitude(mydistance,3))
            {
                if (mathVecMagnitude(otherdistance,3) < 0.05)
                {
                    stage++;
                }
                
            }
}
