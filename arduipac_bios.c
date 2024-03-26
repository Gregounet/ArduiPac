#include <unistd.h>

#include "arduipac_bios.h"
#include "arduipac.h"


int load_bios (const char *biosname)
{
  FILE *fn = NULL;

  fn = fopen (biosname, "rb");
  if (fn == NULL)
    {
      return O2EM_FAILURE;
    }
  fclose (fn);

  return O2EM_SUCCESS;
}
