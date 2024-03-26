#ifndef ARDUIPAC_VMACHINE_H
#define ARDUIPAC_VMACHINE_H

#include <stdint.h>

#include "arduipac_sdl.h"

#define LINECNT 21
#define MAXLINES 500
#define MAXSNAP 50

#define VBLCLK 5493
#define EVBLCLK_PAL 7259

extern uint8_t dbstick1, dbstick2;
extern int last_line;

extern int evblclk;

extern int master_clk;
extern int int_clk;
extern int h_clk;
extern uint8_t coltab[256];
extern int mstate;

extern uint8_t external_ram[];
extern uint8_t external_rom[];
extern uint8_t VDCwrite[256];
extern uint8_t ColorVector[MAXLINES];
extern uint8_t *rom;

extern int frame;
extern unsigned long clk_counter;

extern int enahirq;
extern int pendirq;
extern long regionoff;
extern int sproff;
extern int tweakedaudio;

uint8_t read_P2 ();
int snapline (int pos, uint8_t reg, int t);
void ext_write (uint8_t dat, uint16_t adr);
uint8_t ext_read (uint16_t adr);
void handle_vbl ();
void handle_evbl ();
void handle_evbll ();
uint8_t in_bus ();
void write_p1 (uint8_t d);
uint8_t read_t1 ();
void init_system ();
// void init_roms ();
void run ();

extern struct resource
{
  int bank;
  int stick[2];
  int sticknumber[2];
  int limit;
  int speed;
  int wsize;
  int scanlines;
  int filter;
  int openb;
  int bios;
} app_data;

#endif /* ARDUIPAC_VMACHINE_H */
