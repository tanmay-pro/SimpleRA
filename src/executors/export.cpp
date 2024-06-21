#include "global.h"

/**
 * @brief
 * SYNTAX: EXPORT <relation_name>
 */

bool syntacticParseEXPORT()
{
    logger.log("syntacticParseEXPORT");
    if (tokenizedQuery.size() != 2 && !(tokenizedQuery.size()==3 && tokenizedQuery[1] == "MATRIX"))
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = EXPORT;
    if(tokenizedQuery.size() == 3){
        parsedQuery.isMatrix = true;
        parsedQuery.exportRelationName = tokenizedQuery[2];
    }
    else
        parsedQuery.exportRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    //Table should exist
    if (tableCatalogue.isTable(parsedQuery.exportRelationName))
        return true;
    
    cout << "SEMANTIC ERROR: Relation/Matrix doesn't exist" << endl;

    return false;
}

void executeEXPORT()
{
    logger.log("executeEXPORT");
    if(parsedQuery.isMatrix){
        Matrix* matrix = (Matrix*)tableCatalogue.getTable(parsedQuery.exportRelationName);
        matrix->makePermanent();
        return;
    }
    Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
    table->makePermanent();
    return;
}