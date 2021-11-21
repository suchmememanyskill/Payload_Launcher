#pragma once
#include <SDL.h>
#include <switch.h>

#define MAX_ENTRIES 16

#define BUTTON_A            0xE0E0
#define BUTTON_B            0xE0E1
#define BUTTON_X            0xE0E2
#define BUTTON_Y            0xE0E3
#define BUTTON_L            0xE0E4
#define BUTTON_R            0xE0E5
#define BUTTON_ZL           0xE0E6
#define BUTTON_ZR           0xE0E7
#define BUTTON_SL           0xE0E8
#define BUTTON_SR           0xE0E9
#define BUTTON_UP           0xE0EB
#define BUTTON_DOWN         0xE0EC
#define BUTTON_LEFT         0xE0ED
#define BUTTON_RIGHT        0xE0EE
#define BUTTON_PLUS         0xE0EF
#define BUTTON_MINUS        0xE0F0

typedef struct _menu_item {
    char name[255];
    short property;
} menu_item;

typedef struct _controller_info {
    char name[50];
    u16 glyph;
} controller_info;

typedef struct _notification {
    char text[75];
    int TTL;
    SDL_Color text_color;
    SDL_Color bg_color;
} notification;

// libnx code https://github.com/switchbrew/libnx/blob/a35321b3603923aa360ff090a811683acb8285c4/nx/include/switch/services/hid.h#L342
typedef enum DEPRECATED {
    KEY_A            = HidNpadButton_A,
    KEY_B            = HidNpadButton_B,
    KEY_X            = HidNpadButton_X,
    KEY_Y            = HidNpadButton_Y,
    KEY_LSTICK       = HidNpadButton_StickL,
    KEY_RSTICK       = HidNpadButton_StickR,
    KEY_L            = HidNpadButton_L,
    KEY_R            = HidNpadButton_R,
    KEY_ZL           = HidNpadButton_ZL,
    KEY_ZR           = HidNpadButton_ZR,
    KEY_PLUS         = HidNpadButton_Plus,
    KEY_MINUS        = HidNpadButton_Minus,
    KEY_DLEFT        = HidNpadButton_Left,
    KEY_DUP          = HidNpadButton_Up,
    KEY_DRIGHT       = HidNpadButton_Right,
    KEY_DDOWN        = HidNpadButton_Down,
    KEY_LSTICK_LEFT  = HidNpadButton_StickLLeft,
    KEY_LSTICK_UP    = HidNpadButton_StickLUp,
    KEY_LSTICK_RIGHT = HidNpadButton_StickLRight,
    KEY_LSTICK_DOWN  = HidNpadButton_StickLDown,
    KEY_RSTICK_LEFT  = HidNpadButton_StickRLeft,
    KEY_RSTICK_UP    = HidNpadButton_StickRUp,
    KEY_RSTICK_RIGHT = HidNpadButton_StickRRight,
    KEY_RSTICK_DOWN  = HidNpadButton_StickRDown,
    KEY_SL_LEFT      = HidNpadButton_LeftSL,
    KEY_SR_LEFT      = HidNpadButton_LeftSR,
    KEY_SL_RIGHT     = HidNpadButton_RightSL,
    KEY_SR_RIGHT     = HidNpadButton_RightSR,
    KEY_NES_HANDHELD_LEFT_B = HidNpadButton_HandheldLeftB,

    KEY_HOME         = BIT(18),      ///< HOME button, only available for use with HiddbgHdlsState::buttons.
    KEY_CAPTURE      = BIT(19),      ///< Capture button, only available for use with HiddbgHdlsState::buttons.
    KEY_TOUCH        = BIT(28),      ///< Pseudo-key for at least one finger on the touch screen

    KEY_JOYCON_RIGHT = HidNpadButton_A,
    KEY_JOYCON_DOWN  = HidNpadButton_B,
    KEY_JOYCON_UP    = HidNpadButton_X,
    KEY_JOYCON_LEFT  = HidNpadButton_Y,

    KEY_UP    = HidNpadButton_AnyUp,
    KEY_DOWN  = HidNpadButton_AnyDown,
    KEY_LEFT  = HidNpadButton_AnyLeft,
    KEY_RIGHT = HidNpadButton_AnyRight,
    KEY_SL    = HidNpadButton_AnySL,
    KEY_SR    = HidNpadButton_AnySR,
} HidControllerKeys;
// End libnx code

u32 MakeBasicMenu(menu_item items[], int amount);
void InitControllerInfo();
void AddControllerInfo(char *info, u16 button, u32 map);
void InitTopMenu();
void AddTopMenu(char *item);
int GetTopMenuSelection();
bool MakeMessageBox(char *optionfalse, char *optiontrue, char *title, char *message);
int GetSelection();
void MakeNotification(char *text, int TTL, SDL_Color text_color, SDL_Color bg_color);
void DrawNotification();
void InitController();