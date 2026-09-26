#pragma once

// Presentation only. Radio state and worker lifecycle stay in nRF24_jammer.c.
// Uses the same fonts, 16 px rows and selection helper as Momentum's lists.
#include <gui/elements.h>

static const char* const menu_labels[MENU_COUNT] = {
    "Bluetooth", "Drones", "Wi-Fi", "BLE", "Zigbee", "Custom range", "Settings", "Spectrum"};

static const char* bluetooth_method_label(BluetoothJamMethod method) {
    switch(method) {
    case BLUETOOTH_MODE_RANDOM: return "Random";
    case BLUETOOTH_MODE_BRUTEFORCE: return "Sweep";
    default: return "List";
    }
}

static const char* drone_method_label(DroneJamMethod method) {
    return method == DRONE_MODE_RANDOM ? "Random" : "Sweep";
}

static const char* misc_method_label(MiscMode mode) {
    return mode == MISC_MODE_PACKET_SENDING ? "Packets" : "Sweep";
}

static void ui_header(Canvas* canvas, const char* title, const char* detail) {
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 11, title);
    canvas_set_font(canvas, FontSecondary);
    if(detail) canvas_draw_str_aligned(canvas, 124, 11, AlignRight, AlignBottom, detail);
}

static void ui_value(Canvas* canvas, uint8_t y, const char* label, const char* value) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, y, label);
    canvas_draw_str_aligned(canvas, 124, y, AlignRight, AlignBottom, value);
}

static void ui_row(
    Canvas* canvas,
    uint8_t y,
    const char* label,
    const char* value,
    bool selected,
    bool editable) {
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    if(selected) {
        elements_slightly_rounded_box(canvas, 0, y + 1, 123, 14);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str(canvas, 6, y + 12, label);
    if(editable) {
        canvas_draw_str_aligned(canvas, 92, y + 12, AlignCenter, AlignBottom, value);
        if(selected) {
            canvas_draw_str(canvas, 65, y + 12, "<");
            canvas_draw_str(canvas, 115, y + 12, ">");
        }
    } else if(value) {
        canvas_draw_str_aligned(canvas, 118, y + 12, AlignRight, AlignBottom, value);
    }
    canvas_set_color(canvas, ColorBlack);
}

// Keep the selected row in view, with a neighbour above it where possible.
static uint8_t ui_window_start(uint8_t selected, uint8_t count) {
    uint8_t first = selected > 0 ? selected - 1 : 0;
    if(count <= 3) return 0;
    if(first > count - 3) first = count - 3;
    return first;
}

static void ui_back_hint(Canvas* canvas, const char* text) {
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 61, AlignCenter, AlignBottom, text);
}

static void render_main_menu(Canvas* canvas, const PluginState* state) {
    char modules[12];
    snprintf(modules, sizeof(modules), "%u mod", state->len_modules);
    ui_header(canvas, "nRF24 Jammer", modules);
    uint8_t first = ui_window_start(state->current_menu, MENU_COUNT);
    for(uint8_t row = 0; row < 3; row++) {
        MenuType item = first + row;
        bool direct = item == MENU_BLUETOOTH || item == MENU_DRONE || item == MENU_ZIGBEE;
        ui_row(canvas, 16 + row * 16, menu_labels[item], direct ? "Start" : ">",
               item == state->current_menu, false);
    }
    elements_scrollbar_pos(canvas, 128, 16, 48, state->current_menu, MENU_COUNT);
}

static void render_settings_menu(Canvas* canvas, const PluginState* state) {
    static const char* const labels[] = {"SPI CS", "Modules", "Bluetooth", "Drones"};
    const char* values[] = {
        state->spi_mode == SPI_MODE_EXTRA ? "PC3" : "PA4",
        state->modules_mode == MODULES_MODE_SEPARATE ? "Separate" : "Together",
        bluetooth_method_label(state->bluetooth_jam_method),
        drone_method_label(state->drone_jam_method)};
    ui_header(canvas, "Settings", NULL);
    uint8_t first = ui_window_start(state->selected_setting_item, SETTINGS_ITEM_COUNT);
    for(uint8_t row = 0; row < 3; row++) {
        uint8_t item = first + row;
        ui_row(canvas, 16 + row * 16, labels[item], values[item],
               item == state->selected_setting_item, true);
    }
    elements_scrollbar_pos(canvas, 128, 16, 48, state->selected_setting_item, SETTINGS_ITEM_COUNT);
}

