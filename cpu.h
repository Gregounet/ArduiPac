#ifndef CPU_H
#define CPU_H

#include "types.h"

extern Byte acc;
extern ADDRESS pc;
extern long clk;

extern Byte itimer;
extern Byte reg_pnt;
extern Byte timer_on;
extern Byte count_on;

extern Byte t_flag;

extern Byte psw;
extern Byte sp;

extern Byte p1;
extern Byte p2;

extern Byte xirq_pend;
extern Byte tirq_pend;

void init_cpu (void);
void cpu_exec (void);

void ext_IRQ (void);
void tim_IRQ (void);

Byte acc;
ADDRESS pc;
long clk;

Byte itimer;
Byte reg_pnt;
Byte timer_on;
Byte count_on;
Byte psw;
Byte sp;

Byte p1;
Byte p2;
Byte xirq_pend;
Byte tirq_pend;
Byte t_flag;

ADDRESS lastpc;
ADDRESS A11;
ADDRESS A11ff;
Byte bs;
Byte f0;
Byte f1;
Byte ac;
Byte cy;
Byte xirq_en;
Byte tirq_en;
Byte irq_ex;	

int master_count;

#endif /* CPU_H */
