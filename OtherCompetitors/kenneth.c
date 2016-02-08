#define BLU 0
#define RED 1

// Created by Kenneth Li

ZRState me, other;
int gameState, moveState, picState, side;
int objectives[6];
float energy;
float startPos[3], 
      currentPos[3], 
      targetPos[3], 
      otherPos[3], 
      currentVel[3], 
      targetVel[3], 
      curDispVec[3], 
      totalDispVec[3], 
      curAttVec[3], 
      targetAttVec[3],
      curAttVel[3];
bool setup;

void init(){
    for (int i = 0; i < 3; ++i) {
        startPos[i] = 0;
        currentPos[i] = 0;
        targetPos[i] = 0;
        otherPos[i] = 0;
        currentVel[i] = 0;
        targetVel[i] = 0;
        curDispVec[i] = 0;
        totalDispVec[i] = 0;
        curAttVec[i] = 0;
        targetAttVec[i] = 0;
        curAttVel[i] = 0;
    }
	gameState = 0;
	moveState = 0;
	picState = 0;
	side = 0;
	setup = false;
}

void loop(){
    api.getMyZRState(me);
    api.getOtherZRState(other);
    for (int i = 0; i < 3; ++i) {
        currentPos[i] = me[i];
        currentVel[i] = me[i + 3];
        curAttVec[i] = me[i + 6];
        curAttVel[i] = me[i + 9];
        otherPos[i] = other[i];
    }
    if (!setup) {
        if (currentPos[0] > 0) {
            side = BLU;
            objectives[0] = 8;
            objectives[1] = 4;
            objectives[2] = 0;
            objectives[3] = 6;
            objectives[4] = 3;
            objectives[5] = 2;
        } else {
            side = RED;
            objectives[0] = 7;
            objectives[1] = 5;
            objectives[2] = 1;
            objectives[3] = 6;
            objectives[4] = 3;
            objectives[5] = 2;
        }
    }
    if (game.getCurrentTime() > 165 && game.getMemoryFilled() > 0) {
        picState = 2;
    } else if (game.getMirrorTimeRemaining() > 0) {
        picState = -1;
    } else {
        picState = game.getMemoryFilled();
    }
    energy = game.getEnergy();
    switch (picState) {
        case -1:
            for (int i = 0; i < 3; ++i) {
                targetAttVec[i] = otherPos[i] - currentPos[i];
            }
            if ((((energy > 2 || game.posInLight(currentPos) && game.getLightSwitchTime() > 10) || 
                game.getCurrentTime() > 170) && 
                (!game.posInDark(otherPos) || game.posInDark(otherPos) && game.getLightSwitchTime() < 10)) &&
                (fabsf(acosf(curAttVec[2] / mathVecMagnitude(curAttVec, 3))) < 0.25 && mathVecMagnitude(curAttVel, 3) < 0.05)) {
                game.uploadPics();
            }
            break;
        case 0:    
            if ((energy > 3 || game.posInLight(currentPos) && game.getLightSwitchTime() > 10) &&
                game.posInLight(otherPos) && game.isFacingOther() && game.getPicPoints() > 0) {
                game.takePic();
            } else {
                for (int i = 0; i < 3; ++i) {
                    targetAttVec[i] = otherPos[i] - currentPos[i];
                }
            }
            break;
        case 1:
            if (game.posInLight(otherPos)) {
                if ((energy > 3 || game.posInLight(currentPos) && game.getLightSwitchTime() > 10) &&
                    game.isFacingOther() && game.getPicPoints() > 0) {
                    game.takePic();
                } else {
                    for (int i = 0; i < 3; ++i) {
                        targetAttVec[i] = otherPos[i] - currentPos[i];
                    }
                }
            } else {
                if ((((energy > 2 || game.posInLight(currentPos) && game.getLightSwitchTime() > 10) || 
                    game.getCurrentTime() > 170) && 
                    (!game.posInDark(otherPos) || game.posInDark(otherPos) && game.getLightSwitchTime() < 10)) &&
                    (fabsf(acosf(curAttVec[2] / mathVecMagnitude(curAttVec, 3))) < 0.25 && mathVecMagnitude(curAttVel, 3) < 0.05)) {
                    game.uploadPics();
                } else {
                    targetAttVec[0] = 0;
                    targetAttVec[1] = 0;
                    targetAttVec[2] = 1;
                }
            }
            break;
        case 2:
            if ((((energy > 2 || game.posInLight(currentPos) && game.getLightSwitchTime() > 10) || 
                game.getCurrentTime() > 170) && 
                (!game.posInDark(otherPos) || game.posInDark(otherPos) && game.getLightSwitchTime() < 10)) &&
                (fabsf(acosf(curAttVec[2] / mathVecMagnitude(curAttVec, 3))) < 0.25 && mathVecMagnitude(curAttVel, 3) < 0.05)) {
                game.uploadPics();
            } else {
                targetAttVec[0] = 0;
                targetAttVec[1] = 0;
                targetAttVec[2] = 1;
            }
            break;
    }
    if ((((energy > 3 || game.posInLight(currentPos) && game.getLightSwitchTime() > 10) && 
        (!game.posInDark(otherPos) || game.posInDark(otherPos) && game.getLightSwitchTime() < 10)) &&
        (game.getFuelRemaining() > 5)) || game.getCurrentTime() > 165) {
        api.setAttitudeTarget(targetAttVec);
    }
    if (!game.posInDark(currentPos) && !game.posInDark(targetPos) && game.getNumMirrorsHeld() > 0 && game.getLightSwitchTime() >= 24) {
        game.useMirror();
    }
    if (moveState) {
        mathVecSubtract(curDispVec, targetPos, currentPos, 3);
        move();
        if (mathVecMagnitude(curDispVec, 3) < 0.05 && mathVecMagnitude(currentVel, 3) < 0.01) {
            DEBUG(("Picked up item %d", objectives[gameState]));
            moveState = 0;
            gameState += 1;
        }
    } else {
        switch (gameState) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                if (game.hasItem(objectives[gameState]) != -1) {
                    gameState += 1;
                    break;
                }
                game.getItemLoc(targetPos, objectives[gameState]);
                if (((game.posInLight(currentPos) && game.getLightSwitchTime() > 10) || 
                (game.posInLight(targetPos) && energy > 2) || energy > 3) && game.getFuelRemaining() > 10) {
                    for (int i = 0; i < 3; ++i) {
                        startPos[i] = currentPos[i];
                    }
                    mathVecSubtract(totalDispVec, targetPos, startPos, 3);
                    DEBUG(("Now moving towards item %d", objectives[gameState]));
                    DEBUG(("Now moving towards: %f, %f, %f", targetPos[0], targetPos[1], targetPos[2]));
                    moveState = 1;
                } else {
                    bool correct = false;
                    for (int i = 0; i < 3; ++i) {
                        if (fabsf(currentPos[i]) > 0.5 && sign(currentVel[i]) == sign(currentPos[i])) {
                            targetVel[i] *= -1;
                            correct = true;
                        }
                    }
                    if (correct) {
                        DEBUG(("Correcting velocity...%f %f %f", targetVel[0], targetVel[1], targetVel[2]));
                        api.setVelocityTarget(targetVel);
                    }
                }
	            break;
        }
    }
}

void move() {
    for (int i = 0; i < 3; ++i) {
        if (fabsf(curDispVec[i]) > 0.1) {
            targetVel[i] = curDispVec[i] / fabsf(totalDispVec[i]) * 0.08;
        } else {
            if (fabsf(curDispVec[i]) > 0.05) {
                targetVel[i] = sign(curDispVec[i]) * 0.003;
            } else {
                targetVel[i] = 0;
            }
        }
    }
    api.setVelocityTarget(targetVel);
}

void status() {
    DEBUG(("Displacement: %f, %f, %f, %f", curDispVec[0], curDispVec[1], curDispVec[2], mathVecMagnitude(curDispVec, 3)));
    DEBUG(("Velocity: %f, %f, %f, %f", currentVel[0], currentVel[1], currentVel[2], mathVecMagnitude(currentVel, 3)));
}

int sign(float num) {
    if (num > 0) {
        return 1;
    } else if (num < 0) {
        return -1;
    } else {
        return 0;
    }
}
