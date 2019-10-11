#pragma once

char* keyboard(char* message, size_t size);
void userAppInit(void);
char* addstrings(const char *s1, const char *s2);
bool checkfolder(char* foldloc);
void printarray(char *array[], int highlight, int offset, int max, int arraylength, int starty);
char* shortenstring(const char *in, int maxlength);