//
// Created by adam on 15/12/17.
//

#include <sqlite_file.h>
#include <csv_file.h>
#include <fstream>
#include <Elementary.h>
#include <iostream>
#include "catch.hpp"

TEST_CASE("csv file export") {
    eina_init();

    sqlite_file db;
    db.newFile(":memory:");

    std::vector<std::string> row;
    row.emplace_back("");
    row.emplace_back("Test Subject");
    row.emplace_back("Test URL");
    row.emplace_back("Test\" Notes");
    db.addRow(row);

    std::string fileName("/tmp/test-export.csv");
    csv_file::exportCsv(db, fileName);

    std::ifstream in{fileName, std::ios::binary | std::ios::ate};
    REQUIRE(!in.fail());
    std::string fileContent;
    auto size = static_cast<unsigned long>(in.tellg());
    std::string str(size, '\0');
    in.seekg(0);
    in.read(&str[0], size);
    REQUIRE(str == "\"title\",\"url\",\"notes\"\r\n\"Test Subject\",\"Test URL\",\"Test\"\" Notes\"\r\n");

    elm_shutdown();
}

TEST_CASE("csv file import") {
    eina_init();

    std::string fileName("/tmp/test-export.csv");

    std::ofstream out{fileName, std::ios::binary | std::ios::ate};
    out << "\"title\",\"url\",\"notes\"\r\n\"Test Subject\",\"Test URL\",\"Test\"\" Notes\"\r\n";
    out.close();

    sqlite_file db;
    db.newFile(":memory:", true);

    csv_file::importCsv(db, fileName);

    auto columns = db.listColumns();
    REQUIRE(columns[0] == "id");
    REQUIRE(columns[1] == "title");
    REQUIRE(columns[2] == "url");
    REQUIRE(columns[3] == "notes");
    auto row = db.readRow(0);
    REQUIRE(row[0] == "1");
    REQUIRE(row[1] == "Test Subject");
    REQUIRE(row[2] == "Test URL");
    REQUIRE(row[3] == "Test\" Notes");

    elm_shutdown();

}