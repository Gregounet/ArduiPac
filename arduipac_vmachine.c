#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac.h"
#include "arduipac_timefunc.h"
#include "arduipac_vmachine.h"
#include "arduipac_sdl.h"

static uint8_t x_latch, y_latch;
static int romlatch = 0;
static uint8_t line_count;
static int fps = FPS_NTSC;

static uint8_t snapedlines[MAXLINES + 2 * MAXSNAP][256][2];

int evblclk = EVBLCLK_NTSC;

struct resource app_data;
int frame = 0;
uint8_t dbstick1, dbstick2;

int int_clk;			/* counter for length of /INT pulses */
int master_clk;			/* Master clock */
int h_clk;			/* horizontal clock */
unsigned long clk_counter;
int last_line;
int key2vcnt = 0;
int mstate;

int pendirq = 0;
int enahirq = 1;
int useforen = 0;
long regionoff = 0xFFFF;
int mxsnap = 2;
int sproff = 0;			/* sprite offset */

uint8_t rom_table[8][4096];

uint8_t intRAM[64];
uint8_t extRAM[256];
uint8_t extROM[1024];

uint8_t VDCwrite[256];
uint8_t ColorVector[MAXLINES];
uint8_t AudioVector[MAXLINES];
uint8_t *rom;

static void setvideomode (int t);

void run ()
{
  while (!key_done)
    {
      exec_8048 ();
    }
  close_display ();
}

void handle_vbl ()
{
  draw_region ();
  ext_irq ();
  mstate = 1;
}

void handle_evbl ()
{
  static unsigned long last = 0;
  static int rest_cnt = 0;
  static unsigned long idx = 0;
  static unsigned long first = 0;
  int i;
  unsigned long d, f, tick_tmp;
  int antiloop;

  i = (15 * app_data.speed / 100);
  rest_cnt = (rest_cnt + 1) % (i < 5 ? 5 : i);
  last_line = 0;
  master_clk -= evblclk;
  frame++;

    for (i = 0; i < MAXLINES; i++)
      {
	ColorVector[i] = (VDCwrite[0xA3] & 0x7f) | (p1 & 0x80);
	AudioVector[i] = VDCwrite[0xAA];
      }

  if (key2vcnt++ > 10)
    {
      key2vcnt = 0;
      for (i = 0; i < KEY_MAX; i++)
	key2[i] = 0;
      dbstick1 = dbstick2 = 0;
    }
  if (app_data.limit)
    {
      d = (TICKSPERSEC * 100) / (app_data.speed * fps);
      f = ((d + last - gettimeticks ()) * 1000) / TICKSPERSEC;
      antiloop = 0;
      idx++;
      if (first == 0)
	first = gettimeticks () - 1;
      tick_tmp = gettimeticks ();
      if (idx * TICKSPERSEC / (tick_tmp - first) < fps)
	d = 0;
      if (f > 0)
	{
	}
      while (gettimeticks () - last < d && antiloop < 1000000)
	{
	  antiloop++;
	}
      if (antiloop >= 1000000)
	{
	  printf ("antiloop %d\n", antiloop);
	}
      last = gettimeticks ();
    }
  mstate = 0;

}

void handle_evbll ()
{
  static unsigned long last = 0;
  static int rest_cnt = 0;
  int i;
  unsigned long d, f;
  int antiloop = 0;
  i = (15 * app_data.speed / 100);
  rest_cnt = (rest_cnt + 1) % (i < 5 ? 5 : i);

  for (i = 150; i < MAXLINES; i++)
    {
      ColorVector[i] = (VDCwrite[0xA3] & 0x7f) | (p1 & 0x80);
      AudioVector[i] = VDCwrite[0xAA];
    }

  if (key2vcnt++ > 10)
    {
      key2vcnt = 0;
      for (i = 0; i < KEY_MAX; i++)
	key2[i] = 0;
      dbstick1 = dbstick2 = 0;
    }
  if (app_data.limit)
    {
      d = (TICKSPERSEC * 100) / (app_data.speed * fps);
      f = ((d + last - gettimeticks ()) * 1000) / TICKSPERSEC;
      antiloop = 0;
      while (gettimeticks () - last < d && antiloop < 1000000)
	{
	  antiloop++;
	}
      if (antiloop >= 1000000)
	{
	  printf ("antiloop %d\n", antiloop);
	}
      last = gettimeticks ();
    }
  mstate = 0;
}

void init_system ()
{
  int i, j, k;
  last_line = 0;
  dbstick1 = 0x00;
  dbstick2 = 0x00;
  mstate = 0;
  master_clk = 0;
  h_clk = 0;
  line_count = 0;
  itimer = 0;
  clk_counter = 0;

  init_roms ();

  for (i = 0; i < 256; i++)
    {
      VDCwrite[i] = 0;
      extRAM[i] = 0;
    }
  for (i = 0; i < 64; i++)
    intRAM[i] = 0;

  for (i = 0; i < MAXLINES; i++)
    AudioVector[i] = ColorVector[i] = 0;

  for (i = 0; i < MAXLINES + 2 * MAXSNAP; i++)
    for (j = 0; j < 256; j++)
      for (k = 0; k < 2; k++)
	snapedlines[i][j][k] = 0;

  if (app_data.stick[0] == 2 || app_data.stick[1] == 2)
    {
      i = install_joystick (JOY_TYPE_AUTODETECT);
      if (i || (num_joysticks < 1))
	{
	  fprintf (stderr, "Error: no joystick detected\n");
	  o2em_clean_quit (EXIT_FAILURE);
	}
    }
  for (i = 0; i < KEY_MAX; i++) key2[i] = 0;
  key2vcnt = 0;

  setvideomode (1);

  clear_collision ();
}

