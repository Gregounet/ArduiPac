#ifndef ARDUIPAC_BIOS_H
#define ARDUIPAC_BIOS_H

#include "arduipac_vmachine.h"

int suck_bios ();
int load_bios (const char *biosname, unsigned char *rom_table[4096], struct resource *app_data);

#endif /* ARDUIPAC_BIOS_H */
