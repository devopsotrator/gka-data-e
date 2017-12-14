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
    void updateMenuStates(Eina_Bool itemAvailable = EINA_FALSE);

private:
    Elm_Widget_Item *menuCut;
    Elm_Widget_Item *menuDelete;
    Elm_Widget_Item *menuCopy;
};


#endif //GKA_DATA_E_DATA_MENU_H
