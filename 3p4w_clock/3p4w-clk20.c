//******************************************************************************
//  MSP431F20xx 3 part 4 wire led alarm clock w/ temperature display
//
//  Description; 
//
//
//		  MSP430G2xx1
/*
                 Vc Gnd
   +=====================================================+
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  o  .  o  c  .  .  .  .  .  .  .  | connect o to o
   |  .  .  .  .  v  .  .  .  .  .  .  .  .  .  .  .  .  | connect c to c
   |  .  .  .  +--+--+-(0)-A--F-(1)(2)-B--+--+--+  .  .  | connect v to v
   |           |    |- b6 b7 CK IO a7 a6|       |        |
   |           |    |+ a0 a1 a2 a3 a4 a5|       |        |
   |  .  .  .  |    -+--+--+--+--+--+--+        |        |
   |  .  .  .  +  -  -  E  D (.) C  G (3) -  -  +  .  .  |
   |  .  .  .  c  .  v  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  +--B--+  .  .  c  .  .  .  .  .  .  .  .  | +--B--+ tactile button
   |  .  .  .  .  .  .  .  .  .  +Z-+  .  .  .  .  .  .  | +Z-+    buzzer
   +=====================================================+

		MSP430G2542

                 Vc Gnd
   +=====================================================+
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  c  .  .  .  .  .  .  .  .  .  .  | connect c to c
   |  .  v  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  ---+--+--+-(0)-A--F-(1)(2)-B--+--+--+  .  .  | connect v to v
   |       |- b6 b7 CK IO a7 a6 b5 b4 b3|       |        |
   |       |+ a0 a1 a2 a3 a4 a5 b0 b1 b2|       |        |
   |  .  .  --+--+--+--+--+--+--+--+--+-  .  .  |  .  .  |
   |  .  .  .  +  -  -  E  D (.) C  G (3) -  -  +  .  .  |
   |  .  .  v  .  c  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  +--B--+  .  .  .  .  .  .  .  .  .  .  .  .  | +--B--+ tactile button
   |  .  .  .  .  .  .  +Z-+  .  .  .  .  .  .  .  .  .  | +Z-+    buzzer
   +=====================================================+

*/
//  July 2010
//  code provided as is, no warranty
//
//  this is a hobby project, not a swiss time piece
//  you will be late for work or school if u rely on it
//
//  you cannot use code for commercial purpose w/o my permission
//  nice if you give credit, mention my site if you adopt 50% or more of my code
//
//  c chung (www.simpleavr.com)
//  Oct 3rd, 2010
//  . hidden egg function removed in this version
//  Oct 28t, 2010
//  . make changes to allow for CCS build under windows
//    detect mspgcc build via definition of MSP430
//  . CCS builds larger binaries, need to turn off temperature unit options
//    you have to choice imperial or metric temperature unit at build time
//    there may be ways to further optimize this but i don't know how
//  . CCS settings via menu Project->Properties->C/C++ Build->Basic Options
//    set --opt_level to 3
//    set --opt_for_speed to 0
//    if turn off debug will save u a few bytes
//  Oct 15t, 2012
//  . migrate to v1.5 lauchpad, provide alternate layout for 20pin devices
//  . comment / uncomment #define DEVICE_20P for 14pin and 20pin devices
//  . tried w/ msp430g2452
//  . linux build observe changes in mspgcc, i.e. #include <legacymsp430.h>, etc
//  . now autodect 32khz crystal, if present, will provide much more accurate time
//  . compile in linux (or windows w/ cygwin) like so
//    /cygdrive/c/mspgcc-20120406-p20120502/bin/msp430-gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=3p4w-clock.map,--cref -Wl,--relax -Wl,--gc-sections,--section-start=.text1=0x0da00,--section-start=.rodata1=0x0e000,--section-start=.rodata2=0x0e100,--section-start=.rodata3=0x0e200 -I/cygdrive/c/mspgcc-20120406-p20120502/bin/../msp430/include -mmcu=msp430g2452 -o 3p4w-clock.elf 3p4w-clock.c
//
//  . change mmcu to whatever device u are targeting
//
//  best view w/ vim "set ts=4 sw=4"
//******************************************************************************


#define TASSEL__ACLK	TASSEL_1
#define MC__UP			MC_1 

#define DEVICE_20P		// uncomment for 20 pin devices

