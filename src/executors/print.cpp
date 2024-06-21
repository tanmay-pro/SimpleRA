#include "global.h"
/**
 * @brief
 * SYNTAX: PRINT relation_name
 */
bool syntacticParsePRINT()
{
    logger.log("syntacticParsePRINT");
    if (tokenizedQuery.size() != 2 && !(tokenizedQuery.size() == 3 && tokenizedQuery[1] == "MATRIX"))
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    
    parsedQuery.queryType = PRINT;

    if(tokenizedQuery.size() == 3){
        parsedQuery.isMatrix = true;
        parsedQuery.printRelationName = tokenizedQuery[2];
    }
    else
        parsedQuery.printRelationName = tokenizedQuery[1];

    return true;
}

bool semanticParsePRINT()
{
    logger.log("semanticParsePRINT");
    if (tableCatalogue.isTable(parsedQuery.printRelationName))
        return true;

    cout << "SEMANTIC ERROR: Relation/Matrix doesn't exist" << endl;
    return false;
}

void executePRINT()
{
    logger.log("executePRINT");
    if(parsedQuery.isMatrix)
    {
        Matrix *matrix = (Matrix*)tableCatalogue.getTable(parsedQuery.printRelationName);
        matrix->print();
    }
    else
    {
        Table *table = tableCatalogue.getTable(parsedQuery.printRelationName);
        table->print();
    }

    //Blocks Information
    // cout << "Number of blocks read: " << BLOCKS_READ << endl;
    // cout << "Number of blocks written: " << BLOCKS_WRITTEN << endl;
    // cout << "Number of blocks accessed: " << BLOCKS_READ + BLOCKS_WRITTEN << endl;

    return;
}