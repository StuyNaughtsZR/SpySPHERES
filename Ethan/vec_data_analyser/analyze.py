import glob, re

LOGS = glob.glob('./*.log')
KEY = "FIB"
DECRYPTION_VEC_ME = [-17, 2, 9, 19, -27, -8, 29, 3, -13, -21, 4, 12]
DECRYPTION_VEC_OTHER = [-1 * x for x in DECRYPTION_VEC_ME]

print("KEY: " + KEY)
print("DECRYPTION VECTOR FOR ME:")
print(DECRYPTION_VEC_ME)
print("DECRYPTION VECTOR FOR OTHER:")
print(DECRYPTION_VEC_OTHER)

for log in LOGS:
    f = open(log)
    DATA = f.read().split("\n")
    f.close()
    OUT = ""
    for line in DATA:
        if KEY not in line:
            continue
        array = line.split("DBG: ")[1].split("[")[1].split("]")[0].split(", ")
        assert(array[0] == KEY)
        if array[1] == "0":
            OUT += "TIME, " + array[3] + ", "
        if array[1] == "1":
            OUT += "FRED, %f, %f, %f, %f, %f %f, %f, %f, %f, %f, %f, %f, " % \
            (float(array[3]) / DECRYPTION_VEC_ME[0],
            float(array[4]) / DECRYPTION_VEC_ME[1],
            float(array[5]) / DECRYPTION_VEC_ME[2],
            float(array[6]) / DECRYPTION_VEC_ME[3],
            float(array[7]) / DECRYPTION_VEC_ME[4],
            float(array[8]) / DECRYPTION_VEC_ME[5],
            float(array[9]) / DECRYPTION_VEC_ME[6],
            float(array[10]) / DECRYPTION_VEC_ME[7],
            float(array[11]) / DECRYPTION_VEC_ME[8],
            float(array[12]) / DECRYPTION_VEC_ME[9],
            float(array[13]) / DECRYPTION_VEC_ME[10],
            float(array[14]) / DECRYPTION_VEC_ME[11])
        if array[1] == "2":
            OUT += "GEORGE, %f, %f, %f, %f, %f %f, %f, %f, %f, %f, %f, %f\n" % \
                    (float(array[3]) / DECRYPTION_VEC_OTHER[0],
                    float(array[4]) / DECRYPTION_VEC_OTHER[1],
                    float(array[5]) / DECRYPTION_VEC_OTHER[2],
                    float(array[6]) / DECRYPTION_VEC_OTHER[3],
                    float(array[7]) / DECRYPTION_VEC_OTHER[4],
                    float(array[8]) / DECRYPTION_VEC_OTHER[5],
                    float(array[9]) / DECRYPTION_VEC_OTHER[6],
                    float(array[10]) / DECRYPTION_VEC_OTHER[7],
                    float(array[11]) / DECRYPTION_VEC_OTHER[8],
                    float(array[12]) / DECRYPTION_VEC_OTHER[9],
                    float(array[13]) / DECRYPTION_VEC_OTHER[10],
                    float(array[14]) / DECRYPTION_VEC_OTHER[11])
    f = open(log + ".csv", 'w')
    f.write(OUT)
    f.close()

