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
int amount = 0;
bool noitems = false;

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

    if (!checkfolder(addstrings(folder, ".")))
        strcpy(folder, "");

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

void loadfolder(){
    struct dirent *de;  
    DIR *dr = opendir(addstrings(folder, "."));

    amount = 0;

    while ((de = readdir(dr)) != NULL && amount < 500){
        if (strstr(de->d_name, ".bin") != NULL){
            additem(shortenstring(de->d_name, 79), amount);
            amount++;
        }
    }

    if (amount == 0)
        noitems = true;
    else
        noitems = false;
}

void configmenu(){ //this still needs a keyboard impl
    int highlight = 1, item_amout = 1;
    bool update = true;
    char *folders[6];

    consoleInit(NULL);

    folders[0] = (char*) malloc (20);
    strcpy(folders[item_amout - 1], "/payloads/");

    if(checkfolder("/bootloader/payloads/.")){
        folders[item_amout] = (char*) malloc (50);
        strcpy(folders[item_amout], "/bootloader/payloads/");
        item_amout++;
    }
    
    if(checkfolder("/argon/payloads/.")){
        folders[item_amout] = (char*) malloc (50);
        strcpy(folders[item_amout], "/argon/payloads/");
        item_amout++;
    }

    printf(INV_WHITE BLACK "\x1b[1;1HPayload_Launcher configuration menu                                             " RESET "Select your desired payload folder\n-----------------------" GREEN "\x1b[43;1H(A) Continue\n" RED "(B) Cancel\n" YELLOW "(X) Add custom path" RESET);

    while(1){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_LSTICK_DOWN || kDown & KEY_DDOWN) highlight++, update = true;
        if (kDown & KEY_LSTICK_UP || kDown & KEY_DUP) highlight--, update = true;

        if (highlight > item_amout) highlight = item_amout, update = false;
        else if (highlight < 1) highlight = 1, update = false;

        if (update) {
            //printarray(menulist, highlight, 0, 20, item_amout, 5);
            printarraynew(folders, item_amout, highlight, 0, 5);
            update = false;
        }

        if (kDown & KEY_A) {
            if (highlight == 1)
                if (!checkfolder("/payloads/."))
                    mkdir("sdmc:/payloads/", 0777);

            strcpy(folder, folders[highlight - 1]);
            writeini();
            loadfolder();
            break;
        }

        if (kDown & KEY_X) {
            char* keys = NULL;
            keys = (char*) malloc (512);

            keys = keyboard("Input a folder. Start and end with /", 500);

            if (checkfolder(addstrings(keys, ".")) && strcmp(keys, "")){
                strcpy(folder, keys);
                writeini();
                loadfolder();
                free(keys);
                break;
            }
            free(keys);
        }

        if (kDown & KEY_B)
            break;
        
        consoleUpdate(NULL);
    }
}

void main_menu(){
    int highlight = 1, offset = 0, msgboxres = 0;
    bool update = true;

    while(1){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

        if (!noitems){
            if (kDown & KEY_LSTICK_DOWN || kDown & KEY_DDOWN) 
                highlight++, update = true;

            if (kDown & KEY_LSTICK_UP || kDown & KEY_DUP) 
                highlight--, update = true;

            if (kHeld & KEY_RSTICK_DOWN)
                highlight++, update = true;

            if (kHeld & KEY_RSTICK_UP)
                highlight--, update = true;

            if (highlight + offset > amount) 
                highlight = amount - offset, update = false;

            else if (highlight > MAX_LINES) 
                highlight = MAX_LINES, offset++;

            if (highlight < 1 && offset > 0) 
                highlight = 1, offset--;

            else if (highlight < 1 && offset <= 0) 
                highlight = 1, update = false;

            if (update) {
                printarraynew(menulist, amount, highlight, offset, 5);
                printf(INV_WHITE BLACK "\x1b[1;1HPayload_Launcher main menu                                                      " RESET "Path: %s\n-----------------------" GREEN "\x1b[40;1H(A) Launch Payload\n" RED "(B) Change folder\n" CYAN "(X) Launch favorite payload         \n" MAGENTA "(Y) Set favorite payload                \n" YELLOW "(+) Exit\n" BLUE "(-) Set payload as atmosphere reboot payload" RESET, shortenstring(folder, 70));
                printf(INV_WHITE BLACK "\x1b[1;55H%d / 500 payloads" RESET, amount);
                update = false;
            }

            if (kDown & KEY_A){
                msgboxres = messagebox("Do you want to launch:", menulist[highlight + offset - 1]);
                if (msgboxres == 1)
                    reboot(addstrings(folder, menulist[highlight + offset - 1]));
                update = true;
            }

            if (kDown & KEY_MINUS){
                msgboxres = copy(addstrings(folder, menulist[highlight + offset - 1]), "/atmosphere/reboot_payload.bin");
                if (msgboxres == 0)
                    printf( GREEN "\x1b[45;1HCopy successful                             " RESET);
                else
                    printf( RED "\x1b[45;1HAn error occured (%d)                         " RESET, msgboxres);
            }

            if (kDown & KEY_Y){
                strcpy(favorite, menulist[highlight + offset - 1]);
                writeini();
                printf( GREEN "\x1b[43;1H%s set as favorite!" RESET, shortenstring(menulist[highlight + offset - 1], 23));
            }

            if (kDown & KEY_X){
                if (strcmp(favorite, "") == 0)
                    printf( RED "\x1b[42;1HNo favorite payload is set!" RESET);
                else if (access(addstrings(folder, favorite), F_OK) == -1){
                    printf( RED "\x1b[42;1HFavorite payload file was not found!" RESET);
                    strcpy(favorite, "");
                    writeini();
                }
                else
                    reboot(addstrings(folder, favorite));
            }
        }
        else
            printf(RED "\x1b[3;1HNo payloads detected! press B to go back to the configuration menu" RESET);

        if (kDown & KEY_B){
            highlight = 1;
            configmenu();
            consoleInit(NULL);
            update = true;
        }

        if (kDown & KEY_PLUS)
            break;
        
        consoleUpdate(NULL);
    }
}

int main(int argc, char* argv[])
{
    consoleInit(NULL);

    if (access("payload_launcher.ini", F_OK) != -1) loadini();

    while(!strcmp(folder, ""))
        configmenu();

    consoleInit(NULL);

    loadfolder();

    //printf("\n%s\n%s\n\nMain test loop exited!", folder, favorite);

    //consoleUpdate(NULL);

    main_menu();


    /*
    while (appletMainLoop()){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS) break;
        if (kDown & KEY_B) {
            configmenu();
            consoleInit(NULL);
        }
    }
    */

    /* for (int i = 0; i < 500; i++){
        if (menulist[i] != NULL){
            free(menulist[i]);
        }
    } */

    consoleExit(NULL);
    return 0;    
}