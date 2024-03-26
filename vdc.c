/*
 * memory map
 *
 * 0x10h-0x7Fh: vdc_charX, vdc_quadX
 *
 * 0x80h-0x9fh: vdc_sprX_shape
 * 4 sprites of 8x8
 *
 * */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "vmachine.h"
#include "o2em2.h"
#include "keyboard.h"
#include "cset.h"
#include "timefunc.h"
#include "cpu.h"
#include "vdc.h"
#include "o2em_sdl.h"

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
  0x000000, 0x0e3dd4, 0x00981b, 0x00bbd9, 0xc70008, 0xcc16b3, 0x9d8710, 0xe1dee1,
  0x5f6e6b, 0x6aa1ff, 0x3df07a, 0x31ffff, 0xff4255, 0xff98ff, 0xd9ad5d, 0xffffff
};

PALETTE colors;
PALETTE oldcol;

/* The pointer to the graphics buffer
 * The 10 first and last columns is hide
 * Same for the 5 first and last lines
 * So it is why output is 320 * 240 and BMW=340 and BMPH=250
 *
 * */

static Byte *vscreen = NULL;

static int cached_lines[MAXLINES];

Byte coltab[256];

long clip_low;
long clip_high;

int wsize;

static void create_cmap ();
static void draw_char (Byte ypos, Byte xpos, Byte chr, Byte col);
static void draw_quad (Byte ypos, Byte xpos, Byte cp0l, Byte cp0h, Byte cp1l, Byte cp1h, Byte cp2l, Byte cp2h, Byte cp3l, Byte cp3h);
static void draw_grid ();
void mputvid (unsigned int ad, unsigned int len, Byte d, Byte c);

void draw_region ()
{
  int i;

  if (regionoff == 0xffff)
    i = (master_clk / (LINECNT - 1) - 5);
  else
    i = (master_clk / 22 + regionoff);
  i = (snapline (i, VDCwrite[0xA0], 0));

  if (i < 0)
    i = 0;
  clip_low = last_line * (long) BMPW;
  clip_high = i * (long) BMPW;
  if (clip_high > BMPW * BMPH)
    clip_high = BMPW * BMPH;
  if (clip_low < 0)
    clip_low = 0;
  if (clip_low < clip_high)
    draw_display ();
  last_line = i;
}

static void create_cmap ()
{
  int i;
  /* Initialise parts of the colors array */
  for (i = 0; i < 16; i++)
    {
      /* Use the color values from the color table */
      colors[i + 32].r = colors[i].r = (colortable[i] & 0xff0000) >> 18;
      colors[i + 32].g = colors[i].g = (colortable[i] & 0x00ff00) >> 10;
      colors[i + 32].b = colors[i].b = (colortable[i] & 0x0000ff) >> 2;
    }

  for (i = 16; i < 32; i++)
    {
      /* Half-bright colors for the 50% scanlines */
      colors[i + 32].r = colors[i].r = colors[i - 16].r / 2;
      colors[i + 32].g = colors[i].g = colors[i - 16].g / 2;
      colors[i + 32].b = colors[i].b = colors[i - 16].b / 2;
    }

  for (i = 64; i < 256; i++)
    colors[i].r = colors[i].g = colors[i].b = 0;
  for (i = 0; i < 256; i++)
    {
      colors[i].r *= 4;
      colors[i].g *= 4;
      colors[i].b *= 4;
    }
}

