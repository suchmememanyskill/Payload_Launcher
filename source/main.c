#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h> 
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <switch.h>

// ATMOSPHERE CODE -----------------------------------------------------------------

#define IRAM_PAYLOAD_MAX_SIZE 0x2F000
#define IRAM_PAYLOAD_BASE 0x40010000

static alignas(0x1000) u8 g_reboot_payload[IRAM_PAYLOAD_MAX_SIZE];
static alignas(0x1000) u8 g_ff_page[0x1000];
static alignas(0x1000) u8 g_work_page[0x1000];

void do_iram_dram_copy(void *buf, uintptr_t iram_addr, size_t size, int option) {
    memcpy(g_work_page, buf, size);
    
    SecmonArgs args = {0};
    args.X[0] = 0xF0000201;             /* smcAmsIramCopy */
    args.X[1] = (uintptr_t)g_work_page;  /* DRAM Address */
    args.X[2] = iram_addr;              /* IRAM Address */
    args.X[3] = size;                   /* Copy size */
    args.X[4] = option;                 /* 0 = Read, 1 = Write */
    svcCallSecureMonitor(&args);
    
    memcpy(buf, g_work_page, size);
}

void copy_to_iram(uintptr_t iram_addr, void *buf, size_t size) {
    do_iram_dram_copy(buf, iram_addr, size, 1);
}

void copy_from_iram(void *buf, uintptr_t iram_addr, size_t size) {
    do_iram_dram_copy(buf, iram_addr, size, 0);
}

static void clear_iram(void) {
    memset(g_ff_page, 0xFF, sizeof(g_ff_page));
    for (size_t i = 0; i < IRAM_PAYLOAD_MAX_SIZE; i += sizeof(g_ff_page)) {
        copy_to_iram(IRAM_PAYLOAD_BASE + i, g_ff_page, sizeof(g_ff_page));
    }
}

static void reboot_to_payload(void) {
    clear_iram();
    
    for (size_t i = 0; i < IRAM_PAYLOAD_MAX_SIZE; i += 0x1000) {
        copy_to_iram(IRAM_PAYLOAD_BASE + i, &g_reboot_payload[i], 0x1000);
    }
    
    splSetConfig((SplConfigItem)65001, 2);
}

// END ATMOSPHERE CODE -----------------------------------------------------------------

bool majorError = false, cursorchange = false, readytoboot = false, invalidinput = false;
char *list[20];
int i = 0, cursor = 4, lastcursorvalue = 4, t = 0, a = 0;
char *location = 0;
int temp = 1;

void reboottopayload(const char *payloc){
		readytoboot = true;
        Result rc = splInitialize();
        if (R_FAILED(rc)) {
            readytoboot = false;
            printf("\nFailed to init spl!: 0x%x\n", rc);
        }
           		
        FILE *p = fopen(payloc, "rb");
        if (p == NULL) { 
        readytoboot = false; 
        printf("\nFailed to open %s\n", payloc);
        }

        else {
        fread(g_reboot_payload, 1, sizeof(g_reboot_payload), p);
        fclose(p);
        } 

        if (readytoboot == true) reboot_to_payload();
        splExit(); // if the function above fails this will be run
}

