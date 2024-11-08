# SimpleRA

## About Project

This simplified relational database management system supports basic database operations on integer-only tables and loads, stores, and transposes operations efficiently on arbitrarily large matrices. Supported indexing schemes are linear hash and B+ tree. Supported sorting schemes are two-phase on-disk merge sort, and in-memory buffers are managed by delaying writes. There is no support for transaction or thread safety.

## Compilation Instructions

We use ```make``` to compile all the files and create the server executable. ```make``` is used primarily in Linux systems, so those of you who want to use Windows will probably have to look up alternatives (I hear there are ways to install ```make``` on Windows). To compile

```cd``` into the SimpleRA directory
```
cd SimpleRA
```
```cd``` into the soure directory (called ```src```)
```
cd src
```
To compile
```
make clean
make
```

## To run

Post compilation, an executable names ```server``` will be created in the ```src``` directory
```
./server
```
## Overview

### Important Features

- Relational Algebra Operators
- Integers Only
- No identifiers should have spaces in them
- The RDBMS supports storage of both matrices and tables
---

### Commands

There are 3 kinds of commands in this database:

- Assignment statements
- Non-assignment statements
- Matrix related commands
---

### Internals

- Buffer Manager, Cursors, Tables, Matrices, Executors
---

#### Command Execution Flow

