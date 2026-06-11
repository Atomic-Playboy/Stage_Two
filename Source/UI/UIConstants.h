#pragma once

namespace UIConstants {
    // Top Bar Menu Commands
    enum MenuCommands {
        CMD_OPEN_DASHBOARD = 1001,
        CMD_OPEN_TRANSFORMER = 1002,
        
        CMD_HARDWARE_ANY = 2001,
        CMD_HARDWARE_BASE = 2002,
        
        CMD_CHANNEL_ANY = 2100,
        CMD_CHANNEL_BASE = 2100, // Used to compute 2101 - 2116 dynamically
        
        CMD_WEB_DASHBOARD = 3001,
        CMD_WEB_TRANSFORMER = 3002,
        CMD_EDIT_DASHBOARD = 3004,
        CMD_EDIT_TRANSFORMER = 3005,
        CMD_HEAVY_WEB = 3006,
        CMD_DASH_BLENDER = 3007,
        CMD_TRANS_BLENDER = 3008,
        CMD_GTR_CONFIG = 3009,

        CMD_VIEW_DASHBOARD = 3104,
        CMD_VIEW_GTR = 3105,
        CMD_VIEW_CONSOLE = 3106,
        
        CMD_HELP_MANUAL = 4001
    };

    // Internal Win32 Base ID Blocks for dynamically generated controls
    enum ControlIDs {
        IDC_LOAD_BASE = 5000,
        IDC_LCD_UPPER_BASE = 6000,
        IDC_SEND_LIGHTS_BASE = 7000,
        IDC_SEND_MIDI_BASE = 7500,
        IDC_PC_DOWN_BASE = 8000,
        IDC_PC_UP_BASE = 8500,
        IDC_LCD_AB_BASE = 9000,
        IDC_LCD_BTN_BASE = 10000
    };
}
