//
// Created by adam on 23/11/17.
//

#include "Elementary.h"
#include <sqlite_file.h>
#include <algorithm>
#include "catch.hpp"

void createTableT1(sqlite_file &db, std::vector<std::string> &columns, std::string &tableName) {
    tableName= "t1";
    columns.emplace_back("a INTEGER PRIMARY KEY");
    columns.emplace_back("b TEXT");
    columns.emplace_back(" c DATETIME");
    columns.emplace_back("d");
    db.createTable(tableName, columns);
}

void createTableT2(sqlite_file &db, std::vector<std::string> &columns, std::string &tableName) {
    tableName= "t2";
    columns.emplace_back("a TEXT");
    columns.emplace_back(" b DATETIME");
    columns.emplace_back("c INTEGER PRIMARY KEY");
    columns.emplace_back("d");
    db.createTable(tableName, columns);
}

TEST_CASE("sqlite tables") {
    eina_init();
    int _log_dom = eina_log_domain_register("gka-data", EINA_COLOR_GREEN);

    sqlite_file db;
    db.file(":memory:");

    SECTION("empty table") {
        int rows = db.rowCount();
        REQUIRE(0 == rows);
        std::vector<std::string> savedRow = db.readRow(0);
        REQUIRE(0 == savedRow.size());
    }

    SECTION("with table") {
        std::string tableName;
        std::vector<std::string> columns;
        createTableT1(db, columns, tableName);

        SECTION("create table") {
            auto tables = db.listTables();
            REQUIRE(1 == tables.size());
            REQUIRE(tableName == tables[0]);
            auto savedColumns = db.listColumns(tableName);
            REQUIRE(4 == savedColumns.size());
            REQUIRE(savedColumns[0] == "a");
            REQUIRE(savedColumns[1] == "b");
            REQUIRE(savedColumns[2] == "c");
            REQUIRE(savedColumns[3] == "d");
        }

        SECTION("populate table") {
            std::vector<std::string> row;
            row.emplace_back("");
            row.emplace_back("First Column");
            row.emplace_back("Second Column");
            row.emplace_back("Third Column");
            db.addRow(row);

            SECTION("check title for listing views") {
                std::string savedTitle = db.readRowTitle(0);
                REQUIRE(savedTitle == "First Column");
            }

            SECTION("check full row") {
                int rows = db.rowCount();
                REQUIRE(1 == rows);
                std::vector<std::string> savedRow = db.readRow(0);
                REQUIRE(4 == savedRow.size());
                REQUIRE(savedRow[0] == "1");
                REQUIRE(savedRow[1] == row[1]);
                REQUIRE(savedRow[2] == row[2]);
                REQUIRE(savedRow[3] == row[3]);
            }
        }

        SECTION("delete row in table") {
            std::vector<std::string> row;
            row.emplace_back("");
            row.emplace_back("First Column");
            row.emplace_back("");
            row.emplace_back("Third Column");
            db.addRow(row);
            int rows = db.rowCount();
            REQUIRE(1 == rows);
            std::vector<std::string> savedRow = db.readRow(0);
            db.deleteRow(savedRow);
            rows = db.rowCount();
            REQUIRE(0 == rows);
        }

        SECTION("partial population") {
            std::string sql = "INSERT INTO " + tableName + "(b,d) VALUES('First Column','Third Column');";
            char *sqliteErrMsg = nullptr;
            int rc = sqlite3_exec(db.handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
            if (rc != SQLITE_OK) {
                EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(db.handle), sqliteErrMsg);
                sqlite3_free(sqliteErrMsg);
            }
            int rows = db.rowCount();
            REQUIRE(1 == rows);
            std::vector<std::string> savedRow = db.readRow(0);
            REQUIRE(4 == savedRow.size());
            REQUIRE(savedRow[0] == "1");
            REQUIRE(savedRow[1] == "First Column");
            REQUIRE(savedRow[2] == "");
            REQUIRE(savedRow[3] == "Third Column");
        }
    }

    elm_shutdown();
}

