#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "arduipac_vmachine.h"
#include "arduipac_8245.h"
#include "arduipac_8048.h"
#include "arduipac_vmachine.h"
#include "c52_alien_invaders_usa_eu.h"

void main ()
{
	fprintf(stderr,"Entering main()\n");
  //collision = NULL;

  //install_timer ();
  fprintf(stderr,"  main(): launching init_intel8225()\n");
  init_intel8225 ();
  fprintf(stderr,"  main(): launching init_intel8048()\n");
  init_intel8048 ();
  fprintf(stderr,"  main(): launching init_vmachine()\n");
  init_vmachine();
  fprintf(stderr,"  main(): launching exec_8048()\n");
  //   for (uint16_t int i = 0x0 ; i < 0x1000 ; i++) { fprintf(stderr, "0x%02X\n", ROM(i)); }
  exec_8048();
}
