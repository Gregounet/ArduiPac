#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "vmachine.h"
#include "o2em2.h"
#include "vdc.h"
#include "cpu.h"
#include "keyboard.h"
#include "o2em_sdl.h"
#include "roms.h"
#include "bios.h"

static char bios[MAXC];

int main (int argc, char *argv[])
{
  int i, j;
  static char file[MAXC], attr[MAXC], val[MAXC], *p, *binver;
  int ret;

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
  app_data.exrom = 0;
  app_data.three_k = 0;
  app_data.euro = 0;
  app_data.openb = 0;
  app_data.bios = 0;
  strcpy (file, "");
  memset (app_data.bios_filename, 0, sizeof (app_data.bios_filename));
  strcpy (app_data.bios_filename, "o2rom.bin");
  col = NULL;
  colplus = NULL;
  SCREEN_W = 0;
  SCREEN_H = 0;
  DISPLAY_DEPTH = 8;
  screen = NULL;
  font = NULL;

  ret = search_for_rom (app_data.romdir, file, full_path_to_rom);
  ret = load_cart (full_path_to_rom, &app_data);
    ret = search_for_bios (app_data.biosdir, bios, BIOS_C52);
      ret = search_for_bios (app_data.biosdir, bios, 0);
  ret = load_bios (bios, rom_table, &app_data);

  ret = o2em_init_keyboard ();
  if (ret == O2EM_FAILURE) o2em_clean_quit (EXIT_FAILURE);
  ret = install_timer ();
  if (ret != 0)
    {
      o2em_clean_quit (EXIT_FAILURE);
    }

  ret = init_display ();
  if (ret == O2EM_FAILURE)
    {
      printf ("Error of init_display()\n");
      return EXIT_FAILURE;
    }

  init_cpu ();
  init_system ();
  run ();
  o2em_clean_quit (EXIT_SUCCESS);
  return EXIT_SUCCESS;
}

void o2em_clean_quit (int exitcode)
{
  int i, j;

  if (col != NULL)
    {
      free (col);
    }

  if (colplus != NULL)
    {
      free (colplus);
    }
  SDL_Quit ();
  exit (exitcode);
}
