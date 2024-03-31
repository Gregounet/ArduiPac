#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_vmachine.h"
#include "c52_alien_invaders_usa_eu.h"
#include "mnemonics.h"

#define push(d) {intel8048_ram[sp++] = (d); if (sp > 23) sp = 8;}
#define pull() (sp--, (sp < 8)?(sp=23):0, intel8048_ram[sp])
#define make_psw() {psw = (cy << 7) | ac | f0 | bs | ((sp - 8) >> 1);}
#define illegal(i) {}
#define undef(i) {}

uint8_t intel8048_ram[64];

uint8_t psw;         // Program Status Word
uint8_t cy;          // Carry                                                                            (Values 0x00 / 0x01 : bit 7 of PSW)
uint8_t ac;          // Auxiliary Carry (for BCD operations)                                             (Values 0x00 / 0x40 : bit 6 of PSW)
uint8_t f0;          // Built-in Flag #1                                                                 (Values 0x00 / 0x20 : bit 5 of PSW)
uint8_t bs;          // Bank Select (select which of the two built-in Register Bank is currently in use) (Values 0x00 / 0x10 : bit 4 of PSW) 
uint8_t sp;          // Stack Pointer                                                                    (Values 8 - 23 : saved as 3 bits in PSW bits 0 - 2)
uint8_t reg_pnt;     // Register Pointer (Registers are part of the built-in RAM)
uint8_t f1;          // Built-in Flag #2
uint8_t p1;          // I/O Port #1
uint8_t p2;          // I/O Port #2
uint8_t xirq_pend;   // External Interrupt Pending
uint8_t tirq_pend;   // Timer Interrupt Pending
uint8_t itimer;      // Internal Timer Value
uint8_t timer_on;    // Timer is On
uint8_t count_on;    // Counter is On
uint8_t t_flag;      // Timer Flag
uint8_t xirq_en;     // External Interupt Enabled
uint8_t tirq_en;     // Timer Interupt Enabled
uint8_t irq_ex;	     // ISR executing
uint16_t pc;         // Program Counter (12 bits = 0x0FFF)
uint16_t a11;        // Address 11th bit (control which 2kB ROM bank is in use) (values 0x000 / 0x800)
uint16_t a11_backup; // Backup for Address 11th bit                             (values 0x000 / 0x800)
uint32_t clk;
uint32_t master_count;

void init_intel8048 ()
{
  pc         = 0x000;
  a11        = 0x000;
  a11_backup = 0x000;

  sp = 0x08;
  p1 = 0xFF;
  p2 = 0xFF;
  reg_pnt = 0x00;
  bs = 0x00;
  ac = 0x00;
  cy = 0x00;
  f0 = 0x00;
  f1 = 0x00;

  itimer = 0x00;
  timer_on = 0;
  count_on = 0;
  tirq_en = 0;
  tirq_pend = 0;
  xirq_en = 0;
  irq_ex =  0;
  xirq_pend  = 0;

  fprintf(stderr," Initializing intel8048_ram\n");
  for (uint8_t i = 0 ; i < 0x40 ; i++) intel8048_ram[i] = 0;
}

void ext_irq ()
{
  int_clk = 5;
  if (xirq_en && !irq_ex)
    {
      irq_ex = 1;
      xirq_pend = 0;
      make_psw ();
      push (pc & 0xFF);
      push (((pc & 0xF00) >> 8) | (psw & 0xF0));
      pc = 0x003;  // 0x003 = ISR vector
      a11_backup = a11;
      a11 = 0x000; // ISR are always located in ROM Bank 0
      clk = 2;
    }
  if (pendirq && (!xirq_en)) xirq_pend = 1;
}

void timer_irq ()
{
  if (tirq_en && !irq_ex)
    {
      irq_ex = 2;
      tirq_pend = 0;
      make_psw ();
      push (pc & 0xFF);
      push (((pc & 0xF00) >> 8) | (psw & 0xF0));
      pc = 0x07;   // 0x007 = Timer Interupt Service Routine Vector
      a11_backup = a11;
      a11 = 0x000; // ISR always located in ROM Bank 0
      clk = 2;
    }
  if (pendirq && (!tirq_en)) tirq_pend = 1;
}