#ifdef MSP430

#include <msp430.h>
#include <legacymsp430.h>

#include <stdint.h>
#include <stdio.h>
#define G2452
#define MHZ 1

#else

#include "signal.h"
#include  <msp430x20x2.h>

typedef unsigned char	uint8_t;
typedef unsigned int	uint16_t;
typedef int  			int16_t;

#define NO_TEMP_CHOICE
//#define METRIC_TEMP	// uncomment for oC

#endif

#ifdef DEVICE_20P

//____ this is the pin map for 20pin devices, i just shift the led module right 3 steps

#define SEG_A_P1	(1<<7)
#define SEG_B_P1	0x00
#define SEG_C_P1	0x00
#define SEG_D_P1	(1<<4)
#define SEG_E_P1	(1<<3)
#define SEG_F_P1	(1<<6)
#define SEG_G_P1	0x00
#define SEG_d_P1	(1<<5)
#define DIGIT_0_P1	(1<<1)
#define DIGIT_1_P1	0x00
#define DIGIT_2_P1	0x00
#define DIGIT_3_P1	0x00

#define SEG_A_P2	0x00
#define SEG_B_P2	(1<<3)
#define SEG_C_P2	(1<<0)
#define SEG_D_P2	0x00
#define SEG_E_P2	0x00
#define SEG_F_P2	0x00
#define SEG_G_P2	(1<<1)
#define SEG_d_P2	0x00
#define DIGIT_0_P2	0x00
#define DIGIT_1_P2	(1<<5)
#define DIGIT_2_P2	(1<<4)
#define DIGIT_3_P2	(1<<2)

#define BTNP_P1 	(1<<1)		// button uses p1.1
#define _P2DIR 		(1<<7)		// we want to try use 32khz crystal

#else

//____ this is the original pin map for 14pin devices

#define SEG_A_P1	0x00
#define SEG_B_P1	(1<<6)
#define SEG_C_P1	(1<<3)
#define SEG_D_P1	(1<<1)
#define SEG_E_P1	(1<<0)
#define SEG_F_P1	0x00
#define SEG_G_P1	(1<<4)
#define SEG_d_P1	(1<<2)
#define DIGIT_0_P1	0x00
#define DIGIT_1_P1	(1<<2)
#define DIGIT_2_P1	(1<<7)
#define DIGIT_3_P1	(1<<5)

#define SEG_A_P2	(1<<7)
#define SEG_B_P2	0x00
#define SEG_C_P2	0x00
#define SEG_D_P2	0x00
#define SEG_E_P2	0x00
#define SEG_F_P2	(1<<6)
#define SEG_G_P2	0x00
#define SEG_d_P2	0x00
#define DIGIT_0_P2	(1<<6)
#define DIGIT_1_P2	0x00
#define DIGIT_2_P2	0x00
#define DIGIT_3_P2	0x00

#define BTNP_P1 	(1<<2)		// button uses p1.2
#define _P2DIR 		0

#endif
// calcuate number of segments on individual digits, letters show
// will use to decide how long a digit / letter stays "on"
#define SEGS_STAY(v) \
   (((v & (1<<6)) ? 1 : 0) +\
	((v & (1<<5)) ? 1 : 0) +\
	((v & (1<<4)) ? 1 : 0) +\
	((v & (1<<3)) ? 1 : 0) +\
	((v & (1<<2)) ? 1 : 0) +\
	((v & (1<<1)) ? 1 : 0) +\
	((v & (1<<0)) ? 1 : 0)) | 0x20

