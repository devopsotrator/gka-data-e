//
// Created by adam on 23/11/17.
//
//

#include <sqlite3.h>
#include <utility>
#include <vector>
#include <regex>
#include <Elementary.h>
#include <numeric>
#include "sqlite_file.h"

sqlite_file::~sqlite_file() {
    sqlite3_close(handle);
}

void sqlite_file::file(std::string fileName) {
    int rc = sqlite3_open_v2(fileName.c_str(), &handle, SQLITE_OPEN_READWRITE, nullptr);
    if (rc != SQLITE_OK) {
        EINA_LOG_ERR("Could not open %s: %s", fileName.c_str(), sqlite3_errcode(handle));
    } else {
        state = DATA_CONNECT_TYPE_INIT;
    }
}

void sqlite_file::newFile(std::string fileName) {
    int rc = sqlite3_open_v2(fileName.c_str(), &handle, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        EINA_LOG_ERR("Could not open %s: %s", fileName.c_str(), sqlite3_errcode(handle));
    } else {
        state = DATA_CONNECT_TYPE_INIT;
    }

    if (listTables().empty()) {
        char *sqliteErrMsg = nullptr;
        std::string sql = "CREATE TABLE IF NOT EXISTS notes (id INTEGER PRIMARY KEY, title TEXT, url TEXT, notes TEXT);";
        rc = sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
        if (rc != SQLITE_OK) {
            EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle), sqliteErrMsg);
            sqlite3_free(sqliteErrMsg);
        }
        intPrimaryKey = 0 + 1;
    }
}

std::vector<std::string> sqlite_file::listTables() {
    std::vector<std::string> ret;
    const char *sql = "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY name;";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql, -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    while (sqlite3_step(ppStmt) == SQLITE_ROW) {
        auto text1 = sqlite3_column_text(ppStmt, 0);
        ret.emplace_back(reinterpret_cast<const char*>(text1));
    }
    sqlite3_finalize(ppStmt);
    return ret;
}

void sqlite_file::createTable(std::string &string, std::vector<std::string> &vector) {
    char *sqliteErrMsg = nullptr;

    std::string sql = "CREATE TABLE IF NOT EXISTS " + string + " (";
    for (const auto &field : vector) {
        if (field != vector[0])
            sql += ",";
        sql += field;
    }
    sql += ");";
    int rc=sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
    }
    if (!vector.empty()) {
        for (auto i=0; i < vector.size(); i++) {
            if (strcasestr(vector[i].c_str(), "INTEGER PRIMARY KEY") != NULL) {
                intPrimaryKey = i + 1;
            }
        }
    }
}

std::vector<std::string> sqlite_file::listColumns(std::string table) {
    intPrimaryKey = 0;

    std::vector<std::string> ret;
    table = getTable(table);
    if (table.empty()) {
        return ret;
    }

    std::string sql = "PRAGMA TABLE_INFO("+table+");";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    if (sqlite3_step(ppStmt) == SQLITE_ROW) {
        std::string text = reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 1));
        ret.emplace_back(text);
        //detect use of integer primary key
        auto primaryKey = sqlite3_column_int(ppStmt, 5);
        auto type = reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 2));
        if (primaryKey && strcmp(type, "INTEGER")==0) {
            intPrimaryKey = primaryKey;
        }
    }
    while (sqlite3_step(ppStmt) == SQLITE_ROW) {
        std::string text = reinterpret_cast<const char *>(sqlite3_column_text(ppStmt, 1));
        ret.emplace_back(text);
    }
    sqlite3_finalize(ppStmt);
    return ret;
}

void sqlite_file::addRow(std::vector<std::string> &vector) {
    if (current_table.empty()) {
        current_table = listTables()[0];
    }

    std::string sql = "INSERT OR REPLACE INTO " + current_table + " VALUES(";
    for (int i=1; i<=vector.size(); i++) {
        if (i != 1)
            sql += ",";
        sql += "?"+std::to_string(i);
    }
    sql += ");";

    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    for (int i=1; i<=vector.size(); i++) {
        auto colValue = vector[i - 1];
        if (!colValue.empty()) {
            rc = sqlite3_bind_text(ppStmt, i, colValue.c_str(), -1, SQLITE_TRANSIENT);
        } else {
            rc= sqlite3_bind_null(ppStmt, i);
        }
        if (rc != SQLITE_OK) {
            EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
        }
    }
    if (sqlite3_step(ppStmt) == SQLITE_ROW) {

    }
    sqlite3_finalize(ppStmt);
}

int sqlite_file::rowCount(std::string table) {
    int ret = 0;
    table = getTable(table);
    if (table.empty()) {
        return ret;
    }
    std::vector<std::string> toBind;

    std::string sql = "SELECT COUNT(*) FROM " + table + where(table, toBind) + ";";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    for (int i=0; i<toBind.size(); i++) {
        auto colValue = toBind[i];
        rc = sqlite3_bind_text(ppStmt, i+1, colValue.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
        }
    }
    if (sqlite3_step(ppStmt) == SQLITE_ROW) {
        ret = sqlite3_column_int(ppStmt, 0);
    }
    sqlite3_finalize(ppStmt);
    return ret;
}

std::string &sqlite_file::getTable(std::string &table) {
    if (table.empty()) {
        if (!current_table.empty()) {
            table = current_table;
        } else {
            auto vector = listTables();
            if (!vector.empty()) {
                table = vector[0];
            }
        }
    }
    return table;
}

