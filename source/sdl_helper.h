#pragma once
#include <SDL_ttf.h>
#include <switch.h>

#define COLOR_MAKE(r, g, b, a) (SDL_Color){r, g, b, a}

#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}
#define COLOR_LIGHTBLUE (SDL_Color){0, 243, 243, 255}
#define COLOR_BACKGROUND (SDL_Color){45, 45, 45, 255}
#define COLOR_GREEN (SDL_Color){0, 255, 0, 255}
#define COLOR_RED (SDL_Color){255, 0, 0, 255}
#define COLOR_BLACK (SDL_Color){0, 0, 0, 255}

void ClearScreen();
void DrawOutline(int x, int y, int x2, int y2, int bordersize, SDL_Color color);
void DrawBox(int x, int y, int w, int h, SDL_Color color);
void SDLDeInit();
void SDLInit();
void RenderFrame();
int GetTextSize(char *text);
int GetButtonTextSize(u16 text);
int GetButtonInfoSize(u16 buttontext, char *text);
int SetButtonInfo(u16 button, char *text, int x, int y);
void DrawText(int x, int y, char *text, SDL_Color color, int wrap);
void DrawGlyph(int x, int y, u16 glyph, SDL_Color color);