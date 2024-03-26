#ifndef ARDUIPAC_VMACHINE_H
#define ARDUIPAC_VMACHINE_H

#include <stdint.h>

#include "arduipac_sdl.h"

#define LINECNT 21
#define MAXLINES 500

#define VBLCLK 5493
#define EVBLCLK_PAL 7259

extern int evblclk;

extern int master_clk;
extern int int_clk;
extern int h_clk;
extern uint8_t collision_table[256];
extern int mstate;

extern uint8_t external_ram[];
extern uint8_t external_rom[];
extern uint8_t VDCwrite[256];
extern uint8_t ColorVector[MAXLINES];

extern int frame;
extern unsigned long clk_counter;

extern int enahirq;
extern int pendirq;
extern long regionoff;
extern int sproff;

uint8_t read_P2 ();
void ext_write (uint8_t dat, uint16_t adr);
uint8_t ext_read (uint16_t adr);
void handle_vbl ();
void handle_evbl ();
void handle_evbll ();
uint8_t in_bus ();
void write_p1 (uint8_t d);
uint8_t read_t1 ();
void init_system ();
void run ();

#endif /* ARDUIPAC_VMACHINE_H */
