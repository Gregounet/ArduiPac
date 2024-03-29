#ifndef ARDUIPAC_8048_H
#define ARDUIPAC_8048_H

#define ROM(addr) (rom[(addr) & 0xFFF])

extern uint8_t timer_on;
extern uint8_t count_on;
extern uint8_t p1;
extern uint8_t p2;
extern uint8_t xirq_pend;
extern uint8_t tirq_pend;

void init_8048 (void);
void exec_8048 (void);
void ext_irq (void);
void timer_irq (void);

#endif /*ARDUIPAC_ 8048_H */
