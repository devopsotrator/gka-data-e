//
// Created by adam on 13/12/17.
//

#include <sqlite_file.h>
#include "data_edit_record.h"
#include "data_ui.h"

static void edit_entry_exit_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
    ui.clearFocus();
}

static void edit_entry_ok_cb(void *data, Evas_Object *obj, void *event_info) {
    ui.clearActivePopup();
    ui.saveCurrentRow();
    ui.clearFocus();
}

static void edit_entry_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info) {
    auto *ev = static_cast<Evas_Event_Key_Down *>(event_info);
    auto ctrl = evas_key_modifier_is_set(ev->modifiers, "Control");
    auto currentIndex = (int) (uintptr_t) data;
    EINA_LOG_ERR("KeyUp[%d]: %s - %s - %s", currentIndex, ev->key, ev->compose, ev->string);
    if (!strcmp(ev->key, "Escape")) {
        edit_entry_exit_cb(data, obj, event_info);
    } else if (ctrl && !strcmp(ev->key, "s")) {
        edit_entry_ok_cb(data, obj, event_info);
    } else if (!strcmp(ev->key, "Tab")) {
        ui.editColumnTabFocus(currentIndex+1);
    } else if (!strcmp(ev->key, "ISO_Left_Tab")) {
        ui.editColumnTabFocus(currentIndex-1);
    }
    auto text = elm_entry_markup_to_utf8(elm_object_text_get(obj));
    std::string entryValue = text;
    ui.updateCurrentRowValue(currentIndex, entryValue);
    free(text);
}

static void entry_filter_out_tabs_cb(void *data, Evas_Object *entry, char **text) {
    if (!strcmp(*text, "<tab/>")) {
        char *insert = *text;
        *insert = 0;
    }
}

void data_edit_record::updateCurrentRowValue(int i, std::string value) {
    currentRowValues[i] = std::move(value);
}

void data_edit_record::saveCurrentRow() {
    db.addRow(currentRowValues);
    ui.clearActivePopup();
    ui.repopulateUI();
}

void data_edit_record::populateAndShowEntryPopup(Evas_Object *window, Evas_Object *popup, const std::vector<std::string> &cols) {
#if ELM_VERSION_MAJOR>=1 && ELM_VERSION_MINOR>=20
    elm_popup_scrollable_set(popup, EINA_TRUE);
#else
    elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_MIXED);
#endif

    Evas_Object *popupTable = elm_table_add(window);
    evas_object_size_hint_weight_set(popupTable, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(popupTable, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_padding_set(popupTable, 5, 5, 5, 5);
    elm_table_padding_set(popupTable, 6, 0);
#if ELM_VERSION_MAJOR>=1 && ELM_VERSION_MINOR>=20
    elm_table_align_set(popupTable, 0, 0);
#endif
    elm_table_homogeneous_set(popupTable, EINA_FALSE);
    elm_object_content_set(popup, popupTable);
    evas_object_show(popupTable);

    Evas_Object *focusInput = nullptr;
    currentRowEditors.clear();
    for (int i = (db.getPrimaryKey() ? 1 : 0); i < cols.size(); i++) {
        auto field_name = elm_label_add(popupTable);
        elm_object_text_set(field_name, cols[i].c_str());
        evas_object_size_hint_align_set(field_name, 1, 0);
        elm_table_pack(popupTable, field_name, 0, i, 1, 1);
        evas_object_show(field_name);
        elm_object_focus_allow_set(field_name, EINA_FALSE);

        Evas_Object *input = elm_entry_add(popupTable);
        currentRowEditors.push_back(input);
        if (!currentRowValues[i].empty()) {
            auto text = elm_entry_utf8_to_markup(currentRowValues[i].c_str());
            elm_object_text_set(input, text);
            free(text);
        }
        elm_entry_single_line_set(input, EINA_FALSE);
        elm_entry_editable_set(input, EINA_TRUE);
        elm_entry_cursor_line_end_set(input);
        evas_object_size_hint_weight_set(input, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(input, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_event_callback_add(input, EVAS_CALLBACK_KEY_UP, edit_entry_key_up_cb, (void *) (uintptr_t) i);
        elm_entry_markup_filter_append(input, entry_filter_out_tabs_cb, (void *) (uintptr_t) i);
        elm_table_pack(popupTable, input, 1, i, 1, 1);
        evas_object_show(input);

        if (i == (db.getPrimaryKey() ? 1 : 0)) {
            elm_object_focus_set(input, EINA_TRUE);
            focusInput = input;
        }
    }

    Evas_Object *button = elm_button_add(popup);
    elm_object_text_set(button, _("Cancel (esc)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button1", button);
    evas_object_smart_callback_add(button, "clicked", edit_entry_exit_cb, popup);

    button = elm_button_add(popup);
    elm_object_text_set(button, _("OK (ctrl-s)"));
    elm_object_focus_allow_set(button, EINA_FALSE);
    elm_object_part_content_set(popup, "button2", button);
    evas_object_smart_callback_add(button, "clicked", edit_entry_ok_cb, popup);

    elm_object_focus_allow_set(popup, EINA_FALSE);
    elm_object_focus_allow_set(popupTable, EINA_FALSE);

    ui.showPopup(popup, focusInput);
}

void data_edit_record::addRow(Evas_Object *window) {
    Evas_Object *popup = elm_popup_add(window);
    elm_object_part_text_set(popup, "title,text", _("New Entry"));

    auto cols = db.listColumns();
    currentRowValues.clear();
    for (int i = 0; i < cols.size(); i++) {
        currentRowValues.emplace_back("");
    }

    populateAndShowEntryPopup(window, popup, cols);
}

void data_edit_record::editRow(Evas_Object *window, int selectedRow) {
    Evas_Object *popup = elm_popup_add(window);
    elm_object_part_text_set(popup, "title,text", _("Edit Entry"));

    auto cols = db.listColumns();
    auto rows = db.readRow(selectedRow - 1);
    currentRowValues.clear();
    for (int i = 0; i < cols.size(); i++) {
        currentRowValues.emplace_back(rows[i]);
    }

    populateAndShowEntryPopup(window, popup, cols);
}

data_edit_record::data_edit_record(sqlite_file &_db) : db(_db) {

}

void data_edit_record::editColumnTabFocus(int currentIndex) {
    currentIndex -= db.getPrimaryKey() ? 1 : 0;
    if (currentIndex >= 0 && currentIndex < currentRowEditors.size()) {
        elm_object_focus_set(currentRowEditors[currentIndex], EINA_TRUE);
    }
}
