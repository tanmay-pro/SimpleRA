#include "global.h"
/**
 * @brief
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    string relation = tokenizedQuery[3];
    // remove the comma from the relation name if it is present
    if (relation.back() == ',')
        relation.pop_back();
    parsedQuery.joinFirstRelationName = relation;
    relation = tokenizedQuery[4];
    // remove the comma from the relation name if it is present
    if (relation.back() == ',')
        relation.pop_back();
    parsedQuery.joinSecondRelationName = relation;
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    parsedQuery.joinSecondColumnName = tokenizedQuery[8];

    string binaryOperator = tokenizedQuery[7];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
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

Table *orderbyBySortTable(Table table, vector<int> sortColumnIndices, vector<SortingStrategy> sortingStrategy)
{
    logger.log("orderBySortTable");

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

void equiJoin(Table* firstTable, Table* secondTable , Table* joinTable , int leftTableColumnIndices , int rightTableColumnIndices)
{
    Cursor leftCursor = firstTable->getCursor();
    Cursor rightCursor = secondTable->getCursor();

    vector<int> leftRow = leftCursor.getNext();
    vector<int> rightRow = rightCursor.getNext();
    bool breakForever = false;

    while(true) // forever loop
    {
        while(leftRow[leftTableColumnIndices] != rightRow[rightTableColumnIndices]){
            if(leftRow[leftTableColumnIndices] < rightRow[rightTableColumnIndices]){
                leftRow = leftCursor.getNext();
                if(leftRow.empty()){
                    breakForever = true;
                    break;
                }
            }
            else{
                rightRow = rightCursor.getNext();
                if(rightRow.empty()){
                    breakForever = true;
                    break;
                }
            }
        }

        if(breakForever){
            break;
        }

        Cursor markLeft = leftCursor;
        vector<int> markLeftRow = leftRow;

        while (true)
        {
            while(leftRow[leftTableColumnIndices] == rightRow[rightTableColumnIndices]){
                vector<int> row;
                row.insert(row.end(), leftRow.begin(), leftRow.end());
                row.insert(row.end(), rightRow.begin(), rightRow.end());
                joinTable->writeRow(row);
                leftRow = leftCursor.getNext();
                if(leftRow.empty()){
                    break;
                }
            }

            // vector<int> temp = rightRow;
            rightRow = rightCursor.getNext();
            if(rightRow.empty()){
                breakForever = true;
                break;
            }

            if(markLeftRow[leftTableColumnIndices] == rightRow[rightTableColumnIndices]){
                leftCursor = markLeft;
                leftRow = markLeftRow;
            }
            else
            {
                if(leftRow.empty())
                {
                    breakForever = true;
                    break;
                }
                else{
                    break;
                }
            }

        }
        if(breakForever){
            break;
        }




    }
    return;
}

void unequiJoin(Table* firstTable, Table* secondTable , Table* joinTable , int leftTableColumnIndices , int rightTableColumnIndices , BinaryOperator oper)
{
    // lambda function to compare two rows according to the binary operator
    auto compare = [&](vector<int> &a, vector<int> &b) {
        switch (oper)
        {
        case LESS_THAN:
            return a[leftTableColumnIndices] < b[rightTableColumnIndices];
        case GREATER_THAN:
            return a[leftTableColumnIndices] > b[rightTableColumnIndices];
        case LEQ:
            return a[leftTableColumnIndices] <= b[rightTableColumnIndices];
        case GEQ:
            return a[leftTableColumnIndices] >= b[rightTableColumnIndices];
        default:
            return false;
        }
    };

    Cursor leftCursor = firstTable->getCursor();
    Cursor rightCursor = secondTable->getCursor();

    vector<int> leftRow = leftCursor.getNext();
    vector<int> rightRow = rightCursor.getNext();
    bool breakForever = false;

    while(true) // forever loop
    {
        while(!compare(leftRow, rightRow)){
            breakForever = true;
            break;
        }

        if(breakForever){
            break;
        }

        Cursor markLeft = leftCursor;
        vector<int> markLeftRow = leftRow;

        while (true)
        {
            while(compare(leftRow, rightRow)){
                vector<int> row;
                row.insert(row.end(), leftRow.begin(), leftRow.end());
                row.insert(row.end(), rightRow.begin(), rightRow.end());
                joinTable->writeRow(row);
                leftRow = leftCursor.getNext();
                if(leftRow.empty()){
                    break;
                }
            }

            // vector<int> temp = rightRow;
            rightRow = rightCursor.getNext();
            if(rightRow.empty()){
                breakForever = true;
                break;
            }

            if(compare(markLeftRow, rightRow)){
                leftCursor = markLeft;
                leftRow = markLeftRow;
            }
            else
            {
                breakForever = true;
                break;
            }

        }
        if(breakForever){
            break;
        }
    }
    return;

}



void executeJOIN()
{
    logger.log("executeJOIN");

    Table firstTable = *tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table secondTable = *tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    vector<int> firstTableColumnIndices;
    vector<int> secondTableColumnIndices;

    firstTableColumnIndices.push_back(firstTable.getColumnIndex(parsedQuery.joinFirstColumnName));
    secondTableColumnIndices.push_back(secondTable.getColumnIndex(parsedQuery.joinSecondColumnName));

    vector<SortingStrategy> strategy1;
    vector<SortingStrategy> strategy2;

    if(parsedQuery.joinBinaryOperator == GREATER_THAN || parsedQuery.joinBinaryOperator == GEQ)
    {
        strategy1.push_back(DESC);
        strategy2.push_back(ASC);
    }
    else if(parsedQuery.joinBinaryOperator == EQUAL)
    {
        strategy1.push_back(ASC);
        strategy2.push_back(ASC);
    }
    else
    {
        strategy1.push_back(ASC);
        strategy2.push_back(DESC);
    }

    Table *firstTableSorted = orderbyBySortTable(firstTable, firstTableColumnIndices, strategy1);
    Table *secondTableSorted = orderbyBySortTable(secondTable, secondTableColumnIndices, strategy2);

    vector<string> columnNames;
    columnNames.insert(columnNames.end(), firstTable.columns.begin(), firstTable.columns.end());
    columnNames.insert(columnNames.end(), secondTable.columns.begin(), secondTable.columns.end());

    Table *joinTable = new Table(parsedQuery.joinResultRelationName, columnNames);

    if(parsedQuery.joinBinaryOperator == EQUAL)
    {
        equiJoin(firstTableSorted, secondTableSorted, joinTable, firstTableColumnIndices[0], secondTableColumnIndices[0]);
    }
    else
    {
        unequiJoin(firstTableSorted, secondTableSorted, joinTable, firstTableColumnIndices[0], secondTableColumnIndices[0], parsedQuery.joinBinaryOperator);
    }
    tableCatalogue.deleteTable(firstTableSorted->tableName);
    tableCatalogue.deleteTable(secondTableSorted->tableName);
    if(joinTable->blockify())
    {
        tableCatalogue.insertTable(joinTable);
    }
    else
    {
        cout << "Empty Table" << endl;
        joinTable->unload();
        delete joinTable;
    }

    return;
}