void init_roms ()
{
  rom = rom_table[0];
  romlatch = 0;
}

uint8_t read_t1 ()
{
  /*17 */
  if ((h_clk > 16) || (master_clk > VBLCLK))
    return 1;
  else
    return 0;
}

void write_p1 (uint8_t d)
{
  if ((d & 0x80) != (p1 & 0x80))
    {
      int i, l;
      l =
	snapline ((int) ((float) master_clk / 22.0 + 0.1), VDCwrite[0xA3], 1);
      for (i = l; i < MAXLINES; i++)
	ColorVector[i] = (VDCwrite[0xA3] & 0x7f) | (d & 0x80);
    }
  p1 = d;
  if (app_data.bank == 2)
    {
      rom = rom_table[~p1 & 0x01];
    }
  else if (app_data.bank == 3)
    {
      rom = rom_table[~p1 & 0x03];
    }
  else if (app_data.bank == 4)
    {
      rom = rom_table[(p1 & 1) ? 0 : romlatch];
    }
}

uint8_t read_P2 ()
{
  int i, si, so, km;

  if (NeedsPoll)
    poll_keyboard ();

  if (!(p1 & 0x04))
    {
      si = (p2 & 7);
      so = 0xff;
      if (si < 6)
	{
	  for (i = 0; i < 8; i++)
	    {
	      km = key_map[si][i];
	      if ((key[km]
		   && ((!joykeystab[km]) || (key_shifts & KB_CAPSLOCK_FLAG)))
		  || (key2[km]))
		{
		  so = i ^ 0x07;
		}
	    }
	}
      if (so != 0xff)
	{
	  p2 = p2 & 0x0F;
	  p2 = p2 | (so << 5);
	}
      else
	{
	  p2 = p2 | 0xF0;
	}
    }
  else
    {
      p2 = p2 | 0xF0;
    }
  return p2;
}

uint8_t ext_read (uint16_t adr)
{
  uint8_t d;
  uint8_t si;
  uint8_t m;
  int i;

  if (!(p1 & 0x08) && !(p1 & 0x40))
    {
      /* Handle VDC Read */
      switch (adr)
	{
	case 0xA1:
	  d = VDCwrite[0xA0] & 0x02;
	  if (master_clk > VBLCLK)
	    d = d | 0x08;
	  if (h_clk < (LINECNT - 7))
	    d = d | 0x01;
	  if (sound_IRQ)
	    d = d | 0x04;
	  sound_IRQ = 0;
	  return d;
	case 0xA2:		/* 0xA2 vdc_collision http://soeren.informationstheater.de/g7000/hardware.html */
	  si = VDCwrite[0xA2];
	  m = 0x01;
	  d = 0;
	  for (i = 0; i < 8; i++)
	    {
	      if (si & m)
		{
		  if (coltab[1] & m)
		    d = d | (coltab[1] & (m ^ 0xFF));
		  if (coltab[2] & m)
		    d = d | (coltab[2] & (m ^ 0xFF));
		  if (coltab[4] & m)
		    d = d | (coltab[4] & (m ^ 0xFF));
		  if (coltab[8] & m)
		    d = d | (coltab[8] & (m ^ 0xFF));
		  if (coltab[0x10] & m)
		    d = d | (coltab[0x10] & (m ^ 0xFF));
		  if (coltab[0x20] & m)
		    d = d | (coltab[0x20] & (m ^ 0xFF));
		  if (coltab[0x80] & m)
		    d = d | (coltab[0x80] & (m ^ 0xFF));
		}
	      m = m << 1;
	    }
	  clear_collision ();
	  return d;
	case 0xA5:
	  if (!(VDCwrite[0xA0] & 0x02))
	    {
	      return x_latch;
	    }
	  else
	    {
	      x_latch = h_clk * 12;
	      return x_latch;
	    }
	case 0xA4:
	  if (!(VDCwrite[0xA0] & 0x02))
	    {
	      return y_latch;
	    }
	  else
	    {
	      y_latch = master_clk / 22;
	      if (y_latch > 241)
		y_latch = 0xFF;
	      return y_latch;
	    }
	default:
	  return VDCwrite[adr];
	}
    }
  else if (!(p1 & 0x10))
    {
      return extRAM[adr & 0xFF];
    }
  else if (app_data.exrom && (p1 & 0x02))
    {
      return extROM[(p2 << 8) | (adr & 0xFF)];
    }

  return 0;
}

