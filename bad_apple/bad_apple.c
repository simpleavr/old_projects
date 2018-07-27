#include <signal.h>
#include <stdint.h>
#include <stdio.h>
//  November 2011
//  code provided as is, no warranty
//
//  this is a hobby projec


//  credits 1st
//  . additional credits included in individual header files
//    take a look inside them. ex. mmc/sd access, lcd control, etc
//  . polyphonic tune player taken from this pic project
//    http://www.pic24.ru/doku.php/en/osa/articles/pk2_osa_piano
//    thank you http://www.pic24.ru

//  you cannot use code for commercial purpose w/o my permission
//  nice if you give credit, if you adopt a large portion of my code
//
//  code developed and tested under mspgcc4 in ubuntu and win7 + cygwin environment
//  iar and ccs users please make appropriate changes yourself
//
//  c chung (www.simpleavr.com)
//  November 1, 2011
//  . this project has been test built on f2012,g2231,g2452,g2553
//  . use included zcontent.pl to generate contents on video + sound
//  . see zcontent.pl for more detail and options regarding contents
//  . we used dco stored calibrated values, make sure u have the 16Mhz one
//
/*
  the following is the most basic schematic on a 2k device, i use a F2012 but it will work fine w/ the
  G2231/G2211 that comes w/ the launchpad. this can be breadboarded w/o too much trouble
  you can have different layouts / pin mappings or different devices. we just need enough io pins for
  2 spi interfaces (some pins can be shared) and a timerA
                                                                         
               MSP430F2012         ----------+---------------       ----
             -----------------     |         |              |      /    \
         /|\|              XIN|-   |        ---             |      |    |
          | |                 |   .-.       --- 10-500nF    o      \    /
          --|RST          XOUT|-  | | 1-10K  |            Audio out o  o
            |                 |   |_|        |--------------o
      LED <-|P1.0             |    |        ___                               
            |                 |    |        ///    SD / MMC card       /|\
            |    pwm out  P2.6|-----           ----------------------   |
            |                 |               |                 Vcc 4|---
            |    as CS    P1.7|-------------->|1 CS, select device   |
            |    as MOSI  P1.6|<--------------|7 MISO, data out      |
            |                 |   ----------->|2 MOSI, data in       |
            |                 |   | --------->|5 CLK, spi clock      |
            |                 |   | |         |                 Gnd 3|---
            |                 |   | |          ----------------------   |
            |                 |   | |                                  ---
            |                 |   | |           Nokia 5110 breadout    ///
            |                 |   | |   ----------------------------- 
            |    as SCLK  P1.1|---|-+->|7 SCLK, spi clock            |
            |    as MISO  P1.2|---+--->|6 MOSI, data in              |
            |    as D/C   P1.3|------->|5 D/C, data or control       |
            |    as RST   P1.4|------->|4 RST, reset, low active     |
            |    as SCE   P1.5|------->|3 SCE, select , low active   | /|\
            |                 |    ----|2 Gnd                        |  |
            |                 |    |   |                        Vcc 1|---
                                  ---   -----------------------------
                                  ///

          7 2     1   6		map to port1 i/o
	    __________________
       /  1 2 3 4 5 6 7 8 |
       |  S O - + C   I   | SD/MMC

            7 2   1   6		map to port1 i/o
	    __________________
       /  1 2 3 4 5 6 7 8 |
       |    S O + C - I   | Micro-SD

*/
//#define LCD_7110

#ifdef LCD_7110
	#define MMC_BLOCK_SIZE	32		// can have larger ram blocks
	#include "param_7110.h"
	#define G2452
#else
	#define MMC_BLOCK_SIZE	16		// will read 16 bytes / block from sd card
	#include "param_5110.h"
	#define G2231

#endif

#define MHZ     16

//#define G2231
//#define G2452
//#define G2553


#include "common.h"
#include "mmc.h"
#include "nokia_lcd.h"

typedef struct {
	uint16_t	_freq;
	uint16_t	_phase;
} soundT;

//________ my own less accurate, less storage frequency table
const uint8_t notes[] = { 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, };
volatile soundT ch[MAX_CH];

