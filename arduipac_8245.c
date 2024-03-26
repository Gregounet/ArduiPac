/*
 * memory map
 *
 * 0x10-0x7F (112 bytes): vdc_charX, vdc_quadX
 * 0x80-0x9F ( 32 bytes): vdc_sprX_shape = 4 sprites of 8x8
 *
 * */

#include <stdlib.h>

#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_vmachine.h"
#include "arduipac_cset.h"
#include "arduipac_timefunc.h"
#include "arduipac_sdl.h"

#define COL_SP0   0x01
#define COL_SP1   0x02
#define COL_SP2   0x04
#define COL_SP3   0x08

#define COL_VGRID 0x10
#define COL_HGRID 0x20
#define COL_CHAR  0x80


#define X_START		8
#define Y_START		24

static long colortable[16] = {
  0x000000, 0x0e3DD4, 0x00981B, 0x00BBD9, 0xc70008, 0xCC16B3, 0x9D8710, 0xE1DEE1,
  0x5F6E6B, 0x6AA1FF, 0x3DF07A, 0x31FFFF, 0xFF4255, 0xFF98FF, 0xD9AD5D, 0xFFFFFF
};

PALETTE colors;
PALETTE oldcol;

static uint8_t *vscreen = NULL;

static int cached_lines[MAXLINES];

uint8_t coltab[256];

long clip_low;
long clip_high;

static void create_cmap ();
static void draw_char (uint8_t ypos, uint8_t xpos, uint8_t chr, uint8_t col);
static void draw_quad (uint8_t ypos, uint8_t xpos, uint8_t cp0l, uint8_t cp0h, uint8_t cp1l, uint8_t cp1h, uint8_t cp2l, uint8_t cp2h, uint8_t cp3l, uint8_t cp3h);
static void draw_grid ();
void mputvid (unsigned int ad, unsigned int len, uint8_t d, uint8_t c);

void draw_region ()
{
  int i;

  if (regionoff == 0xFFFF) i = (master_clk / (LINECNT - 1) - 5);
  else i = (master_clk / 22 + regionoff);
  i = (snapline (i, VDCwrite[0xA0], 0));

  if (i < 0) i = 0;
  clip_low = last_line * (long) BMPW;
  clip_high = i * (long) BMPW;
  if (clip_high > BMPW * BMPH) clip_high = BMPW * BMPH;
  if (clip_low < 0) clip_low = 0;
  if (clip_low < clip_high) draw_display ();
  last_line = i;
}

static void create_cmap ()
{
  int i;
  for (i = 0; i < 16; i++)
    {
      colors[i + 32].r = colors[i].r = (colortable[i] & 0xFF0000) >> 18;
      colors[i + 32].g = colors[i].g = (colortable[i] & 0x00FF00) >> 10;
      colors[i + 32].b = colors[i].b = (colortable[i] & 0x0000FF) >> 2;
    }

  for (i = 64; i < 256; i++) colors[i].r = colors[i].g = colors[i].b = 0;
  for (i = 0; i < 256; i++)
    {
      colors[i].r *= 4;
      colors[i].g *= 4;
      colors[i].b *= 4;
    }
}

void grmode ()
{
  clearscr();
  set_display_switch_mode (SWITCH_PAUSE);

}

void clearscr ()
{
  clear_screen(screen);
  clear_screen(bmpcache);
}

