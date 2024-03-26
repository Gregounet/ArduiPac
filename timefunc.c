#include <time.h>
#include <SDL/SDL.h>

#include "timefunc.h"

static time_t first = 0;

unsigned long gettimeticks ()
{
  struct timeval tv;
  time_t ll;
  unsigned long ret;
  if (first == 0)
    {
      if (gettimeofday (&tv, NULL) != 0)
	return ((clock ()) * TICKSPERSEC) / CLOCKS_PER_SEC;
      first = (tv.tv_sec) * 1000000 + tv.tv_usec;
    }
  if (gettimeofday (&tv, NULL) == 0)
    {
      ll = (tv.tv_sec) * 1000000 + tv.tv_usec;
      ret = ((ll - first) * TICKSPERSEC) / 1000000;
      return ret;
    }
  return ((clock ()) * TICKSPERSEC) / CLOCKS_PER_SEC;
}

volatile long ticks_counter = 0;
static int timer_init = 0;

void inc_ticks_counter ()
{
  ticks_counter++;
}

unsigned long gettimeticks ()
{
  if (timer_init == 0)
    {
      ticks_counter = 0;
      LOCK_VARIABLE (ticks_counter);
      LOCK_FUNCTION (inc_ticks_counter);
      install_int (inc_ticks_counter, 1);
      timer_init = 1;
    }
  return ((LONG_LONG) ticks_counter * TICKSPERSEC) / 1000;
}
