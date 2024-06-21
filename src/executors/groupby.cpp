#include "global.h"
/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> HAVING <aggregate(attribute)> <bin_op> <attribute_value> RETURN <aggregate_func(attribute)>
 *
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    if (tokenizedQuery.size() != 13 || tokenizedQuery[2] != "GROUP" || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "HAVING" || tokenizedQuery[11] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupByResultRelationName = tokenizedQuery[0];
    parsedQuery.groupByRelationName = tokenizedQuery[6];
    parsedQuery.groupByColumnName = tokenizedQuery[4];
    if (tokenizedQuery[8].find("(") == string::npos || tokenizedQuery[8].find(")") == string::npos ||
        tokenizedQuery[12].find("(") == string::npos || tokenizedQuery[12].find(")") == string::npos ||
        tokenizedQuery[8].find("(") > tokenizedQuery[8].find(")") || tokenizedQuery[12].find("(") > tokenizedQuery[12].find(")"))
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    string checkOp = tokenizedQuery[8].substr(0, tokenizedQuery[8].find("("));
    if (checkOp == "SUM")
        parsedQuery.groupByCheckColumnOperator = SUM;
    else if (checkOp == "AVG")
        parsedQuery.groupByCheckColumnOperator = AVG;
    else if (checkOp == "MIN")
        parsedQuery.groupByCheckColumnOperator = MIN;
    else if (checkOp == "MAX")
        parsedQuery.groupByCheckColumnOperator = MAX;
    else if (checkOp == "COUNT")
        parsedQuery.groupByCheckColumnOperator = COUNT;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.groupByCheckColumn = tokenizedQuery[8].substr(tokenizedQuery[8].find("(") + 1, tokenizedQuery[8].find(")") - tokenizedQuery[8].find("(") - 1);

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.groupByBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.groupByBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.groupByBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.groupByBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.groupByBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.groupByBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.groupByValueToCheck = stoi(tokenizedQuery[10]);

    string returnOp = tokenizedQuery[12].substr(0, tokenizedQuery[12].find("("));
    if (returnOp == "SUM")
        parsedQuery.groupByReturnColumnOperator = SUM;
    else if (returnOp == "AVG")
        parsedQuery.groupByReturnColumnOperator = AVG;
    else if (returnOp == "MIN")
        parsedQuery.groupByReturnColumnOperator = MIN;
    else if (returnOp == "MAX")
        parsedQuery.groupByReturnColumnOperator = MAX;
    else if (returnOp == "COUNT")
        parsedQuery.groupByReturnColumnOperator = COUNT;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.groupByReturnColumn = tokenizedQuery[12].substr(tokenizedQuery[12].find("(") + 1, tokenizedQuery[12].find(")") - tokenizedQuery[12].find("(") - 1);

    if (parsedQuery.groupByCheckColumn != parsedQuery.groupByReturnColumn)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    return true;
}

bool semanticParseGROUPBY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupByResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }
    if (!tableCatalogue.isTable(parsedQuery.groupByRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByColumnName, parsedQuery.groupByRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByCheckColumn, parsedQuery.groupByRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByReturnColumn, parsedQuery.groupByRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist" << endl;
        return false;
    }

    return true;
}

static bool cmp(const vector<int> &a, const vector<int> &b, vector<int> &sortColumnIndices, vector<SortingStrategy> &sortingStrategies)
{
    for (int i = 0; i < sortColumnIndices.size(); i++)
    {
        // index of the column in the row
        if (a[sortColumnIndices[i]] < b[sortColumnIndices[i]])
        {
            if (sortingStrategies[i] == ASC)
                return true;
            else
                return false;
        }
        else if (a[sortColumnIndices[i]] > b[sortColumnIndices[i]])
        {
            if (sortingStrategies[i] == ASC)
                return false;
            else
                return true;
        }
    }
    return false;
}


