#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "arduipac_vmachine.h"
#include "arduipac_8245.h"
#include "arduipac_8048.h"
#include "arduipac_sdl.h"

void main ()
{
  col = NULL;
  SCREEN_W = 0;
  SCREEN_H = 0;
  screen = NULL;

  install_timer ();
  init_display ();
  init_8048 ();
  init_system ();
  run ();
}
