#include <stdio.h>
#include <furi.h>
#include <gui/gui.h>
#include "fz_nrf24_jammer_icons.h"
#include "nRF24_jammer.h"

static const char* menu_name(MenuType menu) {
    static const char* const names[MENU_COUNT] = {
        "Bluetooth", "Drone", "WiFi", "BLE", "Zigbee", "Misc", "Settings"};
    return (menu < MENU_COUNT) ? names[menu] : "nRF24";
}

static void render_header(Canvas* canvas) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 9, "lukashilverda.nl");
    canvas_draw_line(canvas, 0, 11, 127, 11);
}

static void render_footer(Canvas* canvas, const char* left, const char* right) {
    canvas_draw_line(canvas, 0, 53, 127, 53);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 63, left);
    canvas_draw_str_aligned(canvas, 124, 63, AlignRight, AlignBottom, right);
}

static void render_settings_menu(Canvas* canvas, PluginState* state) {
    const uint8_t item_height = 10;
    const uint8_t visible_items = 4;
    static uint8_t scroll_offset = 0;

    if(state->selected_setting_item < scroll_offset) {
        scroll_offset = state->selected_setting_item;
    } else if(state->selected_setting_item >= scroll_offset + visible_items) {
        scroll_offset = state->selected_setting_item - visible_items + 1;
    }

    render_header(canvas);
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < visible_items && i + scroll_offset < SETTINGS_ITEM_COUNT; i++) {
        uint8_t y = 14 + (i * item_height);
        uint8_t item_index = i + scroll_offset;
        if(item_index == state->selected_setting_item) {
            canvas_draw_frame(canvas, 2, y - 2, 119, item_height);
            canvas_draw_str(canvas, 5, y + 6, "> ");
        }
        switch(item_index) {
            case SETTINGS_ITEM_SPI_MODE:
                canvas_draw_str(canvas, 16, y + 6, "SPI");
                canvas_draw_str(canvas, 70, y + 6, state->spi_mode == SPI_MODE_DEFAULT ? "PA4" : "PC3");
                break;
            case SETTINGS_ITEM_MODULES_MODE:
                canvas_draw_str(canvas, 16, y + 6, "Modules");
                canvas_draw_str(canvas, 70, y + 6, state->modules_mode == MODULES_MODE_SEPARATE ? "Separate" : "Together");
                break;
            case SETTINGS_ITEM_BLUETOOTH_METHOD:
                canvas_draw_str(canvas, 16, y + 6, "Bluetooth");
                canvas_draw_str(canvas, 70, y + 6, state->bluetooth_jam_method == BLUETOOTH_MODE_LIST ? "List" :
                    state->bluetooth_jam_method == BLUETOOTH_MODE_RANDOM ? "Random" : "Bruteforce");
                break;
            case SETTINGS_ITEM_DRONE_METHOD:
                canvas_draw_str(canvas, 16, y + 6, "Drone");
                canvas_draw_str(canvas, 70, y + 6, state->drone_jam_method == DRONE_MODE_BRUTEFORCE ? "Bruteforce" : "Random");
                break;
            case SETTINGS_ITEM_LOGO:
                canvas_draw_str(canvas, 16, y + 6, "Logo");
                canvas_draw_str(canvas, 70, y + 6, state->is_logo == SHOW_LOGO ? "Show" : "Hide");
                break;
            default:
                break;
        }
    }
    if(SETTINGS_ITEM_COUNT > visible_items) {
        canvas_draw_line(canvas, 125, 14, 125, 51);
        canvas_draw_box(canvas, 124, 14 + (scroll_offset * 37 / SETTINGS_ITEM_COUNT), 3, 8);
    }
    render_footer(canvas, "< Back", "< > Change");
}

static void render_settings_screen(Canvas* canvas, PluginState* state) {
    char buffer[32];
    render_header(canvas);
    canvas_set_font(canvas, FontPrimary);
    if(state->misc_state == MISC_STATE_SET_START) {
        snprintf(buffer, sizeof(buffer), "Start channel: %d", state->misc_start);
        canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, buffer);
        canvas_set_font(canvas, FontSecondary);
        snprintf(buffer, sizeof(buffer), state->misc_mode == MISC_MODE_CHANNEL_SWITCHING ?
            "Mode: Channel Switching" : "Mode: Packet Sending");
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, buffer);
        canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignCenter, "OK: set stop");
    } else if(state->misc_state == MISC_STATE_SET_STOP) {
        snprintf(buffer, sizeof(buffer), "Start: %d Stop: %d", state->misc_start, state->misc_stop);
        canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, buffer);
        canvas_set_font(canvas, FontSecondary);
        snprintf(buffer, sizeof(buffer), state->misc_mode == MISC_MODE_CHANNEL_SWITCHING ?
            "Mode: Channel Switching" : "Mode: Packet Sending");
        canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, buffer);
        canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignCenter,
            state->misc_stop > state->misc_start ? "OK: start" : "Start must be < Stop");
    } else if(state->misc_state == MISC_STATE_ERROR) {
        canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, "Invalid range");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 35, AlignCenter, AlignCenter, "OK: edit stop");
    }
    render_footer(canvas, "< Back", "o Continue");
}

