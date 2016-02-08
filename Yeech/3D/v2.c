#define go_to_shadow_realm 1
#define get_epack 2
#define get_spack 3
#define get_mpack 4
#define haul_ass_tolerance 0.2

ZRState me, other;
bool done;
int state, temp, enemy_mirror;
float epacks[3][3], spacks[4][3], mpacks[2][3], target[3];

void init(){
	memcpy(epacks, ITEM_LOC[0], 9*sizeof(float));
	memcpy(spacks, ITEM_LOC[3], 12*sizeof(float));
	memcpy(mpacks, ITEM_LOC[7], 6*sizeof(float));
	state = get_mpack;
	enemy_mirror = which_he_close_to(other, mpacks, 2);
	done = false;
}

void loop(){
	api.getMyZRState(me);
	api.getOtherZRState(other);
	// If I'm running low on juice, Imma get an energy pack
	// (if there is one).
	if (
		game.getEnergy() < 1 &&
		(
			game.hasItem(0) == -1 ||
			game.hasItem(1) == -1 ||
			game.hasItem(2) == -1
			)
		) {
		state = get_epack;
	}
	while (!done) {
		done = true;
		switch(state) {
			case get_mpack:
				// Who has the other guy's mirror?
				temp = game.hasItem(enemy_mirror+7);
				// If no one has it, I want it.
				if (temp == -1) {
					// If I got it, Imma deploy it and get a score pack.
					if (get_item(mpacks[enemy_mirror])) {
						game.useMirror();
						state = get_spack;
						done = false;
					}
				}
				// If the other guy has it, I'll go for my own mirror.
				else {
					// If my own mirror is gone, Imma go get a score pack
					// if there's one in the dark zone.
					if (game.hasItem(8-enemy_mirror) != -1) {
						if (
							game.posInDark(spacks[0]) &&
							(
								game.hasItem(3) == -1 ||
								game.hasItem(4) == -1 ||
								game.hasItem(5) == -1 ||
								game.hasItem(6) == -1
								)
							) {
							state = get_spack;
							done = false;
						}
						// If there are no score packs in the dark zone,
						// Imma go hide and take pics.
						else {
							state = go_to_shadow_realm;
							done = false;
						}
					}
					// If I got my own mirror, Imma deploy it and get
					// a score pack.
					else if (get_item(mpacks[1-enemy_mirror])) {
						game.useMirror();
						state = get_spack;
						done = false;
					}
				}
				break;
			case get_spack:
				// If I'm in the light zone without a mirror,
				// Imma go hide and take pics.
				if (game.posInLight(me) && game.getMirrorTimeRemaining() < 1) {
					state = go_to_shadow_realm;
					done = false;
				}
				// If the first mirror is available and the other guy ain't
				// tryna get it, Imma go get it.
				if (game.hasItem(3) == -1 && which_he_close_to(other, spacks, 4) != 0) {
					get_item(spacks[0]);
				}
		}
	}
}

float distance(float p1[], float p2[]) {
	float temp[3];
	mathVecSubtract(temp, p1, p2, 3);
	return mathVecMagnitude(temp, 3);
}

bool get_item(float target[]) {
	float dist = distance(me, target);
	float velocity = mathVecMagnitude(&me[3], 3);
	if (dist > haul_ass_tolerance) {
		// HAUL ASS!!!
		float temp[3];
		mathVecSubtract(temp, target, me, 3);
		api.setVelocityTarget(temp);
		return false;
	}
	if (dist > 0.05 || velocity > 0.01) {
		api.setPositionTarget(target);
		return false;
	}
	return true;
}

int which_he_close_to(
	ZRState someone,
	float locs[][3],
	int num_locs
	) {
	// Returns the index of the location that is closest to someone.
	float min_dist = 5; // any number bigger than the Interaction Zone's diagonal
	int ans = num_locs;
	float dist;
	while (num_locs > 0) {
		dist = distance(someone, locs[num_locs]);
		// If this is the smallest distance so far, update ans and min_dist.
		if (dist < min_dist) {
			ans = num_locs;
			min_dist = dist;
		}
		num_locs--;
	}
	// Return the index of the location in locs that produced the smallest distance.
	return ans;
}

int which_he_lookin_at(
	ZRState someone,
	float locs[][3],
	int num_locs
	) {
	// Returns the index of the location such that the vector from someone to that
	// location makes the smallest possible angle with someone's attitude.
	float min_angle = 10; // any number bigger than PI
	int ans = num_locs;
	float angle;
	float to_loc[3];
	// Get the attitude vector.
	float attitude[3];
	memcpy(attitude, &someone[6], 3*sizeof(float));
	while (num_locs > 0) {
		// Construct the vector from someone to loc[i].
		to_loc[0] = locs[num_locs][0] - someone[0];
		to_loc[1] = locs[num_locs][1] - someone[1];
		to_loc[2] = locs[num_locs][2] - someone[2];
		mathVecNormalize(to_loc, 3);
		// Find the angle between attitute and to_loc.
		angle = acosf(mathVecInner(attitude, to_loc, 3));
		// If this is the smallest angle so far, update ans and min_angle.
		if (angle < min_angle) {
			ans = num_locs;
			min_angle = angle;
		}
		num_locs--;
	}
	// Return the index of the location in locs that produced the smallest angle.
	return ans;
}