TEST_CASE("sqlite primary key in second column") {
    sqlite_file db;
    db.file(":memory:");
    std::string tableName;
    std::vector<std::string> columns;
    createTableT2(db, columns, tableName);

    std::vector<std::string> row;
    row.emplace_back("First Column");
    row.emplace_back("Second Column");
    row.emplace_back("");
    row.emplace_back("Third Column");
    db.addRow(row);
    std::string savedTitle = db.readRowTitle(0);
    REQUIRE(savedTitle == "First Column");
}

TEST_CASE("default new file") {
    sqlite_file db;
    db.newFile(":memory:");

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("Test Subject");
    row.emplace_back("Test URL");
    row.emplace_back("Test Notes");
    db.addRow(row);
    std::string savedTitle = db.readRowTitle(0);
    REQUIRE(savedTitle == "Test Subject");
}

TEST_CASE("sqlite table search next") {
    sqlite_file db;
    db.newFile(":memory:");

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("Test Subject");
    row.emplace_back("Test URL");
    row.emplace_back("Test Notes");
    db.addRow(row);
    row[1] = "Subject2";
    db.addRow(row);
    row[1] = "Subject3";
    db.addRow(row);
    db.setFilter("Subject2");
    int rows = db.rowCount();
    REQUIRE(1 == rows);
    std::string savedTitle = db.readRowTitle(0);
    REQUIRE(savedTitle == "Subject2");
}

TEST_CASE("sqlite table column reorder") {
    sqlite_file db;
    db.newFile(":memory:");

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("Test Subject");
    row.emplace_back("Test URL");
    row.emplace_back("Test Notes");
    db.addRow(row);
    auto oldColumns = db.listColumns();
    auto newColumns = db.listColumns();
    std::swap(newColumns[1], newColumns[2]);
    std::swap(newColumns[0], newColumns[3]);
    db.setColumns(newColumns);
    int rows = db.rowCount();
    REQUIRE(1 == rows);
    std::string savedTitle = db.readRowTitle(0);
    REQUIRE(savedTitle == "Test URL");
}

TEST_CASE("sqlite table column add") {
    sqlite_file db;
    db.newFile(":memory:");

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("Test Subject");
    row.emplace_back("Test URL");
    row.emplace_back("Test Notes");
    db.addRow(row);
    auto oldColumns = db.listColumns();
    auto newColumns = db.listColumns();
    newColumns.emplace_back("newColumn");
    db.setColumns(newColumns);
    int rows = db.rowCount();
    REQUIRE(1 == rows);
    auto savedColumns = db.listColumns();
    REQUIRE(savedColumns[savedColumns.size()-1] == "newColumn");
}

TEST_CASE("sqlite table column rename") {
    sqlite_file db;
    db.newFile(":memory:");

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("Test Subject");
    row.emplace_back("Test URL");
    row.emplace_back("Test Notes");
    db.addRow(row);
    auto oldColumns = db.listColumns();
    auto newColumns = db.listColumns();
    std::string fromColumn("notes");
    std::string toColumn("comments");
    std::replace(newColumns.begin(), newColumns.end(), fromColumn, toColumn);
    std::map<std::string,std::string> renames;
    renames.emplace(toColumn,fromColumn);
    db.setColumns(newColumns, renames);
    int rows = db.rowCount();
    REQUIRE(1 == rows);
    auto savedColumns = db.listColumns();
    REQUIRE(savedColumns[savedColumns.size()-1] == "comments");
    auto savedRow = db.readRow(0);
    REQUIRE(savedRow[0] == "1");
    REQUIRE(savedRow[1] == "Test Subject");
    REQUIRE(savedRow[2] == "Test URL");
    REQUIRE(savedRow[3] == "Test Notes");
}
