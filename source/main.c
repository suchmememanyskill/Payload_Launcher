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

char folder[512], favorite[512];

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

int main(int argc, char* argv[])
{
    consoleInit(NULL);

    if (access("payload_launcher.ini", F_OK) != -1) loadini();

    printf("%s\n%s", folder, favorite);

    //strcpy(folder, "1");
    //strcpy(favorite, "2");

    //writeini();

    consoleUpdate(NULL);

    while (appletMainLoop()){
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        if (kDown & KEY_PLUS) break;
    }

    consoleExit(NULL);
    return 0;    
}