void exec_8048 ()
{
  uint8_t acc;   // Accumulator
  uint8_t op;    // Op-code
  uint16_t addr; // Address
  uint8_t data;  // Data
  uint16_t temp; // Temporary value
  fprintf(stderr, "Entering exec_8048()\n");
  fprintf(stderr, "PC = 0x%03X\n",pc);

  for (;;)
    {
      clk = 1;
      // op = ROM (pc++); // TODO: il n'y a à ce niveau-la aucune protection contre le franchissement de l'adresse 2047 -> 2048 (ligne d'adresse a11)

      //fprintf(stderr, "0x%03X\t0x%02X\n",pc,op);
      //fprintf(stderr, "0x%03X\t0x%02X\t%s\n",pc,op,lookup[op].mnemonic);
      //fprintf(stderr, "ACC: 0x%02X\tCY: %d\tPC: 0x%03X\tOP: 0x%02X\t%s\n",acc,cy,pc,op,lookup[op].mnemonic);
      op = ROM (pc);
      fprintf(stderr, "BS: %d SP: 0x%02X REGPNT: 0x%02X CY: %d\tR0: 0x%02X R1: 0x%02X R2: 0x%02X R3: 0x%02X R4: 0x%02X R5: 0x%02X R6: 0x%02X R7 :0x%02X\t\tACC: 0x%02X\tPC: 0x%03X (%s)\tOP: 0x%02X\t%s",
		      (bs >>4), sp, reg_pnt, cy,
		      intel8048_ram[reg_pnt], intel8048_ram[reg_pnt+1], intel8048_ram[reg_pnt+2], intel8048_ram[reg_pnt+3],
		      intel8048_ram[reg_pnt+4], intel8048_ram[reg_pnt+5], intel8048_ram[reg_pnt+6], intel8048_ram[reg_pnt+7],
		      acc, pc, (pc < 0x400) ? "bios" : "cart", op, lookup[op].mnemonic);
      pc++;
      switch (op)
	{
	case 0x00:		/* NOP */
	  break;
	case 0x01:		/* ILL */
	case 0x06:		/* ILL */
	case 0x0B:		/* ILL */
	case 0x22:		/* ILL */
	case 0x33:		/* ILL */
	case 0x38:		/* ILL */
	case 0x3B:		/* ILL */
	case 0x63:		/* ILL */
	case 0x66:		/* ILL */
	case 0x73:		/* ILL */
	case 0x82:		/* ILL */
	case 0x87:		/* ILL */
	case 0x8B:		/* ILL */
	case 0x9B:		/* ILL */
	case 0xA2:		/* ILL */
	case 0xA6:		/* ILL */
	case 0xB7:		/* ILL */
	case 0xC0:		/* ILL */
	case 0xC1:		/* ILL */
	case 0xC2:		/* ILL */
	case 0xC3:		/* ILL */
	case 0xD6:		/* ILL */
	case 0xE0:		/* ILL */
	case 0xE1:		/* ILL */
	case 0xE2:		/* ILL */
	case 0xF3:		/* ILL */
	  illegal (op);
	  break;
	case 0x75:		/* EN CLK */
	  undef (op);
	  break;
	case 0x02:		/* OUTL BUS,A */
	case 0x88:		/* BUS,#data */
	case 0x98:		/* ANL BUS,#data */
	  undef (op);
	  clk = 2;
	  break;
	case 0x27:		/* CLR A */
	  acc = 0x00;
	  break;
	case 0x37:		/* CPL A */
	  acc = acc ^ 0xFF;
	  break;
	case 0x07:		/* DEC A */
	  acc--;
	  break;
	case 0x17:		/* INC A */
	  acc++;
	  break;
	case 0x03:		/* ADD A,#data */
	case 0x13:		/* ADDC A,#data */
	  data = ROM (pc++);
	  fprintf(stderr, " 0x%02X",data);
	  ac = 0x00;
	  switch(op) {
		  case 0x03:
	            if (((acc & 0x0F) + (data & 0x0F)) > 0x0F) ac = 0x40;
	            temp = acc + data;
		    break;
		  case 0x13:  
	            if (((acc & 0x0F) + (data & 0x0F) + cy) > 0x0F) ac = 0x40;
	            temp = acc + data + cy;
		    break;
	  }
	  cy = (temp > 0xFF) ? 0x01 : 0x00 ;
	  acc = (temp & 0xFF);
	  clk = 2;
	  break;
	case 0xA3:		/* MOVP A,@A */
	  fprintf(stderr, " (0x%02X)",ROM((pc & 0xF00) | acc));
	  acc = ROM ((pc & 0xF00) | acc);
	  clk = 2;
	  break;
	case 0x47:		/* SWAP A */
	  data = (acc & 0xF0) >> 4;
	  acc  <<= 4;
	  acc  |= data;
	  break;
	case 0x77:		/* RR A */
	  data = acc & 0x01;
	  acc >>= 1;
	  acc = (data) ? 0x80 : 0x7F ;
	  break;
	case 0xE7:		/* RL A */
	  data = acc & 0x80;
	  acc <<= 1;
	  if (data) acc |= 0x01;
	  else acc &= 0xFE;
	  break;
	case 0x04:		/* JMP */
	case 0x24:		/* JMP */
	case 0x44:		/* JMP */
	case 0x64:		/* JMP */
	case 0x84:		/* JMP */
	case 0xA4:		/* JMP */
	case 0xC4:		/* JMP */
	case 0xE4:		/* JMP */
	  pc = ROM (pc) | a11;
          pc |= ((uint16_t) (op & 0xE0)) << 3;
	  fprintf(stderr, " 0x%03X",pc);
	  clk = 2;
	  break;
	case 0x05:		/* EN I */
	  xirq_en = 1;
	  break;
	case 0x15:		/* DIS I */
	  xirq_en = 0;
	  break;
	case 0x08:		/* INS A,BUS */
	  acc = in_bus ();
	  clk = 2;
	  break;
	case 0x09:		/* IN A,Pp */
	case 0x0A:		/* IN A,Pp */
	  acc = (op == 0x09) ? p1 : read_p2() ;
	  clk = 2;
	  break;
	case 0x0C:		/* MOVD A,P4 */
	case 0x0D:		/* MOVD A,P5 */
	case 0x0E:		/* MOVD A,P6 */
	case 0x0F:		/* MOVD A,P7 */
	  // acc = read_pb (op - 0x0C);
	  clk = 2;
	  break;
	case 0x10:		/* INC @Ri */
	case 0x11:		/* INC @Ri */
	  intel8048_ram[intel8048_ram[reg_pnt + (op - 0x10)]]++;
	  break;
	case 0x12:		/* JBb address */
	case 0x32:		/* JBb address */
	case 0x52:		/* JBb address */
	case 0x72:		/* JBb address */
	case 0x92:		/* JBb address */
	case 0xB2:		/* JBb address */
	case 0xD2:		/* JBb address */
	case 0xF2:		/* JBb address */
	  data = ROM (pc);
	  fprintf(stderr, " (0x%02X)",data);
	  if (acc & (0x01 << ((op - 0x12) / 0x20))) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0x16:		/* JTF */
	  data = ROM (pc);
	  fprintf(stderr, " (0x%02X)",data);
	  if (t_flag) pc = (pc & 0xF00) | data;
	  else pc++;
	  t_flag = 0;
	  clk = 2;
	  break;
	case 0x18:		/* INC Rr */
	case 0x19:		/* INC Rr */
	case 0x1A:		/* INC Rr */
	case 0x1B:		/* INC Rr */
	case 0x1C:		/* INC Rr */
	case 0x1D:		/* INC Rr */
	case 0x1E:		/* INC Rr */
	case 0x1F:		/* INC Rr */
	  intel8048_ram[reg_pnt + (op - 0x18)]++;
	  break;
	case 0x20:		/* XCH A,@Ri */
	case 0x21:		/* XCH A,@Ri */
	  data                                              = acc;
	  acc                                               = intel8048_ram[intel8048_ram[reg_pnt + (op - 0x20)]];
	  intel8048_ram[intel8048_ram[reg_pnt + (op - 0x20)]] = data;
	case 0x23:		/* MOV A,#data */
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  acc = ROM (pc++);
	  clk = 2;
	  break;
	case 0x53:		/* ANL A,#data */
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  acc &= ROM (pc++);
	  clk = 2;
	  break;
	case 0x26:		/* JNT0 */
	  data = ROM (pc);
	  pc = (pc & 0xF00) | data;
	  clk = 2;
	  break;
	case 0x36:		/* JT0 */
	  pc++;
	  clk = 2;
	  break;
	case 0x28:		/* XCH A,Rr */
	case 0x29:		/* XCH A,Rr */
	case 0x2A:		/* XCH A,Rr */
	case 0x2B:		/* XCH A,Rr */
	case 0x2C:		/* XCH A,Rr */
	case 0x2D:		/* XCH A,Rr */
	case 0x2E:		/* XCH A,Rr */
	case 0x2F:		/* XCH A,Rr */
	  data                  = acc;
	  acc                   = intel8048_ram[reg_pnt + (op - 0x28)];
	  intel8048_ram[reg_pnt] = data;
	  break;
	case 0x30:		/* XCHD A,@Ri */
	case 0x31:		/* XCHD A,@Ri */
	  addr = intel8048_ram[reg_pnt + (op - 0x30)];
	  intel8048_ram[addr] = (intel8048_ram[addr] & 0xF0) | (acc                & 0x0F);
	  acc                = (acc                & 0xF0) | (intel8048_ram[addr] & 0x0F);
	  break;
	case 0x25:		/* EN TCNTI */
	  tirq_en = 1;
	  break;
	case 0x35:		/* DIS TCNTI */
	  tirq_en = 0;
	  tirq_pend = 0;
	  break;
	case 0x39:		/* OUTL P1,A */
	case 0x3A:		/* OUTL P2,A */
	  // fprintf(stderr, "OPCODE OUTL Pn, A\n");
	  if (op == 0x39) write_p1 (acc);
	  else p2 = acc;
	  clk = 2;
	  break;
	case 0x3C:		/* MOVD P4,A */
	case 0x3D:		/* MOVD P5,A */
	case 0x3E:		/* MOVD P6,A */
	case 0x3F:		/* MOVD P7,A */
	  // write_pb ((op - 0x3C), acc);
	  clk = 2;
	  break;
	case 0x40:		/* ORL A,@Ri */
	case 0x41:		/* ORL A,@Ri */
	  fprintf(stderr, " (0x%02X)",intel8048_ram[intel8048_ram[reg_pnt + (op - 0x40)]]);
	  acc |= intel8048_ram[intel8048_ram[reg_pnt + (op - 0x40)]];
	  break;
	case 0x43:		/* ORL A,#data */
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  acc |= ROM (pc++);
	  clk = 2;
	  break;
	case 0x45:		/* STRT CNT */
	  count_on = 1;
	  break;
	case 0x65:		/* STOP TCNT */
	  count_on = 0;
	  timer_on = 0;
	  break;
	case 0x42:		/* MOV A,T */
	  acc = itimer;
	  break;
	case 0x55:		/* STRT T */
	  timer_on = 1;
	  break;
	case 0x62:		/* MOV T,A */
	  itimer = acc;
	  break;
	case 0x46:		/* JNT1 */
	case 0x56:		/* JT1 */
	  data = ROM (pc);
	  switch(op){
            case 0x46:
              if (!read_t1 ()) pc = (pc & 0xF00) | data;
              else pc++;
	      break;
            case 0x56:
              if (read_t1 ()) pc = (pc & 0xF00) | data;
              else pc++;
	      break;
	  }
          clk = 2;
	  break;
	case 0x57:		/* DA A */
	  if (((acc & 0x0F) > 0x09) || ac)
	    {
	      if (acc > 0xF9) cy = 0x01;
	      acc += 6;
	    }
	  data = (acc & 0xF0) >> 4;
	  if ((data > 0x09) || cy)
	    {
	      data += 6;
	      cy = 0x01;
	    }
	  acc = (acc & 0x0F) | (data << 4);
	  break;
	case 0x50:		/* ANL A,@Ri */
	case 0x51:		/* ANL A,@Ri */
	  acc &= intel8048_ram[intel8048_ram[reg_pnt + (op - 0x50)]];
	  break;
	case 0x48:		/* ORL A,Rr */
	case 0x49:		/* ORL A,Rr */
	case 0x4A:		/* ORL A,Rr */
	case 0x4B:		/* ORL A,Rr */
	case 0x4C:		/* ORL A,Rr */
	case 0x4D:		/* ORL A,Rr */
	case 0x4E:		/* ORL A,Rr */
	case 0x4F:		/* ORL A,Rr */
	  acc |= intel8048_ram[reg_pnt + (op - 0x48)];
	  break;
	case 0x58:		/* ANL A,Rr */
	case 0x59:		/* ANL A,Rr */
	case 0x5A:		/* ANL A,Rr */
	case 0x5B:		/* ANL A,Rr */
	case 0x5C:		/* ANL A,Rr */
	case 0x5D:		/* ANL A,Rr */
	case 0x5E:		/* ANL A,Rr */
	case 0x5F:		/* ANL A,Rr */
	  acc &= intel8048_ram[reg_pnt + (op - 0x58)];
	  break;
	case 0xD8:		/* XRL A,Rr */
	case 0xD9:		/* XRL A,Rr */
	case 0xDA:		/* XRL A,Rr */
	case 0xDB:		/* XRL A,Rr */
	case 0xDC:		/* XRL A,Rr */
	case 0xDD:		/* XRL A,Rr */
	case 0xDE:		/* XRL A,Rr */
	case 0xDF:		/* XRL A,Rr */
	  acc ^= intel8048_ram[reg_pnt + (op - 0xD8)];
	  break;
	case 0xE3:		/* MOVP3 A,@A */
          addr = 0x300 | acc;
	  acc = ROM (addr);
	  clk = 2;
	  break;
	case 0x60:		/* ADD A,@Ri */
	case 0x61:		/* ADD A,@Ri */
	  cy = 0x00;
	  ac = 0x00;
	  data = intel8048_ram[intel8048_ram[reg_pnt] + (op - 0x60)];
	  if (((acc & 0x0F) + (data & 0x0F)) > 0x0F) ac = 0x40;
	  temp = acc + data;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  break;
	case 0x67:		/* RRC A */
	  data = cy;
	  cy   = acc & 0x01;
	  acc  >>= 1;
	  if (data) acc |= 0x80;
	  else acc &= 0x7F;
	  break;
	case 0xF7:		/* RLC A */
	  data = cy;
	  cy   = (acc & 0x80) >> 7;
	  acc  <<= 1;
	  if (data) acc |= 0x01;
	  else acc &= 0xFE;
	  break;
	case 0x68:		/* ADD A,Rr */
	case 0x69:		/* ADD A,Rr */
	case 0x6A:		/* ADD A,Rr */
	case 0x6B:		/* ADD A,Rr */
	case 0x6C:		/* ADD A,Rr */
	case 0x6D:		/* ADD A,Rr */
	case 0x6E:		/* ADD A,Rr */
	case 0x6F:		/* ADD A,Rr */
	  cy = 0x00;
	  ac = 0x00;
	  data = intel8048_ram[reg_pnt + (op - 0x68)];
	  if (((acc & 0x0F) + (data & 0x0F)) > 0x0F) ac = 0x40;
	  temp = acc + data;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  break;
	case 0x70:		/* ADDC A,@Ri */
	case 0x71:		/* ADDC A,@Ri */
	  ac = 0x00;
	  data = intel8048_ram[intel8048_ram[reg_pnt + (op - 0x70)]];
	  if (((acc & 0x0F) + (data & 0x0F) + cy) > 0x0F) ac = 0x40;
	  temp = acc + data + cy;
	  cy = 0x00;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  break;
	case 0x78:		/* ADDC A,Rr */
	case 0x79:		/* ADDC A,Rr */
	case 0x7A:		/* ADDC A,Rr */
	case 0x7B:		/* ADDC A,Rr */
	case 0x7C:		/* ADDC A,Rr */
	case 0x7D:		/* ADDC A,Rr */
	case 0x7E:		/* ADDC A,Rr */
	case 0x7F:		/* ADDC A,Rr */
	  ac = 0x00;
	  data = intel8048_ram[reg_pnt + (op - 0x78)];
	  if (((acc & 0x0F) + (data & 0x0F) + cy) > 0x0F) ac = 0x40;
	  temp = acc + data + cy;
	  cy = 0x00;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  break;
	case 0x80:		/* MOVX A,@Ri */
	case 0x81:		/* MOVX A,@Ri */
	  acc = ext_read (intel8048_ram[reg_pnt + (op - 0x80)]);
	  clk = 2;
	  break;
	case 0x86:		/* JNI address */
	  data = ROM (pc);
	  if (int_clk > 0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0x89:		/* ORL Pp,#data */
	case 0x8A:		/* ORL Pp,#data */
	  // fprintf(stderr, "OPCODE ORL Pn, A\n");
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  if (op == 0x89) write_p1 (p1 | ROM (pc++));
	  else p2 |= ROM (pc++);
	  clk = 2;
	  break;
	case 0x8C:		/* ORLD P4,A */
	case 0x8D:		/* ORLD P5,A */
	case 0x8E:		/* ORLD P6,A */
	case 0x8F:		/* ORLD P7,A */
	  clk = 2;
	  break;
	case 0x90:		/* MOVX @Ri,A */
	case 0x91:		/* MOVX @Ri,A */
	  if (op == 0x90) fprintf(stderr, "\nOPCODE MOVX @R0, A\n");
	  else fprintf(stderr, "\nOPCODE MOVX @R1, A\n");
	  ext_write (acc, intel8048_ram[reg_pnt + (op - 0x90)]);
	  clk = 2;
	  break;
	case 0x83:		/* RET */
	  pc = ((pull () & 0x0F) << 8);
	  pc |= pull ();
	  clk = 2;
	  break;
	case 0x93:		/* RETR */
	  data = pull ();
	  pc = (data & 0x0F) << 8;
	  cy = (data & 0x80) >> 7;
	  ac = data & 0x40;
	  f0 = data & 0x20;
	  bs = data & 0x10;
	  reg_pnt = (bs) ? 0x18 : 0x00;
	  pc |= pull ();
	  irq_ex = 0;
	  a11 = a11_backup;
	  clk = 2;
	  break;
	case 0x97:		/* CLR C */
	  cy = 0x00;
	  break;
	case 0x99:		/* ANL Pp,#data */
	case 0x9A:		/* ANL Pp,#data */
	  // fprintf(stderr, "OPCODE ANL Pn, A\n");
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  if (op == 0x99) write_p1 (p1 & ROM (pc++));
	  else p2 &= ROM (pc++);
	  clk = 2;
	  break;
	case 0x9C:		/* ANLD P4,A */
	case 0x9D:		/* ANLD P5,A */
	case 0x9E:		/* ANLD P6,A */
	case 0x9F:		/* ANLD P7,A */
	  clk = 2;
	  break;
	case 0xA0:		/* MOV @Ri,A */
	case 0xA1:		/* MOV @Ri,A */
	  intel8048_ram[intel8048_ram[reg_pnt + (op - 0xA0)]] = acc;
	  break;
	case 0x85:		/* CLR F0 */
	  f0 = 0x00;
	  break;
	case 0xB6:		/* JF0 address */
	  data = ROM (pc);
	  if (f0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0x95:		/* CPL F0 */
	  f0 ^= 0x20;
	  break;
	case 0xA5:		/* CLR F1 */
	  f1 = 0x00;
	  break;
	case 0x76:		/* JF1 address */
	  data = ROM (pc);
	  if (f1) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0xB5:		/* CPL F1 */
	  f1 ^= 0x01;
	  break;
	case 0xA7:		/* CPL C */
	  cy ^= 0x01;
	  break;
	case 0xA8:		/* MOV Rr,A */
	case 0xA9:		/* MOV Rr,A */
	case 0xAA:		/* MOV Rr,A */
	case 0xAB:		/* MOV Rr,A */
	case 0xAC:		/* MOV Rr,A */
	case 0xAD:		/* MOV Rr,A */
	case 0xAE:		/* MOV Rr,A */
	case 0xAF:		/* MOV Rr,A */
	  intel8048_ram[reg_pnt + (op - 0xA8)] = acc;
	  break;
	case 0xB0:		/* MOV @Ri,#data */
	case 0xB1:		/* MOV @Ri,#data */
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  intel8048_ram[intel8048_ram[reg_pnt + (op - 0xB1)]] = ROM (pc++);
	  clk = 2;
	  break;
	case 0xB3:		/* JMPP @A */
	  addr = (pc & 0xF00) | acc;
	  pc = (pc & 0xF00) | ROM (addr);
	  clk = 2;
	  break;
	case 0xB8:		/* MOV Rr,#data */
	case 0xB9:		/* MOV Rr,#data */
	case 0xBA:		/* MOV Rr,#data */
	case 0xBB:		/* MOV Rr,#data */
	case 0xBC:		/* MOV Rr,#data */
	case 0xBD:		/* MOV Rr,#data */
	case 0xBE:		/* MOV Rr,#data */
	case 0xBF:		/* MOV Rr,#data */
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  intel8048_ram[reg_pnt + (op - 0xB8)] = ROM (pc++);
	  clk = 2;
	  break;
	case 0xC5:		/* SEL RB0 */
	case 0xD5:		/* SEL RB1 */
	  bs = op & 0x10;
	  reg_pnt = (bs == 00) ? 0x00 : 0x18;
	  break;
	case 0xC6:		/* JZ address */
	  data = ROM (pc);
	  if (acc == 0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0x96:		/* JNZ address */
	  data = ROM (pc);
	  fprintf(stderr, " 0x%02X",data);
	  if (acc != 0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0xC7:		/* MOV A,PSW */
	  make_psw ();
	  acc = psw;
	  break;
	case 0xD7:		/* MOV PSW,A */
	  psw = acc;
	  cy = (psw & 0x80) >> 7;
	  ac = psw & 0x40;
	  f0 = psw & 0x20;
	  bs = psw & 0x10;
	  reg_pnt = (bs) ? 0x18 : 0x00;
	  sp = (psw & 0x07) << 1;
	  sp += 0x08;
	  break;
	case 0xC8:		/* DEC Rr */
	case 0xC9:		/* DEC Rr */
	case 0xCA:		/* DEC Rr */
	case 0xCB:		/* DEC Rr */
	case 0xCC:		/* DEC Rr */
	case 0xCD:		/* DEC Rr */
	case 0xCE:		/* DEC Rr */
	case 0xCF:		/* DEC Rr */
	  intel8048_ram[reg_pnt + (op - 0xC8)]--;
	  break;
	case 0xD0:		/* XRL A,@Ri */
	case 0xD1:		/* XRL A,@Ri */
	  acc ^= intel8048_ram[intel8048_ram[reg_pnt + (op - 0xD0)]];
	  break;
	case 0xD3:		/* XRL A,#data */
	  fprintf(stderr, " 0x%02X",ROM(pc));
	  acc ^= ROM (pc++);
	  clk = 2;
	  break;
	case 0xE5:		/* SEL MB0 */ 
	  a11        = 0x000;
	  a11_backup = 0x000;
	  break;
	case 0xF5:		/* SEL MB1 */
          if (irq_ex) a11_backup = 0x800;
	  else
	  {
		  a11        = 0x800;
		  a11_backup = 0x800;
	  }
	  break;
	case 0xF6:		/* JC address */
	  data = ROM (pc);
	  fprintf(stderr, " (0x%02X)",data);
	  if (cy) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0xE6:		/* JNC address */
	  data = ROM (pc);
	  fprintf(stderr, " (0x%02X)",data);
	  if (!cy) pc = (pc & 0xF00) | data;
	  else pc++;
	  break;
	  clk = 2;
	case 0xE8:		/* DJNZ Rr,address */
	case 0xE9:		/* DJNZ Rr,address */
	case 0xEA:		/* DJNZ Rr,address */
	case 0xEB:		/* DJNZ Rr,address */
	case 0xEC:		/* DJNZ Rr,address */
	case 0xED:		/* DJNZ Rr,address */
	case 0xEE:		/* DJNZ Rr,address */
	case 0xEF:		/* DJNZ Rr,address */
	  intel8048_ram[reg_pnt + (op - 0xE8)]--;
	  data = ROM (pc);
	  fprintf(stderr, " 0x%02X",data);
	  if (intel8048_ram[reg_pnt + (op - 0xE8)] != 0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk = 2;
	  break;
	case 0xF0:		/* MOV A,@Ri */
	case 0xF1:		/* MOV A,@Ri */
	  acc = intel8048_ram[intel8048_ram[reg_pnt + (op - 0xF0)]];
	  break;
	case 0x14:		/* CALL */
	case 0x34:		/* CALL */
	case 0x54:		/* CALL */
	case 0x74:		/* CALL */
	case 0x94:		/* CALL */
	case 0xB4:		/* CALL */
	case 0xD4:		/* CALL */
	case 0xF4:		/* CALL */
	  make_psw ();
	  pc++;
	  push (pc & 0xFF);
	  push (((pc & 0xF00) >> 8) | (psw & 0xF0));
	  pc = a11 | ((uint16_t)(op & 0xE0)) << 3 | ROM(pc-1) ;
	  fprintf(stderr, " 0x%03X", pc);
	  clk = 2;
	  break;
	case 0xF8:		/* MOV A,Rr */
	case 0xF9:		/* MOV A,Rr */
	case 0xFA:		/* MOV A,Rr */
	case 0xFB:		/* MOV A,Rr */
	case 0xFC:		/* MOV A,Rr */
	case 0xFD:		/* MOV A,Rr */
	case 0xFE:		/* MOV A,Rr */
	case 0xFF:		/* MOV A,Rr */
	  acc = intel8048_ram[reg_pnt + (op - 0xF8)];
	  break;
	}
      fprintf(stderr,"\n");
      master_clk += clk;
      horizontal_clock += clk;
      
      if (int_clk > clk) int_clk -= clk;
      else int_clk = 0;

      if (xirq_pend) ext_irq ();
      if (tirq_pend) timer_irq ();

      if (horizontal_clock > LINECNT - 1) {
	  horizontal_clock -= LINECNT;
	  if (enahirq && (intel8245_ram[0xA0] & 0x01)) ext_irq ();
	  if (count_on && mstate == 0) {
	      itimer++; // TODO pourquoi incrémenter itimer ici ?
	      if (itimer == 0x00) {
		  t_flag = 1;
		  timer_irq ();
		  //draw_region ();
		}
	    }
	}
      draw_display();

      if (timer_on) {
	  master_count += clk;
	  if (master_count > 31) { // TODO WTf ce 31 ? Ca divise la clock par 32...
	      master_count -= 31;
	      itimer++;
	      if (itimer == 0x00) {
		  t_flag = 1;
		  timer_irq ();
		}
	    }
	}

      if ((mstate == 0) && (master_clk > VBLCLK))  handle_vbl ();
      if ((mstate == 1) && (master_clk > evblclk)) handle_evbl ();

    }
}
