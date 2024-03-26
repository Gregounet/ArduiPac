#include "vmachine.h"

#define BIOS_C52 4
#define CRC_C52 0xA318E8D6

int suck_bios ();
int identify_bios (char *biossux, struct resource *app_data);
int load_bios (const char *biosname, unsigned char *rom_table[4096], struct resource *app_data);
int search_for_bios (char *pathx, char *bios_found, int bios_type);