// macro magic
// what the io ports output for individual digits / letters
// we do this at compile time so that we don't need to use runtime cycles
// to map segment and port pins
#define SEGS_PORT_DET(p, v) \
   (((v & (1<<6)) ? SEG_A_P##p : 0) |	\
	((v & (1<<5)) ? SEG_B_P##p : 0) |	\
	((v & (1<<4)) ? SEG_C_P##p : 0) |	\
	((v & (1<<3)) ? SEG_D_P##p : 0) |	\
	((v & (1<<2)) ? SEG_E_P##p : 0) |	\
	((v & (1<<1)) ? SEG_F_P##p : 0) |	\
	((v & (1<<0)) ? SEG_G_P##p : 0))

#define SEGS_PORT(v)	{SEGS_STAY(v),SEGS_PORT_DET(1, v),SEGS_PORT_DET(2, v)}
// port 1/2 pins used to turn segments on, led anodes
#define SEGS_1 (SEG_A_P1|SEG_B_P1|SEG_C_P1|SEG_D_P1|SEG_E_P1|SEG_F_P1|SEG_G_P1)
#define SEGS_2 (SEG_A_P2|SEG_B_P2|SEG_C_P2|SEG_D_P2|SEG_E_P2|SEG_F_P2|SEG_G_P2|SEG_d_P2)

// port 1/2 pins used turn digits on, led cathodes
#define DIGITS_1 (DIGIT_0_P1|DIGIT_1_P1|DIGIT_2_P1|DIGIT_3_P1)
#define DIGITS_2 (DIGIT_0_P2|DIGIT_1_P2|DIGIT_2_P2|DIGIT_3_P2)

// port 1/2 pins used
#define USED_1 (SEGS_1|DIGITS_1)
#define USED_2 (SEGS_2|DIGITS_2)

/*
       ___a__
      |      |
     f|      | b
       ___g__
     e|      | c
      |      |
       ___d__
*/
// composition of digits and letters we need
//_____________________ abc defg
#define LTR_0 0x7e	// 0111 1110
#define LTR_1 0x30	// 0011 0000
#define LTR_2 0x6d	// 0110 1101
#define LTR_3 0x79	// 0111 1001
#define LTR_4 0x33	// 0011 0011
#define LTR_5 0x5b	// 0101 1011
#define LTR_6 0x5f	// 0101 1111
#define LTR_7 0x70	// 0111 0000
#define LTR_8 0x7f	// 0111 1111
#define LTR_9 0x7b	// 0111 1011
#define BLANK 0x00	// 0000 0000
#define BAR_1 0x40	// 0100 0000
#define BAR_2 0x01	// 0000 0001
#define BAR_3 (BAR_1|BAR_2)
#define LTRdg 0x63	// 0110 0011
#define LTR_C 0x4e	// 0100 1110

#define LTR_c 0x4e	// 0000 1101
#define LTR_A 0x77	// 0111 0111
#define LTR_b 0x1f	// 0001 1111
#define LTR_J 0x3c	// 0011 1100
#define LTR_L 0x0e	// 0000 1110
#define LTR_S 0x5b	// 0101 1011
#define LTR_E 0x4f	// 0100 1111
#define LTR_t 0x0f	// 0000 1111
#define LTR_n 0x15	// 0001 0101
#define LTR_d 0x3d	// 0011 1101
#define LTR_i 0x10	// 0001 0000
#define LTR_H 0x37	// 0011 0111
#define LTR_r 0x05	// 0000 0101
#define LTR_o 0x1d	// 0001 1101
#define LTR_f 0x47	// 0100 0111
#define LTRml 0x66	// 0110 0110
#define LTRmr 0x72	// 0111 0010
#define LTR__ 0x00	// 0000 0000

// port io values for individual digits / letters
// 1st byte cycles to stay
// 2nd byte port 1 value
// 3rd byte port 2 value
static const uint8_t digit2ports[][3] = { 
	SEGS_PORT(LTR_0), SEGS_PORT(LTR_1), SEGS_PORT(LTR_2), SEGS_PORT(LTR_3),
	SEGS_PORT(LTR_4), SEGS_PORT(LTR_5), SEGS_PORT(LTR_6), SEGS_PORT(LTR_7),
	SEGS_PORT(LTR_8), SEGS_PORT(LTR_9), SEGS_PORT(BLANK), SEGS_PORT(LTR_o),
	SEGS_PORT(BAR_2), SEGS_PORT(LTR_b), SEGS_PORT(LTR_f), SEGS_PORT(LTR_C), 
	SEGS_PORT(LTR_t), SEGS_PORT(LTR_H), SEGS_PORT(LTR_n), SEGS_PORT(LTR_r), 
	SEGS_PORT(LTR_J), SEGS_PORT(LTR_d), SEGS_PORT(LTR_L), SEGS_PORT(LTR_i),
	SEGS_PORT(LTR_E), SEGS_PORT(LTR_A),
};

// digits / letters we are using
enum {
	POS_0, POS_1, POS_2, POS_3, POS_4, POS_5, POS_6, POS_7,
	POS_8, POS_9, POS__, POS_o, POSb2, POS_b, POS_f, POS_C, 
	POS_t, POS_H, POS_n, POS_r, POS_J, POS_d, POS_L, POS_i,
	POS_E, POS_A,
};

// menu display
static const uint16_t menu_desc[] = { 
	(POS__<<10)| (POS__<<5)| POS__,		// blank
	(POS_5<<10)| (POS_E<<5)| POS_t,		// time, HHMM

	(POS_A<<10)| (POS_L<<5)| POS_r,		// alarm, HHMM
	(POS_C<<10)| (POS_n<<5)| POS_t,		// count down, MMSS
	(POS_A<<10)| (POS_d<<5)| POS_J,		// clock adjust, xxGG, gain unit in 0.5% clk
	(POS_b<<10)| (POS_r<<5)| POS_i,		// dimmer, not saved in eeprom

	(POS_1<<10)| (POS_2<<5)| POS_H,
	(POS_2<<10)| (POS_4<<5)| POS_H,

	(POS_o<<10)| (POS_f<<5)| POS_f,
	(POS__<<10)| (POS_0<<5)| POS_n,
#ifndef NO_TEMP_CHOICE
	(POS__<<10)| (POS_o<<5)| POS_C,
	(POS__<<10)| (POS_o<<5)| POS_f,
#endif
};

#define LONG_HOLD	2000

#define ST_HOLD		0x80
#define ST_PRESSED	0x40
#define ST_BUTTON   (ST_HOLD|ST_PRESSED)
#define ST_FREE1 	0x20
#define ST_FREE2 	0x10
#define ST_REFRESH	0x08
#define ST_BUZZ     0x04
#define ST_SETUP   	0x03



#define BUZZ_PINP	(1<<4)
#define BUZZ_PINN	(1<<3)

// storage for multiplexing, 3 bytes output x 4 digits
uint8_t output[3 * 4];
// stacked seconds for advancing clock
volatile uint8_t stacked=0;

// loads data into output[] for immediate led multiplexing
//__________________________________________________
uint16_t seg2port(uint16_t data, uint8_t type) {

	//____________ dictates how data is selected, based on type
	static const uint8_t factor[][4] = { 
		{ 16, 10,  6, 10 },			// clock mode
		{ 16, 10, 10, 16 },			// temperature mode
		{ 32, 32, 32, 32 },			// menu mode
	};

	uint16_t adv = type & 0x0f;		// advance for numerics
	uint16_t cue = 1, res = 0;
	type >>= 4;						// type of data

	uint8_t which = 3;
	while (1) {
		uint8_t *pp = output + which * 3;
		uint8_t offset = 3;

		uint8_t facto = factor[type][which];
		uint8_t digit = data % facto;
		if (adv && adv == which) {
			digit++;
			if (digit >= facto) digit = 0;
		}//if
		if (data) data /= facto;
		res += digit * cue;
		cue *= facto;
		if (which==0 && digit==0) digit = POS__;
		do {
			*pp++ = digit2ports[digit][--offset];
		} while (offset);
		if (!which--) break;
	}//while
    return res;

}

//______________________________________________________________________
//uint8_t scan(uint8_t);
volatile uint8_t opts= 0;
volatile uint8_t rounds = 122-1;
volatile uint8_t ticks=0;
uint8_t stays=1;
uint8_t tps = 0;
uint8_t menu_attr = 0;
//______________________________________________________________________
void main(void) {

	//____________ menu attribute TTTDD,
	//             TTT toggle option offset/2
	//             DD  digit setup pos, 
	static const uint8_t menu_attrs[] = { 0x00, 
		(3<<2)|(1),	// time
		(4<<2)|(1),	// alarm
		(0<<2)|(1),	// count down
#ifdef NO_TEMP_CHOICE
		(0<<2)|(2),	// clock gain, no choice for temp units C/F
#else
		(5<<2)|(2),	// clock gain
#endif
		(0<<2)|(3),	// dimmer, shorted
	}; 

	WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;	// stop WDT, enable NMI hi/lo

	uint8_t use_32khz = 1;		// assume we have crystal 1st

	BCSCTL1 = CALBC1_1MHZ;		// Set DCO to 1MHz
	DCOCTL = CALDCO_1MHZ;
	FCTL2 = FWKEY + FSSEL0 + FN1;	// MCLK/3 for Flash Timing Generator

	CCR0 = 4096-1;
	CCTL0 = CCIE;					// CCR0 interrupt enabled
	TACTL = TASSEL__ACLK + MC__UP + TACLR;	// ACLK, upmode
	WDTCTL = WDT_MDLY_8 + WDTNMI + WDTNMIES;	// WDT at 8ms, NMI hi/lo

	while (!tps) {
		IE1 |= WDTIE;					// Enable WDT interrupt
		_BIS_SR(GIE);					// enable interrupt

		// wait for VLO calibration be done
		// timer interrupt to reduce rounds from 122 to 0
		while (rounds);

		if (tps) break;					// works, must have 32khz crystal
		//______ couldn't get tps, now fallback to use VLO
		BCSCTL3 |= LFXT1S_2;			// use VLO as ACLK, ~12Khz
		CCR0 = 64-1;
		//_______ VLO calibration, 
		// WDT counts 122x8ms (on 1 Mhz MCLK) times and see how many ACLK counts per sec
		// now we got tps as ACLK/64 counts in one sec
		// for 1/8 sec per interrupt we will need to set CCR0 to tps * 8
		// next round up to calibrate VLO
		use_32khz = 0;
	}//while


	// save ticks per second value for later setting
	uint8_t use_tps = tps;

	uint16_t time[6];
	char *flash = (char*) 0x1040;
	char *src   = (char*) time;
	uint8_t i=0;

	// read configuration from flash
	for (i=0;i<12;i++) {
		if (*flash != 0xff) *src = *flash;
		src++;
		flash++;
	}//for

	//________ temperature sensing setup G2231 only
	ADC10CTL1 = INCH_10 + ADC10DIV_3;         // Temp Sensor ADC10CLK/4
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;

	uint16_t mode4 = 0;
	uint16_t *show;
	uint8_t sec=45;

	P1SEL = 0;
#ifdef DEVICE_20P
	P2SEL = (1<<6)|(1<<7);		// will use crystal
#else
	P2SEL = 0;					// use all pins for IO
#endif

	uint8_t menu=1;
	uint8_t mode=1;
	uint8_t state=ST_HOLD;
	uint8_t setup=4;
	while (1) { 

		if (!menu && stacked) {
			while (stacked) {
				stacked--;
				if (++sec >= 60) {
					sec = 0;
					time[1]++;
					//________ alarm ?
					if ((*time & (1<<2)) && time[1] == time[2]) {
						state |= (ST_BUZZ|ST_REFRESH);
						mode = 1;
					}//if
				}//if
			}//while
			if ((mode != 1) || !sec) state |= ST_REFRESH;
#ifdef DEVICE_20P
			*(output + (1*3+1)) &= ~SEG_d_P1;
			if (use_32khz)
			if ((mode==1) && (sec&0x01)) *(output + (1*3+1)) |= SEG_d_P1;
#endif
		}//if

		uint8_t adv = 0;
		uint8_t txt = 0;
		//_____________________________________ check input
		if (state & ST_BUTTON) {	// needs attention
			if ((state & ST_BUZZ) || !mode) {	// stop alarm
				state &= ~(ST_BUZZ|ST_BUTTON);
				mode = 1;
				state |= ST_REFRESH;
				continue;
			}//if

			if (setup) {
				if (state & ST_HOLD) {
					switch (setup) {
						case 3: 
							menu_attr >>= 2;
							if (menu_attr) {
								//______ on to options toggle
								state |= ST_PRESSED;
								setup++;
								break;
							}//if
						case 4:
							{
							//___________ done, flash
							char *flash = (char*) 0x1040;
							char *src   = (char*) time;
							FCTL1 = FWKEY + ERASE;
							FCTL3 = FWKEY;
							*flash = 0x0000;
							FCTL1 = FWKEY + WRT;
							for (i=0;i<12;i++) *flash++ = *src++;
							FCTL1 = FWKEY;
							FCTL3 = FWKEY + LOCK;
							//_________ load fresh parameters
							// you can save a lot by omitting boundary checks
							//time[4] %= 100;
							//time[5] %= 10;
							time[4] &= 0x003f;
							time[5] &= 0x0007;
							if (!use_32khz)
								CCR0 = (use_tps-time[4])*8;
							CCTL0 = CCIE;					// CCR0 interrupt enabled
							TACTL = TASSEL__ACLK + MC__UP + TACLR;	// ACLK, upmode
							switch (menu) {
								case 3:
									mode4 = *show;
									mode = 4;
									break;
								default:
									mode = 1;
									break;
							}//switch
							show = time + 1;
							menu = 0;
							setup = 0;
							break;
							}
						default:
							setup++;
							break;
					}//switch
				}//if
				if (state & ST_PRESSED) {
					if (setup == 4) {
						if (!(state & ST_HOLD))
							*time ^= 1<<menu;
						txt = (menu_attr<<1) + ((*time & 1<<menu) ? 1 : 0);
					}//if
					else {
						adv = setup;
					}//else
				}//if
			}//if
			else {
				if (state & ST_PRESSED) {
					if (menu) {
						//_______ menu selected
						//mode = 1;
						show = time + menu;
						setup = menu_attr & 0x03;
					}//if
					else {
						//_______ advance display mode
						mode++; 
						mode &= 0x03;
						seg2port(menu_desc[0], 0x20);
						stays = 0;
					}//else
				}//if
				else {
					//_______ advance menu
					if (mode) {
						//_________ not option, not edit, advance menu
						menu++;
						//___________ see if we need alarm function (buzzer attached?)
						while (opts & (1<<4) && (menu & 0x02)) menu++;
						if (menu > 5) 
							menu = 0;
						else
							txt = menu;
						menu_attr = menu_attrs[menu];
						mode = 1;
					}//if
				}//else
			}//else
		}//if

		if (state&(ST_REFRESH|ST_BUTTON) && !stays) {
			state &= ~(ST_REFRESH|ST_BUTTON);
			if (txt) {
				seg2port(menu_desc[txt], 0x20);
			}//if
			else {
				if (adv == 1) {
					*show += 60;
					adv = 0;
				}//if
				if (*show>=(24*60)) *show -= 24*60;
				switch (mode) {
					case 1:		// hour + min
						*show = seg2port(*show, adv);
						if (!(*time & (1<<1)) && menu < 3) {	// 12HR display
							uint16_t hhmm = *show;
							if (hhmm>=(12*60)) {
								//__________ 12Hr mode, pm indicator
								adv = 1;
								hhmm -= 12*60;
							}//if
							if (hhmm<60) hhmm += 12*60;
							seg2port(hhmm, 0);
						}//if
						break;
					case 2:		// alarm on/off + seconds
						//seg2port(sec+(POS__*100) + ((*time & (1<<2)) ? POSb1*1600 : 0), 0x10);
						seg2port(sec, 0);
						if (*time & (1<<2)) adv = 1;
						break;
					case 3:
						{ // temperature
						int16_t temp = ADC10MEM;
						if (temp) {
							// oF = ((A10/1024)*1500mV)-923mV)*1/1.97mV = A10*761/1024 - 468
							// oC = ((A10/1024)*1500mV)-986mV)*1/3.55mV = A10*423/1024 - 278
							//temp = ((temp - 630) * 761) / 1024; oF
							//temp = ((temp - 673) * 423) / 1024; C   0 offset
							//temp = ((temp - 552) * 423) / 1024; C -50 offset
							// 1.464mv/u 3.55
#ifdef NO_TEMP_CHOICE
							temp = ((temp - 673) * 423) / 1024;
#ifndef METRIC_TEMP
							temp = (temp * 10 / 8) + 32;
#endif
							temp <<= 4;
#ifdef METRIC_TEMP
							if (temp < 0) {
								temp = -temp;
								temp += POSb2 * 1600;
							}//if
#endif
#else
							temp = ((temp - 673) * 423) / 1024;
							if (*time & (1<<4))		// imperial temperature
								temp = (temp * 10 / 8) + 32;
							temp <<= 4;
							//
							if (temp < 0) {
								temp = -temp;
								temp += POSb2 * 1600;
							}//if
#endif

#ifdef NO_TEMP_CHOICE
#ifdef METRIC_TEMP
							seg2port(temp|POS_C, 0x10);
#else
							seg2port(temp|POS_f, 0x10);
#endif
#else
							seg2port(temp|((*time & (1<<4))?POS_f:POS_C), 0x10);
#endif
						}//if
						ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
						__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
						}
						break;
					case 0:			// blank
						_BIS_SR(LPM3_bits + GIE);	// sleep
					case 5:			// blank
						state |= ST_REFRESH;
						break;
					case 4:		// counter
						seg2port(mode4, 0);
						if (mode4) mode4--;
						else	   state |= ST_BUZZ;
					default:
						break;
				}//switch
				if (adv) *(output + (3*3+1)) |= SEG_d_P1;
			}//else
		}//if

		if (state & ST_BUZZ) {
			if (ticks & 0x02) {
				P2DIR  = _P2DIR;
				P1DIR  = BUZZ_PINP|BUZZ_PINN;
				P1OUT ^= BUZZ_PINP; 	// toggle buzzer
			}//if
		}//if

		// allow led segments stays on
		if (stays & 0x0f) { stays--; continue; }

		// time[5] contains dimmer value
		stays >>= time[5];
		// turn off all io pins, led blanks
		P2DIR = _P2DIR;
		P1DIR = 0;
		P2OUT = 0;
		P1OUT = 0;

		// allow led segments stays blank out, dimming
		if (stays) { stays--; continue; }

		P1REN = BTNP_P1;
		if (mode) {
			//____________ only if we are not asleep
			P1DIR = BUZZ_PINN;
			P1OUT = BUZZ_PINP;
			P1REN |= BUZZ_PINP;

			// detects buzzer and show alarm setting menu
			if ((opts & BUZZ_PINP) && !(P1IN & BUZZ_PINP)) {
				menu = 1;
				state |= ST_HOLD;
			}//if
			// store port 1 pin status
			opts = P1IN;
		}//if

		//___________ check button
		//            button must be position at P1.2 as it's tied to RESET pin
		//            we need it be pressed after power's been applied (boot)
		uint16_t wait=0;
		do {
			if (wait == 0x0040) {
				state |= ST_PRESSED;
			}//if
			else {
				if (wait++ > 0x6000) {
					state &= ~ST_PRESSED;
					state |= ST_HOLD;
					//if (wait&0x0f) P1DIR ^= 0x30;
#ifdef DEVICE_20P
					P2DIR ^= 0x06;
#else
					P1DIR ^= 0x30;
#endif
				}//if
			}//else
			wait++;
		} while (P1IN & BTNP_P1);
		P1REN = 0; //P1DIR = 0; P1OUT = 0;

		if ((state & ST_BUTTON) || !mode) continue;

		static uint8_t pos=0; 
		static const uint8_t digit_map1[] = { DIGIT_0_P1, DIGIT_1_P1, DIGIT_2_P1, DIGIT_3_P1, };
		static const uint8_t digit_map2[] = { DIGIT_0_P2, DIGIT_1_P2, DIGIT_2_P2, DIGIT_3_P2, };

		// flasher during individual digit setup
		//___________ load segments and turn on 1 of 4 digits
		//uint8_t setup = state & ST_SETUP;
		uint8_t *ioptr = output + (pos*3);
		if (!setup || !(ticks & 0x02) || (pos != setup && (pos|setup) != 0x01)) {
			// use this (w/ negate) if led is common anode
			//P2OUT = ~(*ioptr & ~digit_map2[pos]);
			P2OUT = *ioptr & ~digit_map2[pos];
			P2DIR = (*ioptr++ | digit_map2[pos]) | _P2DIR;
			// use this (w/ negate) if led is common anode
			//P1OUT = ~(*ioptr & ~digit_map1[pos]);
			P1OUT = *ioptr & ~digit_map1[pos];
			P1DIR = *ioptr++ | digit_map1[pos];
			stays = *ioptr;
		}//if

		pos++;
		pos &= 0x03;

	}//while

}

//______________________________________________________________________
#ifdef MSP430
interrupt(ADC10_VECTOR) ADC10_ISR(void) {
#else
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
#endif
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

//______________________________________________________________________
#ifdef MSP430
interrupt(TIMER0_A0_VECTOR) TIMER0_A0_ISR(void) {
#else
#pragma vector=TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void) {
#endif
	ticks++;
	if (!(ticks&0x07)) stacked++;
	_BIC_SR_IRQ(LPM3_bits);
}

//______________________________________________________________________
#ifdef MSP430
interrupt(WDT_VECTOR) WDT_ISR(void) {
#else
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void) {
#endif
	rounds--;
	if (!rounds) {
		tps = ticks;
		IE1 &= ~WDTIE;				// Disable WDT interrupt
		//WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;
	}//if
}

