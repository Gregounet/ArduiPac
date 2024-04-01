#ifndef ARDUIPAC_8048_H
#define ARDUIPAC_8048_H

#define ROM(addr) (rom[(addr) & 0xFFF])

extern uint8_t p1;
extern uint8_t p2;

void ext_irq (void);
void init_intel8048 (void);
void exec_8048 (void);

#endif /*ARDUIPAC_8048_H */
