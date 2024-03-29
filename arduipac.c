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
  screen = NULL;

  //install_timer ();
	fprintf(stderr,"  main(): launching init_display()\n");
  init_display ();
	fprintf(stderr,"  main(): launching init_8048()\n");
  init_8048 ();
	fprintf(stderr,"  main(): launching init_system()\n");
  init_system ();
	fprintf(stderr,"  main(): launching exec_8048()\n");
//   for (int i = 0 ; i < 4096 ; i++) { fprintf(stderr, "0x%02X\n", ROM(i)); }
  exec_8048();
}
