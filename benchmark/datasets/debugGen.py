import random

size_of_relations = 10000
upper_limit = 9999
seed = 10
dataset_name = "Debug1"

random.seed(seed)
f1 = open(f"{dataset_name}-S.txt", "w")
f2 = open(f"{dataset_name}-R.txt", "w")
for i in range(size_of_relations):
    a = i
    b = i
    r1 = str(a) + "\n"
    r2 = str(b) + "\n"
    f1.write(r1)
    f2.write(r2)
f1.close()
f2.close()
