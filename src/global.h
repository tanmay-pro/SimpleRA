#include"executor.h"

extern float BLOCK_SIZE;
extern uint BLOCK_COUNT;
extern uint PRINT_COUNT;
extern uint BLOCK_READ;
extern uint BLOCK_WRITTEN;
extern vector<string> tokenizedQuery;
extern ParsedQuery parsedQuery;
extern TableCatalogue tableCatalogue;
extern BufferManager bufferManager;