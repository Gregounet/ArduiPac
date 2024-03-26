#include <stdlib.h>

#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_timefunc.h"
#include "arduipac_vmachine.h"
#include "arduipac_graphics.h"

static uint8_t x_latch, y_latch;
static int romlatch = 0;
static uint8_t line_count;
static int fps;

int evblclk;

int frame = 0;

int int_clk;			/* counter for length of /INT pulses */
int master_clk;			/* Master clock */
int h_clk;			/* horizontal clock */
unsigned long clk_counter;
int key2vcnt = 0;
int mstate;

int pendirq = 0;
int enahirq = 1;
long regionoff = 0xFFFF;
int sproff = 0;			/* sprite offset */

uint8_t external_ram[256];
uint8_t external_rom[1024];

void run ()
{
  exec_8048 ();
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

  i = 15 ;
  rest_cnt = (rest_cnt + 1) % (i < 5 ? 5 : i);
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
      for (i = 0; i < KEY_MAX; i++) key2[i] = 0;
      dbstick1 = dbstick2 = 0;
    }

  d = TICKSPERSEC / fps);
  f = ((d + last - gettimeticks ()) * 1000) / TICKSPERSEC;
  antiloop = 0;
  idx++;
  if (first == 0) first = gettimeticks () - 1;
  tick_tmp = gettimeticks ();
  if (idx * TICKSPERSEC / (tick_tmp - first) < fps) d = 0;
  while (gettimeticks () - last < d && antiloop < 1000000) antiloop++;
  last = gettimeticks ();

  mstate = 0;

}

void handle_evbll ()
{
  static unsigned long last = 0;
  static int rest_cnt = 0;
  int i;
  unsigned long d, f;
  int antiloop = 0;
  i = 15;
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

  d = TICKSPERSEC / fps;
  f = ((d + last - gettimeticks ()) * 1000) / TICKSPERSEC;
  antiloop = 0;
  while (gettimeticks () - last < d && antiloop < 1000000) antiloop++;
  last = gettimeticks ();
  mstate = 0;
}

void init_system ()
{
  int i, j, k;
  dbstick1 = 0x00;
  dbstick2 = 0x00;
  mstate = 0;
  master_clk = 0;
  h_clk = 0;
  line_count = 0;
  itimer = 0;
  clk_counter = 0;

  for (i = 0; i < 256; i++) external_ram[i] = 0;

  clear_collision ();
}

uint8_t read_t1 ()
{
  if ((h_clk > 16) || (master_clk > VBLCLK)) return 1;
  else return 0;
}

uint8_t ext_read (uint16_t addr)
{
  uint8_t d;
  uint8_t si;
  uint8_t m;
  int i;

  if (!(p1 & 0x08) && !(p1 & 0x40))
    {
      /* Handle VDC Read */
      switch (addr)
	{
	case 0xA1:
	  d = VDCwrite[0xA0] & 0x02;
	  if (master_clk > VBLCLK) d = d | 0x08;
	  if (h_clk < (LINECNT - 7)) d = d | 0x01;
	  if (sound_IRQ) d = d | 0x04;
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
		  if (collision_table[1] & m) d = d | (collision_table[1] & (m ^ 0xFF));
		  if (collision_table[2] & m) d = d | (collision_table[2] & (m ^ 0xFF));
		  if (collision_table[4] & m) d = d | (collision_table[4] & (m ^ 0xFF));
		  if (collision_table[8] & m) d = d | (collision_table[8] & (m ^ 0xFF));
		  if (collision_table[0x10] & m) d = d | (collision_table[0x10] & (m ^ 0xFF));
		  if (collision_table[0x20] & m) d = d | (collision_table[0x20] & (m ^ 0xFF));
		  if (collision_table[0x80] & m) d = d | (collision_table[0x80] & (m ^ 0xFF));
		}
	      m = m << 1;
	    }
	  clear_collision ();
	  return d;
	case 0xA5:
	  if (!(VDCwrite[0xA0] & 0x02)) return x_latch;
	  else
	    {
	      x_latch = h_clk * 12;
	      return x_latch;
	    }
	case 0xA4:
	  if (!(VDCwrite[0xA0] & 0x02)) return y_latch;
	  else
	    {
	      y_latch = master_clk / 22;
	      if (y_latch > 241) y_latch = 0xFF;
	      return y_latch;
	    }
	default:
	  return VDCwrite[addr];
	}
    }
  else if (!(p1 & 0x10)) return external_ram[addr & 0xFF];
  else if (p1 & 0x02))return external_rom[(p2 << 8) | (addr & 0xFF)];

  return 0;
}

void ext_write (uint8_t dat, uint16_t addr)
{
  int i;
  int l;

  if (!(p1 & 0x08))
    {
      if (addr == 0xA0)
	{
	  if ((VDCwrite[0xA0] & 0x02) && !(dat & 0x02))
	    {
	      y_latch = master_clk / 22;
	      x_latch = h_clk * 12;
	      if (y_latch > 241) y_latch = 0xFF;
	    }
	  if ((master_clk <= VBLCLK) && (VDCwrite[0xA0] != dat)) draw_region ();
	}
      else if (addr == 0xA3)
	{
	  l = snapline ((int) ((float) master_clk / 22.0 + 0.5), dat, 1);
	  for (i = l; i < MAXLINES; i++) ColorVector[i] = (dat & 0x7f) | (p1 & 0x80);
	}
      else if (addr == 0xAA)
	{
	  for (i = master_clk / 22; i < MAXLINES; i++) AudioVector[i] = dat;
	}
      else if ((addr >= 0x40) && (addr <= 0x7f) && ((addr & 2) == 0))
	{
	  /* simulate quad: all 4 sub quad position registers
	   * are mapped to the same internal register */
	  addr = addr & 0x71;
	  /* Another minor thing: the y register always returns
	   * bit 0 as 0 */
	  if ((addr & 1) == 0) dat = dat & 0xfe;
	  VDCwrite[addr] = VDCwrite[addr + 4] = VDCwrite[addr + 8] = VDCwrite[addr + 12] = dat;
	}
      VDCwrite[addr] = dat;
    }
  else if (!(p1 & 0x10) && !(p1 & 0x40))
    {
      addr = addr & 0xFF;
      if (addr < 0x80) external_ram[addr] = dat;
      else if (app_data.bank == 4) romlatch = (~dat) & 7;
    }
}
