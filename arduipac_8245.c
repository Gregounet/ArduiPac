#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_vmachine.h"
#include "arduipac_cset.h"
#include "arduipac_timefunc.h"
#include "arduipac_graphics.h"

#define COLLISION_SP0   0x01
#define COLLISION_SP1   0x02
#define COLLISION_SP2   0x04
#define COLLISION_SP3   0x08

#define COLLISION_VGRID 0x10
#define COLLISION_HGRID 0x20
#define COLLISION_CHAR  0x80

static int cached_lines[MAXLINES];

uint8_t collision_table[256];
uint8_t intel8245_ram[256];

long clip_low;
long clip_high;

static void draw_char (uint8_t ypos, uint8_t xpos, uint8_t chr, uint8_t color);
static void draw_quad (uint8_t ypos, uint8_t xpos, uint8_t cp0l, uint8_t cp0h, uint8_t cp1l, uint8_t cp1h, uint8_t cp2l, uint8_t cp2h, uint8_t cp3l, uint8_t cp3h);
static void draw_grid ();

uint8_t *bmp;
uint8_t *bmpcache;
uint8_t *screen;
uint8_t *vscreen;

void draw_region ()
{
  int last_line;
  int i;

  i = (master_clk / (LINECNT - 1) - 5);

  if (i < 0) i = 0;
  clip_low = last_line * (long) BMPW;
  clip_high = i * (long) BMPW;
  if (clip_high > BMPW * BMPH) clip_high = BMPW * BMPH;
  if (clip_low < 0) clip_low = 0;
  if (clip_low < clip_high) draw_display ();
  last_line = i;
}

void mputvid (uint32_t location, uint16_t len, uint8_t color, uint16_t c)
{
  if (len >= sizeof (collision_table)) return;
  if (c   >= sizeof (collision_table)) return;

  if ((location > (uint32_t) clip_low) && (location < (uint32_t) clip_high))
    {
      if ((len & 0x03) == 0)
      {
	  unsigned long dddd = (((unsigned long) color) & 0xFF) | ((((unsigned long) color) & 0xFF) << 8) | ((((unsigned long) color) & 0xFF) << 16) | ((((unsigned long) color) & 0xFF) << 24);
	  unsigned long cccc = (((unsigned long) color) & 0xFF) | ((((unsigned long) color) & 0xFF) << 8) | ((((unsigned long) color) & 0xFF) << 16) | ((((unsigned long) color) & 0xFF) << 24);

	  for (uint16_t i = 0; i < (len >> 2); i++)
	    {
	      //((unsigned long *) (vscreen + ad)) = dddd; 
	      //cccc |= *((unsigned long *) (col + ad));
	      //*((unsigned long *) (col + ad)) = cccc; 
	      collision_table[c] |= ((cccc | (cccc >> 8) | (cccc >> 16) | (cccc >> 24)) & 0xFF);
	      location += 4;
	    }
	}
      else
	{
	  for (uint16_t i = 0; i < len; i++)
	    {
	      vscreen[location] = color;
	      //col[ad] |= c;
	      //collision_table[c] |= collision[ad++];
	    }
	}
    }
}