volatile uint8_t clicks=0,ticks=0,flip=0;
//______________________________________________________________________
int main() {
	WDTCTL = WDTPW + WDTHOLD;
	__use_cal_clk(MHZ);

	_BIC_SR(GIE);

	//__________________ pwm tone playing setup
	CCTL0 = CCIE;
	CCTL1 = OUTMOD_7;
	CCR0 = 0x400;							// more like 16Khz (16Mhz/1024)
	CCR1 = 0;
	TACTL = TASSEL_2 + MC_1;

	P1SEL = 0;


#ifdef IO_POWER
	P1OUT = BIT6;
	P1DIR = BIT6;
	P2SEL = BIT6;
	P2DIR = BIT0|BIT1|BIT5|BIT6;		// power lcd/sd card thru io pins 
	P2OUT = BIT1;
#else
	P1OUT = 0;
	P1DIR = 0;
	P2SEL = BIT6;
	P2OUT = 0;
	P2DIR = BIT6;
#endif

	led_init();
	led_off();

    lcd_init();

	uint16_t buf16[MMC_BLOCK_SIZE];
	uint8_t  buf8[MMC_BLOCK_SIZE];

	//________ trap w/ led on if mmc / sd fails
	if (mmc_init()) while (1) led_on();
	
	// enclosed used for mmc / sd card debugging
	//____ read signature
	//uint16_t buf16[64];
	//if (mmc_readsector(0, (uint8_t *) buf16)) { while (1) led_on(); }//if
	//__blink_debug(0);
	//if (buf16[0] != 0x81f8) { while (1) led_on(); }//if
	//__blink_debug(0);
	//

	_BIS_SR(GIE);

	uint32_t snd_sector=SOUND_START*(64/MMC_BLOCK_SIZE);
	uint8_t  snd_offset=MMC_BLOCK_SIZE;

	uint8_t  vid_offset=MMC_BLOCK_SIZE;
	uint32_t vid_sector=0;
	uint16_t dot_cnt=0;

	while (1) {
		if (!flip) {
			if (!(vid_sector%30)) led_on();		// progress indicator
			uint8_t i, j, fill=0;
			for (i=0;i<FILM_HEIGHT;i++) {
				lcd_goto_yx(i, 0);
				for (j=0;j<FILM_WIDTH;j++) {
					union u16convert v;
					if (!dot_cnt) {
						if (vid_offset >= (MMC_BLOCK_SIZE)) {
							mmc_readsector(vid_sector++, (uint8_t *) buf16);
							mmc_readsector(vid_sector++, (uint8_t *) (buf16+(MMC_BLOCK_SIZE/2)));
							vid_offset = 0;
						}//if
						v.value = buf16[vid_offset];
						if (!v.value) break;
						if (v.bytes.high & 0xc0) {
							dot_cnt = v.value&0x1fff;
							fill = v.bytes.high>>7;
						}//if
						vid_offset++;
					}//if
					if (dot_cnt) { 
						dot_cnt--; 
						v.bytes.low = fill ? 0xff : 0x00; 
					}//if
					P1OUT |= DATACHAR;
					lcd_send(v.bytes.low);
				}//for
				if (!ticks && (P2DIR&BIT6)) {
					uint8_t steps = 2;
					uint8_t bits = 0;
					uint8_t i = 0;
					while (steps || bits) {
						uint8_t v = 0;
						if (steps || (bits&0x01)) {
							if (snd_offset >= MMC_BLOCK_SIZE) {
								mmc_readsector(snd_sector++, (uint8_t*) buf8);
								snd_offset = 0;
							}//if
							v = buf8[snd_offset++];
						}//if
						if (steps>1) {
							if (v) ticks = v/TICK_DIV;
							else P2DIR &= ~BIT6;
						}//if
						else {
							if (steps) {
								bits = v;
							}//if
							else {
								if (bits&0x01) {
									uint8_t note = v;
									if (note) {
										uint16_t freq = notes[note%12];
										note /= 12;
										while (--note) freq <<= 1;
										ch[i]._freq = freq;
									}//if
									else {
										ch[i]._freq = 0;
									}//else
								}//if
								i++;
								bits >>= 1;
							}//else
						}//else
						if (steps) steps--;
					}//while
				}//if
			}//for
			// set flip for faster device (or u got a big buffer, i.e. more ram)
			flip = 100;
			led_off();
		}//if

		//
		//
    }//while
}

// sine table for a kind of paino-like instructment
const int8_t _ram_samples[] = {
       0,   20,   41,   60,   78,   93,  106,  116,  123,  127,  127,  123,  117,  109,   98,   85, 
      72,   57,   43,   28,   14,    3,   -5,  -14,  -20,  -24,  -26,  -25,  -23,  -18,  -12,   -6, 
       0,    6,   12,   18,   22,   25,   26,   25,   20,   14,    6,   -2,  -14,  -28,  -42,  -57, 
     -72,  -86,  -99, -109, -118, -124, -128, -128, -124, -117, -108,  -95,  -79,  -62,  -42,  -21, };

//________________________________________________________________________________
#ifdef MSP430		// tells us we are using mspgcc
interrupt(TIMERA0_VECTOR) Timer_A0(void) {
#else
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void) {
#endif
	//
	int16_t dac = 0;
	uint8_t i;

	//_________ mixing
	for (i=0;i<MAX_CH;i++) {
		if (ch[i]._freq) {
			dac += _ram_samples[(ch[i]._phase>>8)&0x3f];
			ch[i]._phase += ch[i]._freq;
		}//if
	}//for
	dac /= MAX_CH;
	dac += 0x80;
	CCR1 = dac;
	//

	//_________ timing
	clicks++;
	//_________ adjust clicks for how fast midi plays
	if (clicks >= 164) {
		if (ticks) ticks--;
		clicks = 0;
	}//if
	if (flip) flip--;
}

