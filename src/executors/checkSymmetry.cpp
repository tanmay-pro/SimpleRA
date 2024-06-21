#include "global.h"
/**
 * @brief
 * SYNTAX: CHECKSYMMETRY A
 */
bool syntacticParseCHECKSYMMETRY()
{
    logger.log("syntacticParseCHECKSYMMETRY");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = CHECKSYMMETRY;
    parsedQuery.checkSymmetryRelationName = tokenizedQuery[1];

    return true;
}

bool semanticParseCHECKSYMMETRY()
{
    logger.log("semanticParseCHECKSYMMETRY");

    if (!tableCatalogue.isTable(parsedQuery.checkSymmetryRelationName))
    {
        cout << "SEMANTIC ERROR: Relation/Matrix does not exists" << endl;
        return false;
    }

    return true;
}

void executeCHECKSYMMETRY()
{
    logger.log("executeCHECKSYMMETRY");
    Matrix *matrix = (Matrix*)tableCatalogue.getTable(parsedQuery.checkSymmetryRelationName);

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

            if(i==j)
            {
                Cursor c(parsedQuery.checkSymmetryRelationName, i*n+j, true);
                vector<vector<int>> rows = c.getMatrixPage();

                for(int k=0; k<rowCount; k++)
                {
                    for(int l=0; l<=k; l++)
                    {
                        if(rows[k][l] != rows[l][k])
                        {
                            cout << "FALSE" << endl;
                            goto blockInfo;
                        }
                    }
                }
            }
            else
            {
                Cursor c1(parsedQuery.checkSymmetryRelationName, i*n+j, true);
                Cursor c2(parsedQuery.checkSymmetryRelationName, j*n+i, true);
                vector<vector<int>> rows1 = c1.getMatrixPage();
                vector<vector<int>> rows2 = c2.getMatrixPage();

                // check for symmetry
                for(int k=0; k<rowCount; k++)
                {
                    for(int l=0; l<columnCount; l++)
                    {
                        if(rows1[k][l] != rows2[l][k])
                        {
                            cout << "FALSE" << endl;
                            goto blockInfo;
                        }
                    }
                }
            }
        }
    }
    
    cout << "TRUE" << endl;

blockInfo:
    //Blocks Information
    // cout << "Number of blocks read: " << BLOCKS_READ << endl;
    // cout << "Number of blocks written: " << BLOCKS_WRITTEN << endl;
    // cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << endl;

    return;
}