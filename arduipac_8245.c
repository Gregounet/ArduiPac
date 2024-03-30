#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

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

uint8_t intel8245_ram[256];
uint8_t collision_table[256];

long clip_low;
long clip_high;

uint8_t bmp[BITMAP_WIDTH * BITMAP_HEIGHT] ;

void draw_grid () {
  uint32_t line_pnt, dot_pnt;                                                  // Pointeurs sur des points à afficher dans la bitmpap
  uint8_t mask;                                                                // Masque utilisé pour le décodage des bits de chaque octet
  uint8_t data;                                                                // Octets de la RAM vidéo
  int x;
  int width;
  uint8_t color;

  if (intel8245_ram[0xA0] & 0x40) {                                            // Bit 6 de 0xA0 controle l'affichage des points de la grille 
      for (int j = 0; j < 9; j++) {                                            // j : balayage par lignes de 0 à 8
	  line_pnt = (j * 24 + 24) * BITMAP_WIDTH;                           // Les lignes sont séparées de 24 pixels. line_pnt pointe sur le début de la ligne concernée.
	  for (int i = 0; i < 9; i++) {                                        // i : balayage par colonne de 0 à 8
	      dot_pnt = line_pnt + (i * 32) + 20;                              // dot_pnt pointe sur le point ciblé. Les points sont séparés de 32 pixels.
              bmp[dot_pnt] = 1;
	      /* Points de 4*3 ?
	      mputvid (dot_pnt                   , 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
	      mputvid (dot_pnt + BITMAP_WIDTH    , 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
	      mputvid (dot_pnt + BITMAP_WIDTH * 2, 4, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 0x08), COLLISION_HGRID);
	      */
	   }
	}
    }

  // TODO optimisation future: aller de 1 à 9 (au lieu de 0 à 8) et éviter ainsi ((j*24)+24)*WIDTH pour avoir à la place j*24*WIDTH
  //      optimisation future: je pense que deux boucles distinctes iraient mieux
  //
  mask = 0x01;                                                                  // Tracé des 9 lignes horizontales.
  for (uint8_t j = 0; j < 9; j++) {                                             // j : balayage des lignes 0 à 8
      line_pnt = (j * 24 + 24) * BITMAP_WIDTH;
      for (uint8_t i = 0; i < 9; i++) {                                         // i : balayage par colonnes de 0 à 8
	  dot_pnt = line_pnt + (i * 32) + 20;
	      data = intel8245_ram[0xC0 + i];                                   // 0xC0 - 0xC8 = Lignes horizontales, chaque octet représente une colonne
	      if (j == 8) {
		 data = intel8245_ram[0xD0 + i];                                // 0xD0 - 0xD8 = 9ième ligne horizontale, seul le bit 0 est utile
		 mask = 1;
	      }
	      if (data & mask) for (uint8_t k = 0 ; k < 32 ; k++) bmp[dot_pnt+k] = 1; // 32 ou 36 de large ?
            }
      mask <<= 1;
      }

  mask = 0x01;                                                                 // Tracé des 10 lignes verticales
  width = 4;
  if (intel8245_ram[0xA0] & 0x80) width = 32;                                  // Bit 7 de 0xA0 contrôle la largeur des lignes verticales
  for (int j = 0; j < 10; j++) {                                               // Cette fois il semble qu'on balaye par colonne d'abord
      line_pnt = (j * 32);
      mask = 0x01;
      data = intel8245_ram[0xE0 + j];                                          // 0xE0 - 0xE9 = Lignes verticales, chaque octet représente une ligne
      for (x = 0; x < 8; x++) {                                                // x serait un numéro de ligne !!! Va falloir réorganiser tout ça !!!
	  dot_pnt = line_pnt + (((x * 24) + 24) * BITMAP_WIDTH) + 20;
	  if (data & mask) {
	      for (uint8_t i = 0; i < 24; i++) {
		  bmp[dot_pnt] = 1;
		      /*
		      mputvid (dot_pnt, w, (color & 0x07) | ((color & 0x40) >> 3) | (color & 0x80 ? 0 : 8), COLLISION_VGRID);
		      */
		  dot_pnt += BITMAP_WIDTH;
	      }
	    }
	  mask <<= 1;
	}
    }
}

void show_12chars()
{
  uint8_t arduipac_x;
  uint8_t arduipac_y;
  uint16_t arduipac_car;
  uint8_t arduipac_color;
  
  for (uint8_t i = 0 ; i < 12 ; i++)
  {
    arduipac_x     = intel8245_ram[0x10 + i * 0x04 + 0x01] & 0xFF ;
    arduipac_y     = intel8245_ram[0x10 + i * 0x04 + 0x00] >> 1 ;
    arduipac_car   = intel8245_ram[0x10 + i * 0x04 + 0x03] & 0x01 | intel8245_ram[0x10 + i * 0x04 + 0x02] & 0xFF ;
    arduipac_color = (intel8245_ram[0x10 + i * 0x04 + 0x03] & 0x0E) >> 1;
    // show_1char(arduipac_x, arduipac_y, arduipac_car, arduipac_color);
  }
}

void show_1char (uint8_t x, uint16_t y, uint8_t car, uint8_t color)
{
	uint32_t bmp_pnt;

  	// bmp_pnt = y * BITMAP_WIDTH * 2  +  ((x - 8) * 2)  + 20;
	// fprintf(stderr,"show1_char(): X = %d, Y = %d, indice dans cst = %d, couleur = %d\n", x, y, car, color);
}

