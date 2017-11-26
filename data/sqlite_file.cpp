//
// Created by adam on 23/11/17.
//

#ifdef __cplusplus
extern "C" {
#endif
#include <eina/eina_log.h>
#ifdef __cplusplus
}
#endif

#include <sqlite3.h>
#include <utility>
#include <vector>
#include <regex>
#include "sqlite_file.h"

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
        std::string sql = "CREATE TABLE IF NOT EXISTS notes(id INTEGER PRIMARY KEY, title TEXT, url TEXT, notes TEXT);";
        rc = sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
        if (rc != SQLITE_OK) {
            EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(handle), sqliteErrMsg);
            sqlite3_free(sqliteErrMsg);
        }
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

    std::string sql = "CREATE TABLE IF NOT EXISTS " + string + "(";
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

}

std::vector<std::string> sqlite_file::listColumns(std::string table) {
    intPrimaryKey = EINA_FALSE;

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
    while (sqlite3_step(ppStmt) == SQLITE_ROW) {
        std::string text = reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 1));
        ret.emplace_back(text);
        //detect use of primary key
        auto primaryKey = sqlite3_column_int(ppStmt, 5);
        auto type = reinterpret_cast<const char*>(sqlite3_column_text(ppStmt, 2));
        if (primaryKey && strcmp(type, "INTEGER")==0) {
            intPrimaryKey = EINA_TRUE;
        }
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

    std::string sql = "SELECT COUNT(*) FROM "+table+";";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
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

std::vector<std::string> sqlite_file::readRow(int i, std::string table) {
    std::vector<std::string> ret;
    table = getTable(table);
    if (table.empty()) {
        return ret;
    }

    std::string sql = "SELECT * FROM "+table+" LIMIT 1 OFFSET ?1;";
    sqlite3_stmt* ppStmt = nullptr;
    const char* pzTail = nullptr;
    int rc=sqlite3_prepare_v2(handle, sql.c_str(), -1, &ppStmt, &pzTail);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s", rc, sqlite3_errmsg(handle));
    }
    rc=sqlite3_bind_int(ppStmt, 1, i);
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

    if (intPrimaryKey) {
        return row[1];
    }
    return row[0];
}
