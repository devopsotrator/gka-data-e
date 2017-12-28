//
// Created by adam on 24/11/17.
//

#include <sqlite_file.h>
#include <Elementary.h>
#include <regex>
#include <unordered_set>
#include <csv_file.h>
#include "data_ui.h"
#include "data_label_preferences.h"

static sqlite_file db;
data_ui ui(db);

static char *right_list_text_get_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *part) {
    if (strcmp(part, "elm.text") == 0) {
        auto *buf = static_cast<char *>(malloc(MAX_LIST_LENGTH));
        auto row = db.readRowTitle((int) (uintptr_t) data);
        std::string toFind("<br/>");
        std::string firstLine(row.begin(), std::search(row.begin(), row.end(), toFind.begin(), toFind.end()));
        snprintf(buf, MAX_LIST_LENGTH, "%s", firstLine.c_str());

        return buf;
    } else return nullptr;
}

static void row_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    auto i = (int) (uintptr_t) data;

    ui.rowSelected(i);
}

static void previous_button_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.prevButton();
}

static void next_button_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.nextButton();
}

static void scroller_scroll_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.scrollerScrolled();
}

static void edit_entry_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    auto i = (int) (uintptr_t) data;
    ui.entryClicked(i);
}

static void window_cb_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info) {
    ui.handleKeyDown(event_info);
}

static void window_cb_key_up(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info) {
    ui.handleKeyUp(event_info);
}

void data_ui::handleKeyUp(void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);

    Eina_Bool alt = evas_key_modifier_is_set(ev->modifiers, "Alt");

//    EINA_LOG_ERR("KeyUp: %s - %s - %s", ev->key, ev->compose, ev->string);
    if (!strcmp(ev->key, "Alt_L")) {
        altPressed = false;
    }

    if (!strcmp(ev->key, "Alt_L") && altPressClean) {
        menu.flipMenuActive();
    } else if (altPressClean) {
        altPressClean = false;
    }

    if (menu.isMenuActive()) {
        menu.handleKeyUp(ev);
        return;
    }

    if (alt) {
        menu.handleKeyUp(ev);
    }

    if (!strcmp(ev->key, "Shift_R") || !strcmp(ev->key, "Shift_L")) {
        if (editorSelectionActive) {
            updateEditorSelection();
            editorSelectionActive = false;
        }
    }
}

void data_ui::handleKeyDown(void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    Eina_Bool ctrl, alt, shift;

    ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");
    alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
    shift = evas_key_modifier_is_set(ev->modifiers, "Shift");

//    EINA_LOG_ERR("KeyDown: %s - %s - %s", ev->key, ev->compose, ev->string);

    if (!strcmp(ev->key, "Alt_L")) {
        altPressClean = true;
        altPressed = true;
    }

    if (alt) {
        // ignore alt keys on the down
    } else if (ctrl && shift) {
        if (!strcmp(ev->key, "N")) {
            addRow();
        } else if (!strcmp(ev->key, "E")) {
            editRow();
        } else if (!strcmp(ev->key, "D")) {
            deleteEntry();
        } else if (!strcmp(ev->key, "M")) {
            zoomOut();
        }
    } else if (ctrl) {
        if (!strcmp(ev->key, "n")) {
            newFile();
        } else if (!strcmp(ev->key, "o")) {
            openFile();
        } else if (!strcmp(ev->key, "q")) {
            elm_exit();
        } else if (!strcmp(ev->key, "l")) {
            labelPreferences();
        } else if (!strcmp(ev->key, "t")) {
            tablePreferences();
        } else if (!strcmp(ev->key, "m")) {
            zoomIn();
        } else if (!strcmp(ev->key, "x")) {
            cut();
        } else if (!strcmp(ev->key, "c")) {
            copy();
        } else if (!strcmp(ev->key, "v")) {
            paste();
        } else if (!strcmp(ev->key, "i")) {
            importCsv();
        } else if (!strcmp(ev->key, "p")) {
            exportCsv();
        }
    }

    if (menu.isMenuActive()) {
        menu.handleKeyDown(ev);
        return;
    }

    if (!strcmp(ev->key, "Escape")) {
        if (!popupStackEmpty()) {
            clearActivePopup();
        } else {
            elm_object_text_set(searchEntry, "");
            db.setFilter("");
            repopulateUI();
        }
    } else if (!strcmp(ev->key, "Return")) {
        if (shift) {
            prevButton();
        } else {
            nextButton();
        }
    } else if (!strcmp(ev->key, "Up")) {
        cursorUp(shift);
    } else if (!strcmp(ev->key, "Down")) {
        cursorDown(shift);
    } else if (!strcmp(ev->key, "Left")) {
        cursorLeft(shift);
    } else if (!strcmp(ev->key, "Right")) {
        cursorRight(shift);
    }
}

void data_ui::clearActivePopup() {
    if (!popupStackEmpty()) {
#if ELM_VERSION_MAJOR>=1 && ELM_VERSION_MINOR>=20
        elm_popup_dismiss(popupStackPop());
#else
        auto popup = popupStackPop();
        evas_object_hide(popup);
        evas_object_del(popup);
#endif
    }
}

bool data_ui::nextItem() {
    auto sIt = elm_genlist_selected_item_get(rightList);
    auto it = elm_genlist_item_next_get(sIt);
    if (!it) return false;
    elm_genlist_item_selected_set(it, EINA_TRUE);
    elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
    return true;
}

bool data_ui::prevItem() {
    auto sIt = elm_genlist_selected_item_get(rightList);
    auto it = elm_genlist_item_prev_get(sIt);
    if (!it) return false;
    elm_genlist_item_selected_set(it, EINA_TRUE);
    elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
    return true;
}

static void entry_filter_out_unwanted_cb(void *data EINA_UNUSED, Evas_Object *entry EINA_UNUSED, char **text) {
//    EINA_LOG_ERR("Filter: %s", *text);
    if (!strcmp(*text, "<tab/>") || ui.getMenu().isMenuActive() || ui.isAltPressed()) {
        char *insert = *text;
        *insert = 0;
    }
}

