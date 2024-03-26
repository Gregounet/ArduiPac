#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "cpu.h"
#include "o2em2.h"
#include "vmachine.h"
#include "vdc.h"
#include "keyboard.h"
#include "o2em_sdl.h"

int NeedsPoll = 0;

Byte keycode;
Byte last_key;
Byte new_int = 0;

Byte key_done = 0;

int joykeys[2][5] = { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0} };

int joykeystab[KEY_MAX];
int syskeys[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

void set_defjoykeys (int jn, int sc)
{
  if (sc)
    set_joykeys (jn, KEY_W, KEY_S, KEY_A, KEY_D, KEY_SPACE);
  else
    set_joykeys (jn, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_L);
}

void set_defsystemkeys ()
{
  set_systemkeys (KEY_F12, KEY_F1, KEY_F4, KEY_F5, KEY_F8, KEY_F2, KEY_F3,
		  KEY_F6);
}

void set_joykeys (int jn, int up, int down, int left, int right, int fire)
{
  int i, j;
  if ((jn < 0) || (jn > 1))
    {
      return;
    }
  joykeys[jn][0] = up;
  joykeys[jn][1] = down;
  joykeys[jn][2] = left;
  joykeys[jn][3] = right;
  joykeys[jn][4] = fire;

  for (i = 0; i < KEY_MAX; i++)
    joykeystab[i] = 0;

  for (j = 0; j < 2; j++)
    for (i = 0; i < 5; i++)
      {
	joykeystab[joykeys[j][i]] = 1;
      }
}

void set_systemkeys (int k_quit, int k_pause, int k_debug, int k_reset,
		int k_screencap, int k_save, int k_load, int k_inject)
{
  syskeys[0] = k_quit;
  syskeys[1] = k_pause;
  syskeys[3] = k_reset;
  syskeys[4] = k_screencap;
  syskeys[7] = k_inject;
}

void
handle_key ()
{
  static int scshot_counter = 0;
  char *p;
  char name[1024];
  int stateError;
  if (NeedsPoll)
    poll_keyboard ();

  if (key[syskeys[0]] || key[KEY_ESC])
    {
      do
	{
	  rest (5);
	  if (NeedsPoll)
	    poll_keyboard ();
	}
      while (key[syskeys[0]] || key[KEY_ESC]);
      key_done = 1;
    }

  if (key[syskeys[1]])
    {
      do
	{
	  rest (5);
	  if (NeedsPoll)
	    poll_keyboard ();
	}
      while (key[syskeys[1]]);

      do
	{
	  rest (5);
	  if (NeedsPoll)
	    poll_keyboard ();

	  if (key[KEY_ALT] && key[KEY_ENTER])
	    {
	      app_data.fullscreen = app_data.fullscreen ? 0 : 1;
	      grmode ();
	      abaut ();
	      do
		{
		  rest (5);
		  if (NeedsPoll)
		    poll_keyboard ();
		}
	      while (key[KEY_ENTER]);
	    }

	}
      while ((!key[syskeys[1]]) && (!key[KEY_ESC]) && (!key[syskeys[0]]));
      do
	{
	  rest (5);
	  if (NeedsPoll)
	    poll_keyboard ();
	}
      while (key[syskeys[1]]);

    }

  if (key[syskeys[3]])
    {
      init_cpu ();
      init_roms ();
      clearscr ();
      do
	{
	  rest (5);
	  if (NeedsPoll)
	    poll_keyboard ();
	}
      while (key[syskeys[3]]);
    }

  if (key[KEY_ALT] && key[KEY_ENTER])
    {
      app_data.fullscreen = app_data.fullscreen ? 0 : 1;
      grmode ();
      do
	{
	  rest (5);
	  if (NeedsPoll)
	    poll_keyboard ();
	}
      while (key[KEY_ENTER]);
    }

}

Byte keyjoy (int jn)
{
  Byte d;
  d = 0xFF;
  if ((jn >= 0) && (jn <= 1))
    {
      if (NeedsPoll)
	poll_keyboard ();
      if (key[joykeys[jn][0]])
	d &= 0xFE;
      if (key[joykeys[jn][1]])
	d &= 0xFB;
      if (key[joykeys[jn][2]])
	d &= 0xF7;
      if (key[joykeys[jn][3]])
	d &= 0xFD;
      if (key[joykeys[jn][4]])
	d &= 0xEF;
    }
  return d;
}

int o2em_init_keyboard ()
{
  int ret;
  key_done = 0;
  ret = install_keyboard ();
  if (ret != 0)
    {
      return O2EM_FAILURE;
    }
  new_int = 1;
  NeedsPoll = keyboard_needs_poll ();
  return O2EM_SUCCESS;
}

void Set_Old_Int9 ()
{
  remove_keyboard ();
  new_int = 0;
}
