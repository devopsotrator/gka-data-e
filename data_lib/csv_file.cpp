//
// Created by adam on 15/12/17.
//

#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include "csv_file.h"

std::vector<std::string> csv_file::csvReadRow(std::istream &in, char delimiter) {
    std::stringstream stringFormer;
    bool inQuotes = false;
    std::vector<std::string> row;
    while(in.good()) {
        char c = static_cast<char>(in.get());
        if (!inQuotes && c=='"') {
            inQuotes=true;
        } else if (inQuotes && c=='"') {
            if (in.peek() == '"') {
                stringFormer << static_cast<char>(in.get());
            } else {
                inQuotes=false;
            }
        } else if (!inQuotes && c==delimiter) {
            row.push_back( stringFormer.str());
            stringFormer.str("");
        } else if (!inQuotes && (c=='\r' || c=='\n')) {
            if (in.peek()=='\n') {
                in.get();
            }
            row.push_back(stringFormer.str());
            return row;
        } else {
            stringFormer << c;
        }
    }
    return row;
}

std::string csv_file::replaceAll(std::string str, const std::string &from, const std::string &to) {
    size_t startPos = 0;
    while((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length();
    }
    return str;
}

void csv_file::outputToStream(std::ofstream &out, std::vector<std::string> values, int primaryKeyColumn) {
    for (int i = 0; i < (int)values.size(); i++) {
        if (i != primaryKeyColumn-1) {
            auto value = values[i];
            if (i != 0 && !(primaryKeyColumn == 1 && i == 1)) {
                out << ',';
            }
            out << '"' << replaceAll(value, "\"", "\"\"") << '"';
        }
    }
    out << "\r\n";
}

void csv_file::exportCsv(sqlite_file &db, std::string &fileName) {
    std::ofstream out(fileName);
    auto columns = db.listColumns();
    outputToStream(out, columns, db.getPrimaryKey());
    for (int row = 0; row < db.rowCount(); row++) {
        auto rowValues = db.readRow(row);
        outputToStream(out, rowValues, db.getPrimaryKey());
    }
    out.close();
}

void csv_file::importCsv(sqlite_file &db, std::string &fileName) {
    std::ifstream in(fileName);
    std::string tableName("imported");
    if (in.good()) {
        std::vector<std::string> heading = csvReadRow(in, ',');
        heading.insert(std::begin(heading),"id INTEGER PRIMARY KEY");
        db.createTable(tableName,heading);
    }
    while (in.good()) {
        std::vector<std::string> row = csvReadRow(in, ',');
        if (!row.empty()) {
            row.insert(std::begin(row),"");
            db.addRow(row);
        }
    }
    in.close();
}