void data_ui::init() {
    window = elm_win_util_standard_add("Main", _("Data"));
    elm_win_autodel_set(window, EINA_TRUE);
    evas_object_resize(window, DEFAULT_APP_WIDTH, DEFAULT_APP_HEIGHT);

    right_list_itc = elm_genlist_item_class_new();
    right_list_itc->item_style = "default";
    right_list_itc->func.text_get = right_list_text_get_cb;
    right_list_itc->func.content_get = nullptr;
    right_list_itc->func.state_get = nullptr;
    right_list_itc->func.del = nullptr;

    Evas_Object *panes;

    // Add an elm_panes
    panes = elm_panes_add(window);
    evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(window, panes);
    evas_object_show(panes);

    // Create right hand pane
    filterAppliedLabel = elm_label_add(window);
    elm_object_text_set(filterAppliedLabel, _("[Filter Applied]"));
    evas_object_hide(filterAppliedLabel);

    rightList = elm_genlist_add(window);
    elm_genlist_homogeneous_set(rightList, EINA_TRUE);
    elm_genlist_multi_select_set(rightList, EINA_FALSE);
    evas_object_size_hint_weight_set(rightList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(rightList, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(rightList);

    rightBox = elm_box_add(window);
    elm_box_pack_end(rightBox, rightList);

    elm_object_part_content_set(panes, "right", rightBox);

    // Created fields and search box
    Evas_Object *leftBox = elm_box_add(window);
    elm_object_part_content_set(panes, "left", leftBox);

    // Scroller for fields table
    scroller = elm_scroller_add(window);
    evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
    evas_object_show(scroller);
    elm_box_pack_end(leftBox, scroller);
    evas_object_smart_callback_add(scroller, "scroll", scroller_scroll_cb, nullptr);

    // Create fields table
    fieldsTable = elm_table_add(window);
    evas_object_size_hint_weight_set(fieldsTable, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(fieldsTable, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_padding_set(fieldsTable, 3, 0);
#if ELM_VERSION_MAJOR>=1 && ELM_VERSION_MINOR>=20
    elm_table_align_set(fieldsTable, 0, 0);
#endif
    elm_table_homogeneous_set(fieldsTable, EINA_FALSE);
    evas_object_show(fieldsTable);
    elm_object_content_set(scroller, fieldsTable);

    // Create search box
    Evas_Object *searchBox = elm_box_add(window);
    elm_box_horizontal_set(searchBox, EINA_TRUE);
    elm_bg_color_set(searchBox, 100, 1, 1);
    evas_object_size_hint_weight_set(searchBox, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(searchBox, 0, 1);
    evas_object_show(searchBox);

    Evas_Object *searchLabel = elm_label_add(window);
    elm_object_text_set(searchLabel, _("Find: "));
    evas_object_size_hint_weight_set(searchLabel, 0, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(searchLabel, 0, EVAS_HINT_FILL);
    elm_box_pack_end(searchBox, searchLabel);
    evas_object_show(searchLabel);

    searchEntry = elm_entry_add(window);
    elm_entry_single_line_set(searchEntry, EINA_TRUE);
    elm_entry_editable_set(searchEntry, EINA_TRUE);
    evas_object_size_hint_weight_set(searchEntry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(searchEntry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_entry_markup_filter_append(searchEntry, entry_filter_out_unwanted_cb, nullptr);
    elm_box_pack_end(searchBox, searchEntry);
    elm_object_focus_set(searchEntry, EINA_TRUE);
    evas_object_show(searchEntry);

    Evas_Object *prevButton = elm_button_add(window);
    elm_object_text_set(prevButton, _("Prev"));
    evas_object_size_hint_weight_set(prevButton, 0, 0);
    evas_object_size_hint_align_set(prevButton, 1, 1);
    elm_box_pack_end(searchBox, prevButton);
    evas_object_show(prevButton);
    evas_object_smart_callback_add(prevButton, "clicked", previous_button_cb, nullptr);

    Evas_Object *nextButton = elm_button_add(window);
    elm_object_text_set(nextButton, _("Next"));
    evas_object_size_hint_weight_set(nextButton, 0, 0);
    evas_object_size_hint_align_set(nextButton, 1, 1);
    elm_box_pack_end(searchBox, nextButton);
    evas_object_show(nextButton);
    evas_object_smart_callback_add(nextButton, "clicked", next_button_cb, nullptr);

    elm_box_pack_end(leftBox, searchBox);

    elm_panes_content_left_size_set(panes, 0.7);

    evas_object_event_callback_add(searchEntry, EVAS_CALLBACK_KEY_DOWN, window_cb_key_down, searchEntry);
    evas_object_event_callback_add(searchEntry, EVAS_CALLBACK_KEY_UP, window_cb_key_up, searchEntry);
    elm_object_focus_allow_set(window, EINA_FALSE);
    elm_object_focus_allow_set(panes, EINA_FALSE);
    elm_object_focus_allow_set(scroller, EINA_FALSE);
    elm_object_focus_allow_set(rightList, EINA_FALSE);
    elm_object_focus_allow_set(fieldsTable, EINA_FALSE);
    elm_object_focus_allow_set(leftBox, EINA_FALSE);
    elm_object_focus_allow_set(searchBox, EINA_FALSE);
    elm_object_focus_allow_set(searchLabel, EINA_FALSE);
    elm_object_focus_allow_set(prevButton, EINA_FALSE);
    elm_object_focus_allow_set(nextButton, EINA_FALSE);

    elm_win_center(window, EINA_TRUE, EINA_TRUE);
    evas_object_show(window);

    menu.init(window);
}

void data_ui::setSqliteFile(std::string fileName) {
    db.file(fileName);
    db.setTable("");

    elm_win_title_set(window, getTitleForFileName(fileName).c_str());

    repopulateUI();

    clearFocus();
}

std::string data_ui::getTitleForFileName(const std::string &fileName) const {
    return (_("Data: ") + fileName);
}

void data_ui::setNewFile() {
    ui.clearActivePopup();
    if (newFileName.empty()) {
#if ELM_VERSION_MAJOR >= 1 && ELM_VERSION_MINOR >= 20
        newFileName = eina_environment_home_get();
#else
        newFileName = getenv("HOME");
#endif
        newFileName += "/data.db";
    }
    db.newFile(newFileName);
    db.setTable("");

    elm_win_title_set(window, getTitleForFileName(newFileName).c_str());

    repopulateUI();

    clearFocus();
}

void data_ui::repopulateFieldsTable() {
    elm_table_clear(fieldsTable, EINA_TRUE);
    auto cols = db.listColumns();

    if (selectedRow > db.rowCount()) {
        selectedRow = 0;
    }
    currentEditors.clear();
    currentArrows.clear();
    for (int i = 0; i < (int)cols.size(); i++) {
        auto field_name = elm_label_add(fieldsTable);
        elm_object_text_set(field_name, cols[i].c_str());
        evas_object_size_hint_align_set(field_name, 1, 0);
        evas_object_size_hint_padding_set(field_name, 2, 0, 2, 2);
        evas_object_show(field_name);
        elm_table_pack(fieldsTable, field_name, 0, i, 1, 1);

        auto arrowImage = elm_image_add(fieldsTable);
        std::string arrowPath(elm_app_data_dir_get());
        arrowPath += "/images/arrow.png";
        elm_image_file_set(arrowImage, arrowPath.c_str(), nullptr);
        evas_object_size_hint_align_set(arrowImage, 1, 0);
        evas_object_size_hint_padding_set(arrowImage, 0, 0, 5, 5);
        evas_object_size_hint_min_set(arrowImage, static_cast<Evas_Coord>(10 * elm_config_scale_get()),
                                      static_cast<Evas_Coord>(10 * elm_config_scale_get()));
        if (i == currentEditorWithCursorIndex) {
            evas_object_show(arrowImage);
        } else {
            evas_object_hide(arrowImage);
        }
        elm_table_pack(fieldsTable, arrowImage, 1, i, 1, 1);
        currentArrows.push_back(arrowImage);

        if (selectedRow) {
            auto row = db.readRow(selectedRow - 1);
            auto field_value = elm_entry_add(fieldsTable);
            auto text = elm_entry_utf8_to_markup(row[i].c_str());
            elm_object_text_set(field_value, text);
            free(text);
            elm_entry_single_line_set(field_value, EINA_FALSE);
            elm_entry_editable_set(field_value, EINA_FALSE);
            evas_object_size_hint_weight_set(field_value, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(field_value, EVAS_HINT_FILL, EVAS_HINT_FILL);
            elm_object_focus_allow_set(field_value, EINA_FALSE);
            evas_object_show(field_value);
            elm_table_pack(fieldsTable, field_value, 2, i, 1, 1);
            evas_object_smart_callback_add(field_value, "clicked", edit_entry_clicked_cb, (void *) (uintptr_t) i);

            currentEditors.push_back(field_value);
        }
    }
    menu.updateMenuStates(selectedRow);
}

void data_ui::repopulateRightList(int selected) const {
    elm_genlist_clear(rightList);

    for (int i = 0; i < db.rowCount(); i++) {
        auto item = elm_genlist_item_append(rightList,
                                right_list_itc,
                                (void *) (uintptr_t) i,  // Item data
                                nullptr,                 // Parent item for trees, NULL if none
                                ELM_GENLIST_ITEM_NONE,   // Item type; this is the common one
                                row_selected_cb,         // Callback on selection of the item
                                (void *) (uintptr_t) (i + 1)    // Data for that callback function
        );
        if (i==selected) {
            elm_genlist_item_selected_set(item, EINA_TRUE);
            elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
        }
    }
    if (db.getFilter().length() > 0) {
        if (!evas_object_visible_get(filterAppliedLabel)) {
            evas_object_show(filterAppliedLabel);
            elm_box_pack_start(rightBox, filterAppliedLabel);
        }
    } else {
        evas_object_hide(filterAppliedLabel);
        elm_box_unpack(rightBox, filterAppliedLabel);
    }
}

void data_ui::rowSelected(int i) {
    selectedRow = i;

    repopulateFieldsTable();
}

static void file_open_exit_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    evas_object_del((Evas_Object *) data);

    ui.clearFocus();
}

static void file_open_sqlite_ok_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info) {
    evas_object_del((Evas_Object *) data);

    if (event_info) {
        struct stat s{};
        if (stat((const char *) event_info, &s) == 0) {
            if (s.st_mode & S_IFREG) {
                ui.setSqliteFile((const char *) event_info);
            }
        }
    }
}

static void file_open_csv_ok_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info) {
    evas_object_del((Evas_Object *) data);

    if (event_info) {
        struct stat s{};
        if (stat((const char *) event_info, &s) == 0) {
            if (s.st_mode & S_IFREG) {
                ui.setCsvFile((const char *) event_info);
                ui.newFile(true);
            }
        }
    }
}

Evas_Object *data_ui::standardFileOpener(Evas_Object *win, const char *title, Evas_Smart_Cb okFunc) {

    elm_win_title_set(win, title);
    elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
    evas_object_smart_callback_add(win, "delete,request", file_open_exit_cb, win);

    Evas_Object *fs = elm_fileselector_add(win);
    evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(fs, "delete,request", file_open_exit_cb, win);
    evas_object_smart_callback_add(fs, "done", okFunc, win);
    evas_object_show(fs);

    elm_fileselector_expandable_set(fs, EINA_TRUE);
    elm_fileselector_folder_only_set(fs, EINA_FALSE);
#if ELM_VERSION_MAJOR >= 1 && ELM_VERSION_MINOR >= 20
    elm_fileselector_path_set(fs, eina_environment_home_get());
    elm_fileselector_sort_method_set(fs, ELM_FILESELECTOR_SORT_BY_FILENAME_ASC);
#else
    elm_fileselector_path_set(fs, getenv("HOME"));
#endif
    return fs;
}

void data_ui::showFileOpener(Evas_Object *win, Evas_Object *fs) {
    elm_win_resize_object_add(win, fs);
    evas_object_resize(win, DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_HEIGHT);
    elm_win_center(win, EINA_TRUE, EINA_TRUE);
    evas_object_show(win);
}

void data_ui::openFile() {
    Evas_Object *win = elm_win_add(window, "settings", ELM_WIN_BASIC);
    if (!win) return;

    Evas_Object *fs = standardFileOpener(win, _("Open File"), file_open_sqlite_ok_cb);

    //http://fileformats.archiveteam.org/wiki/DB_(SQLite)
    elm_fileselector_mime_types_filter_append(fs, "application/x-sqlite3", "");

    showFileOpener(win, fs);
}

void data_ui::importCsv() {
    Evas_Object *win = elm_win_add(window, "import", ELM_WIN_BASIC);
    if (!win) return;

    Evas_Object *fs = standardFileOpener(win, _("Import CSV File"), file_open_csv_ok_cb);

    //http://fileformats.archiveteam.org/wiki/CSV
    elm_fileselector_mime_types_filter_append(fs, "text/csv", "");
    elm_fileselector_mime_types_filter_append(fs, "application/csv", "");
    elm_fileselector_mime_types_filter_append(fs, "text/comma-separated-values", "");

    showFileOpener(win, fs);
}

static void file_export_exit_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.clearActivePopup();
    ui.clearFocus();
}

static void file_export_ok_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.clearActivePopup();
    ui.exportFile();
}

static void file_export_key_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    EINA_LOG_INFO("KeyUp: %s - %s - %s", ev->key, ev->compose, ev->string);
    if (!strcmp(ev->key, "Escape")) {
        file_export_exit_cb(data, obj, event_info);
    } else if (!strcmp(ev->key, "Return")) {
        file_export_ok_cb(data, obj, event_info);
    }
#if ELM_VERSION_MAJOR>=1 && ELM_VERSION_MINOR>=20
    std::string newFilePath = eina_environment_home_get();
#else
    std::string newFilePath = "/tmp";
#endif
    newFilePath += "/";
    newFilePath += elm_object_text_get(obj);
    std::regex fileType("(csv$)");
    if (!std::regex_search(newFilePath, fileType)) {
        newFilePath += ".csv";
    }
    ui.updateExportFileName(newFilePath);
}

void data_ui::exportCsv() {
    Evas_Object *popup = elm_popup_add(window);
    elm_object_part_text_set(popup, "title,text", _("Export CSV to File"));

    Evas_Object *input = elm_entry_add(popup);
    elm_entry_single_line_set(input, EINA_TRUE);
    elm_entry_editable_set(input, EINA_TRUE);
    elm_entry_scrollable_set(input, EINA_TRUE);
    evas_object_size_hint_weight_set(input, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(input, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_event_callback_add(input, EVAS_CALLBACK_KEY_UP, file_export_key_up_cb, nullptr);
    evas_object_show(input);
    elm_object_content_set(popup, input);

    Evas_Object *button = elm_button_add(popup);
    elm_object_text_set(button, _("Cancel"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", file_export_exit_cb, popup);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("OK"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button2", button);
    evas_object_smart_callback_add(button, "clicked", file_export_ok_cb, popup);

    showPopup(popup, input);
}

static void file_new_exit_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.clearActivePopup();
    ui.clearFocus();
}

static void file_new_ok_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.clearActivePopup();
    ui.setNewFile();
}

static void file_import_new_ok_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.clearActivePopup();
    ui.setNewFile();
    ui.importCsvFile();
}

static void file_new_key_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    EINA_LOG_INFO("KeyUp: %s - %s - %s", ev->key, ev->compose, ev->string);
    if (!strcmp(ev->key, "Escape")) {
        file_new_exit_cb(data, obj, event_info);
    } else if (!strcmp(ev->key, "Return")) {
        auto forImport = (bool)(uintptr_t) data;
        if (forImport) {
            file_import_new_ok_cb(data, obj, event_info);
        } else {
            file_new_ok_cb(data, obj, event_info);
        }
    }
#if ELM_VERSION_MAJOR>=1 && ELM_VERSION_MINOR>=20
    std::string newFilePath = eina_environment_home_get();
#else
    std::string newFilePath = "/tmp";
#endif
    newFilePath += "/";
    newFilePath += elm_object_text_get(obj);
    std::regex fileType("(db$|sqlite$|sqlite3$)");
    if (!std::regex_search(newFilePath, fileType)) {
        newFilePath += ".db";
    }
    ui.updateNewFileName(newFilePath);
}

void data_ui::newFile(bool forImport) {
    Evas_Object *popup = elm_popup_add(window);
    if (forImport) {
        elm_object_part_text_set(popup, "title,text", _("Import to New File"));
    } else {
        elm_object_part_text_set(popup, "title,text", _("New File"));
    }

    Evas_Object *input = elm_entry_add(popup);
    elm_entry_single_line_set(input, EINA_TRUE);
    elm_entry_editable_set(input, EINA_TRUE);
    elm_entry_scrollable_set(input, EINA_TRUE);
    evas_object_size_hint_weight_set(input, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(input, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_event_callback_add(input, EVAS_CALLBACK_KEY_UP, file_new_key_up_cb, (void *) (uintptr_t) forImport);
    evas_object_show(input);
    elm_object_content_set(popup, input);

    Evas_Object *button = elm_button_add(popup);
    elm_object_text_set(button, _("Cancel"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", file_new_exit_cb, popup);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("OK"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button2", button);
    if (forImport) {
        evas_object_smart_callback_add(button, "clicked", file_import_new_ok_cb, popup);
    } else {
        evas_object_smart_callback_add(button, "clicked", file_new_ok_cb, popup);
    }

    showPopup(popup, input);
}

void data_ui::popupStackPush(Evas_Object *toPush, Evas_Object *focusOn) {
    if (popupStack.topIndex == (STACKMAXSIZE - 1)) {
        EINA_LOG_INFO("PopupStackFull - Push requested");
        return;
    } else {
        popupStack.topIndex++;
        popupStack.sObject[popupStack.topIndex] = toPush;
        popupStack.sFocus[popupStack.topIndex] = focusOn;
    }
}

Evas_Object *data_ui::popupStackPop() {
    Evas_Object *popped;
    Evas_Object *newFocus = nullptr;
    if (popupStackEmpty()) {
        EINA_LOG_INFO("PopupStackEmpty - Pop requested");
        return (nullptr);
    } else {
        Evas_Object *oldFocus = popupStackTopFocus();
        if (oldFocus) {
            elm_object_focus_set(oldFocus, EINA_FALSE);
            elm_object_focus_allow_set(oldFocus, EINA_FALSE);
        }
        popped = popupStack.sObject[popupStack.topIndex];
        popupStack.topIndex--;
        newFocus = popupStackTopFocus();
        if (newFocus) {
            elm_object_focus_allow_set(newFocus, EINA_TRUE);
            elm_object_focus_set(newFocus, EINA_TRUE);
            auto type = elm_object_widget_type_get(newFocus);
            EINA_LOG_INFO("New focus type: %s",type);
            if (!strcmp(type, "Elm.Entry")) {
                elm_entry_cursor_line_end_set(newFocus);
            }
        } else {
            //Once were out of a popup stack tree go back to using the search entry as focus holder
            elm_object_focus_allow_set(searchEntry, EINA_TRUE);
            elm_object_focus_set(searchEntry, EINA_TRUE);
            elm_entry_cursor_line_end_set(searchEntry);
        }
    }
    return (popped);
}

bool data_ui::popupStackEmpty() {
    return popupStack.topIndex == - 1;
}

Evas_Object *data_ui::popupStackTopFocus() {
    if (popupStackEmpty()) {
        EINA_LOG_INFO("PopupStackEmpty - Top requested");
        return (nullptr);
    } else {
        return popupStack.sFocus[popupStack.topIndex];
    }
}

static Eina_Bool delayed_set_focus(void *data) {
    auto *focusOn = static_cast<Evas_Object *>(data);
    elm_object_focus_allow_set(focusOn, EINA_TRUE);

    auto type = elm_object_widget_type_get(focusOn);
    EINA_LOG_INFO("FocusOn type: %s",type);
    if (!strcmp(type, "Elm.Entry")) {
        elm_entry_cursor_line_end_set(focusOn);
    }
    elm_object_focus_set(focusOn, EINA_TRUE);
    return ECORE_CALLBACK_CANCEL;
}

void data_ui::showPopup(Evas_Object *popup, Evas_Object *focusOn) {
    auto oldFocus = popupStackTopFocus();
    if (oldFocus) {
        elm_object_focus_allow_set(oldFocus, EINA_FALSE);
        elm_object_focus_set(oldFocus, EINA_FALSE);
    }
    popupStackPush(popup, focusOn);
    if (popup) {
        evas_object_show(popup);
    }
    if (focusOn) {
        ecore_timer_add(0.3, delayed_set_focus, focusOn);
    }
}

void data_ui::updateNewFileName(std::string fileName) {
    newFileName = std::move(fileName);
}

void data_ui::clearFocus() {
    elm_object_focus_set(searchEntry, EINA_TRUE);
}

static void delete_entry_exit_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.clearActivePopup();
    ui.clearFocus();
}

static void delete_entry_ok_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED) {
    ui.clearActivePopup();
    ui.deleteCurrentRow();
    ui.clearFocus();
}

static void delete_entry_key_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    auto ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");
    EINA_LOG_ERR("KeyUp: %s - %s - %s", ev->key, ev->compose, ev->string);
    if (!strcmp(ev->key, "Escape")) {
        delete_entry_exit_cb(data, obj, event_info);
    } else if (!strcmp(ev->key, "Return")) {
        delete_entry_ok_cb(data, obj, event_info);
    } else if (ctrl && !strcmp(ev->key, "s")) {
        delete_entry_ok_cb(data, obj, event_info);
    }
}

void data_ui::deleteEntry() {
    Evas_Object *popup = elm_popup_add(window);
    elm_object_part_text_set(popup, "title,text", _("Delete Entry"));

    Evas_Object *label = elm_label_add(popup);
    elm_object_text_set(label, _("Are you sure?"));
    elm_object_focus_allow_set(label, EINA_FALSE);
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(label);
    elm_object_content_set(popup, label);

    Evas_Object *button = elm_button_add(popup);
    elm_object_text_set(button, _("Cancel"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", delete_entry_exit_cb, popup);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("OK"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button2", button);
    evas_object_smart_callback_add(button, "clicked", delete_entry_ok_cb, popup);

    elm_object_focus_allow_set(label, EINA_FALSE);
    evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, delete_entry_key_down_cb, popup);

    showPopup(popup, popup);
}

void data_ui::updateCurrentRowValue(int currentRow, std::string value) {
    dataEditRecord.updateCurrentRowValue(currentRow, std::move(value));
}

void data_ui::saveCurrentRow() {
    dataEditRecord.saveCurrentRow();
}

void data_ui::addRow() {
    dataEditRecord.addRow(window);
}

void data_ui::editRow() {
    dataEditRecord.editRow(window,selectedRow);
}

void data_ui::deleteCurrentRow() {
    auto rows = db.readRow(selectedRow - 1);
    db.deleteRow(rows);
    repopulateUI();
}

void data_ui::prevButton() {
    auto filter = elm_object_text_get(searchEntry);
    if (db.getFilter() != filter) {
        db.setFilter(filter);

        repopulateUI();
    }

    if (currentEditors.empty()) {
        return;
    }

    std::string toFind = filter;
    std::transform(toFind.begin(), toFind.end(), toFind.begin(), ::tolower);
    int currentEditor = editorSelectionBeganIn;
    bool stringFound = false;
    do {
        do {
            auto editor = currentEditors[currentEditor];
            auto utf8text = elm_entry_markup_to_utf8(elm_entry_entry_get(editor));
            std::string text = utf8text;
            free(utf8text);
            std::transform(text.begin(), text.end(), text.begin(), ::tolower);
            auto startAt = static_cast<int>(text.length());
            if (currentEditor == editorSelectionBeganIn) {
                startAt = editorSelectionBeganAt - 1;
            }
            auto pos = text.rfind(toFind, static_cast<unsigned long>(startAt));
            if (pos != std::string::npos) {
                editorSelectionEndIn = editorSelectionBeganIn = currentEditor;
                editorSelectionBeganAt = static_cast<int>(pos);
                editorSelectionEndAt = static_cast<int>(pos + strlen(filter));
                stringFound = true;
            } else {
                currentEditor--;
            }
        } while (!stringFound && (currentEditor < (int)currentEditors.size()));
        if (!stringFound) {
            if (prevItem()) {
                currentEditor = static_cast<int>(currentEditors.size() - 1);
                editorSelectionEndIn = 0;
                editorSelectionEndAt = 0;
            }
        }
    } while (!stringFound && (currentEditor < (int)currentEditors.size()));
    updateEditorSelection();
    updateFoundItemDisplay(stringFound, currentEditor);
}

void data_ui::nextButton() {
    auto filter = elm_object_text_get(searchEntry);
    if (db.getFilter() != filter) {
        db.setFilter(filter);

        repopulateUI();
    }

    if (currentEditors.empty()) {
        return;
    }

    std::string toFind = filter;
    std::transform(toFind.begin(), toFind.end(), toFind.begin(), ::tolower);
    int currentEditor = editorSelectionEndIn;
    bool stringFound = false;
    do {
        do {
            auto editor = currentEditors[currentEditor];
            auto utf8text = elm_entry_markup_to_utf8(elm_entry_entry_get(editor));
            std::string text = utf8text;
            free(utf8text);
            std::transform(text.begin(), text.end(), text.begin(), ::tolower);
            int startAt = 0;
            if (currentEditor == editorSelectionEndIn) {
                startAt = editorSelectionEndAt;
            }
            auto pos = text.find(toFind, static_cast<unsigned long>(startAt));
            if (pos != std::string::npos) {
                editorSelectionEndIn = editorSelectionBeganIn = currentEditor;
                editorSelectionBeganAt = static_cast<int>(pos);
                editorSelectionEndAt = static_cast<int>(pos + strlen(filter));
                stringFound = true;
            } else {
                currentEditor++;
            }
        } while (!stringFound && (currentEditor < (int)currentEditors.size()));
        if (!stringFound) {
            if (nextItem()) {
                currentEditor = 0;
                editorSelectionEndIn = 0;
                editorSelectionEndAt = 0;
            }
        }
    } while (!stringFound && (currentEditor < (int)currentEditors.size()));
    updateEditorSelection();
    updateFoundItemDisplay(stringFound, currentEditor);
}

void data_ui::repopulateUI() {
    auto it = elm_genlist_selected_item_get(rightList);
    auto data = elm_object_item_data_get(it);
    auto selectedIndex = (int) (uintptr_t) data;

    repopulateFieldsTable();
    repopulateRightList(selectedIndex);
}

void data_ui::labelPreferences() {
    renames.clear();
    data_label_preferences label_preferences;
    label_preferences.show(window);
}

void data_ui::tablePreferences() {
    table_preferences.clearTablePreferences();
    table_preferences.show(window);
}

void data_ui::saveLabelPreferences() {
    db.setColumns(editableColumns, renames);
    editableColumns.clear();
}

void data_ui::clearLabelPreferences() {
    editableColumns.clear();
}

std::vector<std::string> data_ui::getEditableColumns() {
    if (editableColumns.empty()) {
        editableColumns = db.listColumns();
    }
    return editableColumns;
}

void data_ui::editColumnMoveUp() {
    auto index = editableColumnsIndex;
    if (index > 0) {
        std::swap(editableColumns[index], editableColumns[index - 1]);
        editableColumnsIndex--;
    }
}

void data_ui::editColumnMoveDown() {
    auto index = editableColumnsIndex;
    if (index < (int)editableColumns.size()-1) {
        std::swap(editableColumns[index], editableColumns[index + 1]);
        editableColumnsIndex++;
    }
}

void data_ui::editColumnDelete() {
    auto index = editableColumnsIndex;
    if (index >= 0 && index < (int)editableColumns.size()) {
        editableColumns.erase(editableColumns.begin() + index);
        if (index > 0) {
            editableColumnsIndex--;
        }
    }
}

void data_ui::setEditColumnSelection(int i) {
    editableColumnsIndex = i;
    updateEditLabel(editableColumns[i]);
}

int data_ui::getEditColumnSelection() {
    return editableColumnsIndex;
}

void data_ui::updateEditLabel(std::string label) {
    editableColumnsEditLabel = std::move(label);
}

void data_ui::addEditableLabel() {
    auto index = editableColumnsIndex;
    editableColumns.insert(editableColumns.begin()+index, editableColumnsEditLabel);
}

std::string data_ui::getEditLabel() {
    return editableColumnsEditLabel;
}

void data_ui::saveEditableLabel() {
    if (editableColumns[editableColumnsIndex] != editableColumnsEditLabel) {
        if (renames.find(editableColumns[editableColumnsIndex]) != renames.end()) {
            renames[editableColumnsEditLabel] = renames.find(editableColumns[editableColumnsIndex])->second;
            renames.erase(editableColumns[editableColumnsIndex]);
        } else {
            renames[editableColumnsEditLabel] = editableColumns[editableColumnsIndex];
        }
        editableColumns[editableColumnsIndex] = editableColumnsEditLabel;
    }
}

Eina_Bool data_ui::labelPreferencesAreValid() {
    std::unordered_set<std::string> validCheck;
    for (const auto &column : editableColumns) {
        auto search = validCheck.find(column);
        if(search == validCheck.end()) {
            validCheck.insert(column);
        } else {
            return EINA_FALSE;
        }
    }

    return EINA_TRUE;
}

data_ui::data_ui(sqlite_file &_db) : db(_db), dataEditRecord(_db), table_preferences(_db) {

}

void data_ui::editColumnTabFocus(int currentFocus) {
    dataEditRecord.editColumnTabFocus(currentFocus);
}

void data_ui::zoomOut() {
    auto scale = elm_config_scale_get();
    elm_config_scale_set(scale-0.1);
}

void data_ui::zoomIn() {
    auto scale = elm_config_scale_get();
    elm_config_scale_set(scale+0.1);

}

void data_ui::copy() {
    if (elm_entry_selection_get(searchEntry)) {
        //handled automatically by searchEntry as it has the focus
    } else {
        for (auto field_value : currentEditors) {
            if (elm_entry_selection_get(field_value)) {
                elm_entry_selection_copy(field_value);
                EINA_LOG_ERR("copy %s",elm_entry_selection_get(field_value));
            }
        }
    }
}

void data_ui::cut() {
    //handled automatically by searchEntry as it has the focus, cannot be done on the editor field values
}

void data_ui::paste() {
    //handled automatically by searchEntry as it has the focus
}

void data_ui::cursorUp(Eina_Bool shift) {
    if (currentEditorWithCursorIndex >= 0 && currentEditorWithCursorIndex < (int)currentEditors.size()) {
        auto editor = currentEditors[currentEditorWithCursorIndex];
        auto oldPos = elm_entry_cursor_pos_get(editor);
        if (shift) {
            if (!editorSelectionActive) {
                editorSelectionActive = true;
                editorSelectionBeganIn = currentEditorWithCursorIndex;
                editorSelectionBeganAt = oldPos;
                editorSelectionEndIn = -1;
                editorSelectionEndAt = -1;
            }
        } else {
            editorSelectionActive = false;
            editorSelectionBeganIn = -1;
            editorSelectionBeganAt = -1;
            editorSelectionEndIn = -1;
            editorSelectionEndAt = -1;
        }
        elm_entry_cursor_up(editor);
        auto newPos = elm_entry_cursor_pos_get(editor);
        if (newPos == oldPos && currentEditorWithCursorIndex > 0) {
            updateCurrentEditorAndArrowVisibilityForIndex(currentEditorWithCursorIndex-1);
            if (editorSelectionActive) {
                elm_entry_cursor_end_set(editor);
                elm_entry_cursor_line_begin_set(editor);
                editorSelectionEndAt = elm_entry_cursor_pos_get(editor);
                elm_entry_cursor_pos_set(editor, newPos);
            }
        } else if (newPos == oldPos) {
            if (editorSelectionActive) {
                editorSelectionEndAt = 0;
            }
        } else {
            if (editorSelectionActive) {
                editorSelectionEndAt = newPos;
            }
        }
        if (editorSelectionActive) {
            editorSelectionEndIn = currentEditorWithCursorIndex;
        }
        updateScrollPositions();
        updateArrowGeometry();
    }
    updateEditorSelection();

}

void data_ui::updateEditorSelection() {
//    EINA_LOG_ERR("a: %d, bi: %d, ba: %d, ei: %d, ea: %d",editorSelectionActive,editorSelectionBeganIn,editorSelectionBeganAt,editorSelectionEndIn,editorSelectionEndAt);
    if (editorSelectionBeganIn >= 0 && !currentEditors.empty()) {
        if (editorSelectionBeganIn > editorSelectionEndIn) {
            for (int editorIndex = editorSelectionEndIn; editorIndex <= editorSelectionBeganIn; editorIndex++) {
                auto editor = currentEditors[editorIndex];
                int originalPos = elm_entry_cursor_pos_get(editor);
                int beginPos = 0;
                if (editorIndex == editorSelectionEndIn) {
                    beginPos = editorSelectionEndAt;
                }
                if (beginPos != editorSelectionBeganAt || editorIndex != editorSelectionBeganIn) {
                    elm_entry_cursor_pos_set(editor, beginPos);
                    elm_entry_cursor_selection_begin(editor);
                    if (editorIndex == editorSelectionBeganIn) {
                        elm_entry_cursor_pos_set(editor, editorSelectionBeganAt);
                    } else {
                        elm_entry_cursor_end_set(editor);
                    }
                    elm_entry_cursor_selection_end(editor);
                    elm_entry_cursor_pos_set(editor, originalPos);
                    return;
                }
            }
        }
        for (int editorIndex = editorSelectionBeganIn; editorIndex <= editorSelectionEndIn; editorIndex++) {
            auto editor = currentEditors[editorIndex];
            int originalPos = elm_entry_cursor_pos_get(editor);
            int beginPos = 0;
            if (editorIndex == editorSelectionBeganIn) {
                beginPos = editorSelectionBeganAt;
            }
            if (beginPos != editorSelectionEndAt || editorIndex != editorSelectionEndIn) {
                elm_entry_cursor_pos_set(editor, beginPos);
                elm_entry_cursor_selection_begin(editor);
                if (editorIndex == editorSelectionEndIn) {
                    elm_entry_cursor_pos_set(editor, editorSelectionEndAt);
                } else {
                    elm_entry_cursor_end_set(editor);
                }
                elm_entry_cursor_selection_end(editor);
                elm_entry_cursor_pos_set(editor, originalPos);
            }
        }
    }
}

void data_ui::cursorDown(Eina_Bool shift) {
    if (currentEditorWithCursorIndex >= 0 && currentEditorWithCursorIndex < (int)currentEditors.size()) {
        auto editor = currentEditors[currentEditorWithCursorIndex];
        auto oldPos = elm_entry_cursor_pos_get(editor);
        if (shift) {
            if (!editorSelectionActive) {
                editorSelectionActive = true;
                editorSelectionBeganIn = currentEditorWithCursorIndex;
                editorSelectionBeganAt = oldPos;
                editorSelectionEndIn = -1;
                editorSelectionEndAt = -1;
            }
        } else {
            editorSelectionActive = false;
            editorSelectionBeganIn = -1;
            editorSelectionBeganAt = -1;
            editorSelectionEndIn = -1;
            editorSelectionEndAt = -1;
        }
        elm_entry_cursor_down(editor);
        auto newPos = elm_entry_cursor_pos_get(editor);
        if (newPos == oldPos && currentEditorWithCursorIndex < (int)currentEditors.size()-1) {
            updateCurrentEditorAndArrowVisibilityForIndex(currentEditorWithCursorIndex+1);
            if (editorSelectionActive) {
                editorSelectionEndAt = 0;
            }
        } else if (newPos == oldPos) {
            if (editorSelectionActive) {
                elm_entry_cursor_end_set(editor);
                editorSelectionEndAt = elm_entry_cursor_pos_get(editor);
                elm_entry_cursor_pos_set(editor, newPos);
            }
        } else {
            if (editorSelectionActive) {
                editorSelectionEndAt = newPos;
            }
        }
        if (editorSelectionActive) {
            editorSelectionEndIn = currentEditorWithCursorIndex;
        }
        updateScrollPositions();
        updateArrowGeometry();
    }
    updateEditorSelection();
}

void data_ui::updateArrowGeometry() {
    auto editor = currentEditors[currentEditorWithCursorIndex];
    int cx,cy,cw,ch;
    if (elm_entry_cursor_geometry_get(editor, &cx, &cy, &cw, &ch)) {
        auto arrowImage = currentArrows[currentEditorWithCursorIndex];
//        Evas_Coord ax,ay,aw,ah;
//        evas_object_geometry_get(arrowImage, &ax, &ay, &aw, &ah);
        Evas_Coord ex,ey,ew,eh;
        evas_object_geometry_get(editor, &ex, &ey, &ew, &eh);
//        EINA_LOG_ERR("cx: %d, cy: %d, cw: %d, ch: %d",cx,cy,cw,cy);
//        EINA_LOG_ERR("ex: %d, ey: %d, ew: %d, eh: %d",ex,ey,ew,ey);
//        EINA_LOG_ERR("ax: %d, ay: %d, aw: %d, ah: %d",ax,ay,aw,ay);
//        evas_object_geometry_set(arrowImage, ax, ey+cy, aw, ah);

        evas_object_size_hint_padding_set(arrowImage, 0, 0, cy, 5);
    }
}

void data_ui::cursorLeft(Eina_Bool shift EINA_UNUSED) {
    auto pos = elm_entry_cursor_pos_get(searchEntry);
    if (pos == oldSearchEntryPos) {
        prevItem();
        clearFocus();
    }
    oldSearchEntryPos = pos;
}

void data_ui::cursorRight(Eina_Bool shift EINA_UNUSED) {
//    auto empty = elm_entry_is_empty(searchEntry);

    auto pos = elm_entry_cursor_pos_get(searchEntry);
    if (pos == oldSearchEntryPos) {
        nextItem();
        clearFocus();
    }
    oldSearchEntryPos = pos;
}

data_table_preferences& data_ui::getTablePref() {
    return table_preferences;
}

void data_ui::exportFile() {
    csv_file::exportCsv(db, exportFileName);
}

void data_ui::updateExportFileName(std::string fileName) {
    exportFileName = std::move(fileName);
}

void data_ui::importCsvFile() {
    csv_file::importCsv(db, importFileName);
    repopulateUI();
}

void data_ui::setCsvFile(std::string fileName) {
    importFileName = std::move(fileName);
}

static Eina_Bool delayed_retry_scroll_pos(void *data EINA_UNUSED) {
    ui.updateScrollPositions();
    return ECORE_CALLBACK_CANCEL;
}

void data_ui::updateScrollPositions() {
    Evas_Coord scw, sch;
    elm_scroller_child_size_get(scroller, &scw, &sch);
    Evas_Coord sx,sy,sw,sh;
    elm_scroller_region_get(scroller, &sx, &sy, &sw, &sh);
    auto editor = currentEditors[currentEditorWithCursorIndex];
    int cx,cy,cw,ch;
    if (elm_entry_cursor_geometry_get(editor, &cx, &cy, &cw, &ch)) {
        Evas_Coord ex,ey,ew,eh;
        evas_object_geometry_get(editor, &ex, &ey, &ew, &eh);
//        EINA_LOG_ERR("cx: %d, cy: %d, cw: %d, ch: %d",cx,cy,cw,cy);
//        EINA_LOG_ERR("ex: %d, ey: %d, ew: %d, eh: %d",ex,ey,ew,ey);
//        EINA_LOG_ERR("sx: %d, sy: %d, sw: %d, sh: %d, scw: %d, sch: %d",sx,sy,sw,sh,scw,sch);

        if (ex == 0 && ey == 0 && ew == 0 && eh == 0) {
            ecore_timer_add(0.3, delayed_retry_scroll_pos, nullptr);
            return;
        }

        Evas_Coord newSyDown = (ey+cy+sy) - 6;
        Evas_Coord newSyUp = static_cast<Evas_Coord>((ey + cy + sy) - (sh * .9));
        if (((ey+cy+sy) > (sh*.9)) && (newSyUp > sy)) {
            sy = newSyUp;
        }
        if (newSyDown < (sy)) {
            sy = newSyDown;
        }
//        EINA_LOG_ERR("sy: %d, syU: %d, syD: %d", sy, newSyUp, newSyDown);
        elm_scroller_region_show(scroller, sx, sy, sw, sh);
    }
}

void data_ui::scrollerScrolled() {
    updateArrowGeometry();
}

void data_ui::entryClicked(int i) {
//    EINA_LOG_ERR("i: %d", i);
    editorSelectionActive = false;
    editorSelectionBeganIn = -1;
    editorSelectionBeganAt = -1;
    editorSelectionEndIn = -1;
    editorSelectionEndAt = -1;

    updateCurrentEditorAndArrowVisibilityForIndex(i);
    updateScrollPositions();
    updateArrowGeometry();
    updateEditorSelection();
}

void data_ui::updateCurrentEditorAndArrowVisibilityForIndex(int index) {
    auto arrowImage = currentArrows[currentEditorWithCursorIndex];
    evas_object_hide(arrowImage);
    currentEditorWithCursorIndex = index;
    arrowImage = currentArrows[currentEditorWithCursorIndex];
    evas_object_show(arrowImage);
}

void data_ui::updateFoundItemDisplay(bool found, int editorIndex) {
    if (found) {
        updateCurrentEditorAndArrowVisibilityForIndex(editorIndex);
        auto editor = currentEditors[currentEditorWithCursorIndex];
        elm_entry_cursor_pos_set(editor,editorSelectionBeganAt);
        updateScrollPositions();
        updateArrowGeometry();
    }
}

data_menu data_ui::getMenu() {
    return menu;
}

bool data_ui::isAltPressed() {
    return altPressed;
}