static void render_wifi_channel_select(Canvas* canvas, PluginState* state) {
    char buffer[32];
    render_header(canvas);
    canvas_set_font(canvas, FontPrimary);
    snprintf(buffer, sizeof(buffer), "WiFi channel: %d", state->wifi_channel);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, buffer);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 45, AlignCenter, AlignCenter, "OK: start");
    render_footer(canvas, "< Back", "o Start");
}

static void render_wifi_menu(Canvas* canvas, PluginState* state) {
    render_header(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 7, 22, "All channels");
    canvas_draw_str(canvas, 7, 40, "Select channel");
    canvas_draw_frame(canvas, 2, state->wifi_mode == WIFI_MODE_ALL ? 13 : 31, 119, 14);
    render_footer(canvas, "< Back", "o Select");
}

static void render_active_jamming(Canvas* canvas, MenuType menu) {
    render_header(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 25, AlignCenter, AlignCenter, "ACTIVE");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 37, AlignCenter, AlignCenter, menu_name(menu));
    canvas_draw_frame(canvas, 47, 43, 34, 5);
    render_footer(canvas, "< Back", "o Stop");
}

static void render_main_menu(Canvas* canvas, PluginState* state) {
    static const char* const menu_names[MENU_COUNT] = {
        "Bluetooth", "Drone", "WiFi", "BLE", "Zigbee", "Misc", "Settings"};
    const uint8_t visible_items = 4;
    uint8_t scroll_offset = state->current_menu >= visible_items ?
        state->current_menu - visible_items + 1 : 0;

    render_header(canvas);
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t row = 0; row < visible_items && row + scroll_offset < MENU_COUNT; row++) {
        uint8_t item = row + scroll_offset;
        uint8_t y = 15 + row * 10;
        if(item == state->current_menu) {
            canvas_draw_frame(canvas, 2, y - 2, 119, 10);
            canvas_draw_str(canvas, 5, y + 6, "> ");
        }
        canvas_draw_str(canvas, 16, y + 6, menu_names[item]);
    }
    canvas_draw_line(canvas, 125, 15, 125, 51);
    canvas_draw_box(canvas, 124, 15 + (scroll_offset * 36 / (MENU_COUNT - visible_items + 1)), 3, 8);
    render_footer(canvas, "< Back", "o Open");
}

void nrf24_jammer_render_logo(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignCenter, "lukashilverda.nl");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignCenter, "made by lukas h");
}

void nrf24_jammer_render(Canvas* canvas, void* ctx) {
    PluginState* state = ctx;
    canvas_clear(canvas);
    if(!state->is_modules_connected) {
        char buffer[32];
        render_header(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 25, AlignCenter, AlignTop, "Module status");
        snprintf(buffer, sizeof(buffer), "Connected: %d module(s)", state->len_modules);
        canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignTop, buffer);
        if(state->len_modules == 0) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignTop, "Connect an nRF24 module");
        }
        render_footer(canvas, "< Back", "Scanning...");
    } else if(state->current_menu == MENU_MISC && state->show_jamming_started) {
        render_header(canvas);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignCenter, "Jamming started");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 41, AlignCenter, AlignCenter, "Starting radio sweep...");
        render_footer(canvas, "< Back", "o Stop");
    } else if(state->is_running) {
        render_active_jamming(canvas, state->current_menu);
    } else if(state->current_menu == MENU_SETTINGS) {
        if(state->settings_menu_active) render_settings_menu(canvas, state);
        else render_main_menu(canvas, state);
    } else if(state->current_menu == MENU_MISC && state->misc_state != MISC_STATE_IDLE) {
        render_settings_screen(canvas, state);
    } else if(state->current_menu == MENU_WIFI) {
        if(state->wifi_menu_active) {
            if(state->wifi_channel_select) render_wifi_channel_select(canvas, state);
            else render_wifi_menu(canvas, state);
        } else render_main_menu(canvas, state);
    } else if(state->current_menu == MENU_BLE) {
        if(state->ble_menu_active) {
            render_header(canvas);
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_frame(canvas, 2, state->ble_selected == 0 ? 13 : 31, 119, 14);
            canvas_draw_str(canvas, 7, 22, "Advertising channels");
            canvas_draw_str(canvas, 7, 40, "Data channels");
            render_footer(canvas, "< Back", "o Start");
        } else render_main_menu(canvas, state);
    } else {
        render_main_menu(canvas, state);
    }
}
