//
// Created by adam on 23/11/17.
//

#ifdef __cplusplus
extern "C" {
#endif
#include <eina/eina_main.h>
#include <eina/eina_log.h>
#ifdef __cplusplus
}
#endif

#include <sqlite_file.h>
#include "gtest/gtest.h"

class SqliteFileTests : public testing::Test {
protected:
    virtual void SetUp() {
        eina_init();
        int _log_dom = eina_log_domain_register("gka-data", EINA_COLOR_GREEN);

    }
};

void createTableT1(sqlite_file &db, std::vector<std::string> &columns, std::string &tableName) {
    tableName= "t1";
    columns.emplace_back("a INTEGER PRIMARY KEY");
    columns.emplace_back("b TEXT");
    columns.emplace_back(" c DATETIME");
    columns.emplace_back("d");
    db.createTable(tableName, columns);
}

TEST_F(SqliteFileTests, TableCreation) {
    sqlite_file db;
    db.file(":memory:");
    std::string tableName;
    std::vector<std::string> columns;
    createTableT1(db, columns, tableName);

    auto tables = db.listTables();
    EXPECT_EQ(1, tables.size());
    EXPECT_EQ(tableName, tables[0]);
    auto savedColumns = db.listColumns(tableName);
    EXPECT_EQ(4, savedColumns.size());
    EXPECT_EQ(savedColumns[0], "a");
    EXPECT_EQ(savedColumns[1], "b");
    EXPECT_EQ(savedColumns[2], "c");
    EXPECT_EQ(savedColumns[3], "d");
}

TEST_F(SqliteFileTests, TablePopulation) {
    sqlite_file db;
    db.file(":memory:");
    std::string tableName;
    std::vector<std::string> columns;
    createTableT1(db, columns, tableName);

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("First Column");
    row.emplace_back("Second Column");
    row.emplace_back("Third Column");
    db.addRow(row);
    int rows = db.rowCount();
    EXPECT_EQ(1, rows);
    std::vector<std::string> savedRow = db.readRow(0);
    EXPECT_EQ(4, savedRow.size());
    EXPECT_EQ(savedRow[0], "1");
    EXPECT_EQ(savedRow[1], row[1]);
    EXPECT_EQ(savedRow[2], row[2]);
    EXPECT_EQ(savedRow[3], row[3]);
}

TEST_F(SqliteFileTests, TablePartialPopulation) {
    sqlite_file db;
    db.file(":memory:");
    std::string tableName;
    std::vector<std::string> columns;
    createTableT1(db, columns, tableName);

    std::string sql = "INSERT INTO " + tableName + "(b,d) VALUES('First Column','Third Column');";
    char *sqliteErrMsg = nullptr;
    int rc=sqlite3_exec(db.handle, sql.c_str(), nullptr, nullptr, &sqliteErrMsg);
    if (rc!=SQLITE_OK) {
        EINA_LOG_ERR("SQL error[%d]: %s\n%s", rc, sqlite3_errmsg(db.handle),sqliteErrMsg);
        sqlite3_free(sqliteErrMsg);
    }
    int rows = db.rowCount();
    EXPECT_EQ(1, rows);
    std::vector<std::string> savedRow = db.readRow(0);
    EXPECT_EQ(4, savedRow.size());
    EXPECT_EQ(savedRow[0], "1");
    EXPECT_EQ(savedRow[1], "First Column");
    EXPECT_EQ(savedRow[2], "");
    EXPECT_EQ(savedRow[3], "Third Column");
}

TEST_F(SqliteFileTests, EmptyTable) {
    sqlite_file db;
    db.file(":memory:");
    int rows = db.rowCount();
    EXPECT_EQ(0, rows);
    std::vector<std::string> savedRow = db.readRow(0);
    EXPECT_EQ(0, savedRow.size());
}

TEST_F(SqliteFileTests, TableRowTitle) {
    sqlite_file db;
    db.file(":memory:");
    std::string tableName;
    std::vector<std::string> columns;
    createTableT1(db, columns, tableName);

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("First Column");
    row.emplace_back("Second Column");
    row.emplace_back("Third Column");
    db.addRow(row);
    std::string savedTitle = db.readRowTitle(0);
    EXPECT_EQ(savedTitle, "First Column");
}

TEST_F(SqliteFileTests, TableDefaultNewFile) {
    sqlite_file db;
    db.newFile(":memory:");

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("Test Subject");
    row.emplace_back("Test URL");
    row.emplace_back("Test Notes");
    db.addRow(row);
    std::string savedTitle = db.readRowTitle(0);
    EXPECT_EQ(savedTitle, "Test Subject");
}

TEST(basic_check, test_eq) {
    EXPECT_EQ(1, 1);
}