std::vector<std::string> sqlite_file::readRow(int rowIndex, std::string table) {
    std::vector<std::string> ret;
    table = getTable(table);
    if (table.empty()) {
        return ret;
    }
    std::vector<std::string> toBind;

    std::string sql = "SELECT * FROM "+table+where(table,toBind);
    auto nextBind = toBind.size() + 1;
    sql += " LIMIT 1 OFFSET ?" + std::to_string(nextBind) + ";";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    for (int i=0; i<toBind.size(); i++) {
        auto colValue = toBind[i];
        rc = sqlite3_bind_text(ppStmt, i+1, colValue.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
        }
    }
    rc=sqlite3_bind_int(ppStmt, nextBind, rowIndex);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    if (sqlite3_step(ppStmt) == SQLITE_ROW) {
        int columns = sqlite3_data_count(ppStmt);
        for (int c=0; c < columns; c++) {
            auto raw = sqlite3_column_text(ppStmt, c);
            if (raw != nullptr) {
                std::string text = reinterpret_cast<const char *>(raw);
                ret.emplace_back(text);
            } else {
                ret.emplace_back("");
            }
        }
    }
    sqlite3_finalize(ppStmt);
    return ret;
}

std::string sqlite_file::readRowTitle(int i, std::string table) {
    std::vector<std::string> row = readRow(i, std::move(table));

    if (intPrimaryKey == 0 + 1) {
        return row[1];
    } else {
        return row[0];
    }
}

void sqlite_file::setFilter(const std::string &string) {
    filter = string;
}

std::string sqlite_file::where(std::string table, std::vector<std::string> &toBind) {
    if (filter.empty()) {
        return "";
    } else {
        std::string ret = " WHERE (";
        std::vector<std::string> searchable = listSearchableColumns(table);
        for (auto &search : searchable) {
            if (search != searchable[0])
                ret += " OR ";
            toBind.emplace_back("%" + filter + "%");
            ret += search + " LIKE ?" + std::to_string(toBind.size());
        }
        return ret + ")";
    }
}

std::vector<std::string> sqlite_file::listSearchableColumns(std::string &table) {
    std::vector<std::string> ret;

    std::string sql = "PRAGMA TABLE_INFO("+table+");";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    while (sqlite3_step(ppStmt) == SQLITE_ROW) {
        std::string columnName = reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 1));
        auto type = reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 2));
        if (strcasestr(type, "TEXT")==0 || strcasestr(type, "CHAR")==0) {
            ret.emplace_back(columnName);
        }
    }
    sqlite3_finalize(ppStmt);

    return ret;
}

void sqlite_file::setColumns(std::vector<std::string> newColumns, std::map<std::string,std::string> renames, std::string table) {
    std::vector<std::string> ret;
    table = getTable(table);
    if (table.empty()) {
        return;
    }
    char *sqliteErrMsg = nullptr;

    int rc=sqlite3_exec(handle, "BEGIN TRANSACTION", nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
        return;
    }

    std::vector<std::string> oldName;
    std::vector<std::string> oldType;
    std::vector<int> oldNullPermitted;
    std::vector<std::string> oldDefaultValues;
    std::vector<int> oldPrimaryKey;

    std::string sql = "PRAGMA TABLE_INFO("+table+");";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
        return;
    }
    while (sqlite3_step(ppStmt) == SQLITE_ROW) {
        oldName.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 1)));
        oldType.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 2)));
        oldNullPermitted.emplace_back(sqlite3_column_int(ppStmt, 3));
        auto defaultValue = reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 4));
        if (defaultValue != nullptr) {
            oldDefaultValues.emplace_back(defaultValue);
        } else {
            oldDefaultValues.emplace_back("");
        }
        oldPrimaryKey.emplace_back(sqlite3_column_int(ppStmt, 5));
    }
    sqlite3_finalize(ppStmt);

    sql = "CREATE TABLE new_" + table + "(";
    if (std::accumulate(oldPrimaryKey.begin(), oldPrimaryKey.end(), 0) == 0) {
        sql += "id INTEGER PRIMARY KEY, ";
    }
    for (auto &column : newColumns) {
        if (column != newColumns[0])
            sql += ", ";
        sql += column;
        auto index = std::find(oldName.begin(), oldName.end(), column) - oldName.begin();
        if (index < oldName.size()) {
            sql += " " + oldType[index];
            if (oldPrimaryKey[index] > 0) {
                sql += " PRIMARY KEY";
            }
        }
    }
    sql += ")";
    rc=sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
        return;
    }

    sql = "INSERT INTO new_" + table + "(";
    for (auto &column : newColumns) {
        if ((std::find(oldName.begin(), oldName.end(), column) != std::end(oldName)) || (renames.find(column) != renames.end())) {
            if (column != newColumns[0])
                sql += ", ";
            sql += column;
        }
    }
    sql += ") SELECT ";
    for (auto &column : newColumns) {
        if (std::find(oldName.begin(), oldName.end(), column) != std::end(oldName)) {
            if (column != newColumns[0])
                sql += ", ";
            sql += column;
        } else if (renames.find(column) != renames.end()) {
            if (column != newColumns[0])
                sql += ", ";
            sql += renames.find(column)->second;
        }
    }
    sql += " FROM " + table + ";";
    EINA_LOG_ERR("SQL: %s", sql.c_str());
    rc=sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
        return;
    }

    sql = "DROP TABLE "+table;
    rc=sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
        return;
    }

    sql = "ALTER TABLE new_" + table + " RENAME TO " + table;
    rc=sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
        return;
    }

    rc=sqlite3_exec(handle, "COMMIT TRANSACTION", nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
        return;
    }
}
