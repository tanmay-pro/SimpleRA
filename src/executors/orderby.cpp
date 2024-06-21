#include "global.h"
/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * R <- ORDER BY <attribute> ASC|DESC ON <relation_name>
 *
 */
bool syntacticParseORDERBY()
{
    logger.log("syntacticParseORDERBY");
    if (tokenizedQuery.size() != 8 || tokenizedQuery[3] != "BY" || tokenizedQuery[6] != "ON")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.queryType = ORDERBY;
    parsedQuery.orderByResultRelationName = tokenizedQuery[0];
    parsedQuery.orderByRelationName = tokenizedQuery[7];
    parsedQuery.orderByColumnName = tokenizedQuery[4];
    if (tokenizedQuery[5] == "ASC")
        parsedQuery.orderBySortingStrategy = ASC;
    else if (tokenizedQuery[5] == "DESC")
        parsedQuery.orderBySortingStrategy = DESC;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    return true;
}

bool semanticParseORDERBY()
{
    logger.log("semanticParseORDERBY");

    if (!tableCatalogue.isTable(parsedQuery.orderByRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    if (tableCatalogue.isTable(parsedQuery.orderByResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.orderByColumnName, parsedQuery.orderByRelationName))
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

void orderByFinalizeTable(Table *table)
{
    string oldTableName = table->tableName;
    string oldSource = "../data/temp/" + table->tableName + "_Page";
    string newSource = "../data/temp/" + parsedQuery.orderByResultRelationName + "_Page";

    for (int i = 0; i < table->blockCount; i++)
    {
        string oldName = oldSource + to_string(i);
        string newName = newSource + to_string(i);
        rename(oldName.c_str(), newName.c_str());
    }

    table->tableName = parsedQuery.orderByResultRelationName;

    rename(table->sourceFileName.c_str(), ("../data/temp/" + parsedQuery.orderByResultRelationName + ".csv").c_str());
    table->sourceFileName = "../data/temp/" + parsedQuery.orderByResultRelationName + ".csv";

    tableCatalogue.renameTable(oldTableName, parsedQuery.orderByResultRelationName);
}

void executeORDERBY()
{
    logger.log("executeORDERBY");

    Table table = *tableCatalogue.getTable(parsedQuery.orderByRelationName);

    vector<int> sortColumnIndices;
    sortColumnIndices.push_back(table.getColumnIndex(parsedQuery.orderByColumnName));
    vector<SortingStrategy> sortingStrategies = {parsedQuery.orderBySortingStrategy};

    Cursor cursor = table.getCursor();

    uint numRuns = ceil((double)table.blockCount / (double)BLOCK_COUNT);

    int i = 1;
    Table *sortPhase = new Table(parsedQuery.orderByResultRelationName + "_std_sortPhase", table.columns);
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
             { return cmp(a, b, sortColumnIndices, sortingStrategies); });
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
        orderByFinalizeTable(sortPhase);
        return;
    }

    Table *mergePhase = new Table(parsedQuery.orderByResultRelationName + "_std_mergePhase_0", table.columns);
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
                                          { return cmp(a, b, sortColumnIndices, sortingStrategies); });
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
            orderByFinalizeTable(sortPhase);
            break;
        }
        mergePhase = new Table(parsedQuery.orderByResultRelationName + "_std_mergePhase" + "_" + to_string(i), table.columns);
        a = q;
        s = s * (BLOCK_COUNT - 1);
        i++;
    }
    return;
}