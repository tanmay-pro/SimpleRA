#include "global.h"
/**
 * @brief 
 * SYNTAX: RENAME column_name TO column_name FROM relation_name
 */
bool syntacticParseRENAME()
{
    logger.log("syntacticParseRENAME");
    if (tokenizedQuery.size() == 4 && tokenizedQuery[1] == "MATRIX")
    {
        parsedQuery.isMatrix = true;
        parsedQuery.queryType = RENAME;
        parsedQuery.renameRelationName = tokenizedQuery[2];
        parsedQuery.renameRelationToName = tokenizedQuery[3];
        return true;
    }
    if (tokenizedQuery.size() != 6 || tokenizedQuery[2] != "TO" || tokenizedQuery[4] != "FROM" || tokenizedQuery[1] == "MATRIX")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = RENAME;
    parsedQuery.renameFromColumnName = tokenizedQuery[1];
    parsedQuery.renameToColumnName = tokenizedQuery[3];
    parsedQuery.renameRelationName = tokenizedQuery[5];
    return true;
}

bool semanticParseRENAME()
{
    logger.log("semanticParseRENAME");
    
    if (parsedQuery.isMatrix)
    {
        if (!tableCatalogue.isTable(parsedQuery.renameRelationName))
        {
            cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
            return false;
        }
        if (tableCatalogue.isTable(parsedQuery.renameRelationToName))
        {
            cout << "SEMANTIC ERROR: Relation with name already exists" << endl;
            return false;
        }

        return true;
    }
    
    if (!tableCatalogue.isTable(parsedQuery.renameRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.renameFromColumnName, parsedQuery.renameRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    if (tableCatalogue.isColumnFromTable(parsedQuery.renameToColumnName, parsedQuery.renameRelationName))
    {
        cout << "SEMANTIC ERROR: Column with name already exists" << endl;
        return false;
    }
    return true;
}

void executeRENAME()
{
    logger.log("executeRENAME");
    if(parsedQuery.isMatrix)
    {
        Matrix* table = (Matrix*)tableCatalogue.getTable(parsedQuery.renameRelationName);
        
        //edit all pages in data and the file name in /data/temp
        string oldSource = "../data/temp/" + parsedQuery.renameRelationName + "_Page";
        string newSource = "../data/temp/" + parsedQuery.renameRelationToName + "_Page";
    
        for(int i=0; i<table->blockCount; i++)
        {
            string oldName = oldSource + to_string(i);
            string newName = newSource + to_string(i);
            rename(oldName.c_str(), newName.c_str());
        }
    
        table->tableName = parsedQuery.renameRelationToName;
        table->sourceFileName = newSource;
    
        //update tableCatalogue
        tableCatalogue.renameTable(parsedQuery.renameRelationName, parsedQuery.renameRelationToName);
    }
    else
    {
        Table* table = tableCatalogue.getTable(parsedQuery.renameRelationName);
        table->renameColumn(parsedQuery.renameFromColumnName, parsedQuery.renameToColumnName);
    }
    return;
}