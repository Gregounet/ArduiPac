#include <stdint.h>

#include "arduipac_8048.h"
#include "arduipac_input.h"

void write_p1 (uint8_t data)
{
  p1 = data;
}

uint8_t read_p2 ()
{
  int i;
  int si;
  int so;
  int keymap;

  if (!(p1 & 0x04))
    {
      si = (p2 & 0x07); // les trois bits de droite
      so = 0xFF;
/*
      if (si < 0x06)
	{
	  for (i = 0x00; i < 0x08; i++)
	    {
	      keymap = key_map[si][i];
	      if ((key[keymap] && ((!joykeystab[km]) || (key_shifts & KB_CAPSLOCK_FLAG))) || (key2[keymap])) so = i ^ 0x07;
	    }
	}
*/
      if (so != 0xFF)
	{
	  p2 = p2 & 0x0F;
	  p2 = p2 | (so << 5);
	}
      else p2 = p2 | 0xF0;
    }
  else p2 = p2 | 0xF0;

  return p2;
}

uint8_t in_bus ()
{
  uint8_t data = 0; 
  uint8_t si = 0;
  uint8_t mode = 0; 
  uint8_t jn = 0;

  if ((p1 & 0x08) && (p1 & 0x10))
    {
      if (!(p1 & 0x04)) si = (p2 & 0x07);
      data = 0xFF;
      /*
      if (si == 1)
	{
	  mode = app_data.stick[0];
	  jn = 0;
	  sticknum = app_data.sticknumber[0] - 1;
	}
      else
	{
	  mode = app_data.stick[1];
	  jn = 1;
	  sticknum = app_data.sticknumber[1] - 1;
	}
      switch (mode)
	{
	case 1:
	  data = keyjoy (jn);
	  break;
	case 2:
	  if (joy[sticknum].stick[0].axis[1].d1) data &= 0xFE; // up
	  if (joy[sticknum].stick[0].axis[0].d2) data &= 0xFD; // right
	  if (joy[sticknum].stick[0].axis[1].d2) data &= 0xFB; // down
	  if (joy[sticknum].stick[0].axis[0].d1) data &= 0xF7; // left
	  if (joy[sticknum].button[0].b || joy[jn].button[1].b) data &= 0xEF; // both buttons
	  break;
	}
      if (si == 1) if (dbstick1) data = dbstick1;
      else if (dbstick2) data = dbstick2;
      */
    }
  return data;
}
