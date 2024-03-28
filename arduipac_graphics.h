#ifndef ARDUIPAC_GRAPHICS_H
#define ARDUIPAC_GRAPHICS_H

#define SCREEN_W 256
#define SCREEN_H 192

extern uint8_t *screen;

uint8_t *create_bitmap (int w, int h);
void clear_bitmap (uint8_t * bitmap);
uint8_t * get_raw_pixel_line (uint8_t * bitmap, uint8_t y);
uint8_t * get_raw_pixel (uint8_t * bitmap, uint8_t x, uint8_t y);

#endif /* ARDUIPAC_GRAPHICS_H */
