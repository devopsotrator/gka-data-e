//
// Created by adam on 15/12/17.
//

#include <unordered_set>
#include <algorithm>
#include <Elementary.h>
#include "sqlite_file.h"
#include "data_table_preferences.h"
#include "data_ui.h"

static void table_pref_list_selection_cb(void *data, Evas_Object *obj, void *event_info) {
    auto index = (int)(uintptr_t) data;
    ui.getTablePref().setCurrentTableSelection(index);
}

static void populate_list(Evas_Object *list) {
    auto active = ui.getTablePref().getActiveTable();
    auto selected = ui.getTablePref().getCurrentTableSelection();
    auto tables = ui.getTablePref().getEditableTables();
    elm_list_clear(list);
    for (int i = 0; i < tables.size(); i++) {
        std::string tableName = tables[i];
        if (i == active) {
            tableName.append(" [*]");
        }
        auto item = elm_list_item_append(list, tableName.c_str(), nullptr, nullptr, table_pref_list_selection_cb,
                                         (void *) (uintptr_t) i);
        if (i == selected) {
            elm_list_item_selected_set(item, EINA_TRUE);
        }
    }
}

static void on_elm_popup_event_dismissed(void *data, Evas *e, Evas_Object *obj, void *event_info) {
    auto list = (Evas_Object *) data;
    populate_list(list);
}

static void table_pref_addedit_table_exit_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
}

static void table_pref_edit_table_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.getTablePref().saveEditableTable();
    ui.clearActivePopup();
}

static void table_pref_add_table_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.getTablePref().addEditableTable();
    ui.clearActivePopup();
}

static void table_pref_update_table_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);

    EINA_LOG_INFO("KeyUp: %s - %s - %s", ev->key, ev->compose, ev->string);

    if (!strcmp(ev->key, "Escape")) {
        table_pref_addedit_table_exit_cb(data, obj, event_info);
    } else if (!strcmp(ev->key, "Return")) {
        bool dataBool = data;
        if (dataBool) {
            table_pref_add_table_ok_cb(data, obj, event_info);
        } else {
            table_pref_edit_table_ok_cb(data, obj, event_info);
        }
    }

    std::string table = elm_object_text_get(obj);
    ui.getTablePref().updateEditTable(table);
}

