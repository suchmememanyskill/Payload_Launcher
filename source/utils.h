#pragma once

#define INV_WHITE "\x1b[47m"
#define BLACK CONSOLE_BLACK
#define RESET CONSOLE_RESET
#define RED CONSOLE_RED
#define GREEN CONSOLE_GREEN
#define YELLOW CONSOLE_YELLOW
#define MAGENTA CONSOLE_MAGENTA
#define CYAN CONSOLE_CYAN
#define BLUE CONSOLE_BLUE

#define MAX_LINES 34

#define BUFSIZE 32768

char* keyboard(char* message, size_t size);
void userAppInit(void);
char* addstrings(const char *s1, const char *s2);
bool checkfolder(char* foldloc);
void printarray(char *array[], int highlight, int offset, int max, int arraylength, int starty);
char* shortenstring(const char *in, int maxlength);
void printarraynew(char *array[], int arraylength, int highlight, int offset, int starty);
int messagebox(const char *line1, const char *line2);
int copy(const char* src, const char* dst);