#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>

#define HOLD_DELAY_MS 100
#define MAX_NRF24 4

typedef enum {
    MENU_BLUETOOTH,
    MENU_DRONE,
    MENU_WIFI,
    MENU_BLE,
    MENU_ZIGBEE,
    MENU_MISC,
    MENU_SETTINGS,
    MENU_COUNT
} MenuType;

typedef enum { WIFI_MODE_SELECT, WIFI_MODE_ALL, WIFI_MODE_COUNT } WifiMode;
typedef enum { MISC_STATE_IDLE, MISC_STATE_SET_START, MISC_STATE_SET_STOP, MISC_STATE_ERROR, MISC_STATE_COUNT } MiscState;
typedef enum { MISC_MODE_CHANNEL_SWITCHING, MISC_MODE_PACKET_SENDING, MISC_MODE_COUNT } MiscMode;
typedef enum { BLUETOOTH_MODE_LIST, BLUETOOTH_MODE_RANDOM, BLUETOOTH_MODE_BRUTEFORCE, BLUETOOTH_MODE_COUNT } BluetoothJamMethod;
typedef enum { DRONE_MODE_BRUTEFORCE, DRONE_MODE_RANDOM, DRONE_MODE_COUNT } DroneJamMethod;
typedef enum { MODULES_MODE_SEPARATE, MODULES_MODE_TOGETHER, MODULES_MODE_COUNT } ModulesMode;
typedef enum { SETTINGS_ITEM_SPI_MODE, SETTINGS_ITEM_MODULES_MODE, SETTINGS_ITEM_BLUETOOTH_METHOD, SETTINGS_ITEM_DRONE_METHOD, SETTINGS_ITEM_LOGO, SETTINGS_ITEM_COUNT } SettingsItem;
typedef enum { SHOW_LOGO, HIDE_LOGO, LOGO_COUNT } Is_Logo;
typedef enum { SPI_MODE_DEFAULT, SPI_MODE_EXTRA, SPI_MODE_COUNT } SpiMode;

typedef struct {
    FuriMutex* mutex;
    NotificationApp* notifications;
    FuriThread* thread;
    ViewPort* view_port;
    bool is_running;
    bool is_stop;
    bool wifi_menu_active;
    bool show_jamming_started;
    bool wifi_channel_select;
    bool is_modules_connected;
    bool settings_menu_active;
    bool ble_menu_active;
    uint8_t ble_selected;
    MenuType current_menu;
    WifiMode wifi_mode;
    MiscState misc_state;
    MiscMode misc_mode;
    uint8_t wifi_channel;
    uint8_t misc_start;
    uint8_t misc_stop;
    SpiMode spi_mode;
    ModulesMode modules_mode;
    BluetoothJamMethod bluetooth_jam_method;
    DroneJamMethod drone_jam_method;
    uint8_t is_logo;
    SettingsItem selected_setting_item;
    InputKey held_key;
    uint32_t hold_counter;
    uint32_t last_up_press_time;
    uint32_t last_down_press_time;
    uint8_t up_press_count;
    uint8_t down_press_count;
    uint8_t len_modules;
} PluginState;

void nrf24_jammer_render_logo(Canvas* canvas, void* ctx);
void nrf24_jammer_render(Canvas* canvas, void* ctx);
