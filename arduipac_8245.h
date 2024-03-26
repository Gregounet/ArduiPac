#ifndef ARDUIPAC_8245_H
#define ARDUIPAC_8245_H

#include "arduipac_sdl.h"

#define BITMAP_WIDTH  340
#define BITMAP_HEIGHT 250

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

#define BOX_W     MIN(512, SCREEN_W-16)
#define BOX_H     MIN(256, (SCREEN_H-64)&0xFFF0)

#define BOX_L     ((SCREEN_W - BOX_W) / 2)
#define BOX_R     ((SCREEN_W + BOX_W) / 2)

#define BOX_T     ((SCREEN_H - BOX_H) / 2)
#define BOX_B     ((SCREEN_H + BOX_H) / 2)

extern uint8_t coltab[];
extern long clip_low;
extern long clip_high;

uint8_t *col;

int init_display ();
void draw_display ();
void set_textmode ();
void draw_region ();
void finish_display ();
void grmode ();
void clear_collision ();
void clearscr ();

#endif /* ARDUIPAC_8245_H */
