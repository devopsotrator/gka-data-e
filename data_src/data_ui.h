//
// Created by adam on 24/11/17.
//

#ifndef GKA_DATA_E_DATA_UI_H
#define GKA_DATA_E_DATA_UI_H

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <Elementary.h>
#include "data_menu.h"
#include "data_edit_record.h"
#include "data_table_preferences.h"
#include "config.h"

#define ENABLE_NLS

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(x) gettext(x)
#else
# define _(x) (x)
#endif

static const int MAX_LIST_LENGTH = 255;

static const int DEFAULT_APP_WIDTH = 216 * 3;
static const int DEFAULT_APP_HEIGHT = 108 * 3;
static const int DEFAULT_DIALOG_WIDTH = 216 * 2;
static const int DEFAULT_DIALOG_HEIGHT = DEFAULT_APP_HEIGHT;

static const int  STACKMAXSIZE = 10;
struct stack {
    Evas_Object *sObject[STACKMAXSIZE];
    Evas_Object *sFocus[STACKMAXSIZE];
    int topIndex = -1;
};

typedef struct stack STACK;

class data_ui {

public:
    data_ui(sqlite_file &_db);

    void init();

    void setSqliteFile(std::string fileName);

    data_table_preferences& getTablePref();

    void rowSelected(int i);

    void setNewFile();

    void repopulateFieldsTable();

    void repopulateRightList(int selected=0) const;

    void openFile();

    void newFile(bool forImport=false);

    void updateNewFileName(std::string fileName);

    void clearFocus();

    void addRow();

    void editRow();

    void deleteCurrentRow();

    void updateCurrentRowValue(int currentRow, std::string value);

    void saveCurrentRow();

    std::string getTitleForFileName(const std::string &fileName) const;

    void handleKeyUp(void *event_info);

    void handleKeyDown(void *event_info);

    void prevButton();

    void nextButton();

    bool prevItem();

    bool nextItem();

    void repopulateUI();

    void labelPreferences();

    void saveLabelPreferences();

    std::vector<std::string> getEditableColumns();

    void editColumnMoveUp();

    void editColumnMoveDown();

    void editColumnDelete();

    void setEditColumnSelection(int i);

    int getEditColumnSelection();

    void updateEditLabel(std::string label);

    void addEditableLabel();

    std::string getEditLabel();

    void saveEditableLabel();

    Eina_Bool labelPreferencesAreValid();

    void deleteEntry();

    void clearActivePopup();

    void showPopup(Evas_Object *popup, Evas_Object *focusOn = nullptr);

    void popupStackPush(Evas_Object *toPush, Evas_Object *focusOn);

    Evas_Object *popupStackPop();

    bool popupStackEmpty();

    Evas_Object *popupStackTopFocus();

    void clearLabelPreferences();

    void editColumnTabFocus(int i);

    void zoomIn();

    void zoomOut();

    void cut();

    void copy();

    void paste();

    void tablePreferences();

    void importCsv();

    void exportFile();

    void updateExportFileName(std::string fileName);

    void exportCsv();

    void importCsvFile();

    void setCsvFile(std::string fileName);

    void scrollerScrolled();

    void entryClicked(int i);

    void updateScrollPositions();

    data_menu getMenu();

    bool isAltPressed();

private:
    void cursorUp(Eina_Bool i);

    void cursorDown(Eina_Bool i);

    void cursorLeft(Eina_Bool shift);

    void cursorRight(Eina_Bool shift);

    void updateEditorSelection();

    Evas_Object *standardFileOpener(Evas_Object *win, const char *title, Evas_Smart_Cb okFunc);

    void showFileOpener(Evas_Object *win, Evas_Object *fs);

    void updateArrowGeometry();

    void updateFoundItemDisplay(bool found, int editorIndex);

    void updateCurrentEditorAndArrowVisibilityForIndex(int index);

private:
    sqlite_file &db;
    Evas_Object *filterAppliedLabel;
    Evas_Object *rightBox;
    Evas_Object *searchEntry;
    Evas_Object *rightList;
    Evas_Object *fieldsTable;
    Evas_Object *window;
    Evas_Object *scroller;
    Elm_Genlist_Item_Class *right_list_itc;
    int selectedRow;
    std::string newFileName;
    data_menu menu;
    int oldSearchEntryPos;
    std::vector<std::string> editableColumns;
    int editableColumnsIndex;
    std::string editableColumnsEditLabel;
    std::map<std::string, std::string> renames;
    STACK popupStack;
    data_edit_record dataEditRecord;
    std::vector<Evas_Object*> currentEditors;
    int currentEditorWithCursorIndex;
    std::vector<Evas_Object*> currentArrows;
    int editorSelectionBeganIn;
    int editorSelectionBeganAt;
    int editorSelectionEndIn;
    int editorSelectionEndAt;
    bool editorSelectionActive;
    data_table_preferences table_preferences;
    std::string exportFileName;
    std::string importFileName;
    bool altPressClean;
    bool altPressed;
};

extern data_ui ui;

#endif //GKA_DATA_E_DATA_UI_H
