//******************************************************************************
//  MSP431F20xx Demo - led multiplexing counter
//
//  Description; 
//
//  ACLK = 12khz, MCLK = SMCLK = default DCO
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
   |  .  .  .  .  .  .  .  +--Z--+z-+  .  .  .  .  .  .  | +--Z---+z-+    buzzer (two positions)
   +=====================================================+ Z for tune player, z for alarm


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
//  Nov 15t, 2010
//  . hidden egg uncovered by jbremnamt
//
//  best view w/ vim "set ts=4 sw=4"
//******************************************************************************

#include "signal.h"
#include  <msp430x20x2.h>

#ifdef MSP430
#else

#define TASSEL__ACLK	TASSEL_1
#define MC__UP			MC_1 

typedef unsigned char	uint8_t;
typedef unsigned int	uint16_t;
typedef int  			int16_t;

#endif

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
// to map segment and port pins during runtime
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

	(POS__<<10)| (POS_o<<5)| POS_C,
	(POS__<<10)| (POS_o<<5)| POS_f,
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
#define CNTR_PINP	(1<<1)
#define CNTR_PINN	(1<<3)

#define OPT_ALARM	4
#define OPT_CNTR	1

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

#define TONE
#ifdef TONE
//______________________________________________________________________
// sing a song
enum {	p, dum, dumdum,
	a3, gx2,g2, fx2,f2, e2, dx2,d2,
	cx2,c2, b2, ax2,a2, gx1,g1, fx1,
	f1, e1, dx1,d1, cx1,c1, b1, ax1,
	a1, gx0,g0, fx0,f0, e0, dx0,d0,
	cx0,c0, b0, ax0,a0, };

#define DUR1	1
#define DUR2	2
// frequency values to load into timer
static const uint8_t tune_map[] = {
	0x0f, 0xf1, 0x22, 0x23, 0x22, 0x33, 0x33, 0x34, 0x44, 0x45, 
	0x55, 0x56, 0x66, 0x77, 0x88, 0x99, 0x9a, 0xbb, 0xcd, 0xee,
};

// Fuer Elise, should really find a good way to put these into data flash
// but couldn't do it in mspdebug in a go
// you may want to have a seperate firmware to load the song so
// u can free up program space for other things
static const uint8_t tune[] = {
	(e1<<2)|DUR2, (dx1<<2)|DUR2, (e1<<2)|DUR2, (dx1<<2)|DUR2,
	(e1<<2)|DUR2, (b0<<2)|DUR2,  (d1<<2)|DUR2, (c1<<2)|DUR2, (a0<<2)|DUR1, (p<<2)|DUR2,
	(c0<<2)|DUR2, (e0<<2)|DUR2,  (a0<<2)|DUR2, (b0<<2)|DUR1, 
	(p<<2)|DUR2,  (e0<<2)|DUR2,  (gx0<<2)|DUR2,(b0<<2)|DUR2, (c1<<2)|DUR1, (p<<2)|DUR2, (e0<<2)|DUR2,
	(e1<<2)|DUR2, (dx1<<2)|DUR2, (e1<<2)|DUR2, (dx1<<2)|DUR2,(e1<<2)|DUR2,
	(b0<<2)|DUR2, (d1<<2)|DUR2,  (c1<<2)|DUR2, (a0<<2)|DUR1, (p<<2)|DUR2,  (c0<<2)|DUR2,
	(e0<<2)|DUR2, (a0<<2)|DUR2,  (b0<<2)|DUR1, (p<<2)|DUR2,  
	(e0<<2)|DUR2, (c1<<2)|DUR2,  (b0<<2)|DUR2, (a0<<2)|DUR1,
	(p<<2)|DUR1,
	0,
};
//______________________________________________________________________
/* we are now doing it inside the main loop, tight on flash
void init_tone_player() {
	_BIC_SR(GIE);
	P2OUT = 0;
	P2DIR = 0;
	P1SEL = CNTR_PINP;
	P1DIR = CNTR_PINP|CNTR_PINN;
	P1OUT = 0x00;
	//________________ now we have to use 1Mhz clock for tone playing
	//________________ set up timera comparor toggle output pin P1.1 (buzzer)
	CCTL0 = OUTMOD_4;
	CCR0  = 0;
	TACTL = TASSEL__SMCLK + ID_3 + MC__UP;	// SMCLK/8, upmode
	//________________ wdt timer will be used for tone duration + time keep (kind of)
    WDTCTL = WDT_MDLY_32 + WDTNMI + WDTNMIES;	// WDT at 8ms, NMI hi/lo
	IE1 |= WDTIE;					// Enable WDT interrupt
	_BIS_SR(GIE);
}
 */
