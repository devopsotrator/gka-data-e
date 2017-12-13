//
// Created by adam on 13/12/17.
//

#ifndef GKA_DATA_E_DATA_EDIT_RECORD_H
#define GKA_DATA_E_DATA_EDIT_RECORD_H

#include <string>
#include <vector>
#include <Edje.h>
#include "sqlite_file.h"

class data_edit_record {

public:
    explicit data_edit_record(sqlite_file &_db);

    void addRow(Evas_Object *window);

    void editRow(Evas_Object *window, int selectedRow);

    void editColumnTabFocus(int currentIndex);

    void updateCurrentRowValue(int i, std::string value);

    void saveCurrentRow();

private:
    void populateAndShowEntryPopup(Evas_Object *window, Evas_Object *popup, const std::vector<std::string> &cols);

private:
    std::vector<std::string> currentRowValues;
    std::vector<Evas_Object *> currentRowEditors;
    sqlite_file &db;
};


#endif //GKA_DATA_E_DATA_EDIT_RECORD_H
