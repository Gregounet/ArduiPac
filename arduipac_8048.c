#include <stdint.h>

#include "arduipac_8048.h"
#include "arduipac_8245.h"
#include "arduipac_vmachine.h"
#include "c52_alien_invaders_usa_eu.h"

#define push(d) {internal_ram[sp++] = (d); if (sp > 23) sp = 8;}
#define pull() (sp--, (sp < 8)?(sp=23):0, internal_ram[sp])
#define make_psw() {psw = (cy << 7) | ac | f0 | bs | 0x08; psw = psw | ((sp - 8) >> 1);}
#define illegal(i) {}
#define undef(i) {}

uint8_t internal_ram[64];

uint8_t sp;
uint8_t psw;

uint8_t ac;
uint8_t cy;

uint8_t reg_pnt;

uint8_t itimer;
uint8_t timer_on;
uint8_t count_on;

uint8_t t_flag;

uint8_t p1;
uint8_t p2;

uint8_t xirq_pend;
uint8_t tirq_pend;

uint8_t bs;

uint8_t f0;
uint8_t f1;

uint8_t xirq_en;
uint8_t tirq_en;
uint8_t irq_ex;	

uint16_t pc;
uint16_t a11;
uint16_t a11_backup;

uint32_t clk;
uint32_t master_count;

void init_8048 ()
{
  pc = 0x000;
  a11 = 0x000;
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

  timer_on = 0;
  count_on = 0;

  tirq_en = 0;
  tirq_pend = 0;

  xirq_en = 0;
  irq_ex =  0;
  xirq_pend  = 0;
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
      pc = 0x003;
      a11_backup = a11;
      a11 = 0x000;
      clk += 2;
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
      pc = 0x07;
      a11_backup = a11;
      a11 = 0x000;
      clk += 2;
    }
  if (pendirq && (!tirq_en)) tirq_pend = 1;
}

