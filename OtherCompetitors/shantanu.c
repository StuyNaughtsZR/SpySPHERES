/*
2.0
Shantanu Jha
10/20/2015
*/
//target
float enemy_direction[3], enemy_distance, enemy_position[3];

//enemy
float other[12];

//self
float me[12];
bool red_or_blue;

int stage;
bool state, pic_state;

//items
float energy_1[3], energy_2[3], energy_3[3]; 
float score_1[3], score_2[3], score_3[3], score_4[3];
float mirror_1[3], mirror_2[3];


void init(){
	//set items
	//energy
	energy_1[0] = 0.3;
	energy_1[1] = -0.2;
	energy_1[2] = 0.3;
	
	energy_2[0] = -0.3;
	energy_2[1] = -0.2;
	energy_2[2] = 0.3;
	
	energy_3[0] = 0.0;
	energy_3[1] = 0.0;
	energy_3[2] = 0.3;
	
	//score items
	score_1[0]= 0.0;
	score_1[1]= 0.6;
	score_1[2]= 0.4;
	
	score_2[0]= 0.4;
	score_2[1]= 0.6;
	score_2[2]= 0.0;
	
	score_3[0]= -0.4;
	score_3[1]= 0.6;
	score_3[2]= 0.0;
	
	score_4[0]= 0.0;
	score_4[1]= 0.6;
	score_4[2]= -0.4;
	
	//mirrors
	mirror_1[0] = -0.4;
	mirror_1[1] = 0.15;
	mirror_1[2] = -0.4;
	
	mirror_2[0] = 0.4;
	mirror_2[1] = 0.15;
	mirror_2[2] = -0.4;
	
	//initialize ZR States
	api.getMyZRState(me);
	api.getOtherZRState(other);
	
	//init stage variables
	stage = 0;
	state = true;
	
	float temp;
	temp = me[0];
    
    if (temp > 0) {
        red_or_blue = true;
    }
    else {
        red_or_blue = false;
    }
	
	pic_state = false;
}

void target_enemy(){
    api.getMyZRState(me);
	api.getOtherZRState(other);
    float temp[3]; //stores vector between me and other
    temp[0] = other[0] - me[0];
    temp[1] = other[1] - me[1];
    temp[2] = other[2] - me[2];
    
    enemy_distance = mathVecNormalize(temp, 3);
    enemy_direction[0] = temp[0];
    enemy_direction[1] = temp[1];
    enemy_direction[2] = temp[2];
    
    enemy_position[0] = other[0];
    enemy_position[1] = other[1];
    enemy_position[2] = other[2];
  
}

void pic(){
    float temp[3];
    //for angle
    api.getMyZRState(me);
    target_enemy();
    temp[0] = enemy_direction[0];
    temp[1] = enemy_direction[1];
    temp[2] = enemy_direction[2];
    
    
    float att[3]; //stores me attitude 
    att[0] = me[6];
    att[1] = me[7];
    att[2] = me[8];
    
    
    if (game.posInLight(enemy_position) && angle(att, temp) <= .25 && game.getMemoryFilled() < 2){
                //api.setAttitudeTarget(enemy_direction);
                game.takePic();
    }
    if (game.getMemoryFilled() == 2){
        upload();
    }
}

void upload(){
    
    float temp[3];
    temp[0] = EARTH[0];
    temp[1] = EARTH[1];
    temp[2] = EARTH[2];
    
    api.setAttitudeTarget(temp);
    api.getMyZRState(me);
    
    /*if(angle(temp, temp2) < .15){
        game.uploadPics();
    }*/
    
    game.uploadPics();
}

float angle(float temp[3], float lol[3]){
    api.getMyZRState(me);
    float a, b, c;
    a = mathVecInner(temp, lol, 3);
    b = mathVecMagnitude(temp, 3);
    c = mathVecMagnitude(lol, 3);
    
    return acosf(a/(b*c));
}

void get_stuff(float stuff[3], float number){
    api.setPositionTarget(stuff);
    if(game.hasItem(number) != -1) {
        stage++;
        //state = false;
        
    }
}
        
//use mirror only if angle between other's attitude and vector between other and me is less than .25
void mirror_use(){
    api.getOtherZRState(other);
    target_enemy();
    
    float temp[3];
    temp[0] = other[6];
    temp[1] = other[7];
    temp[2] = other[8];
    
    if(angle(enemy_direction, temp) < .25){
        game.useMirror();
    }
}

void distance(float a[3], float b[3]){
    float temp[3];
    temp[0] = b[0] - a[0];
    temp[1] = b[1] - a[1];
    temp[2] = b[2] - a[2];
    
    if (mathVecMagnitude(temp, 3) <= .05){
        stage++;
    }
}
void loop(){
    float foo[3];
    api.getMyZRState(me);
    foo[0] = me[0];
    foo[1] = me[1];
    foo[2] = me[2];
    
   
    target_enemy();
    api.setAttitudeTarget(enemy_direction);
    
   
    if (red_or_blue){ 
        if (stage == 0){
            get_stuff(mirror_2, 8);
        }
        
        if (stage == 1){
            get_stuff(score_4, 6); //top score block
        }
        
        if (stage == 2){
            get_stuff(mirror_1, 7); // right mirror
        }
        
        if (stage == 3){
            get_stuff(score_2, 4); //left block
        }
        
        if (stage == 4){
            get_stuff(score_3, 5);//right block
        }
        
        
        if (stage == 5){
            get_stuff(energy_2, 1);//right energy
        }
        
    }
    else {
        if (stage == 0){
            get_stuff(mirror_1, 7);
        }
        
        if (stage == 1){
            get_stuff(score_4, 6); //top score block
        }
        
        if (stage == 2){
            get_stuff(mirror_2, 8); //left block
        }
        
        
         if (stage == 3){
            get_stuff(score_3, 5);//right block
        }
        if (stage == 4) {
            get_stuff(score_2, 4);//left block
        }
    
        if (stage == 5){
            get_stuff(energy_1, 0); //left energy
        }
    }
    mirror_use();
    pic();
}
