#!/usr/bin/python

debug_file_prefix = "set_att_rate_target_"
debug_file_suffix = "f.log"

debug_file_vals = ["5.0", "10.0", "15.0", "20.0"]

speed_tolerance = 1.5

def analyze(filename):
    f = open(filename)
    assert("Sphere 1, 0.0s, DBG: -- BEGIN DEBUG LOGS --" in f.readline())
    API_FUNC = f.readline().strip().split(" ")[-1]
    TEST_SPEED = float(f.readline().strip().split(" ")[-1])
    print("BEGIN ANALYSIS:")
    print("Function Used: " + API_FUNC)
    print("Speed: " + str(TEST_SPEED) + " deg/s")
    # Variable to store line
    data = f.readline().strip()
    # Variables to store data
    speeds_while_accel = []
    total = 0.0
    num_entries = 0
    accelerating = True
    while "Sphere 1" in data and "DBG:" in data:
        # Get the speed
        speed = float(data.split(" ")[10])
        if accelerating:
            if TEST_SPEED - speed > speed_tolerance:
                speeds_while_accel.append(speed)
            else:
                accelerating = False
                num_entries += 1
                total += speed
        else:
            num_entries += 1
            total += speed
        data = f.readline().strip()
    print("Tolerance: " + str(speed_tolerance))
    print("Reached desired speed in: " + str(len(speeds_while_accel)) + " seconds")
    print speeds_while_accel
    average = total / float(num_entries)
    print("Average omega after reached tolerance level: " + str(average) + " deg/s")
    print("\n")

def main():
    for val in debug_file_vals:
        analyze(debug_file_prefix + val + debug_file_suffix)

if __name__ == "__main__":
    main()