uint8_t in_bus ()
{
  uint8_t si = 0, d = 0, mode = 0, jn = 0, sticknum = 0;

  if ((p1 & 0x08) && (p1 & 0x10))
    {
      if (!(p1 & 0x04))
	{
	  si = (p2 & 7);
	}
      d = 0xFF;
      if (si == 1)
	{
	  mode = app_data.stick[0];
	  jn = 0;
	  sticknum = app_data.sticknumber[0] - 1;
	}
      else
	{
	  mode = app_data.stick[1];
	  jn = 1;
	  sticknum = app_data.sticknumber[1] - 1;
	}
      switch (mode)
	{
	case 1:
	  d = keyjoy (jn);
	  break;
	case 2:
	  if (joy[sticknum].stick[0].axis[1].d1)
	    d &= 0xFE;		/* joy_up */
	  if (joy[sticknum].stick[0].axis[0].d2)
	    d &= 0xFD;		/* joy_right */
	  if (joy[sticknum].stick[0].axis[1].d2)
	    d &= 0xFB;		/* joy_down */
	  if (joy[sticknum].stick[0].axis[0].d1)
	    d &= 0xF7;		/* joy_left */
	  if (joy[sticknum].button[0].b || joy[jn].button[1].b)
	    d &= 0xEF;		/* both main-buttons */
	  break;
	}
      if (si == 1)
	{
	  if (dbstick1)
	    d = dbstick1;
	}
      else
	{
	  if (dbstick2)
	    d = dbstick2;
	}
    }
  return d;
}

void ext_write (uint8_t dat, uint16_t adr)
{
  int i;
  int l;

  if (!(p1 & 0x08))
    {
      if (adr == 0xA0)
	{
	  if ((VDCwrite[0xA0] & 0x02) && !(dat & 0x02))
	    {
	      y_latch = master_clk / 22;
	      x_latch = h_clk * 12;
	      if (y_latch > 241)
		y_latch = 0xFF;
	    }
	  if ((master_clk <= VBLCLK) && (VDCwrite[0xA0] != dat))
	    {
	      draw_region ();
	    }
	}
      else if (adr == 0xA3)
	{
	  l = snapline ((int) ((float) master_clk / 22.0 + 0.5), dat, 1);
	  for (i = l; i < MAXLINES; i++)
	    ColorVector[i] = (dat & 0x7f) | (p1 & 0x80);
	}
      else if (adr == 0xAA)
	{
	  for (i = master_clk / 22; i < MAXLINES; i++)
	    AudioVector[i] = dat;
	}
      else if ((adr >= 0x40) && (adr <= 0x7f) && ((adr & 2) == 0))
	{
	  /* simulate quad: all 4 sub quad position registers
	   * are mapped to the same internal register */
	  adr = adr & 0x71;
	  /* Another minor thing: the y register always returns
	   * bit 0 as 0 */
	  if ((adr & 1) == 0)
	    dat = dat & 0xfe;
	  VDCwrite[adr] = VDCwrite[adr + 4] = VDCwrite[adr + 8] =
	    VDCwrite[adr + 12] = dat;
	}
      VDCwrite[adr] = dat;
    }
  else if (!(p1 & 0x10) && !(p1 & 0x40))
    {
      adr = adr & 0xFF;

      if (adr < 0x80)
	{
	  /* Handle ext RAM Write */
	  extRAM[adr] = dat;
	}
      else
	{
	  if (app_data.bank == 4)
	    {
	      romlatch = (~dat) & 7;
	      rom = rom_table[(p1 & 1) ? 0 : romlatch];
	    }
	}
    }
  else if (!(p1 & 0x20))
    {
      /* Write to a Videopac+ register */
    }
}

int snapline (int pos, uint8_t reg, int t)
{
  int i;
  if (pos < MAXLINES + MAXSNAP + MAXSNAP)
    {
      for (i = 0; i < mxsnap; i++)
	{
	  if (snapedlines[pos + MAXSNAP - i][reg][t])
	    return pos - i;
	  if (snapedlines[pos + MAXSNAP + i][reg][t])
	    return pos + i;
	}
      snapedlines[pos + MAXSNAP][reg][t] = 1;
    }
  return pos;
}

static void setvideomode (int t)
{
  if (t)
    {
      evblclk = EVBLCLK_PAL;
      fps = FPS_PAL;
    }
  else
    {
      evblclk = EVBLCLK_NTSC;
      fps = FPS_NTSC;
    }
}

#define check_return_of_fxxx(ret, fn) if (ret == 0) { printf("%s:%d ERROR %s\n", __func__, __LINE__, strerror(errno)); fclose(fn); return -1;}

void read_a_char (uint16_t * c, size_t s, FILE * fn)
{
  if (fn == NULL || c == NULL || s == 0)
    {
      fprintf (stderr, "%s Error invalid parameter\n", __func__);
      return;
    }
  if (fread (c, s, 1, fn) < 1)
    {
      fprintf (stderr, "Error fread %s: %s\n", __func__, strerror (errno));
      o2em_clean_quit (EXIT_FAILURE);
    }
}