/* rename this function */
void grmode ()
{
  set_color_depth (8);
  wsize = app_data.wsize;
  if (app_data.fullscreen)
    {
      if (app_data.scanlines)
	{
	  wsize = 2;
	  if (set_gfx_mode (GFX_AUTODETECT_FULLSCREEN, 640, 480, 0, 0))
	    {
	      wsize = 1;
	      if (set_gfx_mode (GFX_AUTODETECT_FULLSCREEN, 320, 240, 0, 0))
		{
		  fprintf (stderr, "Error: could not create screen.\n");
		  o2em_clean_quit (EXIT_FAILURE);
		}
	    }
	}
      else
	{
	  wsize = 2;
	  if (set_gfx_mode (GFX_AUTODETECT_FULLSCREEN, 640, 480, 0, 0))
	    {
	      wsize = 1;
	      printf ("%s trying 320x240\n", __func__);
	      if (set_gfx_mode (GFX_AUTODETECT_FULLSCREEN, 320, 240, 0, 0))
		{
		  fprintf (stderr, "Error: could not create screen.\n");
		  o2em_clean_quit (EXIT_FAILURE);
		}
	    }
	}
    }
  else
    {
      if (set_gfx_mode
	  (GFX_AUTODETECT_WINDOWED, WNDW * wsize, WNDH * wsize, 0, 0))
	{
	  wsize = 2;
	  if (set_gfx_mode
	      (GFX_AUTODETECT_WINDOWED, WNDW * 2, WNDH * 2, 0, 0))
	    {
	      if (set_gfx_mode (GFX_AUTODETECT, WNDW * 2, WNDH * 2, 0, 0))
		{
		  o2em_clean_quit (EXIT_FAILURE);
		}
	    }
	  printf ("Could not set the requested window size\n");
	}
    }

  if ((app_data.scanlines) && (wsize == 1))
    {
      printf ("Could not set scanlines\n");
    }

  set_palette (colors);
  set_window_title (app_data.window_title);
  clearscr ();
  set_display_switch_mode (SWITCH_PAUSE);

}

void clearscr ()
{
  acquire_screen ();
  clear (screen);
  release_screen ();
  clear (bmpcache);
}

/* d is color, what a good variable name!!
 * c is COL_HGRID(0x20 or COL_CHAR(0x80) or 8
 * */
void mputvid (unsigned int ad, unsigned int len, Byte d, Byte c)
{
  unsigned int i;
  if (len >= sizeof (coltab))
    {
      printf ("%s ERROR %u > %lu\n", __func__, len, sizeof (coltab));
      return;
    }
  if (c >= sizeof (coltab))
    {
      printf ("%s ERROR %u > %lu\n", __func__, c, sizeof (coltab));
      return;
    }
  if ((ad > (unsigned long) clip_low) && (ad < (unsigned long) clip_high))
    {
      if (((len & 3) == 0) && (sizeof (unsigned long) == 4))
	{			/* TODO unsigned long is 8 on 64bits, this code will not work */
	  unsigned long dddd =
	    (((unsigned long) d) & 0xff) | ((((unsigned long) d) & 0xff) << 8)
	    | ((((unsigned long) d) & 0xff) << 16) |
	    ((((unsigned long) d) & 0xff) << 24);
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
	      vscreen[ad] = d;
	      col[ad] |= c;
	      coltab[c] |= col[ad++];
	    }
	}
    }
}

