#include <stdlib.h>
#include <stdio.h>
#include <switch.h>
#include <dirent.h> 
#include <sys/stat.h>
#include "menu.h"
#include "sdl_helper.h"

#define BUFSIZE 32768

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

int reboot(const char *payloc){
    Result rc = splInitialize();

    if (R_FAILED(rc)) return 1;

    FILE *payload = fopen(payloc, "rb");
    
    if (payload == NULL){
        MakeNotification("Payload location invalid!", 200, COLOR_RED, COLOR_BLACK);
        return 2;
    }
    else {
        fread(g_reboot_payload, 1, sizeof(g_reboot_payload), payload);
        fclose(payload);
        reboot_to_payload();
    }

    splExit();
    return 0;
}

int reboottoRCM(){
    Result rc = splInitialize();

    if (R_FAILED(rc)) 
        return 1;

    rc = splSetConfig((SplConfigItem) 65001, 1);

    if (R_FAILED(rc)) 
        return 2;
    
    return 3;
}

void AddBasicInfo(){
    InitControllerInfo();
    AddControllerInfo("Select", BUTTON_A, KEY_A);
    AddControllerInfo("Exit", BUTTON_PLUS, KEY_PLUS);
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

void *heap_addr;
extern char *fake_heap_end;

/*
void userAppInit(void){
    if(R_SUCCEEDED(svcSetHeapSize(&heap_addr, 0x4000000))) fake_heap_end = (char*)heap_addr + 0x4000000;
}

void userAppExit(void){
    svcSetHeapSize(&heap_addr, ((u8*)envGetHeapOverrideAddr() + envGetHeapOverrideSize()) - (u8*)heap_addr);
}
*/

bool checkfolder(char* foldloc){
	DIR *tr = opendir(foldloc);
	bool folderexists = false;
	if (tr != NULL){
			folderexists = true;
    	}
    closedir(tr);
    return folderexists;
}

char config[2][255] = {"", ""};

int loadini(){
    FILE *ini = fopen("payload_launcher.ini", "r");
    if (ini == NULL)
        return 1;

    fscanf(ini, "%s\n%s", config[0], config[1]);
    fclose(ini);
    return 0;
}

void writeini(){
    FILE *ini = fopen("payload_launcher.ini", "w+");
    fprintf(ini, "%s\n%s", config[0], config[1]);
    fclose(ini);
}

menu_item extraMenu[] = {
    {"Reboot to RCM", 0},
    {"Reboot to atmosphere/reboot_payload.bin", 0}
};

int folderAmount = 1;
menu_item folderMenu[5] = {
    {"/payloads/", 0}
};

int payloadAmount = 0;
menu_item payloadMenu[500];

menu_item payloadErrorMenu[1] = {
    {"Error! Invalid or empty folder. Change the folder or put payloads in the folder.", 0}
};

char* keyboard(char* message, size_t size){
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

void FillFolderMenu(){
    if (checkfolder("/bootloader/payloads/."))
        folderMenu[folderAmount++] = (menu_item){"/bootloader/payloads/", 0};

    if (checkfolder("/argon/payloads/."))
        folderMenu[folderAmount++] = (menu_item){"/argon/payloads/", 0};

    if (checkfolder("/."))
        folderMenu[folderAmount++] = (menu_item){"/", 0};

    folderMenu[folderAmount++] = (menu_item){"Set custom path...", 0};
}

void readFolder(char *path){
    struct dirent *de;
    DIR *dr = opendir(path);
    payloadAmount = 0;

    while ((de = readdir(dr)) != NULL && payloadAmount < 500){
        if (strstr(de->d_name, ".bin") != NULL){
            strcpy(payloadMenu[payloadAmount].name, de->d_name);
            payloadMenu[payloadAmount].property = 0;
            payloadAmount++;
        }
    }
}

char* addstrings(const char *s1, const char *s2){
    static char *result;
    if (result != NULL){
        free(result);
        result = NULL;
    }
    result = malloc(strlen(s1) + strlen(s2) + 1); 
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(int argc, char *argv[]){
    int menuselect = 1;
    u64 kCheck = 0;

    Result rc;
    rc = plInitialize(0);
    SDLInit();

    InitTopMenu();
    AddTopMenu("Launch");
    AddTopMenu("Payload Folder");
    AddTopMenu("Extras");

    FillFolderMenu();
    loadini();
    InitController();

    while(1){
        AddBasicInfo();
        switch (menuselect){
            case 1:
                readFolder(addstrings(config[0], "."));
                if (payloadAmount == 0){
                    InitControllerInfo();
                    AddControllerInfo("Exit", BUTTON_PLUS, KEY_PLUS);
                    kCheck = MakeBasicMenu(payloadErrorMenu, 1);
                }
                else {
                    AddControllerInfo("Set AMS Payload", BUTTON_MINUS, KEY_MINUS);
                    AddControllerInfo("Set Favorite Payload", BUTTON_Y, KEY_Y);

                    if (strcmp(config[1], ""))
                        AddControllerInfo("Launch Favorite Payload", BUTTON_X, KEY_X);

                    kCheck = MakeBasicMenu(payloadMenu, payloadAmount);
                }

                if (kCheck & KEY_A){
                    if (MakeMessageBox("Back", "Launch", "Reboot to payload", addstrings("Do you want to reboot to: ", payloadMenu[GetSelection() - 1].name)))
                        reboot(addstrings(config[0], payloadMenu[GetSelection() - 1].name));
                }

                if (kCheck & KEY_MINUS){
                    if (!copy(addstrings(config[0], payloadMenu[GetSelection() - 1].name), "/atmosphere/reboot_payload.bin"))
                        MakeNotification("Copy Successful!", 200, COLOR_GREEN, COLOR_BLACK);
                    else
                        MakeNotification("Copy Failed!", 200, COLOR_RED, COLOR_BLACK);
                }

                if (kCheck & KEY_Y){
                    strcpy(config[1], addstrings(config[0], payloadMenu[GetSelection() - 1].name));
                    writeini();
                    MakeNotification("Favorite payload set!", 200, COLOR_GREEN, COLOR_BLACK);
                }

                if (kCheck & KEY_X){
                    if (reboot(config[1])){
                        strcpy(config[1], "");
                        writeini();
                    }
                }

                break;
            case 2:
                kCheck = MakeBasicMenu(folderMenu, folderAmount);

                if (kCheck & KEY_A && GetSelection() != folderAmount){
                    strcpy(config[0], folderMenu[GetSelection() - 1].name);

                    if (GetSelection() == 1)
                        mkdir("sdmc:/payloads/", 0777);

                    MakeNotification(addstrings(folderMenu[GetSelection() - 1].name, " set as Folder!"), 200, COLOR_GREEN, COLOR_BLACK);
                    writeini();
                }

                else if (kCheck & KEY_A && GetSelection() == folderAmount){
                    char *temp;
                    temp = keyboard("Enter a path. Begin and end with /", 254);

                    if (checkfolder(addstrings(temp, ".")) && strcmp(temp, "")){
                        strcpy(config[0], temp);
                        MakeNotification(addstrings(temp, " set as Folder!"), 200, COLOR_GREEN, COLOR_BLACK);
                        writeini();
                    }
                    else if (strcmp(temp, "")){
                        MakeNotification("Invalid path!", 200, COLOR_RED, COLOR_BLACK);
                    }
                }

                break;
            case 3:
                kCheck = MakeBasicMenu(extraMenu, 2);

                if (kCheck & KEY_A){
                    switch (GetSelection()){
                        case 1: // reboot to rcm
                            if (MakeMessageBox("Back", "Reboot to RCM", "Reboot to RCM", "Are you sure you want to reboot to RCM?"))
                                reboottoRCM();
                            break;
                        
                        case 2: //reboot to ams payload
                            if (MakeMessageBox("Back", "Launch", "Reboot to Payload", "Do you want to reboot to reboot_payload.bin?"))
                                reboot("/atmosphere/reboot_payload.bin");
                            break;
                    }
                }
                break;
        }

        menuselect = GetTopMenuSelection();

        if (kCheck & KEY_PLUS)
            break;
    }

    SDLDeInit();
    return 0;
}