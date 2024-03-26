#ifndef __KEYBOARD_H
#define __KEYBOARD_H

void Set_Old_Int9 ();
int o2em_init_keyboard ();
void handle_key ();
void set_joykeys (int joy, int up, int down, int left, int right, int fire);
void set_systemkeys (int k_quit, int k_reset, int k_screencap,int k_inject);
void set_defjoykeys (int joy, int sc);
void set_defsystemkeys ();
Byte keyjoy (int jn);

extern Byte new_int;
extern int NeedsPoll;
extern Byte key_done;
extern int joykeys[2][5];
extern int syskeys[8];

struct keyb
{
  int keybcode;
  char *keybname;
};

extern struct keyb keybtab[];

#endif