#endif
//______________________________________________________________________
volatile uint8_t opts= 0;
volatile uint8_t rounds = 122-1;
volatile uint8_t ticks=0;
uint8_t stays=1;
uint8_t tps = 0;
uint8_t menu_attr = 0;
//______________________________________________________________________
void main(void) {

	//____________ menu attribute TTTDDCCC, 
	//             TTT toggle option offset/2
	//             DD  digit setup pos, 
	static const uint8_t menu_attrs[] = { 0x00, 
		(3<<2)|(1),	// time
		(4<<2)|(1),	// alarm
		(0<<2)|(1),	// count down
		(5<<2)|(2),	// clock gain
		(0<<2)|(3),	// dimmer, shorted
	}; 

	WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;	// stop WDT, enable NMI hi/lo

	BCSCTL3 |= LFXT1S_2;			// use VLO as ACLK, ~12Khz
	//if (CALBC1_1MHZ != 0xff && CALDCO_1MHZ != 0xff) {  
		BCSCTL1 = CALBC1_1MHZ;		// Set DCO to 1MHz
		DCOCTL = CALDCO_1MHZ;
	//}//if 
	FCTL2 = FWKEY + FSSEL0 + FN1;	// MCLK/3 for Flash Timing Generator

	//_______ VLO calibration, 
	// WDT counts 122x8ms (on 1 Mhz MCLK) times and see how many ACLK counts per sec
	// now we got tps as ACLK/64 counts in one sec
	// for 1/8 sec per interrupt we will need to set CCR0 to tps * 8
	CCR0  = 64-1;					// we want one count every 64 clock cycle
	CCTL0 = CCIE;					// CCR0 interrupt enabled
	TACTL = TASSEL__ACLK + MC__UP;	// ACLK, upmode

    WDTCTL = WDT_MDLY_8 + WDTNMI + WDTNMIES;	// WDT at 8ms, NMI hi/lo
	IE1 |= WDTIE;					// Enable WDT interrupt
	_BIS_SR(GIE);					// enable interrupt

	// wait for VLO calibration be done
	// timer interrupt to reduce rounds from 122 to 0
	while (rounds);

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

	uint8_t const *mode5 = (uint8_t *) 0;
	uint16_t mode4 = 0;
	uint16_t *show;
	uint8_t sec=45;

start:
	P1SEL = 0;
	P2SEL = 0;

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
							// only if you are insecure and want to waste 12 bytes, i am
							FCTL1 = FWKEY;
							FCTL3 = FWKEY + LOCK;
							//_________ load fresh parameters
							// you can save a lot by omitting boundary checks
							//time[4] %= 100;
							//time[5] %= 10;
							//time[4] &= 0x003f;
							//time[5] &= 0x0007;
							CCR0 = (use_tps-time[4])*8;
							//CCR0 = use_tps * 8 - time[4];
							//CCR0 = (use_tps << 3) - time[4];
							CCTL0 = CCIE;					// CCR0 interrupt enabled
							TACTL = TASSEL__ACLK + MC__UP;	// ACLK, upmode
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
					//if (mode == 1) {
					if (mode) {
						//_________ not option, not edit, advance menu
						menu++;
						//___________ detect option conditions
						//while ((menu_attrs[menu]&0x07) && opts&(1<<(menu_attrs[menu]&0x07))) menu++;
						//while (opts & (1<<4) && ((menu == 2) || (menu == 3))) menu++;
						while (opts & (1<<4) && (menu & 0x02)) menu++;
						if (menu > 5) 
							menu = 0;
						else
							txt = menu;
						menu_attr = menu_attrs[menu];
						mode = 1;
						//menu_attr = menu_attrs[menu]>>3;
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
				switch (mode) {
					case 1:		// hour + min
						if (adv == 1) {
							*show += 60;
							adv = 0;
						}//if
						while (*show>=(24*60)) *show -= 24*60;
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
						temp = ((temp - 673) * 423) / 1024;
						if (*time & (1<<4))		// imperial temperature
							temp = (temp * 10 / 8) + 32;
						temp <<= 4;
						//
						if (temp < 0) {
							temp = -temp;
							temp += POSb2 * 1600;
						}//if
						seg2port(temp|((*time & (1<<4))?POS_f:POS_C), 0x10);
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


		//
		if (state & ST_BUZZ) {
			if (ticks & 0x02) {
				P2DIR  = 0x00;
				P1DIR  = BUZZ_PINP|BUZZ_PINN;
				P1OUT ^= BUZZ_PINP; 	// toggle buzzer
			}//if
		}//if
		//
		//
		if (stays & 0x0f) { stays--; continue; }

		//stays >>= *time>>8;
		stays >>= time[5];
		//
		P2DIR = 0;
		P1DIR = 0;
		P2OUT = 0;
		P1OUT = 0;
		//

		if (stays) { stays--; continue; }
		//
		//if (mode == 5) continue;

		P1REN = 1<<2;
		if (mode) {
			P1DIR = BUZZ_PINN|CNTR_PINN;
			P1OUT = BUZZ_PINP|CNTR_PINP;
			P1REN |= BUZZ_PINP|CNTR_PINP;

			if ((opts & BUZZ_PINP) && !(P1IN & BUZZ_PINP)) {
				menu = 1;
				state |= ST_HOLD;
			}//if
			opts = P1IN;
			if (!(opts & CNTR_PINP)) {
				if (mode5) {
					if (!*mode5) mode5 = tune;
					CCR0  = 0;
					rounds = 16;
					if (*mode5&0x02) rounds = 8;
					i = *mode5>>2;
					uint8_t const *v = tune_map;
					while (i) {
						CCR0 += *v>>4;
						if (!(--i)) break;
						CCR0 += *v&0x0f;
						v++;
						i--;
					}//while
					IE1 |= WDTIE;
					P1DIR = CNTR_PINP|CNTR_PINN;
					P1SEL = CNTR_PINP;
					P1OUT = 0; P1REN = 0;
					while (rounds);
					mode5++;
					continue;
				}//if
				else {
					//________________ now we have to use 1Mhz clock for tone playing
					//________________ set up timera comparor toggle output pin P1.1 (buzzer)
					//_BIC_SR(GIE);
					CCTL0 = OUTMOD_4;
					TACTL = TASSEL__SMCLK + ID_3 + MC__UP;	// SMCLK/8, upmode
					//________________ wdt timer will be used for tone duration + time keep (kind of)
					WDTCTL = WDT_MDLY_32 + WDTNMI + WDTNMIES;	// WDT at 8ms, NMI hi/lo
					IE1 |= WDTIE;					// Enable WDT interrupt
					//_BIS_SR(GIE);
					mode5 = tune;
				}//else
			}//if
			else {
				if (mode5) {
					mode5 = 0;
					goto start;
				}//if
			}//else
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
					P1DIR ^= 0x30;
				}//if
			}//else
			wait++;
		} while (P1IN & (1<<2));
		P1REN = 0; //P1DIR = 0; P1OUT = 0;

		if ((state & ST_BUTTON) || !mode) continue;

		static uint8_t pos=0; 
		static const uint8_t digit_map1[] = { DIGIT_0_P1, DIGIT_1_P1, DIGIT_2_P1, DIGIT_3_P1, };
		static const uint8_t digit_map2[] = { DIGIT_0_P2, DIGIT_1_P2, DIGIT_2_P2, DIGIT_3_P2, };

		// flasher during individual digit setup
		//___________ load segments
		//uint8_t setup = state & ST_SETUP;
		uint8_t *ioptr = output + (pos*3);
		if (!setup || !(ticks & 0x02) || (pos != setup && (pos|setup) != 0x01)) {
			P2OUT = *ioptr & ~digit_map2[pos];
			P2DIR = *ioptr++ | digit_map2[pos];
			P1OUT = *ioptr & ~digit_map1[pos];
			P1DIR = *ioptr++ | digit_map1[pos];
			stays = *ioptr;
		}//if

		pos++;
		pos &= 0x03;

	}//while

}

//______________________________________________________________________
interrupt(ADC10_VECTOR) ADC10_ISR(void) {
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

//______________________________________________________________________
interrupt(TIMERA0_VECTOR) Timer_A(void) {
	ticks++;
	if (!(ticks&0x07)) stacked++;
	_BIC_SR_IRQ(LPM3_bits);
}

//______________________________________________________________________
interrupt(WDT_VECTOR) wdt_(void) {
	rounds--;
	if (!rounds) {
		tps = ticks;
		IE1 &= ~WDTIE;				// Disable WDT interrupt
		//WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;
	}//if
}

