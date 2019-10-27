#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <switch.h>
#include "utils.h"

bool appletinit = false;

char* keyboard(char* message, size_t size){
	if (!appletinit){
		userAppInit();
		appletinit = true;
	}

	SwkbdConfig	skp; 
	Result keyrc = swkbdCreate(&skp, 0);
	char* out = NULL;
	out = (char *)calloc(sizeof(char), size + 1);

	if (R_SUCCEEDED(keyrc) && out != NULL){
		swkbdConfigMakePresetDefault(&skp);
		swkbdConfigSetGuideText(&skp, message);
		keyrc = swkbdShow(&skp, out, size);
		swkbdClose(&skp);	
	}

	else {
	free(out);
	out = NULL;
	}

	return (out);
}

void userAppInit(void){
	void *addr = NULL;
	if (svcSetHeapSize(&addr, 0x4000000) == (Result)-1) fatalSimple(0);
}

char* addstrings(const char *s1, const char *s2){
    char *result = malloc(strlen(s1) + strlen(s2) + 1); 
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char* shortenstring(const char *in, int maxlength){
	int length;
	char *result = (char*) malloc (maxlength + 1);
	length = strlen(in);

	if (length <= maxlength)
		strcpy(result, in);
	else {
		strlcpy(result, in, maxlength - 2);
		strcat(result, "...");
	}

	return result;
}

bool checkfolder(char* foldloc){
	DIR *tr = opendir(addstrings(foldloc, "."));
	bool folderexists = false;
	if (tr != NULL){
			folderexists = true;
    	}
    closedir(tr);
    return folderexists;
}

void printarray(char *array[], int highlight, int offset, int max, int arraylength, int starty){
	printf("\x1b[%d;1H", starty);
	for (int i = 0; i < max && i < arraylength - offset; i++){
		if (i + offset == highlight - 1) printf("\x1b[42m%s\x1b[0m", array[i + offset]);
		else printf("%s", array[i + offset]);

		for (int i2 = 0; i2 < 40 - strlen(array[i + offset]); i2++) printf(" ");

		printf("\n");
	}
}

void printarraynew(char *array[], int arraylength, int highlight, int offset, int starty){
	int max = arraylength - offset;

	if (max > MAX_LINES)
		max = MAX_LINES;

	printf("\x1b[%d;1H", starty);

	for (int i2 = 0; i2 < MAX_LINES * 2; i2++)
		//printf("----------------------------------------");
		printf("                                        ");

	printf("\x1b[%d;1H", starty);
	for (int i = 0; i < max; i++){		
		if (i == highlight - 1)
			printf(INV_WHITE CYAN "%s", array[i + offset]);
		else
			printf(CYAN "%s", array[i + offset]);

		printf("\n" RESET);
	}

}

int messagebox(const char *line1, const char *line2){
	int temp, temp2;

	printf("\x1b[12;11H+----------------------------------------------------------+");
	for (int i = 0; i < 8; i++)
		printf("\x1b[%d;11H|                                                          |\n", i + 13);
	printf("\x1b[21;11H+-------------------" GREEN "(A) Yes" RESET "----" RED "(B) No" RESET "----------------------+\n");

	temp = ((58 - strlen(line1)) / 2) + 12;
	printf(GREEN "\x1b[14;%dH%s", temp, line1);

	temp2 = strlen(line2);
	if (temp2 > 58)
		temp = 12;
	else
		temp = ((58 - temp2) / 2) + 12;
	printf(MAGENTA "\x1b[17;%dH%s", temp, shortenstring(line2, 58));

	while(1){
	    hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_A)
			return 1;
        if (kDown & KEY_B)
			return 2;
		consoleUpdate(NULL);
	}
}

int copy(const char* src, const char* dst){
    FILE* in = fopen(src, "rb");
    FILE* out = fopen(dst, "wb");
    if(in == NULL)
        return -1;
	if(out == NULL)
		return -2;
    else
    {
        size_t len = 0;
        char buffer[BUFSIZE];
        while((len = fread(buffer, 1, BUFSIZE, in)) > 0)
            fwrite(buffer, 1, len, out);
    }
    if(in)
        fclose(in);
    if(out)
        fclose(out);

	return 0;
}