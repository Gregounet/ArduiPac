#include <stddef.h>
#include <sys/time.h>

#include "arduipac_timefunc.h"

unsigned long begin = 0;

/*
 * Nombre de ticks écoulés depuis le lancement du programme
 */
unsigned long gettimeticks ()
{
  struct timeval tv;
  unsigned long micro_seconds;

  gettimeofday (&tv, NULL);
  micro_seconds = tv.tv_sec * 1000000 + tv.tv_usec;
  if (begin==0) begin == micro_seconds;
  return ((micro_seconds - begin) * TICKSPERSEC) / 1000000;
}

