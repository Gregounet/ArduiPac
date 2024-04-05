#include <stdint.h>
#include <stdio.h>

#include "arduipac_input.h"
#include "arduipac_8048.h"

#define DEBUG 0

void
write_p1 (uint8_t data)
{
  if (DEBUG)
    fprintf (stderr, "write_p1(0x%02X)\n", data);
  p1 = data;
}

uint8_t
read_p2 ()
{
  int i;
  int scan_input;
  int scan_output;
  int keymap;

  if (DEBUG)
    fprintf (stderr, "read_p2()\n");

  if (!(p1 & 0x04))		// Lecture du clavier
    {
      scan_input = (p2 & 0x07);
      scan_output = 0xFF;

      if (scan_input < 0x06)	// Seulement 5 lignes du clavier sont scannées
	{
	  for (i = 0x00; i < 0x08; i++)
	    {
	      // keymap = key_map[scan_input][i];
	      /*
	         if ((key[keymap] && ((!joykeystab[km]) || (key_shifts & KB_CAPSLOCK_FLAG))) || (key2[keymap])) scan_output = i ^ 0x07;
	       */
	    }
	}
      if (scan_output != 0xFF)	// Au moins une touche est pressée
	{
	  p2 &= 0x0F;
	  p2 |= scan_output << 5;
	}
      else
	p2 |= 0xF0;
    }
  else
    p2 |= 0xF0;

  return p2;
}

uint8_t
in_bus ()
{
  uint8_t data = 0;
  uint8_t scan_input = 0;
  uint8_t mode = 0;
  uint8_t jn = 0;

  if (DEBUG)
    fprintf (stderr, "read_p2()\n");

  if ((p1 & 0x08) && (p1 & 0x10))	// Ni le 8245 ni la RAM externe ne sont activés TODO: remplacer par p1 & 0x18 == 0x18
    {
      if (!(p1 & 0x04))
	scan_input = (p2 & 0x07);	// On cherche bien à lire les données du clavier. p2 & 0x07 sont les données précedement ecrites dans p2 pour scanner le clavier
      data = 0xFF;
      data &= 0xFE;		// up
      /*
         Données des joysticks
         data &= 0xFE; // up
         data &= 0xFD; // right
         data &= 0xFB; // down
         data &= 0xF7; // left
         data &= 0xEF; // both buttons
       */
    }
  return data;
}
