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
	printf("\x1b[1;%dH", starty);
	for (int i = 0; i < max && i < arraylength + offset; i++){
		if (i + offset == highlight - 1) printf("\x1b[42m%s\x1b[0m", array[i + offset]);
		else printf("%s", array[i + offset]);

		for (int i2 = 0; i2 < 40 - strlen(array[i + offset]); i2++) printf(" ");

		printf("\n");
	}
}