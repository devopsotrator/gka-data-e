//
// Created by adam on 15/12/17.
//

#ifndef GKA_DATA_E_CSV_FILE_H
#define GKA_DATA_E_CSV_FILE_H


#include "sqlite_file.h"

//CSV Format supported
//https://tools.ietf.org/html/rfc4180

class csv_file {

private:
    static std::vector<std::string> csvReadRow(std::istream &in, char delimiter);

public:
    static void exportCsv(sqlite_file &db, std::string &fileName);

    static void outputToStream(std::ofstream &out, std::vector<std::string> values, int primaryKeyColumn);

    static void importCsv(sqlite_file &db, std::string &fileName);

    static std::string replaceAll(std::string str, const std::string &from, const std::string &to);
};


#endif //GKA_DATA_E_CSV_FILE_H
