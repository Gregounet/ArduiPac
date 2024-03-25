/*
 *   O2EM2 Free Odyssey2 / Videopac+ Emulator
 *
 *   Created by Daniel Boris <dboris@comcast.net>  (c) 1997,1998
 *   Developed by Andre de la Rocha   <adlroc@users.sourceforge.net>
 *             Arlindo M. de Oliveira <dgtec@users.sourceforge.net>
 *
 *   Under development by LABBE Corentin http://o2em2.sourceforge.net
 *
 *
 *
 *   Main O2 machine emulation
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "audio.h"
#include "types.h"
#include "cpu.h"
#include "keyboard.h"
#include "o2em2.h"
#include "debug.h"
#include "vdc.h"
#include "vpp.h"
#include "timefunc.h"
#include "voice.h"
#include "vmachine.h"
#ifndef __O2EM_SDL__
#include "allegro.h"
#else
#include "o2em_sdl.h"
#endif

static Byte x_latch,y_latch;
static int romlatch=0;
static Byte line_count;
static int fps = FPS_NTSC;

static Byte snapedlines[MAXLINES+2*MAXSNAP][256][2];

int evblclk=EVBLCLK_NTSC;

struct resource app_data;
int frame=0;
Byte dbstick1,dbstick2;

int int_clk; 	/* counter for length of /INT pulses */
int master_clk;	/* Master clock */
int h_clk;   /* horizontal clock */
unsigned long clk_counter;
int last_line;
int key2vcnt=0;
int mstate;

int pendirq=0;
int enahirq=1;
int useforen=0;
long regionoff = 0xffff;
int mxsnap=2;
int sproff=0;/* sprite offset*/
int tweakedaudio=0;

Byte rom_table[8][4096];

Byte intRAM[64];
Byte extRAM[256];
Byte extROM[1024];
Byte VDCwrite[256];
Byte ColorVector[MAXLINES];
Byte AudioVector[MAXLINES];
Byte *rom;
Byte *megarom;

/*int key2[128];*/


static unsigned int key_map[6][8]= {
	{KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7},
	{KEY_8, KEY_9, 0, 0, KEY_SPACE, KEY_SLASH, KEY_L, KEY_P},
	{KEY_PLUS_PAD, KEY_W, KEY_E, KEY_R, KEY_T, KEY_U, KEY_I, KEY_O},
	{KEY_Q, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K},
	{KEY_A, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_M, KEY_STOP},
	{KEY_MINUS, KEY_ASTERISK, KEY_SLASH_PAD, KEY_EQUALS, KEY_Y, KEY_N, KEY_DEL, KEY_ENTER}
};


static void do_kluges();
static void setvideomode(int t);

/*============================================================================*/
/*============================================================================*/
void run()
{
	while(!key_done) {
		if (key_debug) {
			app_data.debug=1;
			set_textmode();
			mute_audio();
			mute_voice();
/*			debug(); TODO il faut le remettre plus tard, je lai enelve juste pour les warnings*/
			grmode();
			app_data.debug=0;
			o2em_init_keyboard();
			init_sound_stream();
		}
		cpu_exec();
	}
	close_audio();
	close_voice();
	close_display();
	/*o2em_clean_quit(EXIT_SUCCESS);*/
}

/*============================================================================*/
/*============================================================================*/
void handle_vbl()
{
	if (!app_data.debug) {
		handle_key();
		update_audio();
		update_voice();
	}
	draw_region();
	ext_IRQ();
	mstate = 1;
}