char* keyboard(char* message, size_t size){
	SwkbdConfig	skp; 
	Result keyrc = swkbdCreate(&skp, 0);
	char* out = NULL;
	out = (char *)calloc(sizeof(char), size + 1);
	if (out == NULL) out = NULL;
	else if (R_SUCCEEDED(keyrc)){
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

void userAppInit(void)
{
	void *addr = NULL;
	if (svcSetHeapSize(&addr, 0x4000000) == (Result)-1) fatalSimple(0);
}

char* addstrings(const char *s1, const char *s2){
    char *result = malloc(strlen(s1) + strlen(s2) + 1); 
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void refreshscreen(){
    consoleInit(NULL);
    printf("\x1b[40m\x1b[40;1HFavorite a payload (Y)\nLaunch favorite payload (X)\nSelect payload (A)\nCancel (B)\nAdd selected payload as reboot_payload for Atmosphere (Minus)\nQuit (Plus)");
    printf("\x1b[1;1HPayload launcher (Reboot to payload++). \nCurrent dir: %s\n---------------------------------------\n", location);
    cursorchange = true;
}

void copy(const char* src, const char* dst)
{
    FILE* in = fopen(src, "rb");
    FILE* out = fopen(dst, "wb");
    if(in == NULL || out == NULL)
    {
        printf("\nAn error occured");
    }
    else
    {
        size_t len = 0;
        char buffer[BUFSIZ];
        while((len = fread(buffer, 1, BUFSIZ, in)) > 0)
        {
            fwrite(buffer, 1, len, out);
        }
    }
    if(in)
        fclose(in);
    if(out)
        fclose(out);
}

void checkforfolder(char* foldloc){
	DIR *tr = opendir(addstrings(foldloc, "."));
    	if (tr != NULL){
    		list[temp] = foldloc;
    		temp++;
    	}
    closedir(tr);  
}

bool checkfolder(char* foldloc){
	DIR *tr = opendir(addstrings(foldloc, "."));
	bool rtn = false;
	if (tr != NULL){
			rtn = true;
    	}
    closedir(tr);
    return rtn;
}

int main(int argc, char* argv[])
{
	userAppInit();
    begin:
    consoleInit(NULL);
    FILE* file = fopen("payload_config.ini", "rb");
    majorError = false;
    if (file == NULL){
    	temp = 1;
    	if (location != NULL) printf("\x1b[45;1HCancel (B)\x1b[1;1H");
    	else printf("No config detected! Launching inital setup:\n");
    	if (invalidinput == true){
    		printf("\x1b[41mPath does not exist!\x1b[40m\n");
    		invalidinput = false;
    	}
    	printf("Choose your preferred payload folder location:\x1b[44;1HSelect (A)");
    	list[0] = "/payloads/";
    	checkforfolder("/bootloader/payloads/");
    	checkforfolder("/argon/payloads/");
    	checkforfolder("/");
    	list[temp] = "Set custom path...";
    	temp++;
    	cursorchange = true;
    	cursor = 4;
    	while(1){
    		hidScanInput();
      		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

      		consoleUpdate(NULL);

        	if (kDown & KEY_LSTICK_DOWN || kDown & KEY_DDOWN) cursor = cursor + 1, cursorchange = true;
        	if (kDown & KEY_LSTICK_UP || kDown & KEY_DUP) cursor = cursor - 1, cursorchange = true;
        	if (cursor <= 3) cursor = 4, cursorchange = false;
        	if (cursor >= temp + 4) cursor = temp + 4 - 1, cursorchange = false;  

        	if (kDown & KEY_A){
        		location = list[cursor - 4];
        		break;
        	}

        	if (kDown & KEY_B && location != NULL) break;

    		if (cursorchange == true){
        	    cursorchange = false;
        	    printf("\x1b[4;1H");
        	    for (a = 0; temp - 1 >= a; a++){
        	        if (list[a] != NULL){
        	            if (cursor == a + 4) printf("\x1b[42m%s\n", list[a]); 
        	            else printf("\x1b[40m%s\n", list[a]); 
        	        } 
        	    } 
        	}
    	}
    	if (strcmp(location, "Set custom path...") == 0) location = keyboard("Write your custom path here. Don't forget to add the / at the start and the end of the path!", 150);
    	if (checkfolder(location) == false){
    		invalidinput = true;
    		goto begin;
    	}
    	file = fopen ("payload_config.ini", "w");
    	fputs(location, file);
    }
    	else {
    	 	long length;
    		fseek (file, 0L, SEEK_END);
 			length = ftell (file);
 			fseek (file, 0, SEEK_SET);
 			location = calloc( 1, length+1 );
 			if (location)
 			{
 			  fread (location, 1, length, file);
 			}
    	}

    	fclose(file);

    	refreshscreen();

    	list[0] = "";

        struct dirent *de;  
        DIR *dr = opendir(addstrings(location, ".")); 
      

        if (dr == NULL)
        { 
            printf("\x1b[30;1H\x1b[41mError: Could not open payload directory, Defaulting to /payloads...\x1b[4;1H");
            dr = opendir("/payloads/."); 
            if (dr == NULL) mkdir("sdmc:/payloads/", 0777);
            dr = opendir("/payloads/."); 
            location = "/payloads/";
        } 

        i = 0;
        if (majorError == false){
            while ((de = readdir(dr)) != NULL && i != 20){
                if (strstr(de->d_name, ".bin") != NULL){
                    size_t size = strlen(de->d_name) + 1;
                    list[i] = (char*) malloc (size);
                    strlcpy(list[i], de->d_name, size);
                    if (list[i] != NULL) printf("%s\x1b[40m\n", list[i]); 
                    i = i + 1;
                } 
            } 
        } 
 

    if (strcmp(list[0], "") == 0){
    majorError = true;
    printf("\x1b[40m\nError: Folder does not contain payloads\nPress start to exit, press B to select another payload location"); }
    cursor = 4;

    while (appletMainLoop())
    {

        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break;

        if (kDown & KEY_B){
       		remove("payload_config.ini");
       		goto begin;
       	}

        if (majorError == false){
        	if (kDown & KEY_LSTICK_DOWN || kDown & KEY_DDOWN) cursor = cursor + 1, cursorchange = true;
        	if (kDown & KEY_LSTICK_UP || kDown & KEY_DUP) cursor = cursor - 1, cursorchange = true;
        	if (cursor <= 3) cursor = 4, cursorchange = false;
        	if (cursor >= i + 4) cursor = i + 4 - 1, cursorchange = false;  

        	if (cursorchange == true){
        	    cursorchange = false;
        	    printf("\x1b[4;1H");
        	    for (a = 0; i - 1 >= a; a++){
        	        if (list[a] != NULL){
        	            if (cursor == a + 4) printf("\x1b[42m%s\n", list[a]); 
        	            else printf("\x1b[40m%s\n", list[a]); 
        	        } 
        	    } 
        	}

        	if (kDown & KEY_Y){
        	    printf("\x1b[46m\x1b[38;1HAdding %s as favorite payload... ", list[cursor - 4]);
        	    consoleUpdate(NULL);
        	    copy(addstrings(location, list[cursor - 4]), "favorite.payload");
        	    refreshscreen();
        	    printf("\x1b[46m\x1b[38;1H%s added as favorite payload!", list[cursor - 4]);
        	}

        	if (kDown & KEY_MINUS){
        	    printf("\x1b[46m\x1b[38;1HAdding %s as Atmosphere reboot payload... ", list[cursor - 4]);
        	    consoleUpdate(NULL);
        	    copy(addstrings(location, list[cursor - 4]), "/atmosphere/reboot_payload.bin");
        	    refreshscreen();
        	    printf("\x1b[46m\x1b[38;1H%s added as Atmosphere reboot payload! Note: will only take effect after a reboot.", list[cursor - 4]);
        	}

        	if (kDown & KEY_A){
        	    refreshscreen();
        	    printf("\x1b[40m\x1b[4;1HAre you sure you want to launch %s?\n\n", list[cursor - 4]);
        	    	while(1){
        	        hidScanInput();
        	        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        	        	if (kDown & KEY_A) {
        	        	    reboottopayload(addstrings(location, list[cursor - 4]));
        	        	}

        	        if (kDown & KEY_B) break;
        	        consoleUpdate(NULL);
        	   		}
        	    refreshscreen();
        	}

        	if (kDown & KEY_X){
        		reboottopayload("favorite.payload");
        	}
     	}
        consoleUpdate(NULL);
    }

    closedir(dr);  
    consoleExit(NULL);
    return 0;
}