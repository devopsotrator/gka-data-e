//
// Created by adam on 25/11/17.
//

#ifndef GKA_DATA_E_DATA_MENU_H
#define GKA_DATA_E_DATA_MENU_H

#include <Evas.h>
#include <Elementary.h>

class data_menu {

public:
    void init(Evas_Object *);
    void updateMenuStates(int itemAvailable = 0);

    void flipMenuActive();

    bool isMenuActive();

    void handleKeyDown(Evas_Event_Key_Down *ev);

    void fileOpened();

    void menuDismissed();

private:
    void cursorDown();

    void cursorRight();

    void cursorLeft();

    void cursorUp();

private:
    Evas_Object *menu;

    Elm_Object_Item *menuFile;
    Elm_Object_Item *menuFileNew;
    Elm_Object_Item *menuFileOpen;
    Elm_Object_Item *menuFileImport;
    Elm_Object_Item *menuFileExport;
    Elm_Object_Item *menuFileClose;

    Elm_Object_Item *menuEdit;
    Elm_Object_Item *menuEditCut;
    Elm_Object_Item *menuEditCopy;
    Elm_Object_Item *menuEditPaste;
    Elm_Object_Item *menuEditDelete;
    Elm_Object_Item *menuEditNewEntry;
    Elm_Object_Item *menuEditEntry;

    Elm_Object_Item *menuView;
    Elm_Object_Item *menuViewZoomIn;
    Elm_Object_Item *menuViewZoomOut;

    Elm_Object_Item *menuTools;
    Elm_Object_Item *menuToolsLabelPref;
    Elm_Object_Item *menuToolsTablePref;

    bool menuActive;

    Elm_Object_Item *findSelectedItem();
};


#endif //GKA_DATA_E_DATA_MENU_H