/*============================================================================*/
/*============================================================================*/
void handle_evbl()
{
	static unsigned long last = 0;
	static int rest_cnt = 0;
	static unsigned long idx = 0;
	static unsigned long first = 0;
	int i;
	unsigned long d, f, tick_tmp;
	int antiloop;

	/*#ifndef ALLEGRO_DOS
	yield_timeslice();
	#endif*/
	i = (15 * app_data.speed / 100);
	rest_cnt = (rest_cnt+1)%(i<5?5:i);
	/*
	#ifdef ALLEGRO_WINDOWS
	if (rest_cnt == 0)
		rest(1);
	#endif*/
	last_line = 0;
	master_clk -= evblclk;
	frame++;
	if (!app_data.debug) {
		finish_display();
	}

	if (app_data.crc == 0xA7344D1F) { /* Atlantis */
		for (i=0; i<140; i++) {
			ColorVector[i] = (VDCwrite[0xA3] & 0x7f) | (p1 & 0x80);
			AudioVector[i] = VDCwrite[0xAA];
		}
	} else
		for (i = 0; i < MAXLINES; i++)  {
			ColorVector[i] = (VDCwrite[0xA3] & 0x7f) | (p1 & 0x80);
			AudioVector[i] = VDCwrite[0xAA];
		}

	if (key2vcnt++ > 10) {
		key2vcnt = 0;
		for (i = 0; i < KEY_MAX; i++)
			key2[i] = 0;
		dbstick1 = dbstick2 = 0;
	}
	if (app_data.limit) {
		d = (TICKSPERSEC * 100) / (app_data.speed * fps);
		f = ((d + last - gettimeticks()) * 1000) / TICKSPERSEC;
		antiloop = 0;
		idx++;
		if (first == 0)
			first = gettimeticks() - 1;
		tick_tmp = gettimeticks();
		/*printf("%ld %ld %ld %ld\n", idx, d, tick_tmp - first, idx * TICKSPERSEC / (tick_tmp - first));*/
		/* need this on my windows because the "d" sleep time is incorect
		 * Certainly because it dont test current fps
		 * */
		if (idx * TICKSPERSEC / (tick_tmp - first) < fps)
			d = 0;
		if (f > 0) {
			#ifdef ALLEGRO_WINDOWS
			/*
				f = f - (f%10);
				printf("%ld\n", f);
				if (f > 5)
					rest(f - 5);*/
			#else
				/*#ifndef ALLEGRO_DOS
					yield_timeslice();
				#endif*/
			#endif
		}
		while (gettimeticks() - last < d && antiloop < 1000000) {
			antiloop++;
			/*rest(1);*/
		}
		if (antiloop >= 1000000) {
			printf("antiloop %d\n", antiloop);
		}
		last = gettimeticks();
	}
	mstate = 0;

}

/*============================================================================*/
/*============================================================================*/
void handle_evbll()
{
	static unsigned long last = 0;
	static int rest_cnt = 0;
	int i;
	unsigned long d, f;
	int antiloop = 0;
	/*#ifndef ALLEGRO_DOS
	yield_timeslice();
	#endif*/
	i = (15*app_data.speed/100);
	rest_cnt = (rest_cnt+1)%(i<5?5:i);
	/* why this ???
	#ifdef ALLEGRO_WINDOWS
	if (rest_cnt == 0)
		rest(1);
	#endif
	*/

	/******************* 150 */

	for (i = 150; i < MAXLINES; i++)  {
		ColorVector[i] = (VDCwrite[0xA3] & 0x7f) | (p1 & 0x80);
		AudioVector[i] = VDCwrite[0xAA];
	}

	if (key2vcnt++ > 10) {
		key2vcnt = 0;
		for (i = 0; i < KEY_MAX; i++)
			key2[i] = 0;
		dbstick1 = dbstick2 = 0;
	}
	if (app_data.limit) {
		d = (TICKSPERSEC * 100) / (app_data.speed * fps);
		f = ((d + last - gettimeticks()) * 1000) / TICKSPERSEC;
		if (f > 0) {
			#ifdef ALLEGRO_WINDOWS
				/*
				f = f-(f%10);
				if (f>5) rest(f-5);*/
			#else
				/*#ifndef ALLEGRO_DOS
					yield_timeslice();
				#endif*/
			#endif
		}
		antiloop = 0;
		while (gettimeticks() - last < d && antiloop < 1000000) {
			antiloop++;
		}
		if (antiloop >= 1000000) {
			printf("antiloop %d\n", antiloop);
		}
		last = gettimeticks();
	}
	mstate = 0;
}

/*============================================================================*/
/*============================================================================*/
void init_system()
{
	int i, j, k;
	#ifdef __O2EM_DEBUG__
	printf("%s()\n", __func__);
	#endif
	last_line = 0;
	dbstick1 = 0x00;
	dbstick2 = 0x00;
	mstate = 0;
	master_clk = 0;
	h_clk = 0;
	line_count = 0;
	itimer = 0;
	clk_counter = 0;

	init_roms();

	for (i = 0; i < 256; i++) {
		VDCwrite[i] = 0;
		extRAM[i] = 0;
	}
	for (i = 0; i < 64; i++)
		intRAM[i] = 0;

	for (i = 0; i < MAXLINES; i++)
		AudioVector[i] = ColorVector[i] = 0;

	for (i = 0; i < MAXLINES + 2 * MAXSNAP; i++)
		for (j = 0; j < 256; j++)
			for (k = 0; k < 2; k++)
				snapedlines[i][j][k]=0;

	if (app_data.stick[0] == 2 || app_data.stick[1] == 2) {
		#ifdef __O2EM_DEBUG__
		printf("%s() install_joystick()\n", __func__);
		#endif
		i = install_joystick(JOY_TYPE_AUTODETECT);
		if (i || (num_joysticks < 1)) {
			fprintf(stderr, "Error: no joystick detected\n");
			o2em_clean_quit(EXIT_FAILURE);
		}
	}
	for (i = 0; i < KEY_MAX; i++)
		key2[i] = 0;
	key2vcnt = 0;

	if (app_data.euro)
		setvideomode(1);
	else
		setvideomode(0);

	do_kluges();
	init_vpp();
	clear_collision();
	#ifdef __O2EM_DEBUG__
	printf("end of %s()\n", __func__);
	#endif
}

