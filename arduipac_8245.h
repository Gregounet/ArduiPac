#ifndef ARDUIPAC_8245_H
#define ARDUIPAC_8245_H

#include "arduipac_graphics.h"

#define BITMAP_WIDTH  340
#define BITMAP_HEIGHT 250

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

extern uint8_t collision_table[];
extern uint8_t intel8245_ram[];

extern uint8_t *col;

void init_intel8225 ();
void draw_display ();
void finish_display ();

static void draw_grid ();
static void draw_char (uint8_t ypos, uint8_t xpos, uint8_t chr, uint8_t color);
static void draw_quad (uint16_t ypos, uint16_t xpos, uint16_t cp0l, uint16_t cp0h, uint16_t cp1l, uint16_t cp1h, uint16_t cp2l, uint16_t cp2h, uint16_t cp3l, uint16_t cp3h);
static void draw_sprites();

void show_12chars ();
void show_1char (uint8_t x, uint16_t y, uint8_t car, uint8_t color);
void show_4quads ();
void show_1quad(uint8_t);
void show_4sprites();

void draw_region ();
void clear_collision ();
void clear_screen (uint8_t *);

#endif /* ARDUIPAC_8245_H */
