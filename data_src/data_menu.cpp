//
// Created by adam on 25/11/17.
//

#include <cstring>
#include <cwctype>
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

static void menu_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.getMenu().menuDismissed();
}

void data_menu::init(Evas_Object *window) {
    menu = elm_win_main_menu_get(window);
    evas_object_smart_callback_add(menu, "dismissed", menu_dismissed_cb, nullptr);

    menuFile = elm_menu_item_add(menu, nullptr, nullptr, _("File"), nullptr, nullptr);
    menuFileNew = elm_menu_item_add(menu, menuFile, "document-new", _("Create new file... (ctrl-n)"), menu_new_cb, nullptr);
    menuFileOpen = elm_menu_item_add(menu, menuFile, "document-open", _("Open file... (ctrl-o)"), menu_open_sqlite_cb, nullptr);
    elm_menu_item_separator_add(menu, menuFile);
    menuFileImport = elm_menu_item_add(menu, menuFile, "document-import", _("Import CSV file... (ctrl-i)"), menu_import_csv_cb, nullptr);
    menuFileExport = elm_menu_item_add(menu, menuFile, "document-export", _("Export CSV file... (ctrl-p)"), menu_export_csv_cb, nullptr);
    elm_menu_item_separator_add(menu, menuFile);
    menuFileClose = elm_menu_item_add(menu, menuFile, "application-exit", _("Close (ctrl-q)"), menu_close_cb, nullptr);

    menuEdit = elm_menu_item_add(menu, nullptr, nullptr, _("Edit"), nullptr, nullptr);
    menuEditCut = elm_menu_item_add(menu, menuEdit, "edit-cut", _("Cut (ctrl-x)"), menu_cut_cb, nullptr);
    menuEditCopy = elm_menu_item_add(menu, menuEdit, "edit-copy", _("Copy (ctrl-c)"), menu_copy_cb, nullptr);
    menuEditPaste = elm_menu_item_add(menu, menuEdit, "edit-paste", _("Paste (ctrl-v)"), menu_paste_cb, nullptr);
    elm_menu_item_separator_add(menu, menuEdit);
    menuEditNewEntry = elm_menu_item_add(menu, menuEdit, "", _("New entry... (ctrl-shift-N)"), menu_new_entry_cb, nullptr);
    menuEditEntry = elm_menu_item_add(menu, menuEdit, "", _("Edit entry... (ctrl-shift-E)"), menu_edit_entry_cb, nullptr);
    menuEditDelete = elm_menu_item_add(menu, menuEdit, "", _("Delete entry... (ctrl-shift-D)"), menu_delete_entry_cb, nullptr);

    menuView = elm_menu_item_add(menu, nullptr, nullptr, _("View"), nullptr, nullptr);
    menuViewZoomIn = elm_menu_item_add(menu, menuView, "", _("Zoom In... (ctrl-m)"), menu_view_zoom_in_cb, nullptr);
    menuViewZoomOut = elm_menu_item_add(menu, menuView, "", _("Zoom Out... (ctrl-shift-M)"), menu_view_zoom_out_cb, nullptr);

    menuTools = elm_menu_item_add(menu, nullptr, nullptr, _("Tools"), nullptr, nullptr);
    menuToolsLabelPref = elm_menu_item_add(menu, menuTools, "", _("Label preferences... (ctrl-l)"), menu_label_preferences_cb, nullptr);
    menuToolsTablePref = elm_menu_item_add(menu, menuTools, "", _("Table preferences... (ctrl-t)"), menu_table_preferences_cb, nullptr);

    addMenuItemToShortCuts(menuFile);
    addMenuItemToShortCuts(menuEdit);
    addMenuItemToShortCuts(menuView);
    addMenuItemToShortCuts(menuTools);
}

