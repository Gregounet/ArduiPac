#include <stdint.h>
#include <allegro.h>

#include "arduipac_keys.h"

struct keyb keytab[] = {
    {KEY_A, "A"},
  {KEY_B, "B"},
  {KEY_C, "C"},
  {KEY_D, "D"},
  {KEY_E, "E"},
  {KEY_F, "F"},
  {KEY_G, "G"},
  {KEY_H, "H"},
  {KEY_I, "I"},
  {KEY_J, "J"},
  {KEY_K, "K"},
  {KEY_L, "L"},
  {KEY_M, "M"},
  {KEY_N, "N"},
  {KEY_O, "O"},
  {KEY_P, "P"},
  {KEY_Q, "Q"},
  {KEY_R, "R"},
  {KEY_S, "S"},
  {KEY_T, "T"},
  {KEY_U, "U"},
  {KEY_V, "V"},
  {KEY_W, "W"},
  {KEY_X, "X"},
  {KEY_Y, "Y"},
  {KEY_Z, "Z"},
  {KEY_0, "0"},
  {KEY_1, "1"},
  {KEY_2, "2"},
  {KEY_3, "3"},
  {KEY_4, "4"},
  {KEY_5, "5"},
  {KEY_6, "6"},
  {KEY_7, "7"},
  {KEY_8, "8"},
  {KEY_9, "9"},
  {KEY_SPACE, "SPACE"},
  {KEY_SLASH, "SLASH"},
  {KEY_PLUS_PAD, "PLUS"},
  {KEY_STOP, "STOP"},
  {KEY_MINUS, "MINUS"},
  {KEY_ASTERISK, "ASTERISK"},
  {KEY_SLASH_PAD, "SLASH_PAD"},
  {KEY_EQUALS, "EQUALS"},
  {KEY_DEL, "DEL"},
  {KEY_ENTER, "ENTER"} 
};

uint8_t key_map[6][8] = {
    {KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7},
  {KEY_8, KEY_9, 0, 0, KEY_SPACE, KEY_SLASH, KEY_L, KEY_P},
  {KEY_PLUS_PAD, KEY_W, KEY_E, KEY_R, KEY_T, KEY_U, KEY_I, KEY_O},
  {KEY_Q, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K},
  {KEY_A, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_M, KEY_STOP},
  {KEY_MINUS, KEY_ASTERISK, KEY_SLASH_PAD, KEY_EQUALS, KEY_Y, KEY_N, KEY_DEL,
   KEY_ENTER}
};

