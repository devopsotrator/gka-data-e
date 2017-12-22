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
    Elm_Widget_Item *menuFileOpen;
    Elm_Widget_Item *menuFileImport;
    Elm_Widget_Item *menuFileExport;
    Elm_Widget_Item *menuFileClose;

    Elm_Object_Item *menuEdit;
    Elm_Object_Item *menuEditCut;
    Elm_Object_Item *menuEditCopy;
    Elm_Widget_Item *menuEditPaste;
    Elm_Object_Item *menuEditDelete;
    Elm_Widget_Item *menuEditNewEntry;
    Elm_Widget_Item *menuEditEntry;

    Elm_Object_Item *menuView;
    Elm_Object_Item *menuViewZoomIn;
    Elm_Widget_Item *menuViewZoomOut;

    Elm_Object_Item *menuTools;
    Elm_Object_Item *menuToolsLabelPref;
    Elm_Widget_Item *menuToolsTablePref;

    bool menuActive;

    Elm_Widget_Item * findSelectedItem();
};


#endif //GKA_DATA_E_DATA_MENU_H
