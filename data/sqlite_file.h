//
// Created by adam on 23/11/17.
//

#ifndef GKA_DATA_E_SQLITE_FILE_H
#define GKA_DATA_E_SQLITE_FILE_H

#include <sqlite3.h>
#include <vector>
#include <string>

typedef enum
{
    DATA_CONNECT_TYPE_NONE,
    DATA_CONNECT_TYPE_INIT,
    DATA_CONNECT_TYPE_DATABASE_SET,
    DATA_CONNECT_TYPE_QUERY
} Esql_Connect_Type;

class sqlite_file {

public:
    sqlite3 *handle;
    Esql_Connect_Type state;
    int intPrimaryKey;

    void file(std::string fileName);

    std::vector<std::string> listTables();

    void createTable(std::string &string, std::vector<std::string> &vector);

    std::vector<std::string> listColumns(std::string table="");

    void addRow(std::vector<std::string> &vector);

    std::string current_table;

    int rowCount(std::string string="");

    std::vector<std::string> readRow(int i, std::string table="");

    std::string &getTable(std::string &table);

    std::string readRowTitle(int i, std::string table="");

    void newFile(std::string fileName);
};


#endif //GKA_DATA_E_SQLITE_FILE_H