// c is COL_HGRID(0x20 or COL_CHAR(0x80) or 8
void mputvid (unsigned int ad, unsigned int len, uint8_t color, uint8_t c)
{
  unsigned int i;
  if (len >= sizeof (coltab)) return;
  if (c >= sizeof (coltab)) return;
  if ((ad > (unsigned long) clip_low) && (ad < (unsigned long) clip_high))
    {
      if (((len & 3) == 0) && (sizeof (unsigned long) == 4))
	{			/* TODO unsigned long is 8 on 64bits, this code will not work */
	  unsigned long dddd =
	    (((unsigned long) color) & 0xff) | ((((unsigned long) color) & 0xff) << 8)
	    | ((((unsigned long) color) & 0xff) << 16) |
	    ((((unsigned long) color) & 0xff) << 24);
	  unsigned long cccc =
	    (((unsigned long) c) & 0xff) | ((((unsigned long) c) & 0xff) << 8)
	    | ((((unsigned long) c) & 0xff) << 16) |
	    ((((unsigned long) c) & 0xff) << 24);
	  for (i = 0; i < len >> 2; i++)
	    {
	      *((unsigned long *) (vscreen + ad)) = dddd;
	      cccc |= *((unsigned long *) (col + ad));
	      *((unsigned long *) (col + ad)) = cccc;
	      coltab[c] |=
		((cccc | (cccc >> 8) | (cccc >> 16) | (cccc >> 24)) & 0xff);
	      ad += 4;
	    }
	}
      else
	{
	  for (i = 0; i < len; i++)
	    {
	      vscreen[ad] = color;
	      col[ad] |= c;
	      coltab[c] |= col[ad++];
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

  if (VDCwrite[0xA0] & 0x40)
    {
      for (j = 0; j < 9; j++)
	{
	  pnt = (((j * 24) + 24) * BMPW);
	  for (i = 0; i < 9; i++)
	    {
	      pn1 = pnt + (i * 32) + 20;
	      color = ColorVector[j * 24 + 24];
	      mputvid (pn1, 4,
		       (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80
								 ? 0 : 8),
		       COL_HGRID);
	      color = ColorVector[j * 24 + 25];
	      mputvid (pn1 + BMPW, 4,
		       (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80
								 ? 0 : 8),
		       COL_HGRID);
	      color = ColorVector[j * 24 + 26];
	      mputvid (pn1 + BMPW * 2, 4,
		       (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80
								 ? 0 : 8),
		       COL_HGRID);
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
	  if ((pn1 + BMPW * 3 >= (unsigned long) clip_low)
	      && (pn1 <= (unsigned long) clip_high))
	    {
	      d = VDCwrite[0xC0 + i];
	      if (j == 8)
		{
		  d = VDCwrite[0xD0 + i];
		  mask = 1;
		}
	      if (d & mask)
		{
		  color = ColorVector[j * 24 + 24];
		  mputvid (pn1, 36,
			   (color & 0x07) | ((color & 0x40) >> 3) | (color &
								     0x80 ? 0
								     : 8),
			   COL_HGRID);
		  color = ColorVector[j * 24 + 25];
		  mputvid (pn1 + BMPW, 36,
			   (color & 0x07) | ((color & 0x40) >> 3) | (color &
								     0x80 ? 0
								     : 8),
			   COL_HGRID);
		  color = ColorVector[j * 24 + 26];
		  mputvid (pn1 + BMPW * 2, 36,
			   (color & 0x07) | ((color & 0x40) >> 3) | (color &
								     0x80 ? 0
								     : 8),
			   COL_HGRID);
		}
	    }
	}
      mask = mask << 1;
    }

  mask = 0x01;
  w = 4;
  if (VDCwrite[0xA0] & 0x80)
    w = 32;
  for (j = 0; j < 10; j++)
    {
      pnt = (j * 32);
      mask = 0x01;
      d = VDCwrite[0xE0 + j];
      for (x = 0; x < 8; x++)
	{
	  pn1 = pnt + (((x * 24) + 24) * BMPW) + 20;
	  if (d & mask)
	    {
	      for (i = 0; i < 24; i++)
		{
		  if ((pn1 >= (unsigned long) clip_low)
		      && (pn1 <= (unsigned long) clip_high))
		    {
		      color = ColorVector[x * 24 + 24 + i];
		      mputvid (pn1, w,
			       (color & 0x07) | ((color & 0x40) >> 3) | (color
									 &
									 0x80
									 ? 0 :
									 8),
			       COL_VGRID);
		    }
		  pn1 += BMPW;
		}
	    }
	  mask = mask << 1;
	}
    }
}

unsigned char * get_raw_pixel_line (BITMAP * pSurface, int y)
{
  if (pSurface == NULL)
    {
      return NULL;
    }
  if (y < 0)
    {
      return NULL;
    }
  return pSurface->line[y];
}

void finish_display ()
{
  int x, y, sn;
  static int cache_counter = 0;
  static long index = 0;
  static unsigned long fps_last = 0, t = 0, curr = 0;
  SDL_Surface *screen_resize;
  SDL_Rect dest_rect;

  for (y = 0; y < bmp->h; y++)
    {
      cached_lines[y] = !memcmp (get_raw_pixel_line (bmpcache, y),
				 get_raw_pixel_line (bmp, y), bmp->w);
      if (!cached_lines[y])
	memcpy (get_raw_pixel_line (bmpcache, y), get_raw_pixel_line (bmp, y),
		bmp->w);
    }

  for (y = 0; y < 10; y++)
    cached_lines[(y + cache_counter) % bmp->h] = 0;
  cache_counter = (cache_counter + 10) % bmp->h;

  acquire_screen ();

  for (y = 0; y < WNDH; y++)
    {
      if (!cached_lines[y + 2])
	stretch_blit (bmp, screen, 7, 2 + y, WNDW, 1, 0, y,  WNDW , wsize - sn);
    }

  if (sn)
    {
      for (y = 0; y < WNDH; y++)
	{
	  if (!cached_lines[y + 2])
	    {
	      for (x = 0; x < bmp->w; x++)
		{
		  *get_raw_pixel (bmp, x, y + 2) += 16;
		}
	      stretch_blit (bmp, screen, 7, 2 + y, WNDW, 1, 0,
			    (y + 1) - 1, WNDW , 1);
	      memcpy (get_raw_pixel_line (bmp, y + 2),
		      get_raw_pixel_line (bmpcache, y + 2), bmp->w);
	    }
	}
    }
  clear_screnn(screen);
  dest_rect.x = 0;
  dest_rect.y = 0;
  dest_rect.w = WNDW;
  dest_rect.h = WNDH;
  SDL_BlitSurface (screen_resize, NULL, screen, &dest_rect);
  SDL_FreeSurface (screen_resize);
  release_screen ();
}

void clear_collision ()
{
  load_colplus (col);
  coltab[0x01] = coltab[0x02] = 0;
  coltab[0x04] = coltab[0x08] = 0;
  coltab[0x10] = coltab[0x20] = 0;
  coltab[0x40] = coltab[0x80] = 0;
}

void draw_display ()
{
  int i, j, x, sm, t;
  uint8_t y, b, d1, cl, c;

  unsigned int pnt, pnt2;
  if (BMPW < 0 || vscreen == NULL)
    {
      return;
    }

  for (i = clip_low / BMPW; i < clip_high / BMPW; i++)
    memset (vscreen + i * BMPW, ((ColorVector[i] & 0x38) >> 3) | (ColorVector[i] & 0x80 ? 0 : 8), BMPW);

  if (VDCwrite[0xA0] & 0x08)	/* 0xA0 Bit 3 If this bit is 1 the grid is displayed. */
    draw_grid ();


  /* 10h-7Fh: vdc_charX, vdc_quadX http://soeren.informationstheater.de/g7000/hardware.html
   * Every char (and sub-quad) has a set of 4 control registers.
   * Char control 0       This register holds the Y position of the char.
   * Char control 1       This registers holds the X position of the char.
   * Char control 2       This register holds the lowest 8 bits of the charset pointer.
   * Char control 3       This register is used bitwise.
   *  Bit 0       This is bit 8 of the charset pointer, the highest bit.
   *  Bit 1       This bit is the red component for the char color.
   *  Bit 2       This bit is the green component for the char color.
   *  Bit 3       This bit is the blue component for the char color.
   * */
  for (i = 0x10; i < 0x40; i += 4)
    draw_char (VDCwrite[i], VDCwrite[i + 1], VDCwrite[i + 2],
	       VDCwrite[i + 3]);

  /* draw quads, position mapping happens in ext_write (vmachine.c) */
  for (i = 0x40; i < 0x80; i += 0x10)
    draw_quad (VDCwrite[i], VDCwrite[i + 1], VDCwrite[i + 2], VDCwrite[i + 3],
	       VDCwrite[i + 6], VDCwrite[i + 7],
	       VDCwrite[i + 10], VDCwrite[i + 11],
	       VDCwrite[i + 14], VDCwrite[i + 15]);

  /* draw sprites */
  c = 8;			/* what is 8 */
  for (i = 12; i >= 0; i -= 4)
    {
      pnt2 = 0x80 + (i * 2);
      y = VDCwrite[i];
      x = VDCwrite[i + 1] - 8;	/* This registers holds the 8 highest bits of the X position. */
      t = VDCwrite[i + 2];
      cl = ((t & 0x38) >> 3);	/* 0x38 is 111000 */
      cl = ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8;
      /*174 */
      if ((x < 164) && (y > 0) && (y < 232))
	{			/*TODO why 164 0 and 232 ? */
	  pnt = y * BMPW + (x * 2) + 20 + sproff;
	  if (t & 4)
	    {			/*bit 2 If this bit is 1 the size of the sprite doubles */
	      if ((pnt + BMPW * 32 >= (unsigned long) clip_low)
		  && (pnt <= (unsigned long) clip_high))
		{
		  for (j = 0; j < 8; j++)
		    {
		      sm = (((j % 2 == 0) && (((t >> 1) & 1) != (t & 1)))
			    || ((j % 2 == 1) && (t & 1))) ? 1 : 0;
		      d1 = VDCwrite[pnt2++];
		      for (b = 0; b < 8; b++)
			{
			  if (d1 & 0x01)
			    {
			      if ((x + b + sm < 159) && (y + j < 247))
				{
				  mputvid (sm + pnt, 4, cl, c);
				  mputvid (sm + pnt + BMPW, 4, cl, c);
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
	    {			/* bit 2 is 0 normal sprite size */
	      if ((pnt + BMPW * 16 >= (unsigned long) clip_low)
		  && (pnt <= (unsigned long) clip_high))
		{
		  for (j = 0; j < 8; j++)
		    {
		      sm = (((j % 2 == 0) && (((t >> 1) & 1) != (t & 1)))
			    || ((j % 2 == 1) && (t & 1))) ? 1 : 0;
		      d1 = VDCwrite[pnt2++];
		      for (b = 0; b < 8; b++)
			{
			  if (d1 & 0x01)
			    {
			      if ((x + b + sm < 160) && (y + j < 249))
				{
				  mputvid (sm + pnt, 2, cl, c);
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


/*
 * chr = This register holds the lowest 8 bits of the charset pointer.
 * col bit 0 = This is bit 8 of the charset pointer, the highest bit.
 * */
void draw_char (uint8_t ypos, uint8_t xpos, uint8_t chr, uint8_t col)
{
  int j, c;
  uint8_t cl, d1;
  int y, b, n;
  unsigned int pnt;

  y = (ypos & 0xFE);
  pnt = y * BMPW + ((xpos - 8) * 2) + 20;

  ypos = ypos >> 1;
  n = 8 - (ypos % 8) - (chr % 8);
  if (n < 3)
    n = n + 7;

  if ((pnt + BMPW * 2 * n >= (unsigned long) clip_low)
      && (pnt <= (unsigned long) clip_high))
    {
      c = (int) chr + ypos;
      if (col & 0x01)
	c += 256;
      if (c > 511)
	c -= 512;

      cl = ((col & 0x0E) >> 1);	/* mask 1110 = get only bit 1,2,3 (color bits) */
      cl = ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8;

      if ((y > 0) && (y < 232) && (xpos < 157))
	{
	  for (j = 0; j < n; j++)
	    {
	      d1 = cset[c + j];
	      for (b = 0; b < 8; b++)
		{
		  if (d1 & 0x80)
		    {
		      if ((xpos - 8 + b < 160) && (y + j < 240))
			{
			  mputvid (pnt, 2, cl, COL_CHAR);
			  mputvid (pnt + BMPW, 2, cl, COL_CHAR);
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

/* This quad drawing routine can display the quad cut off effect used in KTAA.
 * It needs more testing with other games, especially the clipping.
 * This code is quite slow and needs a rewrite by somebody with more experience
 * than I (sgust) have */

void draw_quad (uint8_t ypos, uint8_t xpos, uint8_t cp0l, uint8_t cp0h, uint8_t cp1l, uint8_t cp1h, uint8_t cp2l, uint8_t cp2h, uint8_t cp3l, uint8_t cp3h)
{
  /* char set pointers */
  int chp[4];
  /* colors */
  uint8_t col[4];
  /* pointer into screen bitmap */
  unsigned int pnt;
  /* offset into current line */
  unsigned int off;
  /* loop variables */
  int i, j, lines;

  /* get screen bitmap position of quad */
  pnt = (ypos & 0xfe) * BMPW + ((xpos - 8) * 2) + 20;
  /* abort drawing if completely below the bottom clip */
  if (pnt > (unsigned long) clip_high)
    return;
  /* extract and convert char-set offsets */
  chp[0] = cp0l | ((cp0h & 1) << 8);
  chp[1] = cp1l | ((cp1h & 1) << 8);
  chp[2] = cp2l | ((cp2h & 1) << 8);
  chp[3] = cp3l | ((cp3h & 1) << 8);
  for (i = 0; i < 4; i++)
    chp[i] = (chp[i] + (ypos >> 1)) & 0x1ff;
  lines = 8 - (chp[3] + 1) % 8;
  /* abort drawing if completely over the top clip */
  if (pnt + BMPW * 2 * lines < (unsigned long) clip_low)
    return;
  /* extract and convert color information */
  col[0] = (cp0h & 0xe) >> 1;
  col[1] = (cp1h & 0xe) >> 1;
  col[2] = (cp2h & 0xe) >> 1;
  col[3] = (cp3h & 0xe) >> 1;
  for (i = 0; i < 4; i++)
    col[i] = ((col[i] & 2) | ((col[i] & 1) << 2) | ((col[i] & 4) >> 2)) + 8;
  /* now draw the quad line by line controlled by the last quad */
  while (lines-- > 0)
    {
      off = 0;
      /* draw all 4 sub-quads */
      for (i = 0; i < 4; i++)
	{
	  /* draw sub-quad pixel by pixel, but stay in same line */
	  for (j = 0; j < 8; j++)
	    {
	      if ((cset[chp[i]] & (1 << (7 - j))) && (off < BMPW))
		{
		  mputvid (pnt + off, 2, col[i], COL_CHAR);
		  mputvid (pnt + off + BMPW, 2, col[i], COL_CHAR);
		}
	      /* next pixel */
	      off += 2;
	    }
	  /* space between sub-quads */
	  off += 16;
	}
      /* advance char-set pointers */
      for (i = 0; i < 4; i++)
	chp[i] = (chp[i] + 1) & 0x1ff;
      /* advance screen bitmap pointer */
      pnt += BMPW * 2;
    }
}

int init_display ()
{
  get_palette (oldcol);
  create_cmap ();
  if (BMPW * BMPH == 0)
    {
      o2em_clean_quit (EXIT_FAILURE);
    }
  bmp = create_bitmap (BMPW, BMPH);
  if (bmp == NULL)
    {
      return -1;
    }
  bmpcache = create_bitmap (BMPW, BMPH);
  if (bmpcache == NULL)
    {
      return -1;
    }
#ifndef __O2EM_SDL__
  vscreen = (uint8_t *) bmp->dat;
#else
  vscreen = (uint8_t *) bmp->pixels;
#endif
  clear_screen(bmp);
  clear_screen(bmpcache);

  col = (uint8_t *) malloc (BMPW * BMPH);
  if (col == NULL)
    {
      o2em_clean_quit (EXIT_FAILURE);
    }
  memset (col, 0, BMPW * BMPH);
  return 0;
}
