#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "arduipac.h"
#include "arduipac_roms.h"
#include "arduipac_bios.h"

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
  fn = fopen (file, "rb");
  if (fn == NULL)
    {
      fprintf (stderr, "Error loading %s\n", file);
      return O2EM_FAILURE;
    }
  printf ("Loading: \"%s\"  Size: ", file);


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
