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
    
    if (payload == NULL)
        return 2;
    else {
    fread(g_reboot_payload, 1, sizeof(g_reboot_payload), payload);
    fclose(payload);
    reboot_to_payload();
    }

    splExit();
    return 0;
}

char folder[512] = "", favorite[512] = "";
char *menulist[500];

void loadini(){
    char c;
    int i = 0;
    bool file = false;
    
    FILE *ini = fopen("payload_launcher.ini", "r");

    while ((c = fgetc(ini)) && !feof(ini)){
        if (c == '\n'){
            file = true;
            folder[i+1] = '\0';
            i = 0;
            continue;
        }

        if (!file) folder[i] = c;
        else favorite[i] = c;

        i++;
    }

    favorite[i] = '\0';
    fclose(ini);
}

void writeini(){
     FILE *ini = fopen("payload_launcher.ini", "w+");
     fprintf(ini, "%s\n%s", folder, favorite);
     fclose(ini);
}

void additem(const char *item, int spot){
    size_t size = strlen(item) + 1;
    menulist[spot] = (char*) malloc (size);
    strcpy(menulist[spot], item);
}

void configmenu(){ //this still needs a keyboard impl
    int highlight = 1, item_amout = 1;
    bool update = true;

    additem("/payloads/", 0);
    if(checkfolder("/bootloader/payloads/."))
        additem("/bootloader/payloads/", item_amout++);
    
    if(checkfolder("/argon/payloads/."))
        additem("/argon/payloads/", item_amout++);

    if(checkfolder("/."))
        additem("/", item_amout++);    

    printf("\x1b[1;1H\x1b[47m\x1b[30mPayload_Launcher configuration menu                                             \x1b[0mSelect your desired payload folder\n-----------------------\x1b[44;1H\x1b[32m(A) Continue\n\x1b[31m(B) Cancel\x1b[0m");

    while(1){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_LSTICK_DOWN || kDown & KEY_DDOWN) highlight++, update = true;
        if (kDown & KEY_LSTICK_UP || kDown & KEY_DUP) highlight--, update = true;

        if (highlight > item_amout) highlight = item_amout, update = false;
        else if (highlight < 1) highlight = 1, update = false;

        if (update) {
            printarray(menulist, highlight, 0, 20, item_amout, 5);
            update = false;
        }

        if (kDown & KEY_A) {
            if (highlight == 1)
                if (!checkfolder("/payloads/."))
                    mkdir("sdmc:/payloads/", 0777);

            strcpy(folder, menulist[highlight - 1]);
            writeini();
            break;
        }

        if (kDown & KEY_B)
            break;
        
        consoleUpdate(NULL);
    }
}

int main(int argc, char* argv[])
{
    consoleInit(NULL);

    if (access("payload_launcher.ini", F_OK) != -1) loadini();
    else while(!strcmp(folder, ""))
        configmenu();

    printf("\n%s\n%s\n\nMain test loop exited!", folder, favorite);

    consoleUpdate(NULL);

    while (appletMainLoop()){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS) break;
    }

    /* for (int i = 0; i < 500; i++){
        if (menulist[i] != NULL){
            free(menulist[i]);
        }
    } */

    consoleExit(NULL);
    return 0;    
}