/*============================================================================*/
/*============================================================================*/
void init_roms()
{
#ifdef __O2EM_DEBUG__
	printf("%s()\n", __func__);
#endif
	rom = rom_table[0];
	romlatch = 0;
}

/*============================================================================*/
/*============================================================================*/
Byte read_t1()
{
     /*17*/
	if ((h_clk > 16) || (master_clk > VBLCLK))
		return 1;
	else
		return 0;
}

/*============================================================================*/
/*============================================================================*/
void write_p1(Byte d)
{
	if ((d & 0x80) != (p1 & 0x80)) {
		int i,l;
		l = snapline((int)((float)master_clk/22.0+0.1), VDCwrite[0xA3], 1);
		for (i=l; i<MAXLINES; i++) ColorVector[i] = (VDCwrite[0xA3] & 0x7f) | (d & 0x80);
	}
	p1 = d;
	if (app_data.bank == 2) {
		rom = rom_table[~p1 & 0x01];
	} else if (app_data.bank == 3) {
		rom = rom_table[~p1 & 0x03];
	} else if (app_data.bank == 4) {
		rom = rom_table[(p1 & 1)?0:romlatch];
	}
}


/*============================================================================*/
/*============================================================================*/
Byte read_P2()
{
	int i, si, so, km;

	if (NeedsPoll)
		poll_keyboard();

	if (!(p1 & 0x04)) {
		si = (p2 & 7);
		so = 0xff;
		#ifdef __O2EM_KEYBOARD_DEBUG__
		/*printf("%s si=%d\n", __func__, si);*/
		#endif
		if (si < 6) {
			for (i = 0; i < 8; i++) {
				km = key_map[si][i];
				#ifdef __O2EM_KEYBOARD_DEBUG__
				/*if (key[km] > 0 || key[km] < 0)
					printf("%s km=%d si=%d i=%d SDLK_z=%d KEY_1=%d\n", __func__, km, si, i, KEY_Z, KEY_1);*/
				#endif
				if ((key[km] && ((!joykeystab[km]) || (key_shifts & KB_CAPSLOCK_FLAG))) || (key2[km])) {
					so = i ^ 0x07;
				}
			}
		}
		if (so != 0xff) {
			p2 = p2 & 0x0F;
			p2 = p2 | (so << 5);
		} else {
			p2 = p2 | 0xF0;
		}
	} else {
		p2 = p2 | 0xF0;
	}
	#ifdef __O2EM_KEYBOARD_DEBUG__
	/*printf("p2 = %d\n", p2);*/
	#endif
	return p2;
}