void data_menu::addMenuItemToShortCuts(Elm_Object_Item *menuItem) {
    Evas_Object *cO = elm_menu_item_object_get(menuItem);
    auto ptr = elm_object_text_get(cO);
    std::mbtowc(nullptr, nullptr, 0); // reset the conversion state
    const char* end = ptr + std::strlen(ptr);
    wchar_t wc;
    int ret = std::mbtowc(&wc, ptr, end-ptr);
    if (ret > 0) {
        menuShortCuts.emplace(std::towlower(static_cast<wint_t>(wc)), menuItem);
    }
}

void data_menu::updateMenuStates(int itemAvailable) {
    elm_object_item_disabled_set(menuEditCut, EINA_TRUE);
    elm_object_item_disabled_set(menuEditCopy, static_cast<Eina_Bool>(itemAvailable == 0));
    elm_object_item_disabled_set(menuEditEntry, static_cast<Eina_Bool>(itemAvailable == 0));
    elm_object_item_disabled_set(menuEditDelete, static_cast<Eina_Bool>(itemAvailable == 0));
}

void data_menu::flipMenuActive() {
//    EINA_LOG_ERR("flipMenuActive %d",menuActive);
    menuActive = !menuActive;
    if (menuActive) {
        Elm_Object_Item *cMenu = elm_menu_selected_item_get(menu);
        if (cMenu == nullptr) {
            cMenu = menuFile;
        }
        elm_menu_item_selected_set(cMenu, EINA_TRUE);
        Evas_Object *cO = elm_menu_item_object_get(cMenu);
        elm_layout_signal_emit(cO,"elm,action,open", "");
    } else {
        Elm_Object_Item *cMenu = elm_menu_selected_item_get(menu);
        elm_menu_item_selected_set(cMenu, EINA_FALSE);
        elm_menu_close(menu);
    }
}

bool data_menu::isMenuActive() {
//    EINA_LOG_ERR("isMenuActive %d",menuActive);
    return menuActive;
}

void data_menu::handleKeyUp(Evas_Event_Key_Down *ev) {
//    EINA_LOG_ERR("KeyUp: %s - %s - %s", ev->key, ev->compose, ev->string);

    if (ev->string) {
        std::mbtowc(nullptr, nullptr, 0); // reset the conversion state
        const char *end = ev->string + std::strlen(ev->string);
        wchar_t wc;
        int ret = std::mbtowc(&wc, ev->string, end - ev->string);
        if (ret > 0) {
            auto shortCut = menuShortCuts.find(static_cast<const wchar_t &>(std::towlower(static_cast<wint_t>(wc))));
            if (shortCut != menuShortCuts.end()) {
                auto menu = shortCut->second;
                elm_menu_item_selected_set(menu, EINA_TRUE);
                Evas_Object *cO = elm_menu_item_object_get(menu);
                elm_layout_signal_emit(cO, "elm,action,open", "");
                menuActive = true;
            }
        }
    }
}

void data_menu::handleKeyDown(Evas_Event_Key_Down *ev) {
//    EINA_LOG_ERR("KeyDown: %s - %s - %s", ev->key, ev->compose, ev->string);

    if (!strcmp(ev->key, "Up")) {
        cursorUp();
    } else if (!strcmp(ev->key, "Down")) {
        cursorDown();
    } else if (!strcmp(ev->key, "Escape")) {
        flipMenuActive();
    } else if (!strcmp(ev->key, "Left")) {
        cursorLeft();
    } else if (!strcmp(ev->key, "Right")) {
        cursorRight();
    } else if (!strcmp(ev->key, "Return")) {
        Evas_Object *cO = elm_menu_item_object_get(findSelectedItem());
        elm_layout_signal_emit(cO,"elm,action,click", "");
        menuActive = false;
        elm_menu_item_selected_set(elm_menu_selected_item_get(menu), EINA_FALSE);
    }
}

