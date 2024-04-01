#include <stdint.h>
#include <stdio.h>

#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_timefunc.h"
#include "arduipac_vmachine.h"
#include "arduipac_graphics.h"
#include "c52_alien_invaders_usa_eu.h"

#define FPS 50

uint8_t x_latch, y_latch;
uint8_t romlatch = 0;

uint32_t int_clk;
uint32_t master_clk;
uint32_t horizontal_clock;
uint8_t mstate;

uint8_t external_ram[256];

void init_vmachine ()
{
  fprintf(stderr,"Entering init_vmachine()\n");
  master_clk = 0;
  horizontal_clock = 0;
  mstate = 0;

  fprintf(stderr," Initializing external_ram\n");
  for (uint8_t i = 0x00 ; i < 0xFF ; i++) external_ram[i] = 0x00;
  fprintf(stderr," Launching clear_collision()\n");
  clear_collision ();
}

void handle_vbl ()
{
  //fprintf(stderr,"Entering handle_vbl()\n");
  draw_region ();
  ext_irq ();
  mstate = 1;
  ////fprintf(stderr,"Leaving handle_vbl()\n");
}

void handle_evbl ()
{
  static unsigned long first = 0;
  static unsigned long last = 0;
  static unsigned int rest_cnt = 0;
  static unsigned long idx = 0;

  unsigned long delay;
  unsigned long f;
  unsigned long tick_tmp;
  unsigned int antiloop;

  rest_cnt = (rest_cnt + 1) % 15;
  master_clk -= END_VBLCLK;

  /*
  if (key2vcnt++ > 10)
    {
      key2vcnt = 0;
      for (i = 0; i < KEY_MAX; i++) key2[i] = 0;
      dbstick1 = dbstick2 = 0;
    }
  */

  delay = TICKSPERSEC / FPS;                                                   // Ticks par frame = délai entre deux trames en ticks
  f = ((delay + last - gettimeticks ()) * 1000) / TICKSPERSEC;
  antiloop = 0;
  idx++;
  if (first == 0) first = gettimeticks () - 1;
  tick_tmp = gettimeticks ();
  if (idx * TICKSPERSEC / (tick_tmp - first) < FPS) delay = 0;
  /*
  while (gettimeticks () - last < delay && antiloop < 1000000) antiloop++;
  last = gettimeticks ();
  // En gros tout cela constiture une tempo
  */
  mstate = 0;
}

uint8_t read_t1 ()
{
  if ((horizontal_clock > 16) || (master_clk > START_VBLCLK)) return 1;
  else return 0;
}

uint8_t ext_read (uint8_t addr)
{
  uint8_t data;
  uint8_t si;
  uint8_t mask;

  // if (!(p1 & 0x48)) // TODO check
  // TODO Je ne comprends pas la logique de ce test: il me semble 0x08 devrait suffir.
  if (!(p1 & 0x08) && !(p1 & 0x40))                                            // 0x40 (active low) : Copy Mode Enable = write to 8245 RAM and read from external - RAM 0x08 (active low): Enable 8245
    {
      switch (addr)
	{
	case 0xA1:                                                             // 8245 Status byte - Some other bits should normally be set
	  data = intel8245_ram[0xA0] & 0x02;
	  if (master_clk > VBLCLK) data |= 0x08;
	  if (horizontal_clock < (LINECNT - 7)) data = data | 0x01;
	  return data;
	case 0xA2:                                                             // Collision byte
	  si = intel8245_ram[0xA2];
	  mask = 0x01;
	  data = 0;
	  for (uint8_t i = 0; i < 8; i++)  // TODO - optimiser ce code
	    {
	      if (si & mask)
		{
		  data |= collision_table[0x01] & mask;
		  data |= collision_table[0x02] & mask;
		  data |= collision_table[0x04] & mask;
		  data |= collision_table[0x08] & mask;
		  data |= collision_table[0x10] & mask;
		  data |= collision_table[0x20] & mask;
		  data |= collision_table[0x80] & mask;
		}
	      mask <<= 1;
	    }
	  clear_collision ();
	  return data;
	case 0xA4:
	  if ((intel8245_ram[0xA0] & 0x02)) {
	      y_latch = master_clk / 22;
	      if (y_latch > 241) y_latch = 0xFF;
	    }
	  return y_latch;
	case 0xA5:
	  if ((intel8245_ram[0xA0] & 0x02)) {
	      x_latch = horizontal_clock * 12;
	    }
	  return x_latch;
	default:
	  return intel8245_ram[addr];
	}
    }
  else if (!(p1 & 0x10)) return external_ram[addr];                            // p1 & 0x10 : hack lié à la cartouche MegaCart TODO: supprimer
  return 0;
}

