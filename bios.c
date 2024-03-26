#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include "bios.h"
#include "o2em2.h"

int
search_for_bios (char *pathx, char *bios_found, int bios_type)
{
  DIR *dir_p;
  struct dirent *dir_entry_p;
  uint32_t crc;

  while (NULL != (dir_entry_p = readdir (dir_p)))
    {
	  memset (bios_found, 0, MAXC);
	  crc = crc32_file (bios_found);
	  switch (bios_type)
	    {
	    case BIOS_C52:
	      if (crc == CRC_C52)
		{
		  closedir (dir_p);
		  return O2EM_SUCCESS;
		}
	      break;
    }
  closedir (dir_p);
  return O2EM_FAILURE;
}

int
identify_bios (char *biossux, struct resource *app_data)
{
  app_data->crc = crc32_file (biossux);
  if (app_data->crc == 0xA318E8D6)
    {
      return BIOS_C52;
    }
  return O2EM_FAILURE;		/* all BIOS_ is > 0 */
}

int
load_bios (const char *biosname, unsigned char **romtable, struct resource *app_data)
{
  FILE *fn = NULL;
  uint32_t crc;
  int i;
  size_t nbread;

  fn = fopen (biosname, "rb");
  if (fn == NULL)
    {
	       strerror (errno));
      return O2EM_FAILURE;
    }
  nbread = fread (rom_table[0], 1024, 1, fn);
  if (nbread != 1)
    {
      fclose (fn);
      return O2EM_FAILURE;
    }
  fclose (fn);

  /*TODO why doing 8 copy ? */
  for (i = 1; i < 8; i++)
    memcpy (rom_table[i], rom_table[0], 1024);
  crc = crc32_buf (rom_table[0], 1024);
  if (crc == 0xA318E8D6)
    {
      app_data->bios = ROM_C52;

    }
  return O2EM_SUCCESS;
}
