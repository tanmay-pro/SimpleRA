# read two matrices from a file and check if they are transpose of each other

import numpy as np

def checkTranspose(A, B):
    if A.shape != B.shape:
        return False
    for i in range(A.shape[0]):
        for j in range(A.shape[1]):
            if A[i][j] != B[j][i]:
                return False
    return True

def main():
    # read matrices from file csv
    A = np.genfromtxt('M33.csv', delimiter=',')
    B = np.genfromtxt('MT33.csv', delimiter=',')

    # check if A is transpose of B
    if checkTranspose(A, B):
        print("A is transpose of B")
    else:
        print("A is not transpose of B")

if __name__ == "__main__":
    main()