void ext_write (uint8_t data, uint8_t addr)
{
  uint16_t cecette ;

  if (!(p1 & 0x08)) {
      fprintf(stderr, "Accessing video_ram[0x%02X] <- 0x%02X\n", addr, data) ;
      if (addr < 0x10 || (addr >= 0x80 && addr < 0xA0)) {
	      fprintf(stderr, " Ecriture du sprite %d.%s - [0x%02X] <- 0x%02X\n",
			      ((addr<0x10) ? addr/4 : (addr-0x80)/8)+1, 
			      ((addr>=0x80) ? "shape"
			       : ((addr%4 == 0) ? "y"
				       :((addr%4 == 1) ? "x" : "attributes"))
			       ), addr, data) ;
      	if (addr < 0x10) switch (addr%4) {
			case 0: fprintf(stderr, "  Y = %d\n", data); break;
			case 1: fprintf(stderr, "  X = %d\n", data); break;
			case 2: fprintf(stderr, "  Color = 0x%01X, Even shift = %d, Full shift = %d\n", (data & 0x3F) >> 3, (data & 0x02) >> 1, data & 0x01); break;
      	}
      }
      if ((addr >= 0x10) && (addr < 0x40)) {
	      fprintf(stderr, " Ecriture du caractère %d.%s - [0x%02X] <- 0x%02X\n",
		      (addr-0x10)/4 + 1, 
		      ((addr-0x10)%4 == 0)
		       ?"y"
		       :((addr-0x10)%4 == 1) ? "x"
			       : ((addr-0x10)%4 == 2) ? "character" : "color",
			      addr, data) ;
		switch (addr%4) {
			case 0: fprintf(stderr, "  Y_start = %d\n", data); break;
			case 1: fprintf(stderr, "  X = %d\n", data); break;
			case 2: fprintf(stderr, "  Cset pointer (lower part) = 0x%02X\n", data); break;
			case 3: fprintf(stderr, "  Cset pointer (upper part) = 0x%01X, Color = 0x%01X\n", data & 0x01, (data & 0x0E) >> 1);
				cecette = ((data & 0x01) << 8) + intel8245_ram[addr-1];
				fprintf(stderr, "  CSET = 0x%03X - character = 0x%02X\n", cecette, cecette/8);
				fprintf(stderr, "  ou bien CSET = 0x%03X - character = 0x%02X\n", cecette - (intel8245_ram[addr-3]/2), (cecette - intel8245_ram[addr-3]/2)/8);
			       	break;
		} 
      }
      if ((addr >= 0x40) && (addr < 0x80)) {
	      fprintf(stderr, " Ecriture du quad %d, caractère %d, %s - [0x%02X] <- 0x%02X\n", 
	              (addr-0X40)/0x10 + 1, 
		      (((addr-0x40)%0x10)/4 + 1), 
		      ((addr-0x40)%4 == 0) ? "y"
		       : ((addr-0x40)%4 == 1)
			       ? "x"
			       : ((addr-0x40)%4 == 2) ? "character" : "color",
			      addr, data) ;
		switch (addr%4) {
			case 0: fprintf(stderr, "  Y_start = %d\n", data); break;
			case 1: fprintf(stderr, "  X = %d\n", data); break;
			case 2: fprintf(stderr, "  Cset pointer (lower part) = 0x%02X\n", data); break;
			case 3: fprintf(stderr, "  Cset pointer (upper part) = 0x%01X, Color = 0x%01X\n", data & 0x01, (data & 0x0E) >> 1); break;
				cecette = ((data & 0x01) << 8) + intel8245_ram[addr-1];
				fprintf(stderr, "  CSET = 0x%03X - character = 0x%02X\n", cecette, cecette/8);
				fprintf(stderr, "  ou bien CSET = 0x%03X - character = 0x%02X\n", cecette - (intel8245_ram[addr-3]/2), (cecette - intel8245_ram[addr-3]/2)/8);
			       	break;
                }
      }
      if (addr >= 0xA0 && addr <= 0xA3) fprintf(stderr, " Octet de controle - [0x%02X] <- 0x%02X\n", addr, data) ;
      if (addr == 0xA0) {
	  fprintf(stderr, "  Control register: Display enable = %d, Horiz int enable = %d, Grid = %d, Fill mode = %d, Dot grid = %d, Latch position = %d\n",
			  (data & 0x20) >> 5, (data & 0x01), (data & 0x08) >> 3, (data & 0x80) >> 7, (data & 0x40) >> 6, (data & 0x02) >> 1);
	  if (intel8245_ram[0xA0] & 0x02 && !data & 0x02) {
	      y_latch = master_clk / 22;
	      x_latch = horizontal_clock * 12;
	      if (y_latch > 241) y_latch = 0xFF;
	    }
	  if (master_clk <= START_VBLCLK && intel8245_ram[0xA0] != data) draw_region ();
	}
      else if (addr == 0xA1) fprintf(stderr, "  Status register: SHOULD NOT WRITE HERE !\n");
      else if (addr == 0xA2) fprintf(stderr, "  Collision register\n");
      else if (addr == 0xA3) fprintf(stderr, "  Color register: Background color = 0x%1X, Grid color = 0x%1X, Grid lum = %d\n", data & 0x07, (data & 0x38) > 3, (data & 0x40) > 6);
      else if (addr >= 0x40 && addr < 0x80 && addr & (0x02 == 0))                // 0x40 - 0x7F : les quatre Quads, addr & 0x02 == 0 -> les positions X et Y_start du caractère
	{                                                                      // TODO comprendre ce code
          fprintf(stderr, "  Simplifying quad data\n") ;
	  addr = addr & 0x71;
	  if (!(addr & 0x01)) data &= 0xFE;
	  intel8245_ram[addr] = intel8245_ram[addr + 4] = intel8245_ram[addr + 8] = intel8245_ram[addr + 12] = data;
	}
      if (addr >= 0xA7 && addr <= 0xAA) fprintf(stderr, " Son  - [0x%02X] <- 0x%02X\n", addr, data) ;
      intel8245_ram[addr] = data;
    }
  // else if (!(p1 & 0x50)) // TODO: vérifier cette condition - Il est probale que je vais pouvoir trfansformer ceci en & 0x40
  else if (!(p1 & 0x10) && !(p1 & 0x40)) {
      fprintf(stderr, "Accessing external_ram[0x%02X] <- 0x%02X (%s)\n", addr, data, (addr < 0x80) ? "writing" : "doing nothing") ;
      if (addr < 0x80) external_ram[addr] = data; // J'ai bien l'impression que je dois considérer la RAM externe comme 128 et non 256 bits
    }
}