static void draw_grid ()
{
  unsigned int pnt, pn1;
  uint8_t mask, d;
  int j, i, x, w;
  uint8_t color;

  if (intel8245_ram[0xA0] & 0x40)
  {
      for (j = 0; j < 9; j++)
      {
	  pnt = (((j * 24) + 24) * BMPW);
	  for (i = 0; i < 9; i++)
	  {
	      pn1 = pnt + (i * 32) + 20;
	      /*
	      mputvid (pn1, 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_HGRID);
	      mputvid (pn1 + BMPW, 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_HGRID);
	      mputvid (pn1 + BMPW * 2, 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_HGRID);
	      */
	   }
	}
    }

  mask = 0x01;
  for (j = 0; j < 9; j++)
    {
      pnt = (((j * 24) + 24) * BMPW);
      for (i = 0; i < 9; i++)
      {
	  pn1 = pnt + (i * 32) + 20;
	  if ((pn1 + BMPW * 3 >= (unsigned long) clip_low) && (pn1 <= (unsigned long) clip_high))
	  {
	      d = intel8245_ram[0xC0 + i];
	      if (j == 8)
	      {
		 d = intel8245_ram[0xD0 + i];
		 mask = 1;
	      }
	      if (d & mask)
              {
		 mputvid (pn1,            36, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_HGRID);
		 mputvid (pn1 + BMPW,     36, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_HGRID);
		 mputvid (pn1 + BMPW * 2, 36, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_HGRID);
	      }
           }
	}
      mask = mask << 1;
    }

  mask = 0x01;
  w = 4;
  if (intel8245_ram[0xA0] & 0x80) w = 32;
  for (j = 0; j < 10; j++)
    {
      pnt = (j * 32);
      mask = 0x01;
      d = intel8245_ram[0xE0 + j];
      for (x = 0; x < 8; x++)
	{
	  pn1 = pnt + (((x * 24) + 24) * BMPW) + 20;
	  if (d & mask)
	    {
	      for (i = 0; i < 24; i++)
		{
		  if ((pn1 >= (unsigned long) clip_low) && (pn1 <= (unsigned long) clip_high))
		      mputvid (pn1, w, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_VGRID);
		  pn1 += BMPW;
		}
	    }
	  mask = mask << 1;
	}
    }
}

/*
unsigned char * get_raw_pixel_line (BITMAP * pSurface, int y)
{
  if (pSurface == NULL) return NULL;
  if (y < 0) return NULL;
  return pSurface->line[y];
}
*/

void finish_display ()
{
  int x, y, sn;
  static int cache_counter = 0;
  static long index = 0;
  static unsigned long fps_last = 0, t = 0, curr = 0;

  for (y = 0; y < BMPH; y++)
      cached_lines[y] = !memcmp (get_raw_pixel_line (bmpcache, y), get_raw_pixel_line (bmp, y), BMPH);
  if (!cached_lines[y]) memcpy (get_raw_pixel_line (bmpcache, y), get_raw_pixel_line (bmp, y), BMPW);

  for (y = 0; y < 10; y++) cached_lines[(y + cache_counter) % BMPH] = 0;
  cache_counter = (cache_counter + 10) % BMPH;

  /*
  for (y = 0; y < WNDH; y++)
      if (!cached_lines[y + 2]) stretch_blit (bmp, screen, 7, 2 + y, WNDW, 1, 0, y,  WNDW , wsize - sn);
      */

  if (sn)
    {
      for (y = 0; y < WNDH; y++)
	{
	  if (!cached_lines[y + 2])
	    {
	      for (x = 0; x < BMPW; x++) *get_raw_pixel (bmp, x, y + 2) += 16;
	      //stretch_blit (bmp, screen, 7, 2 + y, WNDW, 1, 0, (y + 1) - 1, WNDW , 1);
	      memcpy (get_raw_pixel_line (bmp, y + 2), get_raw_pixel_line (bmpcache, y + 2), BMPH);
	    }
	}
    }
  clear_screen(screen);
  /*
  dest_rect.x = 0;
  dest_rect.y = 0;
  dest_rect.w = WNDW;
  dest_rect.h = WNDH;
  */
}

void clear_screen(uint8_t *bitmap)
{
}

void clear_collision ()
{
  collision_table[0x01] = 0;
  collision_table[0x02] = 0;
  collision_table[0x04] = 0;
  collision_table[0x08] = 0;
  collision_table[0x10] = 0;
  collision_table[0x20] = 0;
  collision_table[0x40] = 0;
  collision_table[0x80] = 0;
}

