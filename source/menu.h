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