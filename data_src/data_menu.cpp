//
// Created by adam on 25/11/17.
//

#include "data_menu.h"
#include "data_ui.h"

#define MENU_ELLIPSIS(S) eina_slstr_printf("%s...", S)

static void menu_new_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.newFile();
}

static void menu_open_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.openFile();
}

static void menu_close_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    elm_exit();
}

static void menu_cut_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {

}

static void menu_copy_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {

}

static void menu_paste_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {

}

static void menu_new_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.newEntry();
}

static void menu_edit_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.editRow();
}

static void menu_delete_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.deleteEntry();
}

static void menu_label_preferences_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.labelPreferences();
}

void data_menu::init(Evas_Object *window) {
    Evas_Object *menu;
    Elm_Object_Item *menu_it;

    menu = elm_win_main_menu_get(window);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("File"), NULL, NULL);
    elm_menu_item_add(menu, menu_it, "document-new", MENU_ELLIPSIS(_("Create new file")), menu_new_cb, NULL);
    elm_menu_item_add(menu, menu_it, "document-open", MENU_ELLIPSIS(_("Open file")), menu_open_cb, NULL);
    elm_menu_item_separator_add(menu, menu_it);
    elm_menu_item_add(menu, menu_it, "application-exit", _("Close"), menu_close_cb, NULL);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("Edit"), NULL, NULL);
//    elm_menu_item_add(menu, menu_it, "edit-cut", _("Cut"), menu_cut_cb, NULL);
//    elm_menu_item_add(menu, menu_it, "edit-copy", _("Copy"), menu_copy_cb, NULL);
//    elm_menu_item_add(menu, menu_it, "edit-paste", _("Paste"), menu_paste_cb, NULL);
//    elm_menu_item_separator_add(menu, menu_it);
    elm_menu_item_add(menu, menu_it, "", MENU_ELLIPSIS(_("New entry")), menu_new_entry_cb, NULL);
    elm_menu_item_add(menu, menu_it, "", MENU_ELLIPSIS(_("Edit entry")), menu_edit_entry_cb, NULL);
    elm_menu_item_add(menu, menu_it, "", MENU_ELLIPSIS(_("Delete entry")), menu_delete_entry_cb, NULL);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("Tools"), NULL, NULL);
    elm_menu_item_add(menu, menu_it, "", MENU_ELLIPSIS(_("Label preferences")), menu_label_preferences_cb, NULL);
}
