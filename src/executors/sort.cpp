#include "global.h"
/**
 * @brief File contains method to process SORT commands.
 *
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 *
 * sorting_order = ASC | DESC
 */
bool syntacticParseSORT()
{
    logger.log("syntacticParseSORT");
    if (tokenizedQuery[2] != "BY")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.queryType = SORT;
    parsedQuery.sortBufferSize = 10;
    parsedQuery.sortRelationName = tokenizedQuery[1];
    int ind = 3;

    while (ind < tokenizedQuery.size() && tokenizedQuery[ind] != "IN")
    {
        // remove the comma from the column name if it is present
        if (tokenizedQuery[ind].back() == ',')
            tokenizedQuery[ind].pop_back();
        parsedQuery.sortColumnNames.push_back(tokenizedQuery[ind]);
        ind++;
    }
    if (ind >= tokenizedQuery.size() - 1)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    int sortColumnCount = parsedQuery.sortColumnNames.size();
    if (ind + sortColumnCount >= tokenizedQuery.size())
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    for (int i = ind + 1; i < tokenizedQuery.size(); i++)
    {
        if (tokenizedQuery[ind].back() == ',')
            tokenizedQuery[ind].pop_back();

        if (tokenizedQuery[i] == "ASC")
            parsedQuery.sortingStrategies.push_back(ASC);
        else if (tokenizedQuery[i] == "DESC")
            parsedQuery.sortingStrategies.push_back(DESC);
        else
        {
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
    }

    return true;
}

bool semanticParseSORT()
{
    logger.log("semanticParseSORT");

    if (!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    for (int i = 0; i < parsedQuery.sortColumnNames.size(); i++)
    {

        if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnNames[i], parsedQuery.sortRelationName))
        {
            cout << "SEMANTIC ERROR: Column doesn't exist" << endl;
            return false;
        }
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

bool checkFunc(vector<vector<int>> currRows)
{
    for (int i = 0; i < currRows.size(); i++)
    {
        if (!currRows[i].empty())
            return true;
    }
    return false;
}

void finalize_table(Table *table)
{

    string oldTableName = table->tableName;
    string oldSource = "../data/temp/" + table->tableName + "_Page";
    string newSource = "../data/temp/" + parsedQuery.sortRelationName + "_Page";

    for (int i = 0; i < table->blockCount; i++)
    {
        string oldName = oldSource + to_string(i);
        string newName = newSource + to_string(i);
        rename(oldName.c_str(), newName.c_str());
    }

    table->tableName = parsedQuery.sortRelationName;
    rename(table->sourceFileName.c_str(), ("../data/temp/" + parsedQuery.sortRelationName + ".csv").c_str());
    table->sourceFileName = "../data/temp/" + parsedQuery.sortRelationName + ".csv";

    // update tableCatalogue
    tableCatalogue.renameTable(oldTableName, parsedQuery.sortRelationName);
}

void executeSORT()
{
    logger.log("executeSORT");

    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);

    vector<int> sortColumnIndices;
    for (int i = 0; i < parsedQuery.sortColumnNames.size(); i++)
        sortColumnIndices.push_back(table.getColumnIndex(parsedQuery.sortColumnNames[i]));

    Cursor cursor = table.getCursor();

    uint numRuns = ceil((double)table.blockCount / (double)BLOCK_COUNT);

    int i = 1;
    Table *sortPhase = new Table(parsedQuery.sortRelationName + "_std_sortPhase", table.columns);
    uint currPageIndex = 0;
    while (i <= numRuns)
    {
        // read BLOCk_COUNT in memory
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
             { return cmp(a, b, sortColumnIndices, parsedQuery.sortingStrategies); });
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

    tableCatalogue.deleteTable(table.tableName);

    i = 1;
    int numPasses = ceil(log(numRuns) / log(BLOCK_COUNT - 1));
    uint a = numRuns;
    int s = BLOCK_COUNT;

    if (numPasses == 0)
    {
        finalize_table(sortPhase);
        return;
    }

    Table *mergePhase = new Table(parsedQuery.sortRelationName + "_std_mergePhase_0", table.columns);

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
                                          { return cmp(a, b, sortColumnIndices, parsedQuery.sortingStrategies); });
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
            finalize_table(sortPhase);
            break;
        }
        mergePhase = new Table(parsedQuery.sortRelationName + "_std_mergePhase" + "_" + to_string(i), table.columns);
        a = q;
        s = s * (BLOCK_COUNT - 1);
        i++;
    }
    return;
}