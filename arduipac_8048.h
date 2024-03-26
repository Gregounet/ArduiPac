#ifndef ARDUIPAC_CPU_H
#define ARDUIPAC_CPU_H

extern uint8_t acc;
extern uint8_t psw;
extern uint16_t pc;
extern long clk;

extern uint8_t itimer;
extern uint8_t reg_pnt;
extern uint8_t timer_on;
extern uint8_t count_on;

extern uint8_t t_flag;

extern uint8_t sp;

extern uint8_t p1;
extern uint8_t p2;

extern uint8_t xirq_pend;
extern uint8_t tirq_pend;

void init_8048 (void);
void exec_8048 (void);

void ext_irq (void);
void timer_irq (void);

#endif /*ARDUIPAC_ CPU_H */
