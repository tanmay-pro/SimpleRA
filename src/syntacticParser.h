#include "tableCatalogue.h"

using namespace std;

enum QueryType
{
    CLEAR,
    CROSS,
    DISTINCT,
    EXPORT,
    INDEX,
    JOIN,
    LIST,
    LOAD,
    PRINT,
    PROJECTION,
    RENAME,
    SELECTION,
    SORT,
    SOURCE,
    UNDETERMINED,
    CHECKSYMMETRY,
    COMPUTE,
    TRANSPOSE,
    ORDERBY,
    GROUPBY
};

enum BinaryOperator
{
    LESS_THAN,
    GREATER_THAN,
    LEQ,
    GEQ,
    EQUAL,
    NOT_EQUAL,
    NO_BINOP_CLAUSE
};

enum GroupByOperator
{
    SUM,
    AVG,
    MIN,
    MAX,
    COUNT,
    NO_GROUPBY_CLAUSE
};

enum SortingStrategy
{
    ASC,
    DESC,
    NO_SORT_CLAUSE
};

enum SelectType
{
    COLUMN,
    INT_LITERAL,
    NO_SELECT_CLAUSE
};

class ParsedQuery
{

public:
    QueryType queryType = UNDETERMINED;
    bool isMatrix = false;

    string checkSymmetryRelationName = "";
    string computeRelationName = "";
    string transposeRelationName = "";

    string clearRelationName = "";

    string crossResultRelationName = "";
    string crossFirstRelationName = "";
    string crossSecondRelationName = "";

    string distinctResultRelationName = "";
    string distinctRelationName = "";

    string exportRelationName = "";

    IndexingStrategy indexingStrategy = NOTHING;
    string indexColumnName = "";
    string indexRelationName = "";

    BinaryOperator joinBinaryOperator = NO_BINOP_CLAUSE;
    string joinResultRelationName = "";
    string joinFirstRelationName = "";
    string joinSecondRelationName = "";
    string joinFirstColumnName = "";
    string joinSecondColumnName = "";

    string loadRelationName = "";

    string printRelationName = "";

    string projectionResultRelationName = "";
    vector<string> projectionColumnList;
    string projectionRelationName = "";

    string renameFromColumnName = "";
    string renameToColumnName = "";
    string renameRelationName = "";
    string renameRelationToName = "";

    SelectType selectType = NO_SELECT_CLAUSE;
    BinaryOperator selectionBinaryOperator = NO_BINOP_CLAUSE;
    string selectionResultRelationName = "";
    string selectionRelationName = "";
    string selectionFirstColumnName = "";
    string selectionSecondColumnName = "";
    int selectionIntLiteral = 0;
    int sortBufferSize = 0;
    SortingStrategy sortingStrategy = NO_SORT_CLAUSE;
    string sortResultRelationName = "";
    string sortColumnName = "";

    string sortRelationName = "";
    vector<string> sortColumnNames;
    vector<SortingStrategy> sortingStrategies;

    string orderByRelationName = "";
    string orderByResultRelationName = "";
    string orderByColumnName = "";
    SortingStrategy orderBySortingStrategy = NO_SORT_CLAUSE;

    string groupByRelationName = "";
    string groupByResultRelationName = "";
    string groupByColumnName = "";
    string groupByCheckColumn = "";
    GroupByOperator groupByCheckColumnOperator = NO_GROUPBY_CLAUSE;
    string groupByReturnColumn = "";
    GroupByOperator groupByReturnColumnOperator = NO_GROUPBY_CLAUSE;
    BinaryOperator groupByBinaryOperator = NO_BINOP_CLAUSE;
    int groupByValueToCheck = 0;

    string sourceFileName = "";

    ParsedQuery();
    void clear();
};

bool syntacticParse();
bool syntacticParseCLEAR();
bool syntacticParseCROSS();
bool syntacticParseDISTINCT();
bool syntacticParseEXPORT();
bool syntacticParseINDEX();
bool syntacticParseJOIN();
bool syntacticParseLIST();
bool syntacticParseLOAD();
bool syntacticParsePRINT();
bool syntacticParsePROJECTION();
bool syntacticParseRENAME();
bool syntacticParseSELECTION();
bool syntacticParseSORT();
bool syntacticParseSOURCE();
bool syntacticParseCHECKSYMMETRY();
bool syntacticParseCOMPUTE();
bool syntacticParseTRANSPOSE();
bool syntacticParseGROUPBY();
bool syntacticParseORDERBY();

bool isFileExists(string tableName);
bool isQueryFile(string fileName);
