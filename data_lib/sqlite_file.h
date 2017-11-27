//
// Created by adam on 23/11/17.
//

#ifndef GKA_DATA_E_SQLITE_FILE_H
#define GKA_DATA_E_SQLITE_FILE_H

#include <sqlite3.h>
#include <vector>
#include <string>
#include <map>

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
    int intPrimaryKey; //0 = not set, 1-n+1 for column indexing
    std::string filter;

    void file(std::string fileName);

    virtual ~sqlite_file();

    std::vector<std::string> listTables();

    void createTable(std::string &string, std::vector<std::string> &vector);

    std::vector<std::string> listColumns(std::string table="");

    void addRow(std::vector<std::string> &vector);

    std::string current_table;

    int rowCount(std::string string="");

    std::vector<std::string> readRow(int i, std::string table="");

    std::string &getTable(std::string &table);

    std::string readRowTitle(int i, std::string table="");

    void deleteRow(std::vector<std::string> vector, std::string table="");

    void newFile(std::string fileName);

    void setFilter(const std::string &string);

    std::string where(std::string table, std::vector<std::string> &toBind);

    std::vector<std::string> listSearchableColumns(std::string &table);

    void setColumns(const std::vector<std::string> vector, const std::map<std::string,std::string> renames={}, std::string table="");

    std::string &getFilter();

private:

    bool setColumnsBeginTransaction() const;

    bool setColumnsExtractOldTableInfo(std::vector<std::string> &oldName, std::vector<std::string> &oldType,
                                       std::vector<int> &oldPrimaryKey, std::vector<int> &oldNullPermitted,
                                       std::vector<std::string> &oldDefaultValues, const std::string &table) const;

    bool setColumnsCreateNewTable(const std::vector<std::string> &newColumns, const std::vector<std::string> &oldName,
                                  const std::vector<std::string> &oldType, const std::vector<int> &oldPrimaryKey,
                                  const std::string &table) const;

    bool setColumnsCopyFromOldToNew(const std::vector<std::string> &newColumns, const std::map<std::string, std::string> &renames,
                                    const std::vector<std::string> &oldName, const std::string &table) const;

    bool setColumnsDropOldTable(const std::string &table) const;

    bool setColumnsRenameTable(const std::string &table) const;

    bool setColumnsCommitTransaction() const;

};


#endif //GKA_DATA_E_SQLITE_FILE_H