/*============================================================================*/
/*============================================================================*/
/* read a value, final destination is base on p1 (controlled by write p1)*/
Byte ext_read(ADDRESS adr){
	Byte d;
	Byte si;
	Byte m;
	int i;

	if (!(p1 & 0x08) && !(p1 & 0x40)) {
		/* Handle VDC Read */
		switch(adr) {
			case 0xA1:
				d = VDCwrite[0xA0] & 0x02;
				if (master_clk > VBLCLK) d = d | 0x08;
				if (h_clk < (LINECNT-7)) d = d | 0x01;
				if (sound_IRQ) d = d | 0x04;
				sound_IRQ=0;
				return d;
			case 0xA2:/* 0xA2 vdc_collision http://soeren.informationstheater.de/g7000/hardware.html */
				si = VDCwrite[0xA2];
				m = 0x01;
				d = 0;
				for(i=0; i<8; i++) {
					if (si & m) {
						if (coltab[1] & m) d = d | (coltab[1] & (m ^ 0xFF));
						if (coltab[2] & m) d = d | (coltab[2] & (m ^ 0xFF));
						if (coltab[4] & m) d = d | (coltab[4] & (m ^ 0xFF));
						if (coltab[8] & m) d = d | (coltab[8] & (m ^ 0xFF));
						if (coltab[0x10] & m) d = d | (coltab[0x10] & (m ^ 0xFF));
						if (coltab[0x20] & m) d = d | (coltab[0x20] & (m ^ 0xFF));
						if (coltab[0x80] & m) d = d | (coltab[0x80] & (m ^ 0xFF));
					}
					m = m << 1;
				}
				clear_collision();
				return d;
			case 0xA5:
				if (!(VDCwrite[0xA0] & 0x02)) {
					return x_latch;
				}
				else {
					x_latch = h_clk * 12;
					return x_latch;
				}
			case 0xA4:
				if (!(VDCwrite[0xA0] & 0x02)) {
					return y_latch;
				}
				else {
					y_latch = master_clk/22;
					if (y_latch > 241) y_latch=0xFF;
					return y_latch;
				}
			default:
				return VDCwrite[adr];
		}
		} else if (!(p1 & 0x10)) {
		/* Handle ext RAM Read */
		if (app_data.megaxrom && (adr >= 0x80)) {
			/* MegaCART registers are mirrored every 4 bytes */
			if ((adr & 0x83) == 0x83) {
				/* TODO: emulate EEPROM data in */
				return 0xff;
			} else return extRAM[adr & 0x83];
		} else return extRAM[adr & 0xFF];
 	} else if (!(p1 & 0x20)) {
		/* Read a Videopac+ register */
		return vpp_read(adr);
	} else if (app_data.exrom && (p1 & 0x02)) {
		/* Handle read from exrom */
		return extROM[(p2 << 8) | (adr & 0xFF)];
	} else if (app_data.megaxrom && !(p1 & 0x02) && !(p1 & 0x40)) {
		/* Handle data read from MegaCART */
		printf("WARNING megarom use \n");
		return megarom[(extRAM[0x81] << 12) | ((p2 & 0x0f) << 8) | (adr & 0xff)];
 	}

	return 0;
}


/*============================================================================*/
/*============================================================================*/
Byte in_bus()
{
	Byte si=0,d=0,mode=0,jn=0,sticknum=0;

	if ((p1 & 0x08) && (p1 & 0x10)) {
		/* Handle joystick read */
		if (!(p1 & 0x04)) {
			si = (p2 & 7);
		}
		d=0xFF;
		if (si == 1) {
			mode = app_data.stick[0];
			jn = 0;
			sticknum = app_data.sticknumber[0]-1;
		} else {
			mode = app_data.stick[1];
			jn = 1;
			sticknum = app_data.sticknumber[1]-1;
		}
		switch(mode) {
			case 1:
				d = keyjoy(jn);
				break;
			case 2:
				/*poll_joystick(); ALLEG*/
				if (joy[sticknum].stick[0].axis[1].d1) d &= 0xFE;			/* joy_up*/
				if (joy[sticknum].stick[0].axis[0].d2) d &= 0xFD;			/* joy_right*/
				if (joy[sticknum].stick[0].axis[1].d2) d &= 0xFB;			/* joy_down*/
				if (joy[sticknum].stick[0].axis[0].d1) d &= 0xF7;			/* joy_left*/
				if (joy[sticknum].button[0].b || joy[jn].button[1].b) d &= 0xEF;	/* both main-buttons*/
				break;
		}
		if (si == 1) {
			if (dbstick1) d = dbstick1;
		} else {
			if (dbstick2) d = dbstick2;
		}
   }
   return d;
}


/*============================================================================*/
/*============================================================================*/
void ext_write(Byte dat, ADDRESS adr){
	int i;
	int l;

	if (!(p1 & 0x08)) {
		/* Handle VDC Write */
		if (adr == 0xA0){
			if ((VDCwrite[0xA0] & 0x02) && !(dat & 0x02)) {
				y_latch = master_clk/22;
				x_latch = h_clk * 12;
				if (y_latch > 241) y_latch=0xFF;
			}
			if ((master_clk <= VBLCLK) && (VDCwrite[0xA0] != dat)) {
				draw_region();
			}
		} else if (adr == 0xA3){
			l = snapline((int)((float)master_clk/22.0+0.5), dat, 1);
			for (i=l; i<MAXLINES; i++) ColorVector[i] = (dat & 0x7f) | (p1 & 0x80);
		} else if (adr == 0xAA) {
			for (i = master_clk/22; i < MAXLINES; i++)
				AudioVector[i] = dat;
		} else if ((adr >= 0x40) && (adr <= 0x7f) && ((adr & 2) == 0)) {
			/* simulate quad: all 4 sub quad position registers
			 * are mapped to the same internal register */
			adr = adr & 0x71;
			/* Another minor thing: the y register always returns
			 * bit 0 as 0 */
			if ((adr & 1) == 0)
				dat = dat & 0xfe;
			VDCwrite[adr] = VDCwrite[adr+4] = VDCwrite[adr+8] = VDCwrite[adr+12] = dat;
		}
		VDCwrite[adr]=dat;
	} else if (!(p1 & 0x10) && !(p1 & 0x40)) {
		adr = adr & 0xFF;

		if (adr < 0x80) {
			/* Handle ext RAM Write */
			extRAM[adr] = dat;
		} else {
			if (app_data.bank == 4) {
				romlatch = (~dat) & 7;
				rom=rom_table[(p1 & 1)?0:romlatch];
			}

			/* Handle The Voice */
			if (!(dat & 0x20))
				reset_voice();
			else {
				if (adr == 0xE4)
					set_voice_bank(0);
				else if ((adr >= 0xE8) && (adr <= 0xEF))
					set_voice_bank(adr-0xE7);
				else if (((adr >= 0x80) && (adr <= 0xDF)) || ((adr >= 0xF0) && (adr <= 0xFF)))
					trigger_voice(adr);
			}
		}
	} else if (!(p1 & 0x20)) {
		/* Write to a Videopac+ register */
		vpp_write(dat,adr);
	}
}


