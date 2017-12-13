//
// Created by adam on 24/11/17.
//

#include <sqlite_file.h>
#include <Elementary.h>
#include <regex>
#include <unordered_set>
#include "data_ui.h"
#include "data_label_preferences.h"

static sqlite_file db;
data_ui ui(db);

static char *right_list_text_get_cb(void *data, Evas_Object *obj, const char *part) {
    if (strcmp(part, "elm.text") == 0) {
        auto *buf = static_cast<char *>(malloc(MAX_LIST_LENGTH));
        auto row = db.readRowTitle((int) (uintptr_t) data);
        std::string toFind("<br/>");
        std::string firstLine(row.begin(), std::search(row.begin(), row.end(), toFind.begin(), toFind.end()));
        snprintf(buf, MAX_LIST_LENGTH, "%s", firstLine.c_str());

        return buf;
    } else return NULL;
}

static void row_selected_cb(void *data, Evas_Object *obj, void *event_info) {
    auto *it = (Elm_Object_Item *) event_info;
    auto i = (int) (uintptr_t) data;

    ui.rowSelected(i);
}

static void previous_button_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.prevButton();
}

static void next_button_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.nextButton();
}

static void window_cb_key_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info) {
    ui.handleKeyDown(event_info);
}

void data_ui::handleKeyDown(void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    Eina_Bool ctrl, alt, shift;

    ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");
    alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
    shift = evas_key_modifier_is_set(ev->modifiers, "Shift");

    EINA_LOG_INFO("KeyDown: %s - %s - %s", ev->key, ev->compose, ev->string);

    if (ctrl && shift) {
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
        } else if (!strcmp(ev->key, "m")) {
            zoomIn();
        }
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
        Evas_Coord x,y,w,h;
        elm_scroller_region_get(scroller, &x, &y, &w, &h);
        y -= 20;
        y = MAX(y,0);
        elm_scroller_region_show(scroller, x, y, w, h);
    } else if (!strcmp(ev->key, "Down")) {
        Evas_Coord x,y,w,h;
        elm_scroller_region_get(scroller, &x, &y, &w, &h);
        y += 20;
        elm_scroller_region_show(scroller, x, y, w, h);
    } else if (!strcmp(ev->key, "Left")) {
        auto pos = elm_entry_cursor_pos_get(searchEntry);
        if (pos == oldSearchEntryPos) {
            prevItem();
            clearFocus();
        }
        oldSearchEntryPos = pos;
    } else if (!strcmp(ev->key, "Right")) {
        auto pos = elm_entry_cursor_pos_get(searchEntry);
        if (pos == oldSearchEntryPos) {
            nextItem();
            clearFocus();
        }
        oldSearchEntryPos = pos;
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

void data_ui::nextItem() {
    auto sIt = elm_genlist_selected_item_get(rightList);
    auto it = elm_genlist_item_next_get(sIt);
    if (!it) it = elm_genlist_first_item_get(rightList);
    elm_genlist_item_selected_set(it, EINA_TRUE);
    elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
}

void data_ui::prevItem() {
    auto sIt = elm_genlist_selected_item_get(rightList);
    auto it = elm_genlist_item_prev_get(sIt);
    if (!it) it = elm_genlist_last_item_get(rightList);
    elm_genlist_item_selected_set(it, EINA_TRUE);
    elm_genlist_item_show(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
}

void data_ui::init() {
    window = elm_win_util_standard_add("Main", _("Data"));
    elm_win_autodel_set(window, EINA_TRUE);
    evas_object_resize(window, DEFAULT_APP_WIDTH, DEFAULT_APP_HEIGHT);

    right_list_itc = elm_genlist_item_class_new();
    right_list_itc->item_style = "default";
    right_list_itc->func.text_get = right_list_text_get_cb;
    right_list_itc->func.content_get = NULL;
    right_list_itc->func.state_get = NULL;
    right_list_itc->func.del = NULL;

    Evas_Object *panes;

    // Add an elm_panes
    panes = elm_panes_add(window);
    evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(window, panes);
    evas_object_show(panes);

    // Create right hand pane
    rightList = elm_genlist_add(window);

    elm_genlist_homogeneous_set(rightList, EINA_TRUE);
    elm_genlist_multi_select_set(rightList, EINA_FALSE);
    evas_object_size_hint_weight_set(rightList, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(rightList, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(rightList);

    elm_object_part_content_set(panes, "right", rightList);

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
    elm_box_pack_end(searchBox, searchEntry);
    elm_object_focus_set(searchEntry, EINA_TRUE);
    evas_object_show(searchEntry);

    Evas_Object *prevButton = elm_button_add(window);
    elm_object_text_set(prevButton, _("Prev"));
    evas_object_size_hint_weight_set(prevButton, 0, 0);
    evas_object_size_hint_align_set(prevButton, 1, 1);
    elm_box_pack_end(searchBox, prevButton);
    evas_object_show(prevButton);
    evas_object_smart_callback_add(prevButton, "clicked", previous_button_cb, NULL);

    Evas_Object *nextButton = elm_button_add(window);
    elm_object_text_set(nextButton, _("Next"));
    evas_object_size_hint_weight_set(nextButton, 0, 0);
    evas_object_size_hint_align_set(nextButton, 1, 1);
    elm_box_pack_end(searchBox, nextButton);
    evas_object_show(nextButton);
    evas_object_smart_callback_add(nextButton, "clicked", next_button_cb, NULL);

    elm_box_pack_end(leftBox, searchBox);

    elm_panes_content_left_size_set(panes, 0.7);

    evas_object_event_callback_add(searchEntry, EVAS_CALLBACK_KEY_DOWN, window_cb_key_down, searchEntry);
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

void data_ui::setFile(std::string fileName) {
    db.file(fileName);
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
        newFileName = "data.db";
    }
    db.newFile(newFileName);
    elm_win_title_set(window, getTitleForFileName(newFileName).c_str());

    repopulateUI();

    clearFocus();
}

void data_ui::repopulateFieldsTable() {
    elm_table_clear(fieldsTable, EINA_TRUE);
    auto cols = db.listColumns();

    for (int i = 0; i < cols.size(); i++) {
        auto field_name = elm_label_add(fieldsTable);
        elm_object_text_set(field_name, cols[i].c_str());
        evas_object_size_hint_align_set(field_name, 1, 0);
        evas_object_size_hint_padding_set(field_name, 2, 0, 2, 2);
        evas_object_show(field_name);
        elm_table_pack(fieldsTable, field_name, 0, i, 1, 1);

        if (selectedRow > db.rowCount()) {
            selectedRow = 0;
        }
        auto arrowImage = elm_image_add(fieldsTable);
        std::string arrowPath(elm_app_data_dir_get());
        arrowPath += "/images/arrow.png";
        elm_image_file_set(arrowImage, arrowPath.c_str(), nullptr);
        evas_object_size_hint_align_set(arrowImage, 1, 0);
        evas_object_size_hint_padding_set(arrowImage, 0, 0, 5, 5);
        evas_object_size_hint_min_set(arrowImage, 10 * elm_config_scale_get(), 10 * elm_config_scale_get());
        evas_object_show(arrowImage);
        elm_table_pack(fieldsTable, arrowImage, 1, i, 1, 1);

        if (selectedRow) {
            auto row = db.readRow(selectedRow - 1);
            auto field_value = elm_entry_add(fieldsTable);
            elm_object_text_set(field_value, row[i].c_str());
            elm_entry_single_line_set(field_value, EINA_FALSE);
            elm_entry_editable_set(field_value, EINA_FALSE);
            evas_object_size_hint_weight_set(field_value, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
            evas_object_size_hint_align_set(field_value, EVAS_HINT_FILL, EVAS_HINT_FILL);
            elm_object_focus_allow_set(field_value, EINA_FALSE);
            evas_object_show(field_value);
            elm_table_pack(fieldsTable, field_value, 2, i, 1, 1);
        }
    }
}

void data_ui::repopulateRightList(int selected) const {
    elm_genlist_clear(rightList);

    for (int i = 0; i < db.rowCount(); i++) {
        auto item = elm_genlist_item_append(rightList,
                                right_list_itc,
                                (void *) (uintptr_t) i,   // Item data
                                NULL,                    // Parent item for trees, NULL if none
                                ELM_GENLIST_ITEM_NONE,   // Item type; this is the common one
                                row_selected_cb,        // Callback on selection of the item
                                (void *) (uintptr_t) (i + 1)    // Data for that callback function
        );
        if (i==selected) {
            elm_genlist_item_selected_set(item, EINA_TRUE);
            elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
        }
    }
}

void data_ui::rowSelected(int i) {
    selectedRow = i;

    repopulateFieldsTable();
}

static void file_open_exit_cb(void *data, Evas_Object *obj, void *event_info) {
    evas_object_del((Evas_Object *) data);

    ui.clearFocus();
}

static void file_open_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    evas_object_del((Evas_Object *) data);

    if (event_info) {
        struct stat s;
        if (stat((const char *) event_info, &s) == 0) {
            if (s.st_mode & S_IFREG) {
                ui.setFile((const char *) event_info);
            }
        }
    }
}

void data_ui::openFile() {
    Evas_Object *win = elm_win_add(window, "settings", ELM_WIN_BASIC);
    if (!win) return;

    elm_win_title_set(win, _("Open File"));
    elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
    evas_object_smart_callback_add(win, "delete,request", file_open_exit_cb, win);

    Evas_Object *fs = elm_fileselector_add(win);
    evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(fs, "delete,request", file_open_exit_cb, win);
    evas_object_smart_callback_add(fs, "done", file_open_ok_cb, win);
    evas_object_show(fs);

    elm_fileselector_expandable_set(fs, EINA_TRUE);
    elm_fileselector_folder_only_set(fs, EINA_FALSE);
#if ELM_VERSION_MAJOR>1 && ELM_VERSION_MINOR>=20
    elm_fileselector_path_set(fs, eina_environment_home_get());
    elm_fileselector_sort_method_set(fs, ELM_FILESELECTOR_SORT_BY_FILENAME_ASC);
#else
    elm_fileselector_path_set(fs, "/home");
#endif
    //http://fileformats.archiveteam.org/wiki/DB_(SQLite)
    elm_fileselector_mime_types_filter_append(fs, "application/x-sqlite3", "");

    elm_win_resize_object_add(win, fs);
    evas_object_resize(win, DEFAULT_DIALOG_WIDTH, DEFAULT_DIALOG_HEIGHT);
    elm_win_center(win, EINA_TRUE, EINA_TRUE);
    evas_object_show(win);
}

static void file_new_exit_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
    ui.clearFocus();
}

static void file_new_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
    ui.setNewFile();
}

static void file_new_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    EINA_LOG_INFO("KeyUp: %s - %s - %s", ev->key, ev->compose, ev->string);
    if (!strcmp(ev->key, "Escape")) {
        file_new_exit_cb(data, obj, event_info);
    } else if (!strcmp(ev->key, "Return")) {
        file_new_ok_cb(data, obj, event_info);
    }
#if ELM_VERSION_MAJOR>1 && ELM_VERSION_MINOR>=20
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

void data_ui::newFile() {
    Evas_Object *popup = elm_popup_add(window);
    elm_object_part_text_set(popup, "title,text", _("New File"));

    Evas_Object *input = elm_entry_add(popup);
    elm_entry_single_line_set(input, EINA_TRUE);
    elm_entry_editable_set(input, EINA_TRUE);
    elm_entry_scrollable_set(input, EINA_TRUE);
    evas_object_size_hint_weight_set(input, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(input, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_event_callback_add(input, EVAS_CALLBACK_KEY_UP, file_new_key_up_cb, NULL);
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
    evas_object_smart_callback_add(button, "clicked", file_new_ok_cb, popup);

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
        ecore_timer_add(0.3,delayed_set_focus,focusOn);
    }
}

void data_ui::updateNewFileName(std::string fileName) {
    newFileName = std::move(fileName);
}

void data_ui::clearFocus() {
    elm_object_focus_set(searchEntry, EINA_TRUE);
}

static void delete_entry_exit_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
    ui.clearFocus();
}

static void delete_entry_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
    ui.deleteCurrentRow();
    ui.clearFocus();
}


static void delete_entry_key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info) {
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
    dataEditRecord.updateCurrentRowValue(currentRow, value);
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
}

void data_ui::nextButton() {
    auto filter = elm_object_text_get(searchEntry);
    if (db.getFilter() != filter) {
        db.setFilter(filter);

        repopulateUI();
    }
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
    if (index < editableColumns.size()-1) {
        std::swap(editableColumns[index], editableColumns[index + 1]);
        editableColumnsIndex++;
    }
}

void data_ui::editColumnDelete() {
    auto index = editableColumnsIndex;
    if (index >= 0 && index < editableColumns.size()) {
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
    editableColumnsEditLabel = label;
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
    for (auto column : editableColumns) {
        auto search = validCheck.find(column);
        if(search == validCheck.end()) {
            validCheck.insert(column);
        } else {
            return EINA_FALSE;
        }
    }

    return EINA_TRUE;
}

data_ui::data_ui(sqlite_file &_db) : db(_db), dataEditRecord(_db) {

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


