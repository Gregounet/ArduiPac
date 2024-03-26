#ifndef VMACHINE_H
#define VMACHINE_H

#include <stdint.h>

#include "types.h"
#include "o2em_sdl.h"

#define LINECNT 21
#define MAXLINES 500
#define MAXSNAP 50

#define VBLCLK 5493
#define EVBLCLK_NTSC 5964
#define EVBLCLK_PAL 7259

#define FPS_NTSC 60
#define FPS_PAL 50
#define WANT_FPS_PAL 1
#define WANT_FPS_NTSC 0

extern Byte dbstick1, dbstick2;
extern int last_line;

extern int evblclk;

extern int master_clk;
extern int int_clk;
extern int h_clk;
extern Byte coltab[256];
extern int mstate;

extern Byte rom_table[8][4096];
extern Byte intRAM[];
extern Byte extRAM[];
extern Byte extROM[];
extern Byte VDCwrite[256];
extern Byte ColorVector[MAXLINES];
extern Byte *rom;

extern int frame;
int key2[KEY_MAX];
extern int key2vcnt;
extern unsigned long clk_counter;

int joykeystab[KEY_MAX];

extern int enahirq;
extern int pendirq;
extern int useforen;
extern long regionoff;
extern int sproff;
extern int tweakedaudio;

Byte read_P2 ();
int snapline (int pos, Byte reg, int t);
void ext_write (Byte dat, ADDRESS adr);
Byte ext_read (ADDRESS adr);
void handle_vbl ();
void handle_evbl ();
void handle_evbll ();
Byte in_bus ();
void write_p1 (Byte d);
Byte read_t1 ();
void init_system ();
void init_roms ();
void run ();
void o2em_clean_quit ();

extern struct resource
{
  int bank;
  int stick[2];
  int sticknumber[2];
  int limit;
  int speed;
  int wsize;
  int fullscreen;
  int scanlines;
  int exrom;
  int three_k;
  int filter;
  int euro;
  int openb;
  int bios;
  uint32_t crc;
  char *window_title;
  char *scshot;
  char romdir[MAXC];
  char biosdir[MAXC];
  char bios_filename[MAXC];
  int need_bios;
  int force_bios;
  int show_fps;
} app_data;

#endif /* VMACHINE_H */
