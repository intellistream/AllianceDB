import random

size_of_relations = 10000
upper_limit = 9999
seed = 10
dataset_name = "Test1"

random.seed(seed)
f1 = open(f"{dataset_name}-S.txt", "w")
f2 = open(f"{dataset_name}-R.txt", "w")
skewness = 5
tbase = 10
for i in range(size_of_relations):
    a = random.randint(0, upper_limit)
    tsa = tbase * i + random.randint(0, skewness) + tbase
    tsb = tbase * i + random.randint(0, skewness) + tbase
    b = random.randint(0, upper_limit)
    r1 = str(a) + "," + str(random.randint(0, upper_limit))+","+str(tsa) + "\n"
    r2 = str(b) + "," + str(random.randint(0, upper_limit))+","+str(tsb) + "\n"
    f1.write(r1)
    f2.write(r2)
f1.close()
f2.close()
