# Project Phase 1 Report

### General Structure and Implementation

- **Technique for handling large matrices and page layout**: 
Every page can store a submatrix of the size the expression below gives. We have taken the square root because we are storing square submatrices. Since the BLOCK_SIZE is 1, the above value comes out to be close to 15. Thus, we store a submatrix of maximum size 15*15 on each page (block).
    
    $$
    (1000 * BLOCK\_SIZE/ SIZEOF(INT))^{0.5}
    $$
    
- The `N`*`N` matrix is divided into various `a`*`b` submatrices where a or b can max be 15 for any page.
- The above technique makes commands like TRANSPOSE, CHECKSYMMETRY and COMPUTE much more efficient.
- A new matrix class is created, which is inherited from the table class. A few more member variables and member functions have been added. Additionally, some of the existing member functions that were valid for tables have been overloaded for matrices. Page design and the buffer manager have been modified to accommodate matrices wherever applicable. Syntactic Parser and semantic parser checks have been added for the new commands. 3 new executors for `transpose`, `checkSymetry`, and `compute` have also been created.

### Commands

- <Command name>: LOAD MATRIX
    - We read the input .csv file once but write on each page multiple times. Let `a` denote the submatrix size which is stored on each page. It will be the maximum submatrix size if the leftover matrix is larger than what can be stored on the page, or it will be the size of the leftover matrix.
    - We read the first elements of the file and write it to the first page. Then, we read the next `a`  elements of the file and write it to the next page. We continue doing this until we reach the end of the file's first row. Then, we move to the next row of the file and start appending back to the first page.
    - After we are done writing `a` rows on a page, we skip around `n/a` pages and then start writing to this new page. We continue doing this operation until we reach the end of the file. This will write the whole matrix in the form of smaller submatrices.
    - Example: Let us consider an input matrix of size `20*20`. We store a `15*15` submatrix on the page with index `0`. Then, the page with index `1` stores a submatrix of size `15*5`. The next page with index `2` stores a submatrix of size `5*15`. The last page with index `3` stores the submatrix of size `5*5`.
    - `BLOCKS_READ = 0` , `BLOCKS_WRITTEN = N*a`, `BLOCKS_ACCESSED = N*a, where N = Number of blocks of a given matrix and a = submatrix size`
- <Command name>: PRINT MATRIX
    - We write the first row of the first page. Then, we move to the next page using the cursor. We write the first row of the next page and then move the cursor to the next page. After writing the entire first row of the matrix, we move the cursor back to the first page (to the second row instead of the first row). After `a` number of rows have been written, the cursor reaches to the end of the page. Now, we move the cursor to the vertically down page (skipping `n/a` pages). We continue doing this until we reach the end of the last page.
    - `BLOCKS_READ = N*a`, `BLOCKS_WRITTEN = 0`, `BLOCKS_ACCESSED = N*a, where N = Number of blocks of a given matrix and a = submatrix size`
- <Command name>: RENAME MATRIX
    - We update the name of all the stored files in `/data/temp` according to the new table name. Additionally, we update the `tableName` and the `sourceFileName` of the matrix. After that, we update the `tableCatalogue` by modifying the map, which stores pointers to tables/matrices according to `tableName`.
    - `BLOCKS_READ = 0`, `BLOCKS_WRITTEN = 0`, `BLOCKS_ACCESSED = 0`
- <Command name>: EXPORT MATRIX
    - Elements of the submatrix are traversed (logic same as PRINT MATRIX command). Now, these elements are outputted to a CSV file using four of std::ofstream.
    - `BLOCKS_READ = N*a`, `BLOCKS_WRITTEN = 0`, `BLOCKS_ACCESSED = N*a, where N = Number of blocks of a given matrix and a = submatrix size`
