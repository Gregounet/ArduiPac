#ifndef ARDUIPAC_8245_H
#define ARDUIPAC_8245_H

#include "arduipac_graphics.h"

#define BITMAP_WIDTH  340
#define BITMAP_HEIGHT 250

#define WNDW 320
#define WNDH 240

#define BMPW 320
#define BMPH 240

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

extern uint8_t collision_table[];
extern uint8_t video_ram[];

extern uint8_t *col;

int init_display ();
void draw_display ();
void finish_display ();

void draw_region ();
void clear_collision ();
void clear_screen (uint8_t *);

#endif /* ARDUIPAC_8245_H */