Elm_Object_Item *data_menu::findSelectedItem() {
    Elm_Object_Item *cMenu = elm_menu_selected_item_get(menu);
    Elm_Object_Item *selected = nullptr;
    if (cMenu == menuFile) {
        if (elm_menu_item_selected_get(menuFileNew)) {
            selected = menuFileNew;
        }
        if (elm_menu_item_selected_get(menuFileOpen)) {
            selected = menuFileOpen;
        }
        if (elm_menu_item_selected_get(menuFileImport)) {
            selected = menuFileImport;
        }
        if (elm_menu_item_selected_get(menuFileExport)) {
            selected = menuFileExport;
        }
        if (elm_menu_item_selected_get(menuFileClose)) {
            selected = menuFileClose;
        }
        if (selected == nullptr) {
            selected = menuFileNew;
        }
    }
    if (cMenu == menuEdit) {
        if (elm_menu_item_selected_get(menuEditCut)) {
            selected = menuEditCut;
        }
        if (elm_menu_item_selected_get(menuEditCopy)) {
            selected = menuEditCopy;
        }
        if (elm_menu_item_selected_get(menuEditPaste)) {
            selected = menuEditPaste;
        }
        if (elm_menu_item_selected_get(menuEditNewEntry)) {
            selected = menuEditNewEntry;
        }
        if (elm_menu_item_selected_get(menuEditEntry)) {
            selected = menuEditEntry;
        }
        if (elm_menu_item_selected_get(menuEditDelete)) {
            selected = menuEditDelete;
        }
        if (selected == nullptr) {
            selected = menuEditCut;
        }
    }
    if (cMenu == menuView) {
        if (elm_menu_item_selected_get(menuViewZoomIn)) {
            selected = menuViewZoomIn;
        }
        if (elm_menu_item_selected_get(menuViewZoomOut)) {
            selected = menuViewZoomOut;
        }
        if (selected == nullptr) {
            selected = menuViewZoomIn;
        }
    }
    if (cMenu == menuTools) {
        if (elm_menu_item_selected_get(menuToolsLabelPref)) {
            selected = menuToolsLabelPref;
        }
        if (elm_menu_item_selected_get(menuToolsTablePref)) {
            selected = menuToolsTablePref;
        }
        if (selected == nullptr) {
            selected = menuToolsLabelPref;
        }
    }
    return selected;
}

void data_menu::cursorUp() {
    Elm_Object_Item *cMenu = elm_menu_selected_item_get(menu);
    Elm_Object_Item *prev = nullptr;
    if (cMenu == menuFile) {
        if (elm_menu_item_selected_get(menuFileNew)) {
            prev = menuFileNew;
        }
        if (elm_menu_item_selected_get(menuFileOpen)) {
            prev = menuFileNew;
        }
        if (elm_menu_item_selected_get(menuFileImport)) {
            prev = menuFileOpen;
        }
        if (elm_menu_item_selected_get(menuFileExport)) {
            prev = menuFileImport;
        }
        if (elm_menu_item_selected_get(menuFileClose)) {
            prev = menuFileExport;
        }
        if (prev == nullptr) {
            prev = menuFileNew;
        }
    }
    if (cMenu == menuEdit) {
        if (elm_menu_item_selected_get(menuEditCut)) {
            prev = menuEditCut;
        }
        if (elm_menu_item_selected_get(menuEditCopy)) {
            prev = menuEditCut;
        }
        if (elm_menu_item_selected_get(menuEditPaste)) {
            prev = menuEditCopy;
        }
        if (elm_menu_item_selected_get(menuEditNewEntry)) {
            prev = menuEditPaste;
        }
        if (elm_menu_item_selected_get(menuEditEntry)) {
            prev = menuEditNewEntry;
        }
        if (elm_menu_item_selected_get(menuEditDelete)) {
            prev = menuEditEntry;
        }
        if (prev == nullptr) {
            prev = menuEditCut;
        }
    }
    if (cMenu == menuView) {
        if (elm_menu_item_selected_get(menuViewZoomIn)) {
            prev = menuViewZoomIn;
        }
        if (elm_menu_item_selected_get(menuViewZoomOut)) {
            prev = menuViewZoomIn;
        }
        if (prev == nullptr) {
            prev = menuViewZoomIn;
        }
    }
    if (cMenu == menuTools) {
        if (elm_menu_item_selected_get(menuToolsLabelPref)) {
            prev = menuToolsLabelPref;
        }
        if (elm_menu_item_selected_get(menuToolsTablePref)) {
            prev = menuToolsLabelPref;
        }
        if (prev == nullptr) {
            prev = menuToolsLabelPref;
        }
    }
    elm_menu_item_selected_set(prev, EINA_TRUE);
}

