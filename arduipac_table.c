#include "cpu.h"
#include "table.h"


struct lookup_tag lookup[] = {
  /* 00 */ {1, 0},
  /* 01 */ {1, 0},
  /* 02 */ {1, 0},
  /* 03 */ {2, 1},
  /* 04 */ {2, 2},
  /* 05 */ {1, 0},
  /* 06 */ {1, 0},
  /* 07 */ {1, 0},
  /* 08 */ {1, 0},
  /* 09 */ {1, 0},
  /* 0A */ {1, 0},
  /* 0B */ {1, 0},
  /* 0C */ {1, 0},
  /* 0D */ {1, 0},
  /* 0E */ {1, 0},
  /* 0F */ {1, 0},
  /* 10 */ {1, 0},
  /* 11 */ {1, 0},
  /* 12 */ {2, 3},
  /* 13 */ {2, 1},
  /* 14 */ {2, 2},
  /* 15 */ {1, 0},
  /* 16 */ {2, 3},
  /* 17 */ {1, 0},
  /* 18 */ {1, 0},
  /* 19 */ {1, 0},
  /* 1A */ {1, 0},
  /* 1B */ {1, 0},
  /* 1C */ {1, 0},
  /* 1D */ {1, 0},
  /* 1E */ {1, 0},
  /* 1F */ {1, 0},
  /* 20 */ {1, 0},
  /* 21 */ {1, 0},
  /* 22 */ {1, 0},
  /* 23 */ {2, 1},
  /* 24 */ {2, 2},
  /* 25 */ {1, 0},
  /* 26 */ {2, 3},
  /* 27 */ {1, 0},
  /* 28 */ {1, 0},
  /* 29 */ {1, 0},
  /* 2A */ {1, 0},
  /* 2B */ {1, 0},
  /* 2C */ {1, 0},
  /* 2D */ {1, 0},
  /* 2E */ {1, 0},
  /* 2F */ {1, 0},
  /* 30 */ {1, 0},
  /* 31 */ {1, 0},
  /* 32 */ {2, 3},
  /* 33 */ {1, 0},
  /* 34 */ {2, 2},
  /* 35 */ {1, 0},
  /* 36 */ {2, 3},
  /* 37 */ {1, 0},
  /* 38 */ {1, 0},
  /* 39 */ {1, 0},
  /* 3A */ {1, 0},
  /* 3B */ {1, 0},
  /* 3C */ {1, 0},
  /* 3D */ {1, 0},
  /* 3E */ {1, 0},
  /* 3F */ {1, 0},
  /* 40 */ {1, 0},
  /* 41 */ {1, 0},
  /* 42 */ {1, 0},
  /* 43 */ {2, 1},
  /* 44 */ {2, 2},
  /* 45 */ {1, 0},
  /* 46 */ {2, 3},
  /* 47 */ {1, 0},
  /* 48 */ {1, 0},
  /* 49 */ {1, 0},
  /* 4A */ {1, 0},
  /* 4B */ {1, 0},
  /* 4C */ {1, 0},
  /* 4D */ {1, 0},
  /* 4E */ {1, 0},
  /* 4F */ {1, 0},
  /* 50 */ {1, 0},
  /* 51 */ {1, 0},
  /* 52 */ {2, 3},
  /* 53 */ {2, 1},
  /* 54 */ {2, 2},
  /* 55 */ {1, 0},
  /* 56 */ {2, 3},
  /* 57 */ {1, 0},
  /* 58 */ {1, 0},
  /* 59 */ {1, 0},
  /* 5A */ {1, 0},
  /* 5B */ {1, 0},
  /* 5C */ {1, 0},
  /* 5D */ {1, 0},
  /* 5E */ {1, 0},
  /* 5F */ {1, 0},
  /* 60 */ {1, 0},
  /* 61 */ {1, 0},
  /* 62 */ {1, 0},
  /* 63 */ {1, 0},
  /* 64 */ {2, 2},
  /* 65 */ {1, 0},
  /* 66 */ {1, 0},
  /* 67 */ {1, 0},
  /* 68 */ {1, 0},
  /* 69 */ {1, 0},
  /* 6A */ {1, 0},
  /* 6B */ {1, 0},
  /* 6C */ {1, 0},
  /* 6D */ {1, 0},
  /* 6E */ {1, 0},
  /* 6F */ {1, 0},
  /* 70 */ {1, 0},
  /* 71 */ {1, 0},
  /* 72 */ {2, 3},
  /* 73 */ {1, 0},
  /* 74 */ {2, 2},
  /* 75 */ {1, 0},
  /* 76 */ {2, 3},
  /* 77 */ {1, 0},
  /* 78 */ {1, 0},
  /* 79 */ {1, 0},
  /* 7A */ {1, 0},
  /* 7B */ {1, 0},
  /* 7C */ {1, 0},
  /* 7D */ {1, 0},
  /* 7E */ {1, 0},
  /* 7F */ {1, 0},
  /* 80 */ {1, 0},
  /* 81 */ {1, 0},
  /* 82 */ {1, 0},
  /* 83 */ {1, 0},
  /* 84 */ {2, 2},
  /* 85 */ {1, 0},
  /* 86 */ {2, 3},
  /* 87 */ {1, 0},
  /* 88 */ {2, 1},
  /* 89 */ {2, 1},
  /* 8A */ {2, 1},
  /* 8B */ {1, 0},
  /* 8C */ {1, 0},
  /* 8D */ {1, 0},
  /* 8E */ {1, 0},
  /* 8F */ {1, 0},
  /* 90 */ {1, 0},
  /* 91 */ {1, 0},
  /* 92 */ {2, 3},
  /* 93 */ {1, 0},
  /* 94 */ {2, 2},
  /* 95 */ {1, 0},
  /* 96 */ {2, 3},
  /* 97 */ {1, 0},
  /* 98 */ {2, 1},
  /* 99 */ {2, 1},
  /* 9A */ {2, 1},
  /* 9B */ {1, 0},
  /* 9C */ {1, 0},
  /* 9D */ {1, 0},
  /* 9E */ {1, 0},
  /* 9F */ {1, 0},
  /* A0 */ {1, 0},
  /* A1 */ {1, 0},
  /* A2 */ {1, 0},
  /* A3 */ {1, 0},
  /* A4 */ {2, 2},
  /* A5 */ {1, 0},
  /* A6 */ {1, 0},
  /* A7 */ {1, 0},
  /* A8 */ {1, 0},
  /* A9 */ {1, 0},
  /* AA */ {1, 0},
  /* AB */ {1, 0},
  /* AC */ {1, 0},
  /* AD */ {1, 0},
  /* AE */ {1, 0},
  /* AF */ {1, 0},
  /* B0 */ {2, 1},
  /* B1 */ {2, 1},
  /* B2 */ {2, 3},
  /* B3 */ {1, 0},
  /* B4 */ {2, 2},
  /* B5 */ {1, 0},
  /* B6 */ {2, 3},
  /* B7 */ {1, 0},
  /* B8 */ {2, 1},
  /* B9 */ {2, 1},
  /* BA */ {2, 1},
  /* BB */ {2, 1},
  /* BC */ {2, 1},
  /* BD */ {2, 1},
  /* BE */ {2, 1},
  /* BF */ {2, 1},
  /* C0 */ {1, 0},
  /* C1 */ {1, 0},
  /* C2 */ {1, 0},
  /* C3 */ {1, 0},
  /* C4 */ {2, 2},
  /* C5 */ {1, 0},
  /* C6 */ {2, 3},
  /* C7 */ {1, 0},
  /* C8 */ {1, 0},
  /* C9 */ {1, 0},
  /* CA */ {1, 0},
  /* CB */ {1, 0},
  /* CC */ {1, 0},
  /* CD */ {1, 0},
  /* CE */ {1, 0},
  /* CF */ {1, 0},
  /* D0 */ {1, 0},
  /* D1 */ {1, 0},
  /* D2 */ {2, 3},
  /* D3 */ {2, 1},
  /* D4 */ {2, 2},
  /* D5 */ {1, 0},
  /* D6 */ {1, 0},
  /* D7 */ {1, 0},
  /* D8 */ {1, 0},
  /* D9 */ {1, 0},
  /* DA */ {1, 0},
  /* DB */ {1, 0},
  /* DC */ {1, 0},
  /* DD */ {1, 0},
  /* DE */ {1, 0},
  /* DF */ {1, 0},
  /* E0 */ {1, 0},
  /* E1 */ {1, 0},
  /* E2 */ {1, 0},
  /* E3 */ {1, 0},
  /* E4 */ {2, 2},
  /* E5 */ {1, 0},
  /* E6 */ {2, 3},
  /* E7 */ {1, 0},
  /* E8 */ {2, 3},
  /* E9 */ {2, 3},
  /* EA */ {2, 3},
  /* EB */ {2, 3},
  /* EC */ {2, 3},
  /* ED */ {2, 3},
  /* EE */ {2, 3},
  /* EF */ {2, 3},
  /* F0 */ {1, 0},
  /* F1 */ {1, 0},
  /* F2 */ {2, 3},
  /* F3 */ {1, 0},
  /* F4 */ {2, 2},
  /* F5 */ {1, 0},
  /* F6 */ {2, 3},
  /* F7 */ {1, 0},
  /* F8 */ {1, 0},
  /* F9 */ {1, 0},
  /* FA */ {1, 0},
  /* FB */ {1, 0},
  /* FC */ {1, 0},
  /* FD */ {1, 0},
  /* FE */ {1, 0},
  /* FF */ {1, 0}
};
