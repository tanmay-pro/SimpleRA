# write a symmetrical n * n matrix to a csv file

import random
import csv

def gen(x):
    with open('sym15.csv', 'w') as f:
        mat = [[] for i in range(x)]
        for i in range(x):
            for j in range(i + 1):
                mat[i].append(random.randint(-100, 100))   
        for i in range(x):
            for j in range(i + 1, x):
                mat[i].append(mat[j][i])
        writer = csv.writer(f)
        for row in mat:
            writer.writerow(row)

if __name__ == '__main__':
    gen(15)
