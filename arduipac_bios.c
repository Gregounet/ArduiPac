#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arduipac_bios.h"
#include "arduipac.h"

uint8_t acc;
uint8_t psw;
uint8_t ac;
uint8_t cy;
uint16_t pc;
uint16_t lastpc;

uint8_t itimer;
uint8_t reg_pnt;
uint8_t timer_on;
uint8_t count_on;
uint8_t sp;

uint8_t p1;
uint8_t p2;
uint8_t xirq_pend;
uint8_t tirq_pend;
uint8_t t_flag;

uint16_t A11;
uint16_t A11ff;
uint8_t bs;
uint8_t f0;
uint8_t f1;
uint8_t xirq_en;
uint8_t tirq_en;
uint8_t irq_ex;	

int master_count;
long clk;

int load_bios (const char *biosname)
{
  FILE *fn = NULL;

  fn = fopen (biosname, "rb");
  if (fn == NULL)
    {
      return O2EM_FAILURE;
    }
  fclose (fn);

  return O2EM_SUCCESS;
}