![image](https://github.com/user-attachments/assets/89e0acce-2f60-4903-86c0-939a30a38d3a)

---

#### Syntactic Parser

- Splits the query into query units

---

#### Semantic Parser

- Makes sure the given query makes semantic sense
---

#### Executors

Every command(COMMAND) has a file in the executors' directory

```
syntacticParseCOMMAND
semanticParseCOMMAND
executeCOMMAND
```

---

#### Buffer Manager

- Load splits and stores the table into blocks.
- It follows a FIFO paradigm.
---

#### Table Catalogue

- The table catalogue is an index of tables & matrices currently loaded into the system
---

#### Cursors

- A cursor is an object that acts like a pointer in a table.
- To read from a table, we need to declare a cursor

![image](https://github.com/user-attachments/assets/8bba0815-feb5-434d-941a-e96599d4c565)


- Example Command: `R <- SELECT a == 1 FROM A` with debugger
---

#### Logger

Every function call is logged in a file named "log"

---

### Non-Assignment Statements

- Non-assignment statements do not create a new table (except load, which loads an existing table) in the process
- LOAD, LIST, PRINT, RENAME, EXPORT, CLEAR, QUIT, INDEX
  
---

#### LOAD

Syntax
```
LOAD <table_name>
```
- To successfully load a table, there should be a CSV file named <table_name>.csv consisting of comma-separated integers in the data folder
- None of the columns in the data file should have the same name
- Every cell in the table should have a value
---

#### LIST TABLES

Syntax
```
LIST TABLES
```
- This command lists all tables that have been loaded or created using assignment statements
---

#### PRINT

Syntax
```
PRINT <table_name>
```

- Displays the first PRINT_COUNT (global variable) rows of the table
- A smaller number of rows are printed if the table has only a few rows (< PRINT_COUNT)
---

#### RENAME

Syntax
```
RENAME <toColumnName> TO <fromColumnName> FROM <table_name>
```

- Naturally <table_name> should be a loaded table in the system and <fromColumnName> should be an exsiting column in the table
- <toColumnName> should not be another column in the table
---

#### EXPORT

Syntax
```
EXPORT <table_name>
```

- All changes made and new tables created exist only within the system and are deleted once execution ends (temp file)
- To keep changes made (RENAME and new tables), the table needs to be exported (data)
---

#### CLEAR

Syntax
```
CLEAR <table_name>
```
- Removes table from the system
- The table has to have previously existed in the system to remove it
- If we want to keep any of the changes we have made to an old table or want to keep the new table, we will have to export.
---

#### QUIT

Syntax
```
QUIT
```

- Clear all tables present in the system (**_WITHOUT EXPORTING THEM_**)  (temp file - empty)
---

#### INDEX

Syntax:
```
INDEX ON <columnName> FROM <table_name> USING <indexing_strategy>
```

Where <indexing_strategy> could be 
- `BTREE` - BTree indexing on a column
- `HASH` - Index via a hashmap
- `NOTHING` - Removes index if present 
---

### Assignment Statements

- All assignment statements lead to the creation of a new table.
- Every statement is of the form ```<new_table_name> <- <assignment_statement>``` 
- Naturally in all cases, <new_table_name> shouldn't already exist in the system
- CROSS, PROJECTION, SELECTION, DISTINCT, JOIN, SORT

---

#### CROSS

Syntax
```
<new_table_name> <- CROSS <table_name1> <table_name2>
```

- Both the tables being crossed should exist in the system
- If there are columns with the same names in the two tables, the columns are indexed with the table name. If both tables being crossed are the same, table names are indexed with '1' and '2'

Example Command: `cross_AA <- CROSS A A`

`A(A, B) x A(A, B) -> cross_AA(A1_A, A1_B, A2_A, A2_B)`

---

#### SELECTION

Syntax
```
<new_table_name> <- SELECT <condition> FROM <table_name>
```

Where <condition> is of either form
```
<first_column_name> <bin_op> <second_column_name>
<first_column_name> <bin_op> <int_literal>
```

Where <bin_op> can be any operator among {>, <, >=, <=, =>, =<, ==, !=}

---

- The selection command only takes one condition at a time

- Example Command: 
  - `R <- SELECT a >= 1 FROM A`
  - `S <- SELECT a > b FROM A`
---

#### PROJECTION

Syntax
```
<new_table_name> <- PROJECT <column1>(,<columnN>)* FROM <table_name>
```

- Naturally, all columns should be present in the original table
---

#### DISTINCT

Syntax
```
<new_table_name> <- DISTINCT <table_name>
```

- Naturally, the table should exist
---

#### JOIN

Syntax
```
<new_relation_name> <- JOIN <table1>, <table2> ON <column1> <bin_op> <column2>
```

Where <bin_op> means the same as it does in the SELECT operator

- Implicitly assumes <column1> is from <table1> and <column2> if from <table2>

- Example Command: `J <- JOIN A, B ON a == a`
---

#### SORT

Syntax
```
<new_table_name> <- SORT <table_name> BY <column_name> IN <sorting_order>
```

Where <sorting_order> can be `ASC` or `DESC`

- Example Command: `S <- SORT A BY b IN ASC`  
---


#### SOURCE

Syntax
```
SOURCE <query_name>
```
- Special command that takes in a file script from the data directory
- File name should end in ".ra," indicating it's a query file
- File to be present in the data folder
---

### Matrix-related statements

- Unlike tables, matrices do not have a column name
- As it is an integer-only application, the values of the matrix are confined to integers
- LOAD, PRINT, TRANSPOSE, RENAME, EXPORT, CHECKSYMMETRY, COMPUTE
---

#### LOAD

Syntax
```
LOAD MATRIX <matrix_name>
```
- To successfully load a matrix, there should be a CSV file named <matrix_name>.csv in the data folder
- Each line of the file represents a row of the matrix
- Values in the row are comma-separated
---

#### PRINT

Syntax
```
PRINT MATRIX <matrix_name>
```

- Displays the first PRINT_COUNT (global variable) rows of the matrix
- A smaller number of rows are printed if the matrix has only a few rows (< PRINT_COUNT)
---

#### TRANSPOSE

Syntax
```
TRANSPOSE MATRIX <matrix_name>
```

- The command computes the transpose of the matrix IN-PLACE (Constant Space Complexity) 
---

#### RENAME 

Syntax
```
RENAME MATRIX <matrix_currentname> TO <matrix_newname>
```

- Renames a Matrix that is present in memory from <matrix_currentname > to < matrix_newname >
---

#### EXPORT

Syntax
```
EXPORT MATRIX <matrix_name>
```

- Similar to tables, the command exports matrix data to the data folder
---

#### CHECKSYMMETRY

Syntax
```
CHECKSYMMETRY <matrix_name>
```

- Checks if the given matrix is symmetric or not
- The function returns a boolean value
- This function is also performed IN-PLACE
---

#### COMPUTE

Syntax
```
COMPUTE <matrix_name>
```

- Computes the expression: `A - A'` where `A'` represents the transpose of the matrix `A`
- The new matrix is stored as `<matrix_name>_RESULT`