void exec_8048 ()
{
  uint8_t acc;

  uint8_t op;
  uint16_t addr;
  uint8_t data;
  uint16_t temp;

  for (;;)
    {
      clk = 0;

      op = ROM (pc++);

      switch (op)
	{
	case 0x00:		/* NOP */
	  clk++;
	  break;
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
	case 0x01:		/* ILL */
	case 0x06:		/* ILL */
	  illegal (op);
	  clk++;
	  break;
	case 0x75:		/* EN CLK */
	  undef (op);
	  clk++;
	  break;
	case 0x02:		/* OUTL BUS,A */
	case 0x88:		/* BUS,#data */
	case 0x98:		/* ANL BUS,#data */
	  undef (op);
	  clk += 2;
	  break;
	case 0x27:		/* CLR A */
	  acc = 0x00;
	  clk++;
	  break;
	case 0x37:		/* CPL A */
	  acc = acc ^ 0xFF;
	  clk++;
	  break;
	case 0x07:		/* DEC A */
	  acc--;
	  clk++;
	  break;
	case 0x17:		/* INC A */
	  acc++;
	  clk++;
	  break;
	case 0x03:		/* ADD A,#data */
	case 0x13:		/* ADDC A,#data */
	  data = ROM (pc++);
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
	  if (temp > 0xFF) cy = 0x01;
	  else             cy = 0x00;
	  acc = (temp & 0xFF);
	  clk += 2;
	  break;
	case 0xA3:		/* MOVP A,@A */
	  acc = ROM ((pc & 0xF00) | acc);
	  clk += 2;
	  break;
	case 0x47:		/* SWAP A */
	  data = (acc & 0xF0) >> 4;
	  acc  = acc << 4;
	  acc  |= data;
	  clk++;
	  break;
	case 0x77:		/* RR A */
	  data = acc & 0x01;
	  acc = acc >> 1;
	  if (data) acc |= 0x80;
	  else acc &= 0x7F;
	  clk++;
	  break;
	case 0xE7:		/* RL A */
	  data = acc & 0x80;
	  acc = acc << 1;
	  if (data) acc |= 0x01;
	  else acc &= 0xFE;
	  clk++;
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
	  switch (op)
	  {
		  case 0x024: pc |= 0x100; break;
		  case 0x044: pc |= 0x200; break;
		  case 0x064: pc |= 0x300; break;
		  case 0x084: pc |= 0x400; break;
		  case 0x0A4: pc |= 0x500; break;
		  case 0x0C4: pc |= 0x600; break;
		  case 0x0E4: pc |= 0x700; break;
	  }
	  clk += 2;
	  break;
	case 0x05:		/* EN I */
	  xirq_en = 1;
	  clk++;
	  break;
	case 0x15:		/* DIS I */
	  xirq_en = 0;
	  clk++;
	  break;
	case 0x08:		/* INS A,BUS */
	  acc = in_bus ();
	  clk += 2;
	  break;
	case 0x09:		/* IN A,Pp */
	case 0x0A:		/* IN A,Pp */
	  if (op == 0x09) acc = p1;
	  else            acc = read_p2 ();
	  clk += 2;
	  break;
	case 0x0C:		/* MOVD A,P4 */
	case 0x0D:		/* MOVD A,P5 */
	case 0x0E:		/* MOVD A,P6 */
	case 0x0F:		/* MOVD A,P7 */
	  // acc = read_pb (op - 0x0C);
	  clk += 2;
	  break;
	case 0x10:		/* INC @Ri */
	case 0x11:		/* INC @Ri */
	  internal_ram[internal_ram[reg_pnt + (op - 0x10)]]++;
	  clk++;
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
	  uint8_t test_bit = 0x01 << ((op - 0x12) / 0x20);
	  if (acc & test_bit) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk += 2;
	  break;
	case 0x16:		/* JTF */
	  data = ROM (pc);
	  if (t_flag) pc = (pc & 0xF00) | data;
	  else pc++;
	  t_flag = 0x00;
	  clk += 2;
	  break;
	case 0x18:		/* INC Rr */
	case 0x19:		/* INC Rr */
	case 0x1A:		/* INC Rr */
	case 0x1B:		/* INC Rr */
	case 0x1C:		/* INC Rr */
	case 0x1D:		/* INC Rr */
	case 0x1E:		/* INC Rr */
	case 0x1F:		/* INC Rr */
	  internal_ram[reg_pnt + (op - 0x18)]++;
	  clk++;
	  break;
	case 0x20:		/* XCH A,@Ri */
	case 0x21:		/* XCH A,@Ri */
	  data = acc;
	  acc                                              = internal_ram[internal_ram[reg_pnt + (op - 0x20)]];
	  internal_ram[internal_ram[reg_pnt + (op -0x20)]] = data;
	  clk++;
	case 0x23:		/* MOV a,#data */
	  acc = ROM (pc++);
	  clk += 2;
	  break;
	case 0x53:		/* ANL A,#data */
	  clk += 2;
	  acc = acc & ROM (pc++);
	  break;
	case 0x26:		/* JNT0 */
	  data = ROM (pc);
	  // if (!get_voice_status ()) pc = (pc & 0xF00) | data; // TODO
	  // else pc++;
	  pc = (pc & 0xF00) | data;
	  clk += 2;
	  break;
	case 0x36:		/* JT0 */
	  data = ROM (pc);
	  // if (get_voice_status ()) pc = (pc & 0xF00) | data;
	  // else pc++;
	  pc++;
	  clk += 2;
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
	  acc                   = internal_ram[reg_pnt + (op - 0x28)];
	  internal_ram[reg_pnt] = data;
	  clk++;
	  break;
	case 0x30:		/* XCHD A,@Ri */
	case 0x31:		/* XCHD A,@Ri */
	  addr = internal_ram[reg_pnt + (op - 0x30)];
	  internal_ram[addr] = (internal_ram[addr] & 0xF0) | (acc                & 0x0F);
	  acc                = (acc                & 0xF0) | (internal_ram[addr] & 0x0F);
	  clk++;
	  break;
	case 0x25:		/* EN TCNTI */
	  tirq_en = 1;
	  clk++;
	  break;
	case 0x35:		/* DIS TCNTI */
	  tirq_en = 0;
	  tirq_pend = 0;
	  clk++;
	  break;
	case 0x39:		/* OUTL P1,A */
	case 0x3A:		/* OUTL P2,A */
	  if (op = 0x39) write_p1 (acc);
	  else p2 = acc;
	  clk += 2;
	  break;
	case 0x3C:		/* MOVD P4,A */
	case 0x3D:		/* MOVD P5,A */
	case 0x3E:		/* MOVD P6,A */
	case 0x3F:		/* MOVD P7,A */
	  // write_pb ((op - 0x3C), acc);
	  clk += 2;
	  break;
	case 0x40:		/* ORL A,@Ri */
	case 0x41:		/* ORL A,@Ri */
	  acc = acc | internal_ram[internal_ram[reg_pnt + (op - 0x40)]];
	  clk++;
	  break;
	case 0x43:		/* ORL A,#data */
	  acc = acc | ROM (pc++);
	  clk += 2;
	  break;
	case 0x45:		/* STRT CNT */
	  count_on = 1;
	  clk++;
	  break;
	case 0x65:		/* STOP TCNT */
	  count_on = 0;
	  timer_on = 0;
	  clk++;
	  break;
	case 0x42:		/* MOV A,T */
	  acc = itimer;
	  clk++;
	  break;
	case 0x55:		/* STRT T */
	  timer_on = 1;
	  clk++;
	  break;
	case 0x62:		/* MOV T,A */
	  itimer = acc;
	  clk++;
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
          clk += 2;
	  break;
	case 0x57:		/* DA A */
	  if (((acc & 0x0F) > 0x09) || ac)
	    {
	      if (acc > 0xf9) cy = 0x01;
	      acc += 6;
	    }
	  data = (acc & 0xF0) >> 4;
	  if ((data > 9) || cy)
	    {
	      data += 6;
	      cy = 0x01;
	    }
	  acc = (acc & 0x0F) | (data << 4);
	  clk++;
	  break;
	case 0x50:		/* ANL A,@Ri */
	case 0x51:		/* ANL A,@Ri */
	  acc = acc & internal_ram[internal_ram[reg_pnt + (op - 0x50)]];
	  clk++;
	  break;
	case 0x48:		/* ORL A,Rr */
	case 0x49:		/* ORL A,Rr */
	case 0x4A:		/* ORL A,Rr */
	case 0x4B:		/* ORL A,Rr */
	case 0x4C:		/* ORL A,Rr */
	case 0x4D:		/* ORL A,Rr */
	case 0x4E:		/* ORL A,Rr */
	case 0x4F:		/* ORL A,Rr */
	  acc = acc | internal_ram[reg_pnt + (op - 0x48)];
	  clk++;
	  break;
	case 0x58:		/* ANL A,Rr */
	case 0x59:		/* ANL A,Rr */
	case 0x5A:		/* ANL A,Rr */
	case 0x5B:		/* ANL A,Rr */
	case 0x5C:		/* ANL A,Rr */
	case 0x5D:		/* ANL A,Rr */
	case 0x5E:		/* ANL A,Rr */
	case 0x5F:		/* ANL A,Rr */
	  acc = acc & internal_ram[reg_pnt + (op - 0x58)];
	  clk++;
	  break;
	case 0xD8:		/* XRL A,Rr */
	case 0xD9:		/* XRL A,Rr */
	case 0xDA:		/* XRL A,Rr */
	case 0xDB:		/* XRL A,Rr */
	case 0xDC:		/* XRL A,Rr */
	case 0xDD:		/* XRL A,Rr */
	case 0xDE:		/* XRL A,Rr */
	case 0xDF:		/* XRL A,Rr */
	  acc = acc ^ internal_ram[reg_pnt + (op - 0xD8)];
	  clk++;
	  break;
	case 0xE3:		/* MOVP3 A,@A */
          addr = 0x300 | acc;
	  acc = ROM (addr);
	  clk += 2;
	  break;
	case 0x60:		/* ADD A,@Ri */
	case 0x61:		/* ADD A,@Ri */
	  cy = 0x00;
	  ac = 0x00;
	  data = internal_ram[internal_ram[reg_pnt] + (op - 0x60)];
	  if (((acc & 0x0F) + (data & 0x0F)) > 0x0F) ac = 0x40;
	  temp = acc + data;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  clk++;
	  break;
	case 0x67:		/* RRC A */
	  data = cy;
	  cy = acc & 0x01;
	  acc = acc >> 1;
	  if (data) acc = acc | 0x80;
	  else acc = acc & 0x7F;
	  clk++;
	  break;
	case 0xF7:		/* RLC A */
	  data = cy;
	  cy = (acc & 0x80) >> 7;
	  acc = acc << 1;
	  if (data) acc = acc | 0x01;
	  else acc = acc & 0xFE;
	  clk++;
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
	  data = internal_ram[reg_pnt + (op - 0x68)];
	  if (((acc & 0x0F) + (data & 0x0F)) > 0x0F) ac = 0x40;
	  temp = acc + data;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  clk++;
	  break;
	case 0x70:		/* ADDC A,@Ri */
	case 0x71:		/* ADDC A,@Ri */
	  ac = 0x00;
	  data = internal_ram[internal_ram[reg_pnt + (op - 0x70)]];
	  if (((acc & 0x0F) + (data & 0x0F) + cy) > 0x0F) ac = 0x40;
	  temp = acc + data + cy;
	  cy = 0x00;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  clk++;
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
	  data = internal_ram[reg_pnt + (op - 0x78)];
	  if (((acc & 0x0F) + (data & 0x0F) + cy) > 0x0F) ac = 0x40;
	  temp = acc + data + cy;
	  cy = 0x00;
	  if (temp > 0xFF) cy = 0x01;
	  acc = (temp & 0xFF);
	  clk++;
	  break;
	case 0x80:		/* MOVX A,@Ri */
	case 0x81:		/* MOVX A,@Ri */
	  acc = ext_read (internal_ram[reg_pnt + (op - 0x80)]);
	  clk += 2;
	  break;
	case 0x86:		/* JNI address */
	  clk += 2;
	  data = ROM (pc);
	  if (int_clk > 0) pc = (pc & 0xF00) | data;
	  else pc++;
	  break;
	case 0x89:		/* ORL Pp,#data */
	case 0x8A:		/* ORL Pp,#data */
	  if (op == 0x89) write_p1 (p1 | ROM (pc++));
	  else p2 = p2 | ROM (pc++);
	  clk += 2;
	  break;
	case 0x8C:		/* ORLD P4,A */
	case 0x8D:		/* ORLD P5,A */
	case 0x8E:		/* ORLD P6,A */
	case 0x8F:		/* ORLD P7,A */
	  //write_pb ((op - 0x8C), read_pb (0) | acc);
	  clk += 2;
	  break;
	case 0x90:		/* MOVX @Ri,A */
	case 0x91:		/* MOVX @Ri,A */
	  ext_write (acc, internal_ram[reg_pnt + (op - 0x90)]);
	  clk += 2;
	case 0x83:		/* RET */
	  pc = ((pull () & 0x0F) << 8);
	  pc |= pull ();
	  clk += 2;
	  break;
	case 0x93:		/* RETR */
	  data = pull ();
	  pc = (data & 0x0F) << 8;
	  cy = (data & 0x80) >> 7;
	  ac = data & 0x40;
	  f0 = data & 0x20;
	  bs = data & 0x10;
	  if (bs) reg_pnt = 0x18;
	  else reg_pnt = 0x00;
	  pc |= pull ();
	  irq_ex = 0;
	  a11 = a11_backup;
	  clk += 2;
	  break;
	case 0x97:		/* CLR C */
	  cy = 0x00;
	  clk++;
	  break;
	case 0x99:		/* ANL Pp,#data */
	case 0x9A:		/* ANL Pp,#data */
	  if (op = 0x99) write_p1 (p1 & ROM (pc++));
	  else p2 = p2 & ROM (pc++);
	  clk += 2;
	  break;
	case 0x9C:		/* ANLD P4,A */
	case 0x9D:		/* ANLD P5,A */
	case 0x9E:		/* ANLD P6,A */
	case 0x9F:		/* ANLD P7,A */
	  // write_pb (0, read_pb (op - 0x9C) & acc); // TODO
	  clk += 2;
	  break;
	case 0xA0:		/* MOV @Ri,A */
	case 0xA1:		/* MOV @Ri,A */
	  internal_ram[internal_ram[reg_pnt + (op - 0xA0)]] = acc;
	  clk++;
	  break;
	case 0x85:		/* CLR F0 */
	  f0 = 0x00;
	  clk++;
	  break;
	case 0xB6:		/* JF0 address */
	  data = ROM (pc);
	  if (f0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk += 2;
	  break;
	case 0x95:		/* CPL F0 */
	  f0 = f0 ^ 0x20;
	  clk++;
	  break;
	case 0xA5:		/* CLR F1 */
	  f1 = 0x00;
	  clk++;
	  break;
	case 0x76:		/* JF1 address */
	  data = ROM (pc);
	  if (f1) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk += 2;
	  break;
	case 0xB5:		/* CPL F1 */
	  f1 = f1 ^ 0x01;
	  clk++;
	  break;
	case 0xA7:		/* CPL C */
	  cy = cy ^ 0x01;
	  clk++;
	  break;
	case 0xA8:		/* MOV Rr,A */
	case 0xA9:		/* MOV Rr,A */
	case 0xAA:		/* MOV Rr,A */
	case 0xAB:		/* MOV Rr,A */
	case 0xAC:		/* MOV Rr,A */
	case 0xAD:		/* MOV Rr,A */
	case 0xAE:		/* MOV Rr,A */
	case 0xAF:		/* MOV Rr,A */
	  internal_ram[reg_pnt + (op - 0xA8)] = acc;
	  clk++;
	  break;
	case 0xB0:		/* MOV @Ri,#data */
	case 0xB1:		/* MOV @Ri,#data */
	  internal_ram[internal_ram[reg_pnt + (op - 0xB1)]] = ROM (pc++);
	  clk += 2;
	  break;
	case 0xB3:		/* JMPP @A */
	  addr = (pc & 0xF00) | acc;
	  pc = (pc & 0xF00) | ROM (addr);
	  clk += 2;
	  break;
	case 0xB8:		/* MOV Rr,#data */
	case 0xB9:		/* MOV Rr,#data */
	case 0xBA:		/* MOV Rr,#data */
	case 0xBB:		/* MOV Rr,#data */
	case 0xBC:		/* MOV Rr,#data */
	case 0xBD:		/* MOV Rr,#data */
	case 0xBE:		/* MOV Rr,#data */
	case 0xBF:		/* MOV Rr,#data */
	  internal_ram[reg_pnt + (op - 0xB8)] = ROM (pc++);
	  clk += 2;
	  break;
	case 0xC5:		/* SEL RB0 */
	case 0xD5:		/* SEL RB1 */
	  switch(op) {
	    case 0xC5:
	      bs = 0x00;
	      reg_pnt = 0x00;
	      break;
	    case 0xD5:
	      bs = 0x10;
	      reg_pnt = 0x18;
	      break;
	  }
	  clk++;
	  break;
	case 0xC6:		/* JZ address */
	  data = ROM (pc);
	  if (acc == 0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk += 2;
	  break;
	case 0x96:		/* JNZ address */
	  data = ROM (pc);
	  if (acc != 0) pc = (pc & 0xF00) | data;
	  else pc++;
	  clk += 2;
	  break;
	case 0xC7:		/* MOV A,PSW */
	  make_psw ();
	  acc = psw;
	  clk++;
	  break;
	case 0xD7:		/* MOV PSW,A */
	  psw = acc;
	  cy = (psw & 0x80) >> 7;
	  ac = psw & 0x40;
	  f0 = psw & 0x20;
	  bs = psw & 0x10;
	  if (bs) reg_pnt = 0x18;
	  else    reg_pnt = 0x00;
	  sp = (psw & 0x07) << 1;
	  sp += 0x08;
	  clk++;
	  break;
	case 0xC8:		/* DEC Rr */
	case 0xC9:		/* DEC Rr */
	case 0xCA:		/* DEC Rr */
	case 0xCB:		/* DEC Rr */
	case 0xCC:		/* DEC Rr */
	case 0xCD:		/* DEC Rr */
	case 0xCE:		/* DEC Rr */
	case 0xCF:		/* DEC Rr */
	  internal_ram[reg_pnt + (op - 0xC8)]--;
	  clk++;
	  break;
	case 0xD0:		/* XRL A,@Ri */
	case 0xD1:		/* XRL A,@Ri */
	  acc = acc ^ internal_ram[internal_ram[reg_pnt + (op - 0xD0)]];
	  clk++;
	  break;
	case 0xD3:		/* XRL A,#data */
	  clk += 2;
	  acc = acc ^ ROM (pc++);
	  break;
	case 0xE5:		/* SEL MB0 */ // TODO pourquoi cete differnce entre les deux SEL MB ?
	  a11 = 0x000;
	  a11_backup = 0x000;
	  clk++;
	  break;
	case 0xF5:		/* SEL MB1 */
          if (irq_ex) a11_backup = 0x800;
	  else
	  {
		  a11 = 0x800;
		  a11_backup = 0x800;
	  }
	  clk++;
	  break;
	case 0xF6:		/* JC address */
	  clk += 2;
	  data = ROM (pc);
	  if (cy) pc = (pc & 0xF00) | data;
	  else pc++;
	  break;
	case 0xE6:		/* JNC address */
	  data = ROM (pc);
	  if (!cy) pc = (pc & 0xF00) | data;
	  else pc++;
	  break;
	  clk += 2;
	case 0xE8:		/* DJNZ Rr,address */
	case 0xE9:		/* DJNZ Rr,address */
	case 0xEA:		/* DJNZ Rr,address */
	case 0xEB:		/* DJNZ Rr,address */
	case 0xEC:		/* DJNZ Rr,address */
	case 0xED:		/* DJNZ Rr,address */
	case 0xEE:		/* DJNZ Rr,address */
	case 0xEF:		/* DJNZ Rr,address */
	  internal_ram[reg_pnt + (op - 0xE8)]--;
	  data = ROM (pc);
	  if (internal_ram[reg_pnt + (op - 0xE8)] != 0)
	    {
	      pc = pc & 0xF00;
	      pc = pc | data;
	    }
	  else pc++;
	  clk += 2;
	  break;
	case 0xF0:		/* MOV A,@Ri */
	case 0xF1:		/* MOV A,@Ri */
	  acc = internal_ram[internal_ram[reg_pnt + (op - 0xF0)]];
	  clk++;
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
	  addr = ROM (pc) | a11;
	  switch(op) {
		  case 0x34: addr |= 0x100; break; 
		  case 0x54: addr |= 0x200; break; 
		  case 0x74: addr |= 0x300; break; 
		  case 0x94: addr |= 0x400; break; 
		  case 0xA4: addr |= 0x500; break; 
		  case 0xC4: addr |= 0x600; break; 
		  case 0xE4: addr |= 0x700; break; 
	  }
	  pc = addr;
	  clk += 2;
	  break;
	case 0xF8:		/* MOV A,Rr */
	case 0xF9:		/* MOV A,Rr */
	case 0xFA:		/* MOV A,Rr */
	case 0xFB:		/* MOV A,Rr */
	case 0xFC:		/* MOV A,Rr */
	case 0xFD:		/* MOV A,Rr */
	case 0xFE:		/* MOV A,Rr */
	case 0xFF:		/* MOV A,Rr */
	  acc = internal_ram[reg_pnt + (op - 0xF8)];
	  clk++;
	  break;
	}

      master_clk += clk;
      horizontal_clock += clk;

      if (int_clk > clk) int_clk -= clk;
      else int_clk = 0;

      if (xirq_pend) ext_irq ();
      if (tirq_pend) timer_irq ();

      if (horizontal_clock > LINECNT - 1)
	{
	  horizontal_clock -= LINECNT;
	  if (enahirq && (intel8245_ram[0xA0] & 0x01)) ext_irq ();
	  if (count_on && mstate == 0)
	    {
	      itimer++;
	      if (itimer == 0)
		{
		  t_flag = 1;
		  timer_irq ();
		  draw_region ();
		}
	    }
	}

      if (timer_on)
	{
	  master_count += clk;
	  if (master_count > 31)
	    {
	      master_count -= 31;
	      itimer++;
	      if (itimer == 0)
		{
		  t_flag = 1;
		  timer_irq ();
		}
	    }
	}

      if ((mstate == 0) && (master_clk > VBLCLK)) handle_vbl ();

      if ((mstate == 1) && (master_clk > evblclk))
	{
	  handle_evbl ();
	  break;
	}
    }
}
