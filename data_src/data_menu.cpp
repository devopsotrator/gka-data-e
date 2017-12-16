//
// Created by adam on 25/11/17.
//

#include "data_menu.h"
#include "data_ui.h"

static void menu_new_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.newFile();
}

static void menu_open_sqlite_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.openFile();
}

static void menu_import_csv_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.importCsv();
}

static void menu_export_csv_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.exportCsv();
}

static void menu_close_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    elm_exit();
}

static void menu_cut_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.cut();
}

static void menu_copy_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.copy();
}

static void menu_paste_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.paste();
}

static void menu_new_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.addRow();
}

static void menu_edit_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.editRow();
}

static void menu_delete_entry_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.deleteEntry();
}

static void menu_view_zoom_in_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.zoomIn();
}

static void menu_view_zoom_out_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.zoomOut();
}

static void menu_label_preferences_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.labelPreferences();
}

static void menu_table_preferences_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.tablePreferences();
}

void data_menu::init(Evas_Object *window) {
    Evas_Object *menu;
    Elm_Object_Item *menu_it;

    menu = elm_win_main_menu_get(window);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("File"), NULL, NULL);
    elm_menu_item_add(menu, menu_it, "document-new", _("Create new file... (ctrl-n)"), menu_new_cb, NULL);
    elm_menu_item_add(menu, menu_it, "document-open", _("Open file... (ctrl-o)"), menu_open_sqlite_cb, NULL);
    elm_menu_item_separator_add(menu, menu_it);
    elm_menu_item_add(menu, menu_it, "document-import", _("Import CSV file... (ctrl-i)"), menu_import_csv_cb, NULL);
    elm_menu_item_add(menu, menu_it, "document-export", _("Export CSV file... (ctrl-p)"), menu_export_csv_cb, NULL);
    elm_menu_item_separator_add(menu, menu_it);
    elm_menu_item_add(menu, menu_it, "application-exit", _("Close (ctrl-q)"), menu_close_cb, NULL);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("Edit"), NULL, NULL);
    menuCut = elm_menu_item_add(menu, menu_it, "edit-cut", _("Cut (ctrl-x)"), menu_cut_cb, NULL);
    menuCopy = elm_menu_item_add(menu, menu_it, "edit-copy", _("Copy (ctrl-c)"), menu_copy_cb, NULL);
    elm_menu_item_add(menu, menu_it, "edit-paste", _("Paste (ctrl-v)"), menu_paste_cb, NULL);
    elm_menu_item_separator_add(menu, menu_it);
    elm_menu_item_add(menu, menu_it, "", _("New entry... (ctrl-shift-N)"), menu_new_entry_cb, NULL);
    elm_menu_item_add(menu, menu_it, "", _("Edit entry... (ctrl-shift-E)"), menu_edit_entry_cb, NULL);
    menuDelete = elm_menu_item_add(menu, menu_it, "", _("Delete entry... (ctrl-shift-D)"), menu_delete_entry_cb, NULL);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("View"), NULL, NULL);
    elm_menu_item_add(menu, menu_it, "", _("Zoom In... (ctrl-m)"), menu_view_zoom_in_cb, NULL);
    elm_menu_item_add(menu, menu_it, "", _("Zoom Out... (ctrl-shift-M)"), menu_view_zoom_out_cb, NULL);

    menu_it = elm_menu_item_add(menu, NULL, NULL, _("Tools"), NULL, NULL);
    elm_menu_item_add(menu, menu_it, "", _("Label preferences... (ctrl-l)"), menu_label_preferences_cb, NULL);
    elm_menu_item_add(menu, menu_it, "", _("Table preferences... (ctrl-t)"), menu_table_preferences_cb, NULL);
}

void data_menu::updateMenuStates(Eina_Bool itemAvailable) {
    elm_object_item_disabled_set(menuCut, EINA_TRUE);
    elm_object_item_disabled_set(menuCopy, !itemAvailable);
    elm_object_item_disabled_set(menuDelete, !itemAvailable);

}