/*============================================================================*/
/*============================================================================*/
static void do_kluges()
{
#ifdef __O2EM_DEBUG__
	printf("%s()\n", __func__);
#endif
	/* TODO use a switch here and add a printf for warning that a kludge is done 
	 * caution on multi kludge for a game:)
	 * */
	switch (app_data.crc) {
		case 0xA7344D1F: /* Atlantis */
		case 0xFB83171E: /* Blockout */
		case 0xD38153F6: /* Blockout (french) */
		case 0x881CEAE4: /* Wall Street */
			pendirq = 1;
			#ifdef __O2EM_VERBOSE__
			printf("%s QUIRK pendirq=1\n", __func__);
			#endif
			break;
		case 0x9E42E766: /* Turtles */
		case 0x1C750349: /* Turtles (European version) */
		case 0x202F2749: /* Q*bert */
		case 0x06861A9C: /* Flashpoint 5 (Videopac adaption) */
			useforen = 1;
			#ifdef __O2EM_VERBOSE__
			printf("%s QUIRK useforen=1\n", __func__);
			#endif
			break;
		default:
			#ifdef __O2EM_VERBOSE__
			printf("%s NO QUIRK\n", __func__);
			#endif
			break;
	}

	if (app_data.crc == 0xFB83171E) enahirq=0;				/* Blockout*/
	if (app_data.crc == 0xD38153F6) enahirq=0;				/* Blockout (french) */

	if (app_data.crc == 0xFB83171E) regionoff=1;			/* Blockout*/
	if (app_data.crc == 0xD38153F6) regionoff=1;			/* Blockout (french) */
	if (app_data.crc == 0x202F2749) regionoff=0;			/* Q*bert */
	if (app_data.crc == 0x5216771A) regionoff=1;			/* Popeye */
	if (app_data.crc == 0x0C2E4811) regionoff=11;			/* Out of this World! / Helicopter Rescue! */
	if (app_data.crc == 0x67069924) regionoff=11;			/* Smithereens! */
	if (app_data.crc == 0x44D1A8A5) regionoff=11;			/* Smithereens! (European version) */
	if (app_data.crc == 0x2391C2FB) regionoff=11;			/* Smithereens! + */
	if (app_data.crc == 0xBB4AD548) regionoff=11;			/* Smithereens! modified 1 */
	if (app_data.crc == 0x25057C11) regionoff=11;			/* Smithereens! modified 2 */
	if (app_data.crc == 0xB936BD78) regionoff=12;			/* Type & Tell */
	if (app_data.crc == 0xAD8B9AE0) regionoff=2;			/* Type & Tell modified 1 */
	if (app_data.crc == 0x5C02BEE6) regionoff=2;			/* Type & Tell modified 2 */
	if (app_data.crc == 0xDC30AD3D) regionoff=10;			/* Dynasty! */
	if (app_data.crc == 0x7810BAD5) regionoff=8;			/* Dynasty! (European) */
	if (app_data.crc == 0xA7344D1F) regionoff=0;			/* Atlantis */
	if (app_data.crc == 0xD0BC4EE6) regionoff=12;			/* Frogger */
	if (app_data.crc == 0xA57D84F3) regionoff=8;			/* Frogger BR */
	if (app_data.crc == 0x825976A9) regionoff=0;			/* Mousing Cat 8kb */
	if (app_data.crc == 0xF390BFEC) regionoff=0;			/* Mousing Cat 4kb */
	if (app_data.crc == 0x61A350E6) regionoff=0;			/* Mousing Cat (french) */
	if (app_data.crc == 0x3BFEF56B) regionoff=1;			/* Four in 1 Row! */
	if (app_data.crc == 0x7C747245) regionoff=1;			/* Four in 1 Row! modified */
	if (app_data.crc == 0x9B5E9356) regionoff=1;			/* Four in 1 Row! (french) */

	if (app_data.crc == 0x6CEBAB74) regionoff=12;			/* P.T. Barnum's Acrobats! (European version) */
	if (app_data.crc == 0xE7B26A56) regionoff=12;			/* P.T. Barnum's Acrobats! (European version - Extra keys) */

	if (app_data.crc == 0xFB83171E) mxsnap=3;				/* Blockout*/
	if (app_data.crc == 0xD38153F6) mxsnap=3;				/* Blockout (french) */
	if (app_data.crc == 0xA57E1724) mxsnap=12;				/* Catch the Ball / Noughts and Crosses */
	if (app_data.crc == 0xBE4FF48E) mxsnap=12;				/* Catch the Ball / Noughts and Crosses modified */
	if (app_data.crc == 0xFD179F6D) mxsnap=3;				/* Clay Pigeon! */
	if (app_data.crc == 0x9C9DDDF9) mxsnap=3;				/* Verkehr */
	if (app_data.crc == 0x95936B07) mxsnap=3;				/* Super Cobra */
	if (app_data.crc == 0x881CEAE4) mxsnap=3;				/* Wall Street */
	if (app_data.crc == 0x9E42E766) mxsnap=0;				/* Turtles */
	if (app_data.crc == 0x1C750349) mxsnap=0;				/* Turtles (European version) */
	if (app_data.crc == 0xD0BC4EE6) mxsnap=3;				/* Frogger */
	if (app_data.crc == 0xA57D84F3) mxsnap=3;				/* Frogger BR */
	if (app_data.crc == 0x3BFEF56B) mxsnap=6;				/* Four in 1 Row! */
	if (app_data.crc == 0x9B5E9356) mxsnap=6;				/* Four in 1 Row! (french) */
	if (app_data.crc == 0x7C747245) mxsnap=6;				/* Four in 1 Row! modified */

	if (app_data.crc == 0xA7344D1F) setvideomode(WANT_FPS_PAL);	/* Atlantis */
	if (app_data.crc == 0x39E31BF0) setvideomode(WANT_FPS_PAL);	/* Jake */
	if (app_data.crc == 0x92D0177B) setvideomode(1);		/* Jake (hack) */
	if (app_data.crc == 0x3351FEDA) setvideomode(1);		/* Power Lords */
	if (app_data.crc == 0x40AE062D) setvideomode(1);		/* Power Lords (alternate) */
	if (app_data.crc == 0xD158EEBA) setvideomode(1);		/* Labirinth */
	if (app_data.crc == 0x26B0FF5B) setvideomode(1);		/* Nightmare */
	if (app_data.crc == 0xDF36683F) setvideomode(1);		/* Shark Hunter */
	if (app_data.crc == 0xAF307559) setvideomode(1);		/* Super Bee 8Kb */
	if (app_data.crc == 0x9585D511) setvideomode(1);		/* Super Bee 4Kb */
	if (app_data.crc == 0x58FA6766) setvideomode(1);		/* War of the Nerves */
	if (app_data.crc == 0x58FA6766) setvideomode(1);		/* War of the Nerves */
	if (app_data.crc == 0x39989464) setvideomode(1);		/* Hockey! / Soccer! */
	if (app_data.crc == 0x3BFEF56B) setvideomode(1);		/* Four in 1 Row! */
	if (app_data.crc == 0x9B5E9356) setvideomode(1);		/* Four in 1 Row! (french) */
	if (app_data.crc == 0x7C747245) setvideomode(1);		/* Four in 1 Row! modified */
	if (app_data.crc == 0x68560DC7) setvideomode(1);		/* Jopac Moto Crash */
	if (app_data.crc == 0x020FCA15) setvideomode(1);		/* Jopac Moto Crash modified (non VP+) */
	if (app_data.crc == 0xC4134DF8) setvideomode(1);		/* Helicopter Rescue + */
	if (app_data.crc == 0x0D2D721D) setvideomode(1);		/* Trans American Rally + */
	if (app_data.crc == 0x9D72D4E9) setvideomode(1);		/* Blobbers */
	if (app_data.crc == 0xB2F0F0B4) setvideomode(1);		/* Le Tresor Englouti + */
	if (app_data.crc == 0x0B2DEB61) setvideomode(1);		/* Tutankham */
	if (app_data.crc == 0x313547EB) setvideomode(1);		/* VP53 */
	if (app_data.crc == 0x06861A9C) setvideomode(1);		/* Flashpoint 5 (Videopac adaption) */
	if (app_data.crc == 0xA57E1724) setvideomode(0);		/* Catch the Ball / Noughts and Crosses */
	if (app_data.crc == 0xBE4FF48E) setvideomode(0);		/* Catch the Ball / Noughts and Crosses modified */
	if (app_data.crc == 0xFB83171E) setvideomode(0);		/* Blockout*/
	if (app_data.crc == 0xD38153F6) setvideomode(0);		/* Blockout (french) */
	if (app_data.crc == 0x9BFC3E01) setvideomode(0);		/* Demon Attack */
	if (app_data.crc == 0x50AF9D45) setvideomode(0);		/* Demon Attack + */
	if (app_data.crc == 0x9884EF36) setvideomode(0);		/* Demon Attack + modified */
	if (app_data.crc == 0x4A578DFE) setvideomode(0);		/* Restaurant ntsc */
	if (app_data.crc == 0x863D5E2D) setvideomode(0);		/* Shark Hunter ntsc */

	if (app_data.crc == 0xD62814A3) evblclk=12000;			/* Pick Axe Pete */
	if (app_data.crc == 0xB2FFB353) evblclk=12000;			/* Pick Axe Pete + */
	if (app_data.crc == 0x81C20196) evblclk=12000;			/* Pick Axe Pete + (modified) */

	if ((app_data.crc == 0xF390BFEC) || (app_data.crc == 0x825976A9) || (app_data.crc ==  0x61A350E6)){	/* Mousing Cat */
		setvideomode(1);
		evblclk=7642;
	}

	if (app_data.crc == 0xD0BC4EE6) {					    /* Frogger */
		setvideomode(1);
		evblclk=7642;

	}
    if (app_data.crc == 0x26517E77) {						/* Commando Noturno */
		setvideomode(1);
		evblclk=6100;
		regionoff=12;
	}
    if (app_data.crc == 0xA57E1724) {						/* Catch the ball*/
		regionoff=5;
		sproff=1;

    }

    if ((app_data.crc == 0x2DCB77F0) || (app_data.crc == 0xF6882734)) {	/* Depth Charge / Marksman */
		setvideomode(1);
		evblclk=8000;
	}
	if (app_data.crc == 0x881CEAE4) {						/* Wall Street */
		setvideomode(1);
		evblclk=6100;
	}
	if (app_data.crc == 0xD0BC4EE6) tweakedaudio=1;			/* Frogger */
	if (app_data.crc == 0xA57D84F3) tweakedaudio=1;			/* Frogger BR */
	if (app_data.crc == 0x5216771A) tweakedaudio=1;			/* Popeye */
	if (app_data.crc == 0xAFB23F89) tweakedaudio=1;			/* Musician */
	if (app_data.crc == 0xC4134DF8) tweakedaudio=1;			/* Helicopter Rescue + */
	if (app_data.crc == 0x0D2D721D) tweakedaudio=1;			/* Trans American Rally + */

	if (app_data.crc == 0xD3B09FEC) sproff=1;				/* Volleyball! */
	if (app_data.crc == 0x551E38A2) sproff=1;				/* Volleyball! (french) */

}