static void render_module_status(Canvas* canvas, const PluginState* state) {
    char buffer[24];
    ui_header(canvas, "nRF24 Jammer", NULL);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 29, AlignCenter, AlignBottom,
                           state->len_modules ? "Module detected" : "No module found");
    canvas_set_font(canvas, FontSecondary);
    if(state->len_modules) {
        snprintf(buffer, sizeof(buffer), "%u connected", state->len_modules);
        canvas_draw_str_aligned(canvas, 64, 43, AlignCenter, AlignBottom, buffer);
    } else {
        canvas_draw_str_aligned(canvas, 64, 43, AlignCenter, AlignBottom, "Connect an nRF24 module");
    }
    snprintf(buffer, sizeof(buffer), "SPI CS: %s", state->spi_mode == SPI_MODE_EXTRA ? "PC3" : "PA4");
    ui_back_hint(canvas, buffer);
}

static void render_settings_screen(Canvas* canvas, const PluginState* state) {
    char buffer[32];
    if(state->misc_state == MISC_STATE_ERROR) {
        ui_header(canvas, "Custom range", NULL);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 29, AlignCenter, AlignBottom, "Invalid range");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 43, AlignCenter, AlignBottom, "End must be above start");
        elements_button_center(canvas, "Edit");
        return;
    }

    bool set_start = state->misc_state == MISC_STATE_SET_START;
    ui_header(canvas, "Custom range", set_start ? "1/2" : "2/2");
    if(set_start) {
        snprintf(buffer, sizeof(buffer), "%u", state->misc_start);
        ui_value(canvas, 26, "Start channel", buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "%u - %u", state->misc_start, state->misc_stop);
        ui_value(canvas, 26, "End channel", buffer);
    }
    snprintf(buffer, sizeof(buffer), "< %s >", misc_method_label(state->misc_mode));
    ui_value(canvas, 38, "Mode", buffer);
    if(!set_start && state->misc_stop <= state->misc_start) {
        canvas_draw_str_aligned(canvas, 64, 49, AlignCenter, AlignBottom, "End must exceed start");
        ui_back_hint(canvas, "Back: Edit start");
    } else {
        canvas_draw_str_aligned(canvas, 64, 49, AlignCenter, AlignBottom, "Up/down: channel");
        elements_button_center(canvas, set_start ? "Next" : "Start");
    }
}

static void render_wifi_channel_select(Canvas* canvas, const PluginState* state) {
    char buffer[4];
    ui_header(canvas, "Wi-Fi channel", NULL);
    snprintf(buffer, sizeof(buffer), "%u", state->wifi_channel);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignBottom, buffer);
    canvas_set_font(canvas, FontSecondary);
    elements_button_left(canvas, "-");
    elements_button_center(canvas, "Start");
    elements_button_right(canvas, "+");
}

static void render_wifi_menu(Canvas* canvas, const PluginState* state) {
    ui_header(canvas, "Wi-Fi", NULL);
    ui_row(canvas, 16, "Select channel", NULL, state->wifi_mode == WIFI_MODE_SELECT, false);
    ui_row(canvas, 32, "All channels", NULL, state->wifi_mode == WIFI_MODE_ALL, false);
    elements_button_center(canvas, state->wifi_mode == WIFI_MODE_SELECT ? "Next" : "Start");
}

static void render_ble_menu(Canvas* canvas, const PluginState* state) {
    ui_header(canvas, "BLE", NULL);
    ui_row(canvas, 16, "Advertising channels", NULL, state->ble_selected == 0, false);
    ui_row(canvas, 32, "Data channels", NULL, state->ble_selected != 0, false);
    elements_button_center(canvas, "Start");
}

