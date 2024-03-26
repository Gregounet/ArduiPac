#ifndef ARDUIPAC_SDL_H
#define ARDUIPAC_SDL_H

#define JOY_TYPE_AUTODETECT 0

#define GFX_AUTODETECT_FULLSCREEN 1
#define GFX_AUTODETECT_WINDOWED 2
#define GFX_AUTODETECT 3
#define GFX_TEXT 4
#define SWITCH_BACKGROUND     3

int SCREEN_W;
int SCREEN_H;

#define SWITCH_PAUSE 1
#define SYSTEM_AUTODETECT  0

// typedef SDL_Surface BITMAP;

// extern SDL_Color colors[256];

struct Axis
{
  int d1, d2;
};

struct Stick
{
  struct Axis axis[2];
};

struct Button
{
  int b;
};

struct Joystick
{
  struct Stick stick[4];
  struct Button button[4];
};

struct Joystick joy[2];

int DISPLAY_DEPTH;
void *font;

// BITMAP *screen;

int num_joysticks;

int install_joystick (int joytype);

int set_display_switch_mode (int mode);
// BITMAP *create_bitmap (int w, int h);
// void clear (BITMAP * bitmap);
void set_color_depth (int depth);
int set_gfx_mode (int card, int w, int h, int v_w, int v_h);
// void set_palette (SDL_Color * p);
// void get_palette (SDL_Color * p);
// int check_palette (SDL_Color * p);
void set_window_title (char *name);
void acquire_screen ();
void release_screen ();
// unsigned char *get_raw_pixel_line (SDL_Surface * pSurface, int y);
// unsigned char *get_raw_pixel (SDL_Surface * pSurface, int x, int y);

// void textout_centre_ex (BITMAP * bmp, void *f, const char *s, int x, int y, int color, int bg);
// void textprintf_ex (BITMAP * bmp, void *f, int x, int y, int color, int bg, const char *fmt, ...);

// void stretch_blit (BITMAP * source, BITMAP * dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height);

// void rectfill (BITMAP * bmp, int x1, int y1, int x2, int y2, int color);
// void line (BITMAP * bmp, int x1, int y1, int x2, int y2, int color);

char *strlwr (char *str);
char *strupr (char *str);
int install_timer ();
void rest (unsigned int time);

#endif /* ARDUIPAC_SDL_H */
