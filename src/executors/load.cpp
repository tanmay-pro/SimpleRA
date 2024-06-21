#include "global.h"
/**
 * @brief
 * SYNTAX: LOAD relation_name
 */
bool syntacticParseLOAD()
{
    logger.log("syntacticParseLOAD");
    if (tokenizedQuery.size() != 2 && !(tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX"))
    {
        cout << tokenizedQuery.size() << endl;
        cout << tokenizedQuery[1] << endl;
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD;
    
    if(tokenizedQuery.size() == 3){
        parsedQuery.loadRelationName = tokenizedQuery[2];
        parsedQuery.isMatrix = true;
    }
    else
        parsedQuery.loadRelationName = tokenizedQuery[1];
    
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");
    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    if (tableCatalogue.isTable(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Relation/Matrix already exixts" << endl;
        return false;
    }

    return true;
}

void executeLOAD()
{
    logger.log("executeLOAD");

    if(parsedQuery.isMatrix)
    {
        Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
        if(matrix->load())
        {
            tableCatalogue.insertTable(matrix);
            cout << "Loaded Matrix. Column Count: " << matrix->columnCount << " Row Count: " << matrix->rowCount << endl;
        }
    }
    else
    {
        Table *table = new Table(parsedQuery.loadRelationName);
        if (table->load())
        {
            tableCatalogue.insertTable(table);
            cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;

        }
    }
            //Blocks Information
            // cout << "Number of blocks read: " << BLOCKS_READ << endl;
            // cout << "Number of blocks written: " << BLOCKS_WRITTEN << endl;
            // cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << endl;
    
    return;
}