Table *groupBySortTable(Table table, vector<int> sortColumnIndices, vector<SortingStrategy> sortingStrategy)
{
    logger.log("groupBySortTable");

    Cursor cursor = table.getCursor();

    uint numRuns = ceil((double)table.blockCount / (double)BLOCK_COUNT);

    int i = 1;
    Table *sortPhase = new Table(table.tableName + "_std_sortPhase", table.columns);
    uint currPageIndex = 0;
    while (i <= numRuns)
    {
        vector<vector<int>> sortRows;
        for (uint j = currPageIndex; j < min(currPageIndex + BLOCK_COUNT, table.blockCount); j++)
        {

            for (int k = 0; k < table.maxRowsPerBlock; k++)
            {
                vector<int> row = cursor.getNext();
                if (row.empty())
                    break;
                sortRows.push_back(row);
            }
        }

        sort(sortRows.begin(), sortRows.end(), [&](const vector<int> &a, const vector<int> &b)
             { return cmp(a, b, sortColumnIndices, sortingStrategy); });
        for (uint j = 0; j < sortRows.size(); j++)
        {
            sortPhase->writeRow(sortRows[j]);
        }

        i++;
        currPageIndex = min(currPageIndex + BLOCK_COUNT, table.blockCount);
    }

    if (sortPhase->blockify())
    {
        tableCatalogue.insertTable(sortPhase);
    }
    else
    {
        cout << "Empty Table" << endl;
        sortPhase->unload();
        delete sortPhase;
    }

    i = 1;
    int numPasses = ceil(log(numRuns) / log(BLOCK_COUNT - 1));
    uint a = numRuns;
    int s = BLOCK_COUNT;

    if (numPasses == 0)
    {
        return sortPhase;
    }

    Table *mergePhase = new Table(table.tableName + "_std_mergePhase_0", table.columns);

    while (i <= numPasses)
    {
        int n = 1;

        int q = ceil((double)a / (double)(BLOCK_COUNT - 1));
        int curr = 0;

        while (n <= q)
        {
            vector<Cursor> curs;
            vector<vector<int>> currRows;
            vector<int> subfilenum;
            for (int j = curr; j < min(curr + BLOCK_COUNT - 1, a); j++)
            {
                curs.push_back(sortPhase->getCursor());
                curs[j - curr].nextPage(j * s);

                vector<int> temp_row = curs[j - curr].getNext();
                currRows.push_back(temp_row);
                subfilenum.push_back(j);
            }

            while (curs.size())
            {
                auto minVal = min_element(currRows.begin(), currRows.end(), [&](const vector<int> &a, const vector<int> &b)
                                          { return cmp(a, b, sortColumnIndices, sortingStrategy); });
                int j = minVal - currRows.begin();
                mergePhase->writeRow(*minVal);

                currRows[j] = curs[j].getNext();
                if (curs[j].pageIndex == (subfilenum[j] + 1) * s || currRows[j].empty())
                {
                    currRows.erase(currRows.begin() + j);
                    curs.erase(curs.begin() + j);
                    subfilenum.erase(subfilenum.begin() + j);
                }
            }

            curr = min(curr + BLOCK_COUNT - 1, a);
            n++;
        }

        if (mergePhase->blockify())
        {
            tableCatalogue.insertTable(mergePhase);
        }
        else
        {
            cout << "Empty Table" << endl;
            mergePhase->unload();
        }

        tableCatalogue.deleteTable(sortPhase->tableName);
        sortPhase = mergePhase;
        if (i == numPasses)
        {
            return sortPhase;
        }
        mergePhase = new Table(table.tableName + "_std_mergePhase" + "_" + to_string(i), table.columns);
        a = q;
        s = s * (BLOCK_COUNT - 1);
        i++;
    }

    return sortPhase;
}

