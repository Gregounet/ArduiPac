// #include <stdlib.h>
#include <stdint.h>

#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_timefunc.h"
#include "arduipac_vmachine.h"
#include "arduipac_graphics.h"
#include "c52_alien_invaders_usa_eu.h"

static uint8_t x_latch, y_latch;
static int romlatch = 0;
static uint8_t line_count;
static int fps = 60;

int evblclk;

int int_clk;
int master_clk;
int horizontal_clock;
unsigned long clk_counter;

uint8_t mstate;

int pendirq = 0;
int enahirq = 1;

int sprite_offset = 0;

uint8_t intel8245_ram[256];

void handle_vbl ()
{
  draw_region ();
  ext_irq ();
  mstate = 1;
}

void handle_evbl ()
{
  static unsigned long first = 0;
  static unsigned long last = 0;
  static int rest_cnt = 0;
  static unsigned long idx = 0;

  unsigned long delay;
  unsigned long f;
  unsigned long tick_tmp;
  int antiloop;

  rest_cnt = (rest_cnt + 1) % 15;
  master_clk -= evblclk;

  /*
  if (key2vcnt++ > 10)
    {
      key2vcnt = 0;
      for (i = 0; i < KEY_MAX; i++) key2[i] = 0;
      dbstick1 = dbstick2 = 0;
    }
  */

  delay = TICKSPERSEC / fps; // Ticks par frame = délai entre deux trames en ticks
  f = ((delay + last - gettimeticks ()) * 1000) / TICKSPERSEC;
  antiloop = 0;
  idx++;
  if (first == 0) first = gettimeticks () - 1;
  tick_tmp = gettimeticks ();
  if (idx * TICKSPERSEC / (tick_tmp - first) < fps) delay = 0;
  while (gettimeticks () - last < delay && antiloop < 1000000) antiloop++;
  last = gettimeticks ();
  // En gros tout cela constiture une tempo

  mstate = 0;
}

void handle_evbll ()
{
  static unsigned long last = 0;
  static int rest_cnt = 0;
  unsigned long delay;
  unsigned long f;
  int antiloop = 0;
  rest_cnt = (rest_cnt + 1) % 15;

  /*
  if (key2vcnt++ > 10)
    {
      key2vcnt = 0;
      for (i = 0; i < KEY_MAX; i++)
	key2[i] = 0;
      dbstick1 = dbstick2 = 0;
    }
  */

  delay = TICKSPERSEC / fps;
  f = ((delay + last - gettimeticks ()) * 1000) / TICKSPERSEC;
  antiloop = 0;
  while (gettimeticks () - last < delay && antiloop < 1000000) antiloop++;
  last = gettimeticks ();

  mstate = 0;
}

void init_system ()
{
  int i, j, k;
  mstate = 0;
  master_clk = 0;
  horizontal_clock = 0;
  line_count = 0;
  itimer = 0;
  clk_counter = 0;

  for (i = 0; i < 256; i++) intel8245_ram[i] = 0;
  clear_collision ();
}

uint8_t read_t1 ()
{
  if ((horizontal_clock > 16) || (master_clk > VBLCLK)) return 1; // VBLCLK = 5493
  else return 0;
}

/*
 * Appelée par MOVX A, @Rr
 *
 */
uint8_t ext_read (uint8_t addr)
{
  uint8_t data;
  uint8_t si;
  uint8_t m; // Peut-être un uint16_t pour m ?

  if (!(p1 & 0x08) && !(p1 & 0x40))
    {
      switch (addr)
	{
	case 0xA1:
	  data = video_ram[0xA0] & 0x02;
	  if (master_clk > VBLCLK) data = data | 0x08;
	  if (horizontal_clock < (LINECNT - 7)) data = data | 0x01;
	  return data;
	case 0xA2:
	  si = video_ram[0xA2];
	  m = 0x01;
	  data = 0;
	  for (uint8_t i = 0; i < 8; i++)
	    {
	      if (si & m)
		{
		  if (collision_table[1] & m) data = data | (collision_table[1] & (m ^ 0xFF));
		  if (collision_table[2] & m) data = data | (collision_table[2] & (m ^ 0xFF));
		  if (collision_table[4] & m) data = data | (collision_table[4] & (m ^ 0xFF));
		  if (collision_table[8] & m) data = data | (collision_table[8] & (m ^ 0xFF));
		  if (collision_table[0x10] & m) data = data | (collision_table[0x10] & (m ^ 0xFF));
		  if (collision_table[0x20] & m) data = data | (collision_table[0x20] & (m ^ 0xFF));
		  if (collision_table[0x80] & m) data = data | (collision_table[0x80] & (m ^ 0xFF));
		}
	      m = m << 1;
	    }
	  clear_collision ();
	  return data;
	case 0xA4:
	  if (!(video_ram[0xA0] & 0x02)) return y_latch;
	  else
	    {
	      y_latch = master_clk / 22;
	      if (y_latch > 241) y_latch = 0xFF;
	      return y_latch;
	    }
	case 0xA5:
	  if (!(video_ram[0xA0] & 0x02)) return x_latch;
	  else
	    {
	      x_latch = horizontal_clock * 12;
	      return x_latch;
	    }
	default:
	  return video_ram[addr];
	}
    }
  else if (!(p1 & 0x10)) return intel8245_ram[addr];
  else if (  p1 & 0x02)  return ROM((p2 << 8) | addr);

  return 0;
}

void ext_write (uint8_t data, uint8_t addr)
{
  int i;
  int l;

  if (!(p1 & 0x08))
    {
      if (addr == 0xA0)
	{
	  if ((video_ram[0xA0] & 0x02) && !(data & 0x02))
	    {
	      y_latch = master_clk / 22;
	      x_latch = horizontal_clock * 12;
	      if (y_latch > 241) y_latch = 0xFF;
	    }
	  if ((master_clk <= VBLCLK) && (video_ram[0xA0] != data)) draw_region ();
	}
      else if (addr == 0xA3) for (i = l; i < MAXLINES; i++) ; 
      else if ((addr >= 0x40) && (addr <= 0x7F) && ((addr & 2) == 0))
	{
	  /* simulate quad: all 4 sub quad position registers
	   * are mapped to the same internal register */
	  addr = addr & 0x71;
	  if (!(addr & 0x01)) data &= 0xFE;
	  video_ram[addr] = video_ram[addr + 4] = video_ram[addr + 8] = video_ram[addr + 12] = data;
	}
      video_ram[addr] = data;
    }
  else if (!(p1 & 0x10) && !(p1 & 0x40))
    {
      addr = addr & 0xFF;
      if (addr < 0x80) intel8245_ram[addr] = data;
    }
}