/*============================================================================*/
/*============================================================================*/
/* TODO what is t? (can only be 0 or 1)*/
int snapline(int pos, Byte reg, int t){
	int i;
	if (pos < MAXLINES + MAXSNAP + MAXSNAP) {
		for (i = 0; i < mxsnap; i++){
			if (snapedlines[pos+MAXSNAP-i][reg][t]) return pos - i;
			if (snapedlines[pos+MAXSNAP+i][reg][t]) return pos + i;
		}
		snapedlines[pos+MAXSNAP][reg][t]=1;
	}
	return pos;
}

/*============================================================================*/
/*============================================================================*/
/* we could replace 0/1 by FPS_PAL FPS_NTSC*/
static void setvideomode(int t)
{
	if (t) {
		#ifdef __O2EM_VERBOSE__
		printf("%s WANT_FPS_PAL %d %d\n", __func__, WANT_FPS_PAL, t);
		#endif
		evblclk = EVBLCLK_PAL;
		fps = FPS_PAL;
	} else {
		#ifdef __O2EM_VERBOSE__
		printf("%s WANT_FPS_NTSC\n", __func__);
		#endif
		evblclk = EVBLCLK_NTSC;
		fps = FPS_NTSC;
	}
}

#define check_return_of_fxxx(ret, fn) if (ret == 0) { printf("%s:%d ERROR %s\n", __func__, __LINE__, strerror(errno)); fclose(fn); return -1;}

