/* Data Collector */
/*
 * This program is made to acquire data for StuyNaughts - ZR.
 * Task:
 * Acquire data on how many degrees per second the satellite moves when we set
 * attitude to a constant.
 *
 * Maybe find how much fuel and energy turning costs. Also, report how long it
 * takes to slow down and break.
 */

#define ANGLES_PER_SECOND 20.0f
#define FUNCTION_VER "api.setAttRateTarget"

// Constants for task
float current_attitude;
float target_rate[3];

// Regular ZR constants
ZRState me;

void init(){
    api.getMyZRState(me);
    current_attitude = 0.0f;
    target_rate[0] = target_rate[1] = 0.0f;
    target_rate[2] = deg2rad(ANGLES_PER_SECOND);
    // Begin printing logs
    DEBUG(("-- BEGIN DEBUG LOGS --\n"));
    DEBUG(("Using func: %s\n", FUNCTION_VER));
    DEBUG(("Degrees / Second: %2.1f\n", ANGLES_PER_SECOND));
}

void loop(){
    api.getMyZRState(me);
    DEBUG(("Omega: %1.3f rad / s | %1.3f deg / s)", me[11], rad2deg(me[11])));
    api.setAttRateTarget(target_rate);
}

float rad2deg(float rad) {
    return rad * 180.0f / PI;
}

float deg2rad(float deg) {
    return deg * PI / 180.0f;   
}