void draw_display ()
{
  int x;
  int sm;
  int t;
  uint8_t d1;
  uint8_t y;
  uint8_t cl;
  uint8_t c;

  unsigned int pnt;
  unsigned int pnt2;

  for (int i = clip_low / BMPW; i < clip_high / BMPW; i++) memset (vscreen + i * BMPW,  0x38 >> 3, BMPW);

  if (intel8245_ram[0xA0] & 0x08) draw_grid ();
  for (int i = 0x10; i < 0x40; i += 4) draw_char (intel8245_ram[i], intel8245_ram[i + 1], intel8245_ram[i + 2], intel8245_ram[i + 3]);

  for (int i = 0x40; i < 0x80; i += 0x10)
    draw_quad (intel8245_ram[i], intel8245_ram[i + 1], intel8245_ram[i + 2], intel8245_ram[i + 3], intel8245_ram[i + 6], intel8245_ram[i + 7],
		    intel8245_ram[i + 10], intel8245_ram[i + 11], intel8245_ram[i + 14], intel8245_ram[i + 15]);

  c = 8;
  for (int i = 12; i >= 0; i -= 4)
    {
      pnt2 = 0x80 + (i * 2);

      y = intel8245_ram[i];
      x = intel8245_ram[i + 1] - 8;
      t = intel8245_ram[i + 2];

      cl = ((t & 0x38) >> 3);
      cl = ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8;

      if ((x < 164) && (y > 0) && (y < 232))
	{
	  pnt = y * BMPW + (x * 2) + 20;
	  if (t & 4)
	    {		
	      if ((pnt + BMPW * 32 >= (unsigned long) clip_low) && (pnt <= (unsigned long) clip_high))
		{
		  for (int j = 0; j < 8; j++)
		    {
		      sm = (((j % 2 == 0) && (((t >> 1) & 1) != (t & 1))) || ((j % 2 == 1) && (t & 1))) ? 1 : 0; d1 = intel8245_ram[pnt2++];
		      for (int b = 0; b < 8; b++)
			{
			  if (d1 & 0x01)
			    {
			      if ((x + b + sm < 159) && (y + j < 247))
				{
				  mputvid (sm + pnt           , 4, cl, c);
				  mputvid (sm + pnt +     BMPW, 4, cl, c);
				  mputvid (sm + pnt + 2 * BMPW, 4, cl, c);
				  mputvid (sm + pnt + 3 * BMPW, 4, cl, c);
				}
			    }
			  pnt += 4;
			  d1 = d1 >> 1;
			}
		      pnt += BMPW * 4 - 32;
		    }
		}
	    }
	  else
	    {
	      if ((pnt + BMPW * 16 >= (unsigned long) clip_low) && (pnt <= (unsigned long) clip_high))
		{
		  for (int j = 0; j < 8; j++)
		    {
		      sm = (((j % 2 == 0) && (((t >> 1) & 1) != (t & 1))) || ((j % 2 == 1) && (t & 1))) ? 1 : 0;
		      d1 = intel8245_ram[pnt2++];
		      for (int b = 0; b < 8; b++)
			{
			  if (d1 & 0x01)
			    {
			      if ((x + b + sm < 160) && (y + j < 249))
				{
				  mputvid (sm + pnt       , 2, cl, c);
				  mputvid (sm + pnt + BMPW, 2, cl, c);
				}
			    }
			  pnt += 2;
			  d1 = d1 >> 1;
			}
		      pnt += BMPW * 2 - 16;
		    }
		}
	    }
	}
      c = c >> 1;
    }
}

