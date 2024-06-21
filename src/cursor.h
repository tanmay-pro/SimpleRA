#include"bufferManager.h"
/**
 * @brief The cursor is an important component of the system. To read from a
 * table, you need to initialize a cursor. The cursor reads rows from a page one
 * at a time.
 *
 */
class Cursor{
    public:
    Page page;
    int pageIndex;
    string tableName;
    int pagePointer;
    bool isMatrix = false;

    public:
    Cursor(string tableName, int pageIndex, bool isMatrix = false);
    vector<int> getNext();
    vector<vector<int>> getMatrixPage();
    vector<vector<int>> getTablePage();
    void nextPage(int pageIndex);
};