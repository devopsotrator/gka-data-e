//
// Created by adam on 15/12/17.
//

#ifndef GKA_DATA_E_DATA_TABLE_PREFERENCES_H
#define GKA_DATA_E_DATA_TABLE_PREFERENCES_H

#include <Edje.h>

class data_table_preferences {

public:
    data_table_preferences(sqlite_file &_db);

    void show(Evas_Object *window);

    void setCurrentTableSelection(int index);

    void saveEditableTable();

    void addEditableTable();

    void updateEditTable(std::string table);

    std::string getEditTable();

    void clearTablePreferences();

    void saveTablePreferences();

    void editTableDelete();

    bool tablePreferencesAreValid();

    void setTableSelection(int tableIndex);

    int getActiveTable();

    std::vector<std::string> getEditableTables();

    void setActiveTableFromSelection();

    int getCurrentTableSelection();

private:
    sqlite_file &db;
    std::vector<std::string> editableTables;
    int editableTablesIndex;
    int activeTableIndex;
    std::string editableTablesCurrentEdit;
    std::map<std::string, std::string> renames;
    Evas_Object *list;

};


#endif //GKA_DATA_E_DATA_TABLE_PREFERENCES_H
