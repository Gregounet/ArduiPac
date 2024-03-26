#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

#include "o2em2.h"
#include "roms.h"
#include "bios.h"

int search_for_rom (char *pathx, char *rom_searched, char *full_path_to_rom)
{
  DIR *dir_p;
  struct dirent *dir_entry_p;

  if (pathx == NULL || full_path_to_rom == NULL || rom_searched == NULL)
    {
      fprintf (stderr, "bad usage of %s\n", __func__);
      return O2EM_FAILURE;
    }
  dir_p = opendir (pathx);
  if (dir_p == NULL)
    {
      fprintf (stderr, "dir '%s' not found !\n", pathx);
      return O2EM_FAILURE;
    }
  while (0 != (dir_entry_p = readdir (dir_p)))
    {
      if (strcmp (dir_entry_p->d_name, ".") != 0
	  && strcmp (dir_entry_p->d_name, "..") != 0)
	{
	  memset (full_path_to_rom, 0, MAXC);
	  snprintf (full_path_to_rom, MAXC, "%s/%s", pathx,
		    dir_entry_p->d_name);
	  if (strcmp (dir_entry_p->d_name, rom_searched) == 0)
	    {
	      closedir (dir_p);
	      return O2EM_SUCCESS;
	    }
	}
    }
  closedir (dir_p);
  return O2EM_FAILURE;
}

long filesize (FILE * stream)
{
  long curpos, length;
  if (stream == NULL)
    {
      fprintf (stderr, "%s stream is NULL\n", __func__);
      return -1;
    }
  curpos = ftell (stream);
  if (curpos == -1)
    {
      fprintf (stderr, "%s Error %s\n", __func__, strerror (errno));
      return -1;
    }
  fseek (stream, 0L, SEEK_END);
  length = ftell (stream);
  fseek (stream, curpos, SEEK_SET);
  return length;
}

int load_cart (char *file, struct resource *app_data)
{
  FILE *fn;
  long l;
  int i, nb;
  size_t nbread;

  if (app_data == NULL)
    {
      fprintf (stderr, "%s app_data NULL\n", __func__);
      return O2EM_FAILURE;
    }
      app_data->need_bios = BIOS_ODYSSEY2;
    }
  fn = fopen (file, "rb");
  if (fn == NULL)
    {
      fprintf (stderr, "Error loading %s\n", file);
      return O2EM_FAILURE;
    }
  printf ("Loading: \"%s\"  Size: ", file);
  l = filesize (fn);

  if ((l % 1024) != 0)
    {
      fprintf (stderr, "Error: file %s is an invalid ROM dump\n", file);
      fclose (fn);
      return O2EM_FAILURE;
    }

  if (((l % 3072) == 0))
    {
      app_data->three_k = 1;
      nb = l / 3072;

      for (i = nb - 1; i >= 0; i--)
	{
	  if (fread (&rom_table[i][1024], 3072, 1, fn) != 1)
	    {
	      fprintf (stderr, "Error loading %s\n", file);
	      fclose (fn);
	      o2em_clean_quit (EXIT_FAILURE);
	    }
	}
      printf ("%dK\n", nb * 3);

    }
  else
    {
      nb = l / 2048;
      if ((nb == 2) && (app_data->exrom))
	{
	  if (fread (&extROM[0], 1024, 1, fn) != 1)
	    {
	      fprintf (stderr, "Error loading %s\n", file);
	      fclose (fn);
	      return O2EM_FAILURE;
	    }
	  if (fread (&rom_table[0][1024], 3072, 1, fn) != 1)
	    {
	      fprintf (stderr, "Error loading %s\n", file);
	      fclose (fn);
	      return O2EM_FAILURE;
	    }
	  printf ("3K EXROM\n");
	}
      else
	{
	  for (i = nb - 1; i >= 0; i--)
	    {
	      nbread = fread (&rom_table[i][1024], 2048, 1, fn);
	      if (nbread != 1)
		{
		  fprintf (stderr, "Error reading %s %s\n", file,
			   strerror (errno));
		  fclose (fn);
		  return O2EM_FAILURE;
		}
	      memcpy (&rom_table[i][3072], &rom_table[i][2048], 1024);	/* simulate missing A10 */
	    }
	  printf ("%dK\n", nb * 2);
	}
    }
  fclose (fn);
  if (nb == 1)
    app_data->bank = 1;
  else if (nb == 2)
    app_data->bank = app_data->exrom ? 1 : 2;
  else if (nb == 4)
    app_data->bank = 3;
  else
    app_data->bank = 4;

  if ((rom_table[nb - 1][1024 + 12] == 'O') &&
      (rom_table[nb - 1][1024 + 13] == 'P')
      && (rom_table[nb - 1][1024 + 14] == 'N')
      && (rom_table[nb - 1][1024 + 15] == 'B'))
    {
      app_data->openb = 1;
      printf ("  openb ROM\n");
    }
  printf ("  %d bank(s)\n", app_data->bank);
  return O2EM_SUCCESS;
}