void executeGROUPBY()
{
    logger.log("executeGROUPBY");

    // Sort the table
    Table *table = tableCatalogue.getTable(parsedQuery.groupByRelationName);
    vector<int> sortColumnIndices;
    sortColumnIndices.push_back(table->getColumnIndex(parsedQuery.groupByColumnName));
    vector<SortingStrategy> sortingStrategies;
    sortingStrategies.push_back(ASC);

    Table *sorted = groupBySortTable(*table, sortColumnIndices, sortingStrategies);

    // Group the table according the groupByColumnName and return the result in a new table where each row is a group that satisfies the HAVING clause
    vector<string> columns;
    columns.push_back(parsedQuery.groupByColumnName);
    switch (parsedQuery.groupByReturnColumnOperator)
    {
        case SUM:
            columns.push_back("SUM" + parsedQuery.groupByReturnColumn );
            break;
        case AVG:
            columns.push_back("AVG" + parsedQuery.groupByReturnColumn );
            break;
        case MIN:
            columns.push_back("MIN" + parsedQuery.groupByReturnColumn );
            break;
        case MAX:
            columns.push_back("MAX" + parsedQuery.groupByReturnColumn );
            break;
        case COUNT:
            columns.push_back("COUNT" + parsedQuery.groupByReturnColumn );
            break;
        default:
            break;
    }

    Table *grouped = new Table(parsedQuery.groupByResultRelationName, columns);
    Cursor cursor = sorted->getCursor();
    vector<int> currRow = cursor.getNext();
    int havingColumnIndex = sorted->getColumnIndex(parsedQuery.groupByCheckColumn);
    int returnColumnIndex = sorted->getColumnIndex(parsedQuery.groupByReturnColumn);
    int groupByColumnIndex = sorted->getColumnIndex(parsedQuery.groupByColumnName);
    int groupByColumnValue = currRow[groupByColumnIndex];
    int aggregateHaving = 0;
    int aggregateReturn = 0;
    int count = 0;

    if (parsedQuery.groupByCheckColumnOperator == MIN)
    {
        aggregateHaving = INT_MAX;
    }
    else if (parsedQuery.groupByCheckColumnOperator == MAX)
    {
        aggregateHaving = INT_MIN;
    }

    if (parsedQuery.groupByReturnColumnOperator == MIN)
    {
        aggregateReturn = INT_MAX;
    }
    else if (parsedQuery.groupByReturnColumnOperator == MAX)
    {
        aggregateReturn = INT_MIN;
    }

    while (!currRow.empty())
    {
        if(groupByColumnValue != currRow[groupByColumnIndex]){
            if(parsedQuery.groupByCheckColumnOperator == AVG){
                aggregateHaving /= count;
            }
            if(parsedQuery.groupByReturnColumnOperator == AVG){
                aggregateReturn /= count;
            }

            vector<int> newRow;
            newRow.push_back(groupByColumnValue);
            newRow.push_back(aggregateReturn);

            switch (parsedQuery.groupByBinaryOperator)
            {
            case LESS_THAN:
                if (aggregateHaving < parsedQuery.groupByValueToCheck)
                {
                    grouped->writeRow(newRow);
                }
                break;
            case GREATER_THAN:
                if (aggregateHaving > parsedQuery.groupByValueToCheck)
                {
                    grouped->writeRow(newRow);
                }
                break;
            case GEQ:
                if (aggregateHaving >= parsedQuery.groupByValueToCheck)
                {
                    grouped->writeRow(newRow);
                }
                break;
            case LEQ:
                if (aggregateHaving <= parsedQuery.groupByValueToCheck)
                {
                    grouped->writeRow(newRow);
                }
                break;
            case EQUAL:
                if (aggregateHaving == parsedQuery.groupByValueToCheck)
                {
                    grouped->writeRow(newRow);
                }
                break;
            case NOT_EQUAL:
                if (aggregateHaving != parsedQuery.groupByValueToCheck)
                {
                    grouped->writeRow(newRow);
                }
                break;
            default:
                break;
            }

            groupByColumnValue = currRow[groupByColumnIndex];
            aggregateHaving = 0;
            aggregateReturn = 0;
            count = 0;

            if (parsedQuery.groupByCheckColumnOperator == MIN)
            {
                aggregateHaving = INT_MAX;
            }
            else if (parsedQuery.groupByCheckColumnOperator == MAX)
            {
                aggregateHaving = INT_MIN;
            }

            if (parsedQuery.groupByReturnColumnOperator == MIN)
            {
                aggregateReturn = INT_MAX;
            }
            else if (parsedQuery.groupByReturnColumnOperator == MAX)
            {
                aggregateReturn = INT_MIN;
            }
        }

        switch (parsedQuery.groupByCheckColumnOperator)
        {
        case SUM:
            aggregateHaving += currRow[havingColumnIndex];
            break;
        case AVG:
            aggregateHaving += currRow[havingColumnIndex];
            break;
        case MIN:
            aggregateHaving = min(aggregateHaving, currRow[havingColumnIndex]);
            break;
        case MAX:
            aggregateHaving = max(aggregateHaving, currRow[havingColumnIndex]);
            break;
        case COUNT:
            aggregateHaving++;
            break;
        default:
            break;
        }

        switch (parsedQuery.groupByReturnColumnOperator)
        {
        case SUM:
            aggregateReturn += currRow[returnColumnIndex];
            break;
        case AVG:
            aggregateReturn += currRow[returnColumnIndex];
            break;
        case MIN:
            aggregateReturn = min(aggregateReturn, currRow[returnColumnIndex]);
            break;
        case MAX:
            aggregateReturn = max(aggregateReturn, currRow[returnColumnIndex]);
            break;
        case COUNT:
            aggregateReturn++;
            break;
        default:
            break;
        }

        count++;

        currRow = cursor.getNext();

    }


    if(parsedQuery.groupByCheckColumnOperator == AVG){
        aggregateHaving /= count;
    }
    if(parsedQuery.groupByReturnColumnOperator == AVG){
        aggregateReturn /= count;
    }

    vector<int> newRow;
    newRow.push_back(groupByColumnValue);
    newRow.push_back(aggregateReturn);

    switch (parsedQuery.groupByBinaryOperator)
    {
    case LESS_THAN:
        if (aggregateHaving < parsedQuery.groupByValueToCheck)
        {
            grouped->writeRow(newRow);
        }
        break;
    case GREATER_THAN:
        if (aggregateHaving > parsedQuery.groupByValueToCheck)
        {
            grouped->writeRow(newRow);
        }
        break;
    case GEQ:
        if (aggregateHaving >= parsedQuery.groupByValueToCheck)
        {
            grouped->writeRow(newRow);
        }
        break;
    case LEQ:
        if (aggregateHaving <= parsedQuery.groupByValueToCheck)
        {
            grouped->writeRow(newRow);
        }
        break;
    case EQUAL:
        if (aggregateHaving == parsedQuery.groupByValueToCheck)
        {
            grouped->writeRow(newRow);
        }
        break;
    case NOT_EQUAL:
        if (aggregateHaving != parsedQuery.groupByValueToCheck)
        {
            grouped->writeRow(newRow);
        }
        break;
    default:
        break;
    }

    if (grouped->blockify())
    {
        tableCatalogue.insertTable(grouped);
    }
    else
    {
        cout << "Empty Table" << endl;
        grouped->unload();
        delete grouped;
    }

    tableCatalogue.deleteTable(sorted->tableName);

}