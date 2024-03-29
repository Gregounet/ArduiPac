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

/*
LINECNT 21
MAXLINES 500

VBLCLK 5493
EVBLCLK_PAL 7259

PAL:
fps = 60
donc des cycles de 16 ms
625 lignes


horloge CPU = 1,79 MHz ensuite divisé par 5 donc 0,36 MHz (= 360 KhZ = 360.000 NHz) 
soit des cycles de 2,8 us

(1/60)/(1/(1,79/5) = (1/60)/(5/1,79) = (1/60)*(1,79/5) = 1,79/(5*60) = 1790000/300 = 17900/3 = 5966

Donc 5966 cycles CPU par trame video

Ou bien 7160 cycles CPU en NTSC avec une fps = 50

*/

static int cached_lines[MAXLINES];

uint8_t collision_table[256];
uint8_t intel8245_ram[256];

long clip_low;
long clip_high;

static void draw_char (uint8_t ypos, uint8_t xpos, uint8_t chr, uint8_t color);
static void draw_quad (uint16_t ypos, uint16_t xpos, uint16_t cp0l, uint16_t cp0h, uint16_t cp1l, uint16_t cp1h, uint16_t cp2l, uint16_t cp2h, uint16_t cp3l, uint16_t cp3h);
static void draw_grid ();

uint8_t *bmp;
uint8_t *bmpcache;
uint8_t *screen;
uint8_t *vscreen;

void draw_region ()
{
  int last_line; // Cette variable devrait être globale TODO
  int i;

  i = (master_clk / (LINECNT - 1) - 5);  // TODO LINECNT = 21
  if (i < 0) i = 0;

  clip_low  = last_line * (long) BITMAP_WIDTH; // 340
  clip_high = i         * (long) BITMAP_WIDTH;

  if (clip_high > BITMAP_WIDTH * BITMAP_HEIGHT) clip_high = BITMAP_WIDTH * BITMAP_HEIGHT;

  if (clip_low < 0) clip_low = 0;
  if (clip_low < clip_high) draw_display (); // C'est là que tout se passe: on appelle draw_display() !!!! TODO

  last_line = i;
}

void mputvid (uint32_t location, uint16_t len, uint8_t color, uint16_t c)
	//
	// A priori tout ce code gère les collisions et non le display donc c'est super important
	// En fait ce chip "video" gere:
	// - L'affichage
	//   * foreground
	//   * background
	// - Le son
	// - Les collisions
	//
	// Est-ce aussi lui qui gère clavier et joysticks ?
	//
{
  if (len >= sizeof (collision_table)) return;
  if (c   >= sizeof (collision_table)) return;

  if ((location > (uint32_t) clip_low) && (location < (uint32_t) clip_high)) {
      if ((len & 0x03) == 0) { // TODO C'est quoi ce 0x03 ???
	  unsigned long dddd = (((unsigned long) color)) | ((((unsigned long) color)) << 8) | ((((unsigned long) color)) << 16) | ((((unsigned long) color)) << 24);
	  unsigned long cccc = (((unsigned long) color)) | ((((unsigned long) color)) << 8) | ((((unsigned long) color)) << 16) | ((((unsigned long) color)) << 24);
	  // TODO Mais c'est quoi tout ce bordel ???
	  // en fait dddd == cccc soit au total 8 octets identiques !
	  // regarder le code d'origine: j'ai fait de la merde ici TODO TODO TODO

	  for (uint16_t i = 0; i < (len >> 2); i++) {
	      //((unsigned long *) (vscreen + ad)) = dddd; 
	      //cccc |= *((unsigned long *) (col + ad));
	      //*((unsigned long *) (col + ad)) = cccc; 
	      collision_table[c] |= ((cccc | (cccc >> 8) | (cccc >> 16) | (cccc >> 24)) & 0xFF);
	      location += 4;
	    }
	}
      else {
	  for (uint16_t i = 0; i < len; i++) {
	      vscreen[location] = color;
	      //col[ad] |= c;
	      //collision_table[c] |= collision[ad++];
	    }
	}
    }
}