void data_menu::cursorDown() {
    Elm_Object_Item *cMenu = elm_menu_selected_item_get(menu);
    Elm_Object_Item *next = nullptr;
    if (cMenu == menuFile) {
        if (elm_menu_item_selected_get(menuFileNew)) {
            next = menuFileOpen;
        }
        if (elm_menu_item_selected_get(menuFileOpen)) {
            next = menuFileImport;
        }
        if (elm_menu_item_selected_get(menuFileImport)) {
            next = menuFileExport;
        }
        if (elm_menu_item_selected_get(menuFileExport)) {
            next = menuFileClose;
        }
        if (elm_menu_item_selected_get(menuFileClose)) {
            next = menuFileClose;
        }
        if (next == nullptr) {
            next = menuFileNew;
        }
    }
    if (cMenu == menuEdit) {
        if (elm_menu_item_selected_get(menuEditCut)) {
            next = menuEditCopy;
        }
        if (elm_menu_item_selected_get(menuEditCopy)) {
            next = menuEditPaste;
        }
        if (elm_menu_item_selected_get(menuEditPaste)) {
            next = menuEditNewEntry;
        }
        if (elm_menu_item_selected_get(menuEditNewEntry)) {
            next = menuEditEntry;
        }
        if (elm_menu_item_selected_get(menuEditEntry)) {
            next = menuEditDelete;
        }
        if (elm_menu_item_selected_get(menuEditDelete)) {
            next = menuEditDelete;
        }
        if (next == nullptr) {
            next = menuEditCut;
        }
    }
    if (cMenu == menuView) {
        if (elm_menu_item_selected_get(menuViewZoomIn)) {
            next = menuViewZoomOut;
        }
        if (elm_menu_item_selected_get(menuViewZoomOut)) {
            next = menuViewZoomOut;
        }
        if (next == nullptr) {
            next = menuViewZoomIn;
        }
    }
    if (cMenu == menuTools) {
        if (elm_menu_item_selected_get(menuToolsLabelPref)) {
            next = menuToolsTablePref;
        }
        if (elm_menu_item_selected_get(menuToolsTablePref)) {
            next = menuToolsTablePref;
        }
        if (next == nullptr) {
            next = menuToolsLabelPref;
        }
    }
    elm_menu_item_selected_set(next, EINA_TRUE);
}

void data_menu::cursorLeft() {
//    EINA_LOG_ERR("selected menu item %x",elm_menu_selected_item_get(menu));
    Elm_Object_Item *prev = elm_menu_item_prev_get(elm_menu_selected_item_get(menu));
//    EINA_LOG_ERR("prevSet %x",prev);
    elm_menu_item_selected_set(prev, EINA_TRUE);
    Evas_Object *cO = elm_menu_item_object_get(prev);
    elm_layout_signal_emit(cO,"elm,action,open", "");
}

void data_menu::cursorRight() {
//    EINA_LOG_ERR("selected menu item %x",elm_menu_selected_item_get(menu));
    Elm_Object_Item *next = elm_menu_item_next_get(elm_menu_selected_item_get(menu));
//    EINA_LOG_ERR("nextSet %x",next);
    elm_menu_item_selected_set(next, EINA_TRUE);
    Evas_Object *cO = elm_menu_item_object_get(next);
    elm_layout_signal_emit(cO,"elm,action,open", "");
}

void data_menu::menuDismissed() {
//    EINA_LOG_ERR("menuDismissed");
    menuActive = false;
}

