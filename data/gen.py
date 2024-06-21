import random
import csv

def gen(x):
    with open('M33.csv', 'w') as f:
        writer = csv.writer(f)
        for i in range(x):
            row = []
            for j in range(x):
                row.append(random.randint(-100, 100))
            writer.writerow(row)
        
if __name__ == '__main__':
    gen(33)