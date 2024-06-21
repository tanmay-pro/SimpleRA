#include "global.h"
/**
 * @brief
 * SYNTAX: TRANSPOSE A
 */
bool syntacticParseTRANSPOSE()
{
    logger.log("syntacticParseTRANSPOSE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = TRANSPOSE;
    parsedQuery.transposeRelationName = tokenizedQuery[1];

    return true;
}

bool semanticParseTRANSPOSE()
{
    logger.log("semanticParseTRANSPOSE");
    if(!tableCatalogue.isTable(parsedQuery.transposeRelationName))
    {
        cout << "SEMANTIC ERROR: Relation/Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void transposeMatrix(vector<vector<int>> &rows)
{
    //transpose non-square matrix
    int n = rows.size();
    int m = rows[0].size();

    vector<vector<int>> transposedMatrix(m, vector<int>(n, 0));

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<m; j++)
        {
            transposedMatrix[j][i] = rows[i][j];
        }
    }

    rows = transposedMatrix;
}

void executeTRANSPOSE()
{
    logger.log("executeTRANSPOSE");
    Matrix *matrix = (Matrix*)tableCatalogue.getTable(parsedQuery.transposeRelationName);

    int n = sqrt(matrix->blockCount);

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<=i; j++)
        {
            int columnCount = matrix->maxSubMatrixSize;
            if(j == n-1 && matrix->columnCount%matrix->maxSubMatrixSize!=0)
                columnCount = matrix->columnCount%matrix->maxSubMatrixSize;

            int rowCount = matrix->maxSubMatrixSize;
            if(i==n-1 && matrix->rowCount%matrix->maxSubMatrixSize!=0) 
                rowCount = matrix->rowCount%matrix->maxSubMatrixSize;

            if(i == j)
            {
                Cursor c(parsedQuery.transposeRelationName, i*n+j, true);
                vector<vector<int>> rows = c.getMatrixPage();

                transposeMatrix(rows);
                Page p(parsedQuery.transposeRelationName, i*n+j, rows, rows.size());
                bufferManager.updatePage(p);
                p.writePage(rows);
            }
            else
            {
                Cursor c1(parsedQuery.transposeRelationName, i*n+j, true);
                Cursor c2(parsedQuery.transposeRelationName, j*n+i, true);
                vector<vector<int>> rows1 = c1.getMatrixPage();
                vector<vector<int>> rows2 = c2.getMatrixPage();

                transposeMatrix(rows1);
                transposeMatrix(rows2);

                //write rows1 to j*n+i and rows2 to i*n+j
                Page p1(parsedQuery.transposeRelationName, j*n+i, rows1, rows1.size());
                bufferManager.updatePage(p1);
                p1.writePage(rows1);

                Page p2(parsedQuery.transposeRelationName, i*n+j, rows2, rows2.size());
                bufferManager.updatePage(p2);
                p2.writePage(rows2);
            }
        }
    }

    //Blocks Information
    // cout << "Number of blocks read: " << BLOCKS_READ << endl;
    // cout << "Number of blocks written: " << BLOCKS_WRITTEN << endl;
    // cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << endl;

    return;
}