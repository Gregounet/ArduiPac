#ifndef ARDUIPAC_8245_H
#define ARDUIPAC_8245_H

#include "arduipac_graphics.h"

#define BITMAP_WIDTH  340
#define BITMAP_HEIGHT 250

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

extern uint8_t collision_table[];

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