/*============================================================================*/
/*============================================================================*/
int savestate(char* filename)
{
	FILE *fn;
	size_t ret;

        fn = fopen(filename,"wb");
	if (fn == NULL) {
		fprintf(stderr, "Error opening state-file %s: %s\n", filename, strerror(errno));
		return errno;
	}

	/* TODO check return value*/
	ret = fwrite(&app_data.crc,sizeof(app_data.crc),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&app_data.bios,sizeof(app_data.bios),1,fn);
	check_return_of_fxxx(ret, fn);

	ret = fwrite(VDCwrite, 256, 1, fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(extRAM, 256, 1, fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(intRAM, 64, 1, fn);
	check_return_of_fxxx(ret, fn);

	ret = fwrite(&pc, sizeof(pc),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&sp, sizeof(sp),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&bs, sizeof(bs),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&p1, sizeof(p1),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&p2, sizeof(p2),1,fn);
	check_return_of_fxxx(ret, fn);

	ret = fwrite(&ac, sizeof(ac),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&cy, sizeof(cy),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&f0, sizeof(f0),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&A11, sizeof(A11),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&A11ff, sizeof(A11ff),1,fn);
	check_return_of_fxxx(ret, fn);

	ret = fwrite(&timer_on, sizeof(timer_on),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&count_on, sizeof(count_on),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&reg_pnt, sizeof(reg_pnt),1,fn);
	check_return_of_fxxx(ret, fn);

	ret = fwrite(&tirq_en, sizeof(tirq_en),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&xirq_en, sizeof(xirq_en),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&irq_ex, sizeof(irq_ex),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&xirq_pend, sizeof(xirq_pend),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fwrite(&tirq_pend, sizeof(tirq_pend),1,fn);
	check_return_of_fxxx(ret, fn);

	fclose(fn);
	return 0;
}

/*============================================================================*/
/*============================================================================*/
void read_a_char(ADDRESS *c, size_t s, FILE *fn)
{
	if (fn == NULL || c == NULL || s == 0) {
		fprintf(stderr, "%s Error invalid parameter\n", __func__);
		return ;
	}
	if (fread(c, s, 1, fn) < 1) {
		fprintf(stderr, "Error fread %s: %s\n", __func__, strerror(errno));
		o2em_clean_quit(EXIT_FAILURE);
	}
}


/*============================================================================*/
/*============================================================================*/
int loadstate(char *filename)
{
	FILE *fn;
	int bios;
	uint32_t crc;
	size_t ret;

	if (filename == NULL)
		return O2EM_FAILURE;
	#ifdef __O2EM_DEBUG__
	printf("%s DEBUG %s\n", __func__, filename);
	#endif
	fn = fopen(filename, "rb");
	if (fn == NULL) {
		fprintf(stderr, "fopen error %s: %s\n", __func__, strerror(errno));
		return O2EM_FAILURE;
	}

	ret = fread(&crc, sizeof(crc), 1, fn);
	check_return_of_fxxx(ret, fn);
	if (crc != app_data.crc) { /* wrong cart*/
		fclose(fn);
		return 199;/* TODO made 199 a #define WRONG_BIOS*/
	}

	ret = fread(&bios, sizeof(bios), 1, fn);
	check_return_of_fxxx(ret, fn);
	if (bios!=app_data.bios) { /* wrong bios*/
		fclose(fn);
		return 200 + bios;
	}

	ret = fread(VDCwrite, 256, 1, fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(extRAM, 256, 1, fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(intRAM, 64, 1, fn);
	check_return_of_fxxx(ret, fn);

/*	fread(&pc, sizeof(pc),1,fn);*/
	read_a_char(&pc, sizeof(pc), fn);
	ret = fread(&sp, sizeof(sp), 1, fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&bs, sizeof(bs),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&p1, sizeof(p1),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&p2, sizeof(p2),1,fn);
	check_return_of_fxxx(ret, fn);

	ret = fread(&ac, sizeof(ac),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&cy, sizeof(cy),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&f0, sizeof(f0),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&A11, sizeof(A11),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&A11ff, sizeof(A11ff),1,fn);
	check_return_of_fxxx(ret, fn);

	ret = fread(&timer_on, sizeof(timer_on),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&count_on, sizeof(count_on),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&reg_pnt, sizeof(reg_pnt),1,fn);
	check_return_of_fxxx(ret, fn);

	ret = fread(&tirq_en, sizeof(tirq_en),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&xirq_en, sizeof(xirq_en),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&irq_ex, sizeof(irq_ex),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&xirq_pend, sizeof(xirq_pend),1,fn);
	check_return_of_fxxx(ret, fn);
	ret = fread(&tirq_pend, sizeof(tirq_pend),1,fn);
	check_return_of_fxxx(ret, fn);

	fclose(fn);
	return O2EM_SUCCESS;
}