- <Command name>: TRANSPOSE
    - We are using the following formulation for transposing a matrix. Here, each `A`, `B`, `C` and `D` are sub-matrices.
        
        $$
        \begin{bmatrix} 
        	A & B \\
        	C & D \\
        \end{bmatrix} ^{T} =
        \begin{bmatrix} 
        	A^T & C^T \\
        	B^T & D^T \\
        \end{bmatrix} 
        
        $$
        
    - Since each page is a single submatrix, we can use the above formulation to transpose a given matrix.
    - Let `i` indicate the row position of any submatrix, whereas `j` is the column position of any submatrix in the matrix. We load every `i,j` page in the memory, where `i <= j`.
    - Now, if `i` is equal to `j`, then we update the particular page (in-place) to store the transpose of the submatrix on that page (The page is written back to its original location).
    - If `i` is not equal to `j`, then we take the transpose of the submatrix at the `i,j` page (in-place) and the transpose of the submatrix at the `j,i` page. Now, we interchange these pages so that the transpose of the submatrix originally at `i,j` now comes to `j, i` and vice versa.
    - `BLOCKS_READ = N`, `BLOCKS_WRITTEN = N`, `BLOCKS_ACCESSED = 2N, where N = Number of blocks of a given matrix`
    
- <Command name>: CHECKSYMMETRY
    - We derive this command's logic from the TRANSPOSE command's logic.
    - We again take the following: Let `i` indicate the row position of any submatrix, whereas `j` is the column position of any submatrix in the matrix. Now, we load every `i,j` page in the memory, where `i <= j`.
    - If `i` is equal to `j`, we need to check the symmetry of the submatrix with itself. Therefore, we go to every `row, column` element of the submatrix and check if it is the same as the element at the position `column`. If the above is true for all the elements, then the submatrix is symmetric.
    - If `i` is not equal to `j`, we need to check the symmetry of the submatrix at `i,j` with the submatrix at `j,i`. Thus, we will traverse to every `row, column` element of the submatrix on both the `i,j` and `j,i` pages and check if they are the same. The complete matrix is symmetric if it is the same for all the elements. Otherwise, not symmetric.
    - `BLOCKS_READ = Number of blocks`, `BLOCKS_WRITTEN = 0`, `BLOCKS_ACCESSED = Number of blocks`
    
- <Command name>: COMPUTE
    - We again take the following: Let `i` indicate the row position of any submatrix, whereas `j` be the column position of any submatrix in the matrix. Now, we load every `i,j` page in the memory, where `i <= j`.
    - If `i` is equal to `j`, then we traverse every element of the submatrix at `i,j`. Every element can be again represented as `row, column`. So, we subtract every `row, column` element by `column, row` element of the same submatrix.
    - If `i` is not equal to `j`, we again traverse every element of the submatrix at `i,j`. Every element can be again represented as `row, column`. However, we subtract every `row, column` element by `column, row` element of the submatrix at `j, i`. Similarly, we perform the subtractions for the submatrix at `j,i`, subtracting `column, row` element of the submatrix at `i,j`.
    - `BLOCKS_READ = 2N - sqrt(N)`, `BLOCKS_WRITTEN = N`, `BLOCKS_ACCESSED = 3N - sqrt(N), where N = Number of blocks of a given matrix`

### Assumptions

- When exporting a renamed matrix, the original matrix is not removed from the data folder.
- The input must be a valid `N*N` matrix for every matrix-related command. Inputting a table for such commands is prohibited.
- Blocks read, written, accessed are accounted when and where we use the disk storage.

### Learnings

- `Handling large data`: Dealing with large matrices (up to n x n, where n <= 10^4) has taught you how to optimize memory usage and efficiently perform operations on substantial datasets.
- `In-place operations`: We have learnt the concepts of performing operations in-place, which can significantly reduce disk I/O operations and improve efficiency. This is particularly important when working with large datasets.
- `Error-handling and testing`: Handling syntactic and semantic errors, as well as checking whether matrices are loaded or not, is essential for ensuring the reliability and stability of your system. We generated various test cases to check the correctability and integrity of the code.
- `Assumptions and decision-making`: We have made assumptions and various design decisions during the project. This experience has shown us how important it is to document and integrate these assumptions into the design.
- `Team Collaboration`: Working as a team on this project has taught you how to collaborate effectively with others, coordinate tasks, and merge code changes into a shared repository.

### Contributions

- Dhruv Hirpara - LOAD command, PRINT command, EXPORT command
- Soveet Nayak -  COMPUTE command, RENAME command, Page Design
- Tanmay Goyal - TRANSPOSE command, CHECKSYMMETRY command, Report