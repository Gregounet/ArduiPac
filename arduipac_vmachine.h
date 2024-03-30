#ifndef ARDUIPAC_VMACHINE_H
#define ARDUIPAC_VMACHINE_H

#include <stdint.h>

#include "arduipac_graphics.h"

#define LINECNT 21
#define MAXLINES 500

#define VBLCLK 5493
#define EVBLCLK_PAL 7259

extern uint32_t evblclk;
extern uint32_t master_clk;
extern uint32_t int_clk;
extern uint32_t horizontal_clock;
extern uint8_t collision_table[256];
extern uint8_t mstate;
extern uint8_t intel8245_ram[];
extern uint8_t enahirq;
extern uint8_t pendirq;

uint8_t read_p2 ();
void ext_write (uint8_t dat, uint8_t addr);
uint8_t ext_read (uint8_t addr);
void handle_vbl ();
void handle_evbl ();
void handle_evbll ();
uint8_t in_bus ();
void write_p1 (uint8_t d);
uint8_t read_t1 ();
void init_vmachine ();
void run ();

#endif /* ARDUIPAC_VMACHINE_H */