static void draw_grid ()
{
  unsigned int pnt, pn1;
  Byte mask, d;
  int j, i, x, w;
  Byte color;

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

unsigned char *
get_raw_pixel_line (BITMAP * pSurface, int y)
{
  if (pSurface == NULL)
    {
      fprintf (stderr, "%s Error surface is NULL\n", __func__);
      return NULL;
    }
  if (y < 0)
    {
      fprintf (stderr, "%s Error y < 0\n", __func__);
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

  /* calculate FPS */
  if (app_data.show_fps == 1)
    {
      if (fps_last <= 0)
	fps_last = gettimeticks ();
      index = (index + 1) % 200;
      if (!index)
	{
	  t = gettimeticks ();
	  curr = t - fps_last;
	  fps_last = t;
	}
      if (curr)
	{
	  textprintf_ex (bmp, font, 20, 4, 7, 0, "FPS: %3d",
			 (int) ((200.0 * TICKSPERSEC) / curr + 0.5));
	}
    }
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

  sn = ((wsize > 1) && (app_data.scanlines)) ? 1 : 0;

  for (y = 0; y < WNDH; y++)
    {
      if (!cached_lines[y + 2])
	stretch_blit (bmp, screen,
		      7, 2 + y,
		      WNDW, 1, 0, y * wsize, WNDW * wsize, wsize - sn);
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
			    (y + 1) * wsize - 1, WNDW * wsize, 1);
	      memcpy (get_raw_pixel_line (bmp, y + 2),
		      get_raw_pixel_line (bmpcache, y + 2), bmp->w);
	    }
	}
    }
  clear (screen);
  screen_resize = rotozoomSurface (bmp, 0, wsize, 0);
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
  Byte y, b, d1, cl, c;

  unsigned int pnt, pnt2;
  if (BMPW < 0 || vscreen == NULL)
    {
      fprintf (stderr, "%s error\n", __func__);
      return;
    }

  for (i = clip_low / BMPW; i < clip_high / BMPW; i++)
    memset (vscreen + i * BMPW,
	    ((ColorVector[i] & 0x38) >> 3) | (ColorVector[i] & 0x80 ? 0 : 8),
	    BMPW);

  if (VDCwrite[0xA0] & 0x08)	/* 0xA0 Bit 3 If this bit is 1 the grid is displayed. */
    draw_grid ();

  if (useforen && (!(VDCwrite[0xA0] & 0x20)))	/* if bit 5 of 0xA0 is not set, dont display chars and quad */
    return;

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
void draw_char (Byte ypos, Byte xpos, Byte chr, Byte col)
{
  int j, c;
  Byte cl, d1;
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

void draw_quad (Byte ypos, Byte xpos, Byte cp0l, Byte cp0h, Byte cp1l, Byte cp1h, Byte cp2l, Byte cp2h, Byte cp3l, Byte cp3h)
{
  /* char set pointers */
  int chp[4];
  /* colors */
  Byte col[4];
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

static void txtmsg (int x, int y, int c, const char *s)
{
  textout_centre_ex (bmp, font, s, x + 1, y + 1, 32, -1);
  textout_centre_ex (bmp, font, s, x, y, c, -1);
}

void display_msg (char *msg, int waits)
{
  rectfill (bmp, 60, 72, 271, 90, 9 + 32);
  line (bmp, 60, 72, 271, 72, 15 + 32);
  line (bmp, 60, 72, 60, 90, 15 + 32);
  line (bmp, 61, 90, 271, 90, 1 + 32);
  line (bmp, 271, 90, 271, 72, 1 + 32);
  txtmsg (166, 76, 15 + 32, msg);
  finish_display ();
  rest (waits * 100);
}

int init_display ()
{
  get_palette (oldcol);
  create_cmap ();
  if (BMPW * BMPH == 0)
    {
      fprintf (stderr, "BMPW * BMPH == 0\n");
      o2em_clean_quit (EXIT_FAILURE);
    }
  bmp = create_bitmap (BMPW, BMPH);
  if (bmp == NULL)
    {
      fprintf (stderr, "Could not allocate memory for screen buffer.\n");
      return O2EM_FAILURE;
    }
  bmpcache = create_bitmap (BMPW, BMPH);
  if (bmpcache == NULL)
    {
      /*TODO deallocate bmp */
      fprintf (stderr, "Could not allocate memory for screen buffer.\n");
      return O2EM_FAILURE;
    }
#ifndef __O2EM_SDL__
  vscreen = (Byte *) bmp->dat;
#else
  vscreen = (Byte *) bmp->pixels;
#endif
  clear (bmp);
  clear (bmpcache);
  col = (Byte *) malloc (BMPW * BMPH);
  if (col == NULL)
    {
      fprintf (stderr, "Could not allocate memory for collision buffer.\n");
      o2em_clean_quit (EXIT_FAILURE);
    }
  memset (col, 0, BMPW * BMPH);
  return O2EM_SUCCESS;
}