static void render_active_jamming(Canvas* canvas, const PluginState* state) {
    char buffer[24];
    ui_header(canvas, menu_labels[state->current_menu], "Active");
    switch(state->current_menu) {
    case MENU_BLUETOOTH:
        ui_value(canvas, 27, "Method", bluetooth_method_label(state->bluetooth_jam_method));
        break;
    case MENU_DRONE:
        ui_value(canvas, 27, "Method", drone_method_label(state->drone_jam_method));
        break;
    case MENU_WIFI:
        if(state->wifi_mode == WIFI_MODE_ALL) {
            ui_value(canvas, 27, "Channels", "All");
        } else {
            snprintf(buffer, sizeof(buffer), "%u", state->wifi_channel);
            ui_value(canvas, 27, "Channel", buffer);
        }
        break;
    case MENU_BLE:
        ui_value(canvas, 27, "Channels", state->ble_selected == 0 ? "Advertising" : "Data");
        break;
    case MENU_ZIGBEE:
        ui_value(canvas, 27, "Channels", "11 - 26");
        break;
    case MENU_MISC:
        snprintf(buffer, sizeof(buffer), "%u - %u", state->misc_start, state->misc_stop);
        ui_value(canvas, 25, "Range", buffer);
        ui_value(canvas, 37, "Mode", misc_method_label(state->misc_mode));
        break;
    default: break;
    }
    snprintf(buffer, sizeof(buffer), "%u / %s", state->len_modules,
             state->modules_mode == MODULES_MODE_SEPARATE ? "Separate" : "Together");
    ui_value(canvas, state->current_menu == MENU_MISC ? 49 : 43, "Modules", buffer);
    ui_back_hint(canvas, "Back: Stop");
}

static void render_spectrum(Canvas* canvas, const PluginState* state) {
    uint8_t max_activity = 0;
    for(uint8_t channel = 0; channel < 126; channel++) {
        if(state->analyzer_activity[channel] > max_activity) {
            max_activity = state->analyzer_activity[channel];
        }
    }

    ui_header(canvas, "Spectrum", "Receive only");
    canvas_draw_line(canvas, 4, 52, 123, 52);
    for(uint8_t bin = 0; bin < 16; bin++) {
        uint8_t bin_max = 0;
        uint8_t first_channel = bin * 8;
        uint8_t last_channel = first_channel + 8;
        if(last_channel > 126) last_channel = 126;
        for(uint8_t channel = first_channel; channel < last_channel; channel++) {
            if(state->analyzer_activity[channel] > bin_max) {
                bin_max = state->analyzer_activity[channel];
            }
        }

        uint8_t height = max_activity ? (bin_max * 32) / max_activity : 0;
        uint8_t x = 6 + bin * 7;
        if(height > 0) canvas_draw_box(canvas, x, 52 - height, 5, height);
        if((bin % 4) == 0) canvas_draw_line(canvas, x, 53, x, 55);
    }
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 63, "0");
    canvas_draw_str(canvas, 31, 63, "32");
    canvas_draw_str(canvas, 59, 63, "64");
    canvas_draw_str(canvas, 87, 63, "96");
    canvas_draw_str(canvas, 112, 63, "125");
}

static void render_callback(Canvas* canvas, void* ctx) {
    const PluginState* state = ctx;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    if(!state->is_modules_connected) {
        render_module_status(canvas, state);
    } else if(state->is_running && state->current_menu == MENU_SPECTRUM) {
        render_spectrum(canvas, state);
    } else if(state->is_running ||
              (state->current_menu == MENU_MISC && state->show_jamming_started)) {
        render_active_jamming(canvas, state);
    } else if(state->current_menu == MENU_SETTINGS && state->settings_menu_active) {
        render_settings_menu(canvas, state);
    } else if(state->current_menu == MENU_MISC && state->misc_state != MISC_STATE_IDLE) {
        render_settings_screen(canvas, state);
    } else if(state->current_menu == MENU_WIFI && state->wifi_menu_active) {
        if(state->wifi_channel_select) {
            render_wifi_channel_select(canvas, state);
        } else {
            render_wifi_menu(canvas, state);
        }
    } else if(state->current_menu == MENU_BLE && state->ble_menu_active) {
        render_ble_menu(canvas, state);
    } else {
        render_main_menu(canvas, state);
    }
}