static void table_pref_edit_cb(void *data, Evas_Object *obj, void *event_info) {
    auto list = (Evas_Object *) data;

    Evas_Object *popup = elm_popup_add(list);
    elm_object_part_text_set(popup, "title,text", _("Edit Table"));

    Evas_Object *input = elm_entry_add(popup);
    elm_entry_single_line_set(input, EINA_TRUE);
    elm_entry_editable_set(input, EINA_TRUE);
    elm_entry_scrollable_set(input, EINA_TRUE);
    auto inputValue = ui.getTablePref().getEditTable();
    elm_object_text_set(input, inputValue.c_str());
    elm_entry_cursor_line_end_set(input);
    evas_object_size_hint_weight_set(input, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(input, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_event_callback_add(input, EVAS_CALLBACK_KEY_UP, table_pref_update_table_key_up_cb, NULL);
    evas_object_show(input);
    elm_object_content_set(popup, input);

    Evas_Object *button = elm_button_add(popup);
    elm_object_text_set(button, _("Cancel (esc)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", table_pref_addedit_table_exit_cb, popup);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("OK (enter)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button2", button);
    evas_object_smart_callback_add(button, "clicked", table_pref_edit_table_ok_cb, popup);

    ui.showPopup(popup,input);

    evas_object_event_callback_add(popup, EVAS_CALLBACK_HIDE, on_elm_popup_event_dismissed, list);

    //populate_list(list);
}

static void table_pref_delete_cb(void *data, Evas_Object *obj, void *event_info) {
    auto list = (Evas_Object *) data;
    ui.getTablePref().editTableDelete();
    populate_list(list);
}

static void table_pref_exit_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
    ui.getTablePref().clearTablePreferences();
    ui.clearFocus();
}

static void table_pref_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    if (ui.getTablePref().tablePreferencesAreValid()) {
        ui.clearActivePopup();
        ui.getTablePref().saveTablePreferences();
        ui.repopulateUI();
        ui.clearFocus();
    }
}

static void table_pref_add_cb(void *data, Evas_Object *obj, void *event_info) {
    auto list = (Evas_Object *) data;

    Evas_Object *popup = elm_popup_add(list);
    elm_object_part_text_set(popup, "title,text", _("Add Table"));

    Evas_Object *input = elm_entry_add(popup);
    elm_entry_single_line_set(input, EINA_TRUE);
    elm_entry_editable_set(input, EINA_TRUE);
    elm_entry_scrollable_set(input, EINA_TRUE);
    evas_object_size_hint_weight_set(input, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(input, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_event_callback_add(input, EVAS_CALLBACK_KEY_UP, table_pref_update_table_key_up_cb, (void *) true);
    evas_object_show(input);
    elm_object_content_set(popup, input);

    Evas_Object *button = elm_button_add(popup);
    elm_object_text_set(button, _("Cancel (esc)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", table_pref_addedit_table_exit_cb, popup);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("OK (enter)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button2", button);
    evas_object_smart_callback_add(button, "clicked", table_pref_add_table_ok_cb, popup);

    ui.showPopup(popup, input);

    evas_object_event_callback_add(popup, EVAS_CALLBACK_HIDE, on_elm_popup_event_dismissed, list);

    populate_list(list);
}

static void table_pref_select_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.getTablePref().setActiveTableFromSelection();
}

static void table_preferences_key_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    Eina_Bool ctrl, alt, shift;

    ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");
    alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
    shift = evas_key_modifier_is_set(ev->modifiers, "Shift");

    EINA_LOG_INFO("KeyDown: %s - %s - %s", ev->key, ev->compose, ev->string);

    if (ctrl) {
        if (!strcmp(ev->key, "t")) {
            table_pref_select_cb(data, obj, event_info);
        } else if (!strcmp(ev->key, "a")) {
            table_pref_add_cb(data, obj, event_info);
        } else if (!strcmp(ev->key, "e")) {
            table_pref_edit_cb(data, obj, event_info);
        } else if (!strcmp(ev->key, "d")) {
            table_pref_delete_cb(data, obj, event_info);
        } else if (!strcmp(ev->key, "s")) {
            table_pref_ok_cb(data, obj, event_info);
        }
    }

    if (!strcmp(ev->key, "Escape")) {
        table_pref_exit_cb(data, obj, event_info);
    }
}

void data_table_preferences::show(Evas_Object *window) {
    std::string table;
    table = db.getTable(table);
    std::vector<std::string> tables = db.listTables();
    activeTableIndex = static_cast<int>(std::begin(tables) - std::find(std::begin(tables), std::end(tables), table));

    Evas_Object *popup = elm_popup_add(window);
    elm_object_part_text_set(popup, "title,text", _("Table preferences"));
#if ELM_VERSION_MAJOR>=1 && ELM_VERSION_MINOR>=20
    elm_popup_scrollable_set(popup, EINA_TRUE);
#endif
    elm_object_focus_allow_set(popup, EINA_FALSE);

    auto hbox = elm_box_add(popup);
    elm_box_horizontal_set(hbox, EINA_TRUE);
    evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_content_set(popup, hbox);
    elm_object_focus_allow_set(hbox, EINA_FALSE);
    evas_object_show(hbox);

    list = elm_list_add(popup);
    evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_event_callback_add(list, EVAS_CALLBACK_KEY_DOWN, table_preferences_key_down_cb, list);
    populate_list(list);
    elm_list_go(list);
    elm_object_focus_set(list, EINA_TRUE);
    evas_object_show(list);
    elm_box_pack_end(hbox, list);

    auto box = elm_box_add(popup);
    elm_object_focus_allow_set(box, EINA_FALSE);
    evas_object_size_hint_weight_set(box, 0, 0);
    evas_object_size_hint_align_set(box, 1, 0.5);
    elm_box_pack_end(hbox, box);
    evas_object_show(box);

    Evas_Object *button = elm_button_add(popup);
    elm_object_text_set(button, _("Set Table (ctrl-t)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(button, "clicked", table_pref_select_cb, list);
    elm_box_pack_end(box, button);
    evas_object_show(button);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("Edit... (ctrl-e)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(button, "clicked", table_pref_edit_cb, list);
    elm_box_pack_end(box, button);
    evas_object_show(button);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("Add... (ctrl-a)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(button, "clicked", table_pref_add_cb, list);
    elm_box_pack_end(box, button);
    evas_object_show(button);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("Delete (ctrl-d)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(button, "clicked", table_pref_delete_cb, list);
    elm_box_pack_end(box, button);
    evas_object_show(button);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("Cancel (esc)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", table_pref_exit_cb, popup);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("OK (ctrl-s)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button2", button);
    evas_object_smart_callback_add(button, "clicked", table_pref_ok_cb, popup);

    ui.showPopup(popup, list);
}

void data_table_preferences::saveEditableTable() {
    if (editableTables[editableTablesIndex] != editableTablesCurrentEdit) {
        if (renames.find(editableTables[editableTablesIndex]) != renames.end()) {
            renames[editableTablesCurrentEdit] = renames.find(editableTables[editableTablesIndex])->second;
            renames.erase(editableTables[editableTablesIndex]);
        } else {
            renames[editableTablesCurrentEdit] = editableTables[editableTablesIndex];
        }
        editableTables[editableTablesIndex] = editableTablesCurrentEdit;
    }
}

void data_table_preferences::addEditableTable() {
    editableTables.push_back(editableTablesCurrentEdit);
}

void data_table_preferences::updateEditTable(std::string table) {
    editableTablesCurrentEdit = table;
}

void data_table_preferences::editTableDelete() {
    auto index = editableTablesIndex;
    if (index >= 0 && index < editableTables.size()) {
        editableTables.erase(editableTables.begin() + index);
        if (index > 0) {
            editableTablesIndex--;
        }
    }
}

std::string data_table_preferences::getEditTable() {
    return editableTablesCurrentEdit;
}

std::vector<std::string> data_table_preferences::getEditableTables() {
    if (editableTables.empty()) {
        editableTables = db.listTables();
    }
    return editableTables;
}

bool data_table_preferences::tablePreferencesAreValid() {
    std::unordered_set<std::string> validCheck;
    for (auto table : editableTables) {
        auto search = validCheck.find(table);
        if(search == validCheck.end()) {
            validCheck.insert(table);
        } else {
            return false;
        }
    }

    return true;
}

void data_table_preferences::clearTablePreferences() {
    editableTables.clear();
    renames.clear();
}

void data_table_preferences::setCurrentTableSelection(int index) {
    editableTablesIndex = index;
    updateEditTable(editableTables[index]);
}

int data_table_preferences::getCurrentTableSelection() {
    return editableTablesIndex;
}

void data_table_preferences::setActiveTableFromSelection() {
    activeTableIndex = editableTablesIndex;
    populate_list(list);
}

int data_table_preferences::getActiveTable() {
    return activeTableIndex;
}

void data_table_preferences::saveTablePreferences() {
    std::string activeTable;
    if (activeTableIndex >= 0) {
        activeTable = editableTables[activeTableIndex];
    }
    auto oldTables = db.listTables();
    for (auto table : oldTables) {
        if (renames.find(table) != renames.end()) {
            db.renameTable(table, renames[table]);
        }
    }
    auto tables = db.listTables();
    std::vector<std::string> additions;
    std::set_difference(editableTables.begin(), editableTables.end(), tables.begin(), tables.end(),
                        std::inserter(additions, additions.begin()));
    std::vector<std::string> removals;
    std::set_difference(tables.begin(), tables.end(), editableTables.begin(), editableTables.end(),
                        std::inserter(removals, removals.begin()));
    for (auto toAdd : additions) {
        db.addTable(toAdd);
    }
    for (auto toRemove : removals) {
        db.deleteTable(toRemove);
    }
    if (!activeTable.empty()) {
        db.setTable(activeTable);
    }

    editableTables.clear();
}

data_table_preferences::data_table_preferences(sqlite_file &_db) : db(_db) {

}
