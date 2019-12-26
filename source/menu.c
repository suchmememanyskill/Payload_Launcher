#include "sdl_helper.h"
#include "menu.h"
#include <string.h>
#include <switch.h>

controller_info controllerinfo[10];
int controllerinfocount = 0;
u32 controllermap = 0;

void InitControllerInfo(){
    controllerinfocount = 0;
    controllermap = 0;
}

void AddControllerInfo(char *info, u16 button, u32 map){
    strcpy(controllerinfo[controllerinfocount].name, info);
    controllerinfo[controllerinfocount].glyph = button;
    controllerinfocount++;
    controllermap |= map;
}

void DrawControllerInfo(){
    int xpos = 50;
    for (int i = 0; i < controllerinfocount; i++){
        xpos += SetButtonInfo(controllerinfo[i].glyph, controllerinfo[i].name, xpos, 1010) + 50;
    }
}

char top_menu[10][50];
int top_menu_selected = 1, top_menu_amount = 0;

void InitTopMenu(){
    top_menu_selected = 1;
    top_menu_amount = 0;
}

void AddTopMenu(char *item){
    strcpy(top_menu[top_menu_amount++], item);
}

void DrawTopMenu(){
    int xpos = 150, w;
    for (int i = 0; i < top_menu_amount; i++){
        DrawText(xpos, 28, top_menu[i], COLOR_WHITE, 1920);
        w = GetTextSize(top_menu[i]);

        if (i == top_menu_selected - 1)
            DrawBox(xpos - 20, 80, w + 40, 13, COLOR_LIGHTBLUE);

        xpos += w + 50;
    }
}

bool ChangeTopMenu(bool Up){
    if (Up){
        if (top_menu_amount > top_menu_selected){
            top_menu_selected++;
            return true;
        }
        else
            return false;
    }
    else {
        if (top_menu_selected > 1){
            top_menu_selected--;
            return true;
        }
        else 
            return false;
    }
}

int GetTopMenuSelection(){
    return top_menu_selected;
}

char *shortenstring(char *string){
    static char ret[91] = "";
    if (strlen(string) > 90){
        strlcpy(ret, string, 87);
        for (int i = 86; i < 89; i++)
            ret[i] = '.';
        ret[90] = '\0';
    }
    else 
        strcpy(ret, string);
    
    return ret;
}

void DrawMenuEntry(int x, int y, menu_item item, bool highlighted){
    if (highlighted)
        DrawBox(x - 20, y - 3, 5, 40, COLOR_LIGHTBLUE);

    DrawText(x, y, shortenstring(item.name), (highlighted) ? COLOR_LIGHTBLUE : COLOR_WHITE, 1920);
}

void ClearScreenWithElements(){
    ClearScreen();
    DrawBox(0, 90, 1920, 3, COLOR_WHITE);
    DrawBox(0, 990, 1920, 3, COLOR_WHITE);
    DrawGlyph(50, 20, BUTTON_L, COLOR_WHITE);
    DrawGlyph(1870 - GetButtonTextSize(BUTTON_R), 20, BUTTON_R, COLOR_WHITE);
}

int selection = 0, offset = 0;

int GetSelection(){
    return selection + offset;
}

u32 MakeBasicMenu(menu_item items[], int amount){
    int ypos = 150;

    selection = 1, offset = 0;

    while (1){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
        ClearScreenWithElements();

        if ((kDown & KEY_LSTICK_UP || kHeld & KEY_RSTICK_UP))
            selection--;
        if ((kDown & KEY_LSTICK_DOWN || kHeld & KEY_RSTICK_DOWN))
            selection++;

        if (selection + offset > amount)
            selection = amount - offset;

        else if (selection > MAX_ENTRIES)
            selection = MAX_ENTRIES, offset++;

        if (selection < 1 && offset > 0)
            selection = 1, offset--;

        else if (selection < 1 && offset <= 0)
            selection = 1;

        if (kDown & controllermap)
            return kDown;
        
        if (kDown & KEY_L)
            if (ChangeTopMenu(false))
                return kDown;
        
        if (kDown & KEY_R)
            if (ChangeTopMenu(true))
                return kDown;


        for (int i = 0; i < ((amount > MAX_ENTRIES) ? MAX_ENTRIES : amount); i++){
            DrawMenuEntry(90, ypos, items[i + offset], (i == selection - 1));
            ypos += 50;
        }
        ypos = 150;

        DrawControllerInfo();
        DrawTopMenu();
        DrawNotification();
        RenderFrame();
    }
}

bool MakeMessageBox(char *optionfalse, char *optiontrue, char *title, char *message){
    ClearScreen();

    DrawOutline(480, 90, 1440, 990, 5, COLOR_WHITE); // draws the outer "box"
    DrawBox(480, 180, 960, 5, COLOR_WHITE); // draws the top line
    DrawBox(480, 890, 960, 5, COLOR_WHITE); // draws the bottom line
    DrawBox(958, 890, 4, 100, COLOR_WHITE); // draws the inbetween line
    DrawText(500, 110, title, COLOR_WHITE, 920);
    DrawText(500, 200, message, COLOR_WHITE, 920);
    SetButtonInfo(BUTTON_B, optionfalse, 505, 915);
    SetButtonInfo(BUTTON_A, optiontrue, 982, 915);

    RenderFrame();

    while (1){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_A)
            return true;
        
        if (kDown & KEY_B)
            return false;
    }
}

notification Notification = {"", 0, COLOR_BACKGROUND, COLOR_BACKGROUND};

void MakeNotification(char *text, int TTL, SDL_Color text_color, SDL_Color bg_color){
    strcpy(Notification.text, text);
    Notification.TTL = TTL;
    Notification.text_color = text_color;
    Notification.bg_color = bg_color;
}

void DrawNotification(){
    if (Notification.TTL > 0){
        int textlength, x;

        textlength = GetTextSize(Notification.text);
        x = (1920 - textlength) / 2;
        DrawBox(x - 20, 890, textlength + 40, 100, Notification.bg_color);
        DrawText(x, 925, Notification.text, Notification.text_color, x + textlength);
        Notification.TTL--;
    }
}