#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "arduipac_vmachine.h"
#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_bios_rom.h"

#define DEBUG 0

void
main ()
{
  if (DEBUG) fprintf(stderr,"Entering main()\n");
  //collision = NULL;

  //install_timer ();
  if (DEBUG) fprintf(stderr,"  main(): launching init_intel8225()\n");
  init_intel8225 ();
  if (DEBUG) fprintf(stderr,"  main(): launching init_intel8048()\n");
  init_intel8048 ();
  if (DEBUG) fprintf(stderr,"  main(): launching init_vmachine()\n");
  init_vmachine ();
  if (DEBUG) fprintf(stderr,"  main(): launching exec_8048()\n");
  //for (uint16_t int i = 0x0 ; i < 0x1000 ; i++) { fprintf(stderr, "0x%02X\n", ROM(i)); }
  exec_8048 ();
}
