# Project Phase 2 Report

### External Sort Algorithm

The external sort algorithm is a technique used for sorting large datasets that cannot fit entirely in memory. It divides the sorting process into two phases: the Sorting Phase and the Merging Phase. Here's a brief explanation of how this algorithm works:

**Sorting Phase**:

1. **Initialization**:

   - `i`: Index to track runs (portions) of the file.
   - `j`: Size of the file in blocks.
   - `k`: Size of the buffer in blocks (available main memory).
   - `m`: Number of subfiles, each of which fits in the buffer.

2. **Sorting**:
   - Read the next `k` blocks of the file into the buffer. If there are fewer than `k` blocks remaining, read in the remaining blocks.
   - Sort the records in the buffer using an internal sorting algorithm.
   - Write the sorted records back to disk as temporary sorted subfiles (or runs).
   - Increment `i` to move to the next run.

**Merging Phase**:

1. **Initialization**:
   - `i`: Reset to 1.
   - `p`: Number of passes for the merging phase (calculated based on buffer size and the number of initial runs).
   - `j`: Number of subfiles.
2. **Merging Passes**:

   - During each merge pass, a certain number of sorted subfiles are merged into larger sorted subfiles.
   - `dM` (degree of merging): The number of sorted subfiles that can be merged in each merge step. It is determined by the number of available buffer blocks and the number of initial runs.
   - In each merge step, one buffer block is needed for each disk block from the subfiles being merged, and an additional buffer block is used for the merge result.
   - The number of merge passes is determined by the formula ⎡(log_dM(nR))⎤.

3. **Merging Subfiles**:

   - The algorithm proceeds through each pass, merging subfiles into larger subfiles.
   - The number of subfiles to write in each pass, `q`, is calculated based on `j` and `dM`.

4. **Merging Operation**:
   - In each merge step, you read the next `k-1` subfiles or the remaining subfiles from the previous pass one block at a time.
   - Merge and write the merged results as new subfiles, one block at a time.
   - This process continues until you've completed all the passes.

The final result of the external sort algorithm is one fully sorted file, which has been achieved through a series of merge passes, merging multiple subfiles at each step.

The algorithm is efficient for large datasets because it limits the number of blocks in memory at any given time and efficiently sorts and merges the data. It's a key technique for managing large-scale data processing tasks.

### Commands

- <Command name>: SORT

  - Syntax : SORT <relation_name> BY <column_name1, column_name2, column_name3, .. column_namek> IN <SortingStrategy1, ... SortingStrategyk>
  - We create subfiles during the sorting and merging phases.
  - Then we delete the intermediate temporarily stored files/tables and overwrite the original table.

- <Command name>: ORDER BY

  - Syntax : R <- ORDER BY <column_name> <ASC|DESC> ON <relation_name>
  - We order by the given <column_name> to obtain a new sorted table.
  - To order, we basically use external sort on one column (i.e. the given column) and store it into another relation.

- <Command name>: JOIN

  - Syntax : R <- JOIN <relation_name1>, <relation_name2> ON <column_name1> bin_op <column_name2>
  - First, the given tables are sorted using the external sorting algorithm to give us two sorted tables R and S.
  - Let i,j represent a row in R and S repsectively. If `R[i][column_name1] <bin_op> S[j][column_name2]`, the particular row combination is added to the resultant table.
  - If <bin_op> is `==`, then we sort both the files in ascending order and both files are scanned concurrently in order of the join attributes, matching the records with the same value of the join attribute are added to final join file (Normal Sort-merge join algo)
  - If <bin_op> is `< || <= || > || >=`, then we follow the following algorithm,
   - Assume that the <bin_op> is `<=` and to join T1 and T2 on the join attribute A1 and A2.
   - T1 is sorted **ASC** on A1 and T2 is sorted **DESC** on A2.
   - We start with both the cursor at the first record of T1 and T2 respectively.
   - If (T1.A1 <= T2.A2) == FALSE, then the resultants of the join will be empty as large as smallest value of A1 is greater than the largest value of A2.
   - If (T1.A1 <= T2.A2) == TRUE, WE follow the following
      - Store the marker of T1
      - Advance the cursor of T1 until (T1.A1 <= T2.A2) == TRUE while keep adding the records of join of T1.A1 and T2.A2 to the resultant table.
      - Advance the cursor of T2 once.
      - Restore the cursor of T1 to the stored marker.
      - Check if (T1.A1 <= T2.A2) == TRUE, if yes, repeat the above steps, else return the resultant table.
  - Lastly, the sorted individual tables are removed from the buffer and memory.

- <Command name>: GROUP BY

  - Syntax: R <- GROUP BY <grouping_attribute> FROM <table_name> HAVING <aggregate(attribute)> <bin_op> <attribute_value> RETURN <aggregate_func(attribute)>
  - First, we sort the original table by the grouping attribute using the external sort algorithm.
  - We then check <aggregate(attribute)> <bin_op> <attribute_value> for all unique grouping attributes.
  - If the check condition is satisfied, a new row is inserted in the new table with the values: <grouping_attribute>, <aggregate_func(attribute)>.

## Assumptions

- Size of a record is less than or equal to block size.
- Values are always integer. Sum of values in a GROUP BY command does not overflows.
- Test cases provided will give atleast one row.
- All though the code can be extended to handle multiple columns, but as per requirements, GROUP BY, ORDER BY use only one column and JOIN uses one column from each of the two relation.
- As per the grammar in the requirements, we have assumed that the check column and the return column will be the same in the GROUP BY command.

## Learnings

- `External sort algorithm` and its implementation.
- `In-place operations`: We have learnt the concepts of performing operations in-place, which can significantly reduce disk I/O operations and improve efficiency.

## Contribution

- Dhurv Hirpara - SORT command, Testing & Error Handling, Report
- Soveet Nayak - GROUP BY command, ORDER BY command, Report
- Tanmay Goyal - JOIN command, Testing & Error Handling, Report