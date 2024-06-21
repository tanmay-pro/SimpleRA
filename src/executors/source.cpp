#include "global.h"
/**
 * @brief
 * SYNTAX: SOURCE filename
 */
bool syntacticParseSOURCE()
{
    logger.log("syntacticParseSOURCE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = SOURCE;
    parsedQuery.sourceFileName = tokenizedQuery[1];
    return true;
}

bool semanticParseSOURCE()
{
    logger.log("semanticParseSOURCE");
    if (!isQueryFile(parsedQuery.sourceFileName))
    {
        cout << "SEMANTIC ERROR: File doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeSOURCE()
{
    logger.log("executeSOURCE");
    string file_name = "../data/" + tokenizedQuery[1] + ".ra";
    ifstream ra_file(file_name);
    string command;
    int command_no = 1;
    regex delim("[^\\s,]+");
    while (!ra_file.eof())
    {
        // cout << "\n> ";
        // cout << command << endl;
        tokenizedQuery.clear();
        parsedQuery.clear();
        logger.log("\nReading  Command no: " + to_string(command_no));
        getline(ra_file, command);
        logger.log(command);

        auto words_begin = std::sregex_iterator(command.begin(), command.end(), delim);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i)
            tokenizedQuery.emplace_back((*i).str());

        if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT")
        {
            exit(0);
        }

        if (tokenizedQuery.empty())
        {
            continue;
        }

        if (tokenizedQuery.size() == 1)
        {
            cout << "SYNTAX ERROR in command no" + to_string(command_no) << endl;
            break;
        }

        logger.log("Running command no " + to_string(command_no));
        if (syntacticParse() && semanticParse())
        {
            executeCommand();
            cout << "Number of blocks read: " << BLOCK_READ << endl;
            cout << "Number of blocks written: " << BLOCK_WRITTEN << endl;
            cout << "Number of blocks accessed: " << BLOCK_READ + BLOCK_WRITTEN << endl;
            BLOCK_READ = 0;
            BLOCK_WRITTEN = 0;
        }
        else
        {
            break;
        }

        command_no++;
    }

    return;
}