/*
  for (uint8_t i = 0x10; i < 0x40; i += 0x04) draw_char (intel8245_ram[i], intel8245_ram[i + 1], intel8245_ram[i + 2], intel8245_ram[i + 3]);
  uint16_t c;  // Il s'agit d'un indice de base dans cset[]
  uint8_t cl;
  uint8_t d1;  // Ce serait un octet représentant 8 pixels qui proviendrait de cset[]
  uint8_t y;
  uint8_t n;
  uint32_t pnt;
  y = (ypos & 0xFE);
  pnt = y * BITMAP_WIDTH  +  ((xpos - 8) * 2)  + 20;
  ypos >>= 1;
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
		  d1 <<= 1;
		}
	      pnt += BITMAP_WIDTH * 2 - 16;
	    }
	}
    }
}
*/

void show_4quads()
{
  // for (uint8_t i = 0x40; i < 0x80; i += 0x10) show_1quad(i);
  // for (uint8_t i = 0x40 ; i < 0x80; i++) fprintf(stderr, "0x%02X\t", intel8245_ram[i]);
  //fprintf(stderr, "\n");
  
}

void show_1quad (uint8_t quad_indx)
{
  uint8_t arduipac_x;
  uint8_t arduipac_y;
  uint16_t arduipac_car;
  uint8_t arduipac_color;

  arduipac_x     = intel8245_ram[quad_indx + 0x01] & 0xFF ;
  arduipac_y     = intel8245_ram[quad_indx + 0x00] >> 1 ;
  arduipac_car   = intel8245_ram[quad_indx + 0x03] & 0x01 | intel8245_ram[quad_indx + 0x02] & 0xFF ;
  arduipac_color = (intel8245_ram[quad_indx + 0x03] & 0x0E) >> 1;
  show_1char(arduipac_x, arduipac_y, arduipac_car, arduipac_color);

  arduipac_x     = intel8245_ram[quad_indx + 0x05] & 0xFF ;
  arduipac_y     = intel8245_ram[quad_indx + 0x04] >> 1 ;
  arduipac_car   = intel8245_ram[quad_indx + 0x07] & 0x01 | intel8245_ram[quad_indx + 0x06] & 0xFF ;
  arduipac_color = (intel8245_ram[quad_indx + 0x0A] & 0x0E) >> 1;
  show_1char(arduipac_x, arduipac_y, arduipac_car, arduipac_color);

  arduipac_x     = intel8245_ram[quad_indx + 0x09] & 0xFF ;
  arduipac_y     = intel8245_ram[quad_indx + 0x08] >> 1 ;
  arduipac_car   = intel8245_ram[quad_indx + 0x0B] & 0x01 | intel8245_ram[quad_indx + 0x0A] & 0xFF ;
  arduipac_color = (intel8245_ram[quad_indx + 0x0B] & 0x0E) >> 1;
  show_1char(arduipac_x, arduipac_y, arduipac_car, arduipac_color);

  arduipac_x     = intel8245_ram[quad_indx + 0x0D] & 0xFF ;
  arduipac_y     = intel8245_ram[quad_indx + 0x0C] >> 1 ;
  arduipac_car   = intel8245_ram[quad_indx + 0x0E] & 0x01 | intel8245_ram[quad_indx + 0x0E] & 0xFF ;
  arduipac_color = (intel8245_ram[quad_indx + 0x0F] & 0x0E) >> 1;
  show_1char(arduipac_x, arduipac_y, arduipac_car, arduipac_color);
}

void show_4sprites()
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
				      /*
				  mputvid (sm + pnt                   , 4, cl, c);
				  mputvid (sm + pnt +     BITMAP_WIDTH, 4, cl, c);
				  mputvid (sm + pnt + 2 * BITMAP_WIDTH, 4, cl, c);
				  mputvid (sm + pnt + 3 * BITMAP_WIDTH, 4, cl, c);
				  */
				}
			    }
			  pnt += 4;
			  d1 >>= 1;
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
				      /*
				  mputvid (sm + pnt               , 2, cl, c);
				  mputvid (sm + pnt + BITMAP_WIDTH, 2, cl, c);
				  */

				}
			    }
			  pnt += 2;
			  d1 >>= 1;
			}
		      pnt += BITMAP_WIDTH * 2 - 16;
		    }
		}
	    }
	}
      c >>= 1;
    }
}


void draw_region ()
{
  draw_display (); // C'est là que tout se passe: on appelle draw_display() !!!! TODO
}

/*
void mputvid (uint32_t location, uint16_t len, uint8_t color, uint16_t c)
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
	      //vscreen[location] = color;
	      //col[ad] |= c;
	      //collision_table[c] |= collision[ad++];
	    }
	}
    }
}
*/

void clear_collision()
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

  //for (int i = clip_low / BITMAP_WIDTH; i < clip_high / BITMAP_WIDTH; i++) memset (vscreen + i * BITMAP_WIDTH,  0x0E, BITMAP_WIDTH);

  if (intel8245_ram[0xA0] & 0x08) draw_grid ();
  show_12chars();
  show_4quads();
  show_4sprites();

}

void init_intel8225 ()
{
  fprintf(stderr," Initializing intel8225_ram\n");
  for (uint8_t i = 0x00 ; i < 0xFF ; i++) intel8245_ram[i] = 0x00;
  fprintf(stderr," Initializing bitmap\n");
  for (uint32_t i = 0 ; i < BITMAP_WIDTH * BITMAP_HEIGHT ; i++) bmp[i] = 0x00;
}
