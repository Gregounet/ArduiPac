#include "arduipac_sdl.h"
#include "arduipac.h"

#include <SDL/SDL_gfxPrimitives.h>

BITMAP * create_bitmap (int w, int h)
{
  SDL_Surface *temp;
  int rmask, gmask, bmask, ret;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
#endif

  temp = SDL_CreateRGBSurface (SDL_HWSURFACE, w, h, DISPLAY_DEPTH,
			       rmask, gmask, bmask, 0);

  if (temp == NULL)
    {
      return NULL;
    }
  ret = SDL_SetPalette (temp, SDL_LOGPAL | SDL_PHYSPAL, colors, 0, 256);
  return temp;
}

void clear (BITMAP * bitmap)
{
  if (bitmap != NULL) SDL_FillRect (bitmap, 0, 0);
}

void set_color_depth (int depth)
{
  if (depth == 8 || depth == 16 || depth == 24 || depth == 32)
    DISPLAY_DEPTH = depth;
  else
}

int set_gfx_mode (int card, int w, int h, int v_w, int v_h)
{
  int video_depth = DISPLAY_DEPTH;
  int flags = SDL_HWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF;	/* TODO perhaps add SDL_RESIZABLE */

  SCREEN_H = h;
  SCREEN_W = w;

  if (screen == NULL)
    {
      SDL_ShowCursor (0);
      if (card == GFX_AUTODETECT_FULLSCREEN)
	flags |= SDL_FULLSCREEN;
      screen = SDL_SetVideoMode (SCREEN_W, SCREEN_H, video_depth, flags);
    }
  else
    {
      return O2EM_FAILURE;
    }
  if (screen == NULL)
    {
      return O2EM_FAILURE;
    }
  return O2EM_SUCCESS;
}

void rest (unsigned int time)
{
  SDL_Delay (time);
}

void stretch_blit (BITMAP * source, BITMAP * dest, int source_x, int source_y,
	      int source_width, int source_height, int dest_x, int dest_y,
	      int dest_width, int dest_height)
{
  int ret;
  SDL_Rect srcrect, dstrect;
  if (source == NULL || dest == NULL)
    {
      return;
    }
  srcrect.x = source_x;
  srcrect.y = source_y;
  srcrect.w = source_width;
  srcrect.h = source_height;
  dstrect.x = dest_x;
  dstrect.y = dest_y;
  dstrect.w = dest_width;
  dstrect.h = dest_height;
  ret = SDL_BlitSurface (source, &srcrect, dest, &dstrect);
  if (ret != 0)
    {
    }
}

void rectfill (BITMAP * bmp, int x1, int y1, int x2, int y2, int color)
{
  SDL_Rect dst;
  dst.x = x1;
  dst.y = y1;
  dst.w = x2 - x1;
  dst.h = y2 - y1;

  SDL_FillRect (bmp, &dst, color);
}

int check_palette (SDL_Color * p)
{
  int i;
  for (i = 0; i < 64; i++)
    {
      if (p[i].r == 0 && p[i].g == 0 && p[i].b == 0)
	{
	  return 1;
	}
    }
  return 0;
}

void set_palette (SDL_Color * p)
{
  check_palette (p);
  SDL_SetPalette (screen, SDL_LOGPAL | SDL_PHYSPAL, p, 0, 256);
}

unsigned char * get_raw_pixel_line (SDL_Surface * pSurface, int y)
{
  if (pSurface == NULL)
    {
      return NULL;
    }
  if (y < 0)
    {
      return NULL;
    }
  return (unsigned char *) pSurface->pixels + (pSurface->pitch * y);
}

unsigned char * get_raw_pixel (SDL_Surface * pSurface, int x, int y)
{
  if (pSurface == NULL)
    {
      return NULL;
    }
  if (y < 0)
    {
      return NULL;
    }
  return (unsigned char *) pSurface->pixels + (pSurface->pitch * y) + x;
}

