#include "global.h"
/**
 * @brief
 * SYNTAX: COMPUTE A
 */
bool syntacticParseCOMPUTE()
{
    logger.log("syntacticParseCOMPUTE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = COMPUTE;
    parsedQuery.computeRelationName = tokenizedQuery[1];

    return true;
}

bool semanticParseCOMPUTE()
{
    logger.log("semanticParseCOMPUTE");

    if (!tableCatalogue.isTable(parsedQuery.computeRelationName))
    {
        cout << "SEMANTIC ERROR: Relation/Matrix does not exists" << endl;
        return false;
    }
    if (tableCatalogue.isTable(parsedQuery.computeRelationName+"_RESULT"))
    {
        cout << "SEMANTIC ERROR: Resultant Matrix already exists" << endl;
        return false;
    }

    return true;
}

void executeCOMPUTE()
{
    logger.log("executeCOMPUTE");
    Matrix* matrix = (Matrix*)tableCatalogue.getTable(parsedQuery.computeRelationName);
    Matrix* resultantMatrix = new Matrix(parsedQuery.computeRelationName + "_RESULT", matrix->columns);
    resultantMatrix->columnCount = matrix->columnCount;
    resultantMatrix->rowCount = matrix->rowCount;
    resultantMatrix->blockCount = matrix->blockCount;
    resultantMatrix->maxRowsPerBlock = matrix->maxRowsPerBlock;
    tableCatalogue.insertTable(resultantMatrix);
    int n = sqrt(matrix->blockCount);

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            int columnCount = matrix->maxSubMatrixSize;
            if(j == n-1 && matrix->columnCount%matrix->maxSubMatrixSize!=0)
                columnCount = matrix->columnCount%matrix->maxSubMatrixSize;

            int rowCount = matrix->maxSubMatrixSize;
            if(i==n-1 && matrix->rowCount%matrix->maxSubMatrixSize!=0) 
                rowCount = matrix->rowCount%matrix->maxSubMatrixSize;
            vector<vector<int>> rows;
            vector<vector<int>> rows2;
            if(i==j)
            {
                Cursor c(parsedQuery.computeRelationName, i*n+j, true);
                rows = c.getMatrixPage();
                rows2 = rows;
                for(int k=0; k<rowCount; k++)
                {
                    for(int l=0; l<columnCount; l++)
                    {
                        rows[k][l] -= rows2[l][k]; 
                    }
                }

            }
            else
            {
                Cursor c1(parsedQuery.computeRelationName, i*n+j, true);
                Cursor c2(parsedQuery.computeRelationName, j*n+i, true);
                rows = c1.getMatrixPage();
                rows2 = c2.getMatrixPage();

                // check for symmetry
                for(int k=0; k<rowCount; k++)
                {
                    for(int l=0; l<columnCount; l++)
                    {
                        rows[k][l] -= rows2[l][k];
                    }
                }
            }
            //  print rows
            Page p(parsedQuery.computeRelationName + "_RESULT", i*n+j, rows, rowCount);
            p.writePage();
        }
    }
    



    //Blocks Information
    // cout << "Number of blocks read: " << BLOCKS_READ << endl;
    // cout << "Number of blocks written: " << BLOCKS_WRITTEN << endl;
    // cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << endl;

    return;
}