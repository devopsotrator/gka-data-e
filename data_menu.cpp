//
// Created by adam on 25/11/17.
//

#include "data_menu.h"
#include "data_ui.h"

#define MENU_ELLIPSIS(S) eina_slstr_printf("%s...", S)

static void _edi_menu_new_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.newFile();
}

static void _edi_menu_open_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.openFile();
}

static void _edi_menu_close_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    elm_exit();
}

static void _edi_menu_cut_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {

}

static void _edi_menu_copy_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {

}

static void _edi_menu_paste_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {

}

static void _edi_menu_new_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.newRow();
}

static void _edi_menu_edit_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.editRow();
}

static void _edi_menu_delete_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {

}

void data_menu::init(Evas_Object *window) {
    Evas_Object *menu;
    Elm_Object_Item *menu_it;

    menu = elm_win_main_menu_get(window);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("File"), NULL, NULL);
    elm_menu_item_add(menu, menu_it, "document-new", MENU_ELLIPSIS(_("Create new file")), _edi_menu_new_cb, NULL);
    elm_menu_item_add(menu, menu_it, "document-open", MENU_ELLIPSIS(_("Open file")), _edi_menu_open_cb, NULL);
    elm_menu_item_separator_add(menu, menu_it);
    elm_menu_item_add(menu, menu_it, "application-exit", _("Close"), _edi_menu_close_cb, NULL);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("Edit"), NULL, NULL);
//    elm_menu_item_add(menu, menu_it, "edit-cut", _("Cut"), _edi_menu_cut_cb, NULL);
//    elm_menu_item_add(menu, menu_it, "edit-copy", _("Copy"), _edi_menu_copy_cb, NULL);
//    elm_menu_item_add(menu, menu_it, "edit-paste", _("Paste"), _edi_menu_paste_cb, NULL);
//    elm_menu_item_separator_add(menu, menu_it);
    elm_menu_item_add(menu, menu_it, "", MENU_ELLIPSIS(_("New entry")), _edi_menu_new_entry_cb, NULL);
    elm_menu_item_add(menu, menu_it, "", MENU_ELLIPSIS(_("Edit entry")), _edi_menu_edit_entry_cb, NULL);
//    elm_menu_item_add(menu, menu_it, "", MENU_ELLIPSIS(_("Delete entry")), _edi_menu_delete_entry_cb, NULL);
}
