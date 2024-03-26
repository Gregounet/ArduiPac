#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "arduipac_vmachine.h"
#include "arduipac.h"
#include "arduipac_8245.h"
#include "arduipac_8048.h"
#include "arduipac_sdl.h"
#include "arduipac_roms.h"
#include "arduipac_bios.h"

void main ()
{
  memset (&app_data, 0, sizeof (app_data));
  app_data.stick[0] = app_data.stick[1] = 1;
  app_data.sticknumber[0] = app_data.sticknumber[1] = 0;
  set_defjoykeys (0, 0);
  set_defjoykeys (1, 1);
  set_defsystemkeys ();
  app_data.bank = 0;
  app_data.limit = 1;
  app_data.speed = 100;
  app_data.wsize = 2;
  app_data.scanlines = 0;
  app_data.filter = 0;
  app_data.openb = 0;
  app_data.bios = 0;
  strcpy (file, "");
  memset (app_data.bios_filename, 0, sizeof (app_data.bios_filename));
  strcpy (app_data.bios_filename, "o2rom.bin");
  col = NULL;
  SCREEN_W = 0;
  SCREEN_H = 0;
  DISPLAY_DEPTH = 8;
  screen = NULL;
  font = NULL;

  load_cart (full_path_to_rom, &app_data);
  load_bios (bios, rom_table, &app_data);

  install_timer ();
  init_display ();
  init_8048 ();
  init_system ();
  run ();
}