static void draw_grid () {
  unsigned int pnt, pn1; // Pointeurs sur des points à afficher dans la bitmpa
  uint8_t mask;          // Masque utilisé pour le décodage des bits de chaque octet
  uint8_t data;          // Octets de la RAM vidéo
  int x;
  int w;
  uint8_t color;

  if (intel8245_ram[0xA0] & 0x40) { // Bit 6 de 0xA0 controle l'affichage des points de la grille 
      for (int j = 0; j < 9; j++) {
	  pnt = (((j * 24) + 24) * BITMAP_WIDTH);
	  for (int i = 0; i < 9; i++) {
	      pn1 = pnt + (i * 32) + 20;
	      mputvid (pn1                   , 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
	      mputvid (pn1 + BITMAP_WIDTH    , 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
	      mputvid (pn1 + BITMAP_WIDTH * 2, 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
	   }
	}
    }

  mask = 0x01;
  for (uint8_t j = 0; j < 9; j++)                 // Tracé des 9 lignes horizontales
    {
      pnt = (((j * 24) + 24) * BITMAP_WIDTH);
      for (uint8_t i = 0; i < 9; i++) {
	  pn1 = pnt + (i * 32) + 20;
	  if ((pn1 + BITMAP_WIDTH * 3 >= (unsigned long) clip_low) && (pn1 <= (unsigned long) clip_high)) {
	      data = intel8245_ram[0xC0 + i];    // 0xC0 - 0xC8 = Lignes horizontales, chaque octet représente une colonne
	      if (j == 8) {
		 data = intel8245_ram[0xD0 + i]; // 0xD0 - 0xD8 = 9ième ligne horizontale, seul le bit 0 est utile
		 mask = 1;
	      }
	      if (data & mask) {
		 mputvid (pn1,                    36, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
		 mputvid (pn1 + BITMAP_WIDTH,     36, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
		 mputvid (pn1 + BITMAP_WIDTH * 2, 36, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
	      }
           }
	}
      mask = mask << 1;
    }

  mask = 0x01; // Tracé des 10 lignes verticales
  w = 4;
  if (intel8245_ram[0xA0] & 0x80) w = 32; // Bit 7 de 0xA0 contrôle la largeur des lignes verticales
  for (int j = 0; j < 10; j++) {
      pnt = (j * 32);
      mask = 0x01;
      data = intel8245_ram[0xE0 + j];    // 0xE0 - 0xE9 = Lignes verticales, chaque octet représente une ligne
      for (x = 0; x < 8; x++) {
	  pn1 = pnt + (((x * 24) + 24) * BITMAP_WIDTH) + 20;
	  if (data & mask) {
	      for (uint8_t i = 0; i < 24; i++) {
		  if ((pn1 >= clip_low) && (pn1 <= clip_high))
		      mputvid (pn1, w, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_VGRID);
		  pn1 += BITMAP_WIDTH;
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

  for (y = 0; y < BITMAP_HEIGHT; y++)
      cached_lines[y] = !memcmp (get_raw_pixel_line (bmpcache, y), get_raw_pixel_line (bmp, y), BITMAP_HEIGHT);
  if (!cached_lines[y]) memcpy (get_raw_pixel_line (bmpcache, y), get_raw_pixel_line (bmp, y), BITMAP_WIDTH);

  for (y = 0; y < 10; y++) cached_lines[(y + cache_counter) % BITMAP_HEIGHT] = 0;
  cache_counter = (cache_counter + 10) % BITMAP_HEIGHT;

  /*
  for (y = 0; y < WNDH; y++)
      if (!cached_lines[y + 2]) stretch_blit (bmp, screen, 7, 2 + y, WNDW, 1, 0, y,  WNDW , wsize - sn);
      */

  if (sn) {
      for (y = 0 ; y < BITMAP_HEIGHT ; y++) {
	  if (!cached_lines[y + 2]) {
	      for (x = 0; x < BITMAP_WIDTH; x++) *get_raw_pixel (bmp, x, y + 2) += 16;
	      //stretch_blit (bmp, screen, 7, 2 + y, WNDW, 1, 0, (y + 1) - 1, WNDW , 1);
	      memcpy (get_raw_pixel_line (bmp, y + 2), get_raw_pixel_line (bmpcache, y + 2), BITMAP_HEIGHT);
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

  for (int i = clip_low / BITMAP_WIDTH; i < clip_high / BITMAP_WIDTH; i++) memset (vscreen + i * BITMAP_WIDTH,  0x0E, BITMAP_WIDTH);

  if (intel8245_ram[0xA0] & 0x08) draw_grid ();

  for (uint8_t i = 0x10; i < 0x40; i += 0x04) draw_char (intel8245_ram[i], intel8245_ram[i + 1], intel8245_ram[i + 2], intel8245_ram[i + 3]);

  for (uint8_t i = 0x40; i < 0x80; i += 0x10)
    draw_quad (intel8245_ram[i], intel8245_ram[i + 1], intel8245_ram[i + 2], intel8245_ram[i + 3], intel8245_ram[i + 6], intel8245_ram[i + 7],
		    intel8245_ram[i + 10], intel8245_ram[i + 11], intel8245_ram[i + 14], intel8245_ram[i + 15]);
// TODO TODO TODO vérifier les indices des caractères ci-dessus (10,11,14,15) qui me paraissent bien étranges

  // Il faudrait metre l'affichage des sprites dans une fonction au même titre que le reste TODO
  c = 8; // TODO vérifier que c va être utilisé
  for (int i = 12; i >= 0; i -= 4) {
      pnt2 = 0x80 + (i * 2);

      y = intel8245_ram[i];
      x = intel8245_ram[i+1] - 8;
      t = intel8245_ram[i+2];

      cl = ((t & 0x38) >> 3);
      cl = ((cl & 2) | ((cl & 1) << 2) | ((cl & 4) >> 2)) + 8; // Il faudrait peut-être écrire une fonction pour cela pour gagner de la mémoire ? TODO

      if ((x < 164) && (y > 0) && (y < 232)) {
	  pnt = y * BITMAP_WIDTH + (x * 2) + 20;
	  if (t & 4) {		
	      if ((pnt + BITMAP_WIDTH * 32 >= clip_low) && (pnt <= clip_high)) {
		  for (uint8_t j = 0; j < 8; j++) {
		      sm = (((j % 2 == 0) && (((t >> 1) & 1) != (t & 1))) || ((j % 2 == 1) && (t & 1))) ? 1 : 0; d1 = intel8245_ram[pnt2++];
		      for (uint8_t b = 0; b < 8; b++) {
			  if (d1 & 0x01) {
			      if ((x + b + sm < 159) && (y + j < 247)) {
				  mputvid (sm + pnt                   , 4, cl, c);
				  mputvid (sm + pnt +     BITMAP_WIDTH, 4, cl, c);
				  mputvid (sm + pnt + 2 * BITMAP_WIDTH, 4, cl, c);
				  mputvid (sm + pnt + 3 * BITMAP_WIDTH, 4, cl, c);
				}
			    }
			  pnt += 4;
			  d1 = d1 >> 1;
			}
		      pnt += BITMAP_WIDTH * 4 - 32;
		    }
		}
	    }
	  else
	    {
	      if ((pnt + BITMAP_WIDTH * 16 >= clip_low) && (pnt <= clip_high)) {
		  for (uint8_t j = 0; j < 8; j++) {
		      sm = (((j % 2 == 0) && (((t >> 1) & 1) != (t & 1))) || ((j % 2 == 1) && (t & 1))) ? 1 : 0;
		      d1 = intel8245_ram[pnt2++];
		      for (uint8_t b = 0; b < 8; b++) {
			  if (d1 & 0x01) {
			      if ((x + b + sm < 160) && (y + j < 249)) {
				  mputvid (sm + pnt               , 2, cl, c);
				  mputvid (sm + pnt + BITMAP_WIDTH, 2, cl, c);
				}
			    }
			  pnt += 2;
			  d1 = d1 >> 1;
			}
		      pnt += BITMAP_WIDTH * 2 - 16;
		    }
		}
	    }
	}
      c = c >> 1;
    }
}

void draw_char (uint8_t ypos, uint8_t xpos, uint8_t chr, uint8_t col)
{
  uint16_t c;  // Il s'agit d'un indice de base dans cset[]
  uint8_t cl;
  uint8_t d1;  // Ce serait un octet représentant 8 pixels qui proviendrait de cset[]
  uint8_t y;
  uint8_t n;
  uint32_t pnt;

  y = (ypos & 0xFE);
  pnt = y * BITMAP_WIDTH  +  ((xpos - 8) * 2)  + 20;

  ypos = ypos >> 1;
  n = 8 - (ypos % 0x08) - (chr % 0x08); // Donc n <= 8
  if (n < 3) n = n + 7;                 // Donc 3 <= n <= 9 TODO Wtf ???

  if ((pnt + BITMAP_WIDTH * 2 * n >= clip_low) && (pnt <= clip_high)) {
      c = (uint16_t) chr + ypos;
      if (col & 0x01) c += 256;
      if (c > 511) c -= 512;

      // Je penche pour des couleurs sur 3 bits (RGB) plus que pour des collisions
      cl = ((col & 0x0E) >> 1);
      cl = ((cl & 0x02) | ((cl & 0x01) << 2) | ((cl & 0x04) >> 2)) + 8;

      if ((y > 0) && (y < 232) && (xpos < 157)) {                        // TODO Comme y est un uint8_t le test > 0 est inutile
	  for (uint8_t j = 0; j < n; j++) {                                  // On va donc looper entre 2 et 8 fois
	      d1 = cset[c + j];                         
	      for (uint8_t b = 0; b < 8; b++) {                              // On parcourt les 8 bits (pixels) de chaque octet provenant de cset[]
		  if (d1 & 0x80) {
		      if ((xpos - 8 + b < 160) && (y + j < 240)) {       // x est exprimé sous la forme [0-159] et non [0-319]
			  mputvid (pnt               , 2, cl, COLLISION_CHAR);
			  mputvid (pnt + BITMAP_WIDTH, 2, cl, COLLISION_CHAR);
			}
		    }
		  pnt += 2;
		  d1 = d1 << 1;
		}
	      pnt += BITMAP_WIDTH * 2 - 16;
	    }
	}
    }
}

void draw_quad (uint16_t ypos, uint16_t xpos, uint16_t cp0l, uint16_t cp0h, uint16_t cp1l, uint16_t cp1h, uint16_t cp2l, uint16_t cp2h, uint16_t cp3l, uint16_t cp3h)
{
  uint16_t chp[4]; // Des indices dans cset[]
  uint16_t col[4];
  uint8_t lines;
  uint32_t pnt;
  uint16_t offset; // TODO plutot 32 bits ?

  pnt = (ypos & 0xFE) * BITMAP_WIDTH + ((xpos - 8) * 2) + 20;
  if (pnt > clip_high) return;

  chp[0] = ((cp0h & 0x01) << 8) | cp0l;
  chp[1] = ((cp1h & 0x01) << 8) | cp1l;
  chp[2] = ((cp2h & 0x01) << 8) | cp2l;
  chp[3] = ((cp3h & 0x01) << 8) | cp3l;

  for (uint8_t i = 0; i < 4; i++) chp[i] = (chp[i] + (ypos >> 1)) & 0x1FF;

  lines = 8 - (chp[3] + 1) % 8;
  if (pnt + BITMAP_WIDTH * 2 * lines < clip_low) return;

  col[0] = (cp0h & 0x0E) >> 1;
  col[1] = (cp1h & 0x0E) >> 1;
  col[2] = (cp2h & 0x0E) >> 1;
  col[3] = (cp3h & 0x0E) >> 1;

      // Je penche pour des couleurs sur 3 bits (RGB) plus que pour des collisions
  for (uint8_t i = 0; i < 4; i++) col[i] = ((col[i] & 2) | ((col[i] & 1) << 2) | ((col[i] & 4) >> 2)) + 8;

  while (lines-- > 0)
    {
      offset = 0;
      for (uint8_t i = 0; i < 4; i++) {                        // Pour chacun des 4 caractères
	  for (uint8_t j = 0; j < 8; j++) {                    // Pour chacune des 8 lignes d'un caractère
	      if ((cset[chp[i]] & (0x01 << (7 - j))) && (offset < BITMAP_WIDTH)) {
		  mputvid (pnt + offset               , 2, col[i], COLLISION_CHAR);
		  mputvid (pnt + offset + BITMAP_WIDTH, 2, col[i], COLLISION_CHAR);
		}
	      offset += 2;
	    }
	  offset += 16;
	}
      for (int i = 0; i < 4; i++) chp[i] = (chp[i] + 1) & 0x1FF;
      pnt += BITMAP_WIDTH * 2;
    }
}

int init_display ()
{
  bmp      = create_bitmap (BITMAP_WIDTH, BITMAP_HEIGHT);
  bmpcache = create_bitmap (BITMAP_WIDTH, BITMAP_HEIGHT);

  clear_screen(bmp);
  clear_screen(bmpcache);

  // vscreen = (uint8_t *) BITMAP_HEIGHT; TODO: trop con cette ligne

  // col = (uint8_t *) malloc (BITMAP_WIDTH * BITMAP_HEIGHT);
  // memset (col, 0, BITMAP_WIDTH * BITMAP_HEIGHT);
  return 0;
}