void draw_char (uint8_t ypos, uint8_t xpos, uint8_t chr, uint8_t col)
{
  int c;
  uint8_t cl;
  uint8_t d1;
  uint8_t y;
  int n;
  unsigned int pnt;

  y = (ypos & 0xFE);
  pnt = y * BMPW + ((xpos - 8) * 2) + 20;

  ypos = ypos >> 1;
  n = 8 - (ypos % 0x08) - (chr % 0x08);
  if (n < 3) n = n + 7;

  if ((pnt + BMPW * 2 * n >= (unsigned long) clip_low) && (pnt <= (unsigned long) clip_high))
    {
      c = (int) chr + ypos;
      if (col & 0x01) c += 256;
      if (c > 511) c -= 512;

      cl = ((col & 0x0E) >> 1);
      cl = ((cl & 0x02) | ((cl & 0x01) << 2) | ((cl & 0x04) >> 2)) + 8;

      if ((y > 0) && (y < 232) && (xpos < 157))
	{
	  for (int j = 0; j < n; j++)
	    {
	      d1 = cset[c + j];
	      for (int b = 0; b < 8; b++)
		{
		  if (d1 & 0x80)
		    {
		      if ((xpos - 8 + b < 160) && (y + j < 240))
			{
			  mputvid (pnt,        2, cl, COLLISION_CHAR);
			  mputvid (pnt + BMPW, 2, cl, COLLISION_CHAR);
			}
		    }
		  pnt += 2;
		  d1 = d1 << 1;
		}
	      pnt += BMPW * 2 - 16;
	    }
	}
    }
}

void draw_quad (uint8_t ypos, uint8_t xpos, uint8_t cp0l, uint8_t cp0h, uint8_t cp1l, uint8_t cp1h, uint8_t cp2l, uint8_t cp2h, uint8_t cp3l, uint8_t cp3h)
{
  uint8_t chp[4];
  uint8_t col[4];
  uint8_t lines;
  uint16_t pnt;
  uint16_t offset;

  pnt = (ypos & 0xFE) * BMPW + ((xpos - 8) * 2) + 20;
  if (pnt > (uint16_t) clip_high) return;

  chp[0] = cp0l | ((cp0h & 0x01) << 8);
  chp[1] = cp1l | ((cp1h & 0x01) << 8);
  chp[2] = cp2l | ((cp2h & 0x01) << 8);
  chp[3] = cp3l | ((cp3h & 0x01) << 8);

  for (int i = 0; i < 4; i++) chp[i] = (chp[i] + (ypos >> 1)) & 0x1FF;

  lines = 8 - (chp[3] + 1) % 8;
  if (pnt + BMPW * 2 * lines < (uint16_t) clip_low) return;

  /*
  col[0] = (cp0h & 0x0E) >> 1;
  col[1] = (cp1h & 0x0E) >> 1;
  col[2] = (cp2h & 0x0E) >> 1;
  col[3] = (cp3h & 0x0E) >> 1;
  */

  // for (int i = 0; i < 4; i++) col[i] = ((col[i] & 2) | ((col[i] & 1) << 2) | ((col[i] & 4) >> 2)) + 8;

  while (lines-- > 0)
    {
      offset = 0;
      for (int i = 0; i < 4; i++)
	{
	  for (int j = 0; j < 8; j++)
	    {
	      if ((cset[chp[i]] & (0x01 << (7 - j))) && (offset < BMPW))
		{
		  // mputvid (pnt + offset       , 2, col[i], COLLISION_CHAR);
		  // mputvid (pnt + offset + BMPW, 2, col[i], COLLISION_CHAR);
		}
	      offset += 2;
	    }
	  offset += 16;
	}
      for (int i = 0; i < 4; i++) chp[i] = (chp[i] + 1) & 0x1FF;
      pnt += BMPW * 2;
    }
}

int init_display ()
{
  bmp      = create_bitmap (BMPW, BMPH);
  bmpcache = create_bitmap (BMPW, BMPH);

  clear_screen(bmp);
  clear_screen(bmpcache);

  // vscreen = (uint8_t *) BMPH; TODO: trop con cette ligne

  // col = (uint8_t *) malloc (BMPW * BMPH);
  // memset (col, 0, BMPW * BMPH);
  return 0;
}
