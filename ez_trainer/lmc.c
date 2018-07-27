//
//  paradiso ui shield, sweetens your launchpad
//
//  Nov 2012
//  code provided as is, no warranty
//
//  this is a hobby project
//
//  you cannot use code for commercial purpose w/o my permission
//  nice if you give credit, mention my site if you adopt 50% or more of my code
//
//  c chung (www.simpleavr.com)
//  Nov 2012
//
/*


gcc version 4.6.3 20120301 (mspgcc LTS 20120406 patched to 20120502) (GCC)

/cygdrive/c/mspgcc-20120406-p20120502/bin/msp430-gcc -Os -Wall -ffunction-sections \
-fdata-sections -fno-inline-small-functions -Wl,-Map=lmc.map,--cref -Wl,\
--relax -Wl,--gc-sections,\
--section-start=.rodata_lmc0=0x0f000,\
--section-start=.rodata_lmc1=0x0f100 \
-I/cygdrive/c/mspgcc-20120406-p20120502/bin/../msp430/include -mmcu=msp430g2553 -o lmc.elf lmc.c

/cygdrive/c/mspgcc-20120406-p20120502/bin/msp430-objcopy -O ihex lmc.elf lmc.hex

mspdebug rf2500 "prog lmc.elf"

or

MSP430Flasher -n MSP430F2012 -w "lmc.hex" -g -z [VCC]
*/




//******************************************************************************

#include "signal.h"
#include  <msp430.h>


#include <stdint.h>
//#define G2553
#define MHZ 8


/*
   * ui booster / led module mapping
   . a0..a7 are p1.0 to p1.7, likewise b0..b7 are p2.0 to p2.7

                                         Gnd b6 b7       a7 a6 a5 b4 a3
   +=============================================================================+
   |  .  .  .  .  .  .  .  .  .  .  .  .  o  o  o  o  o  o  o  o  o  o  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |_____   |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  |_____   |  |  |  |  |  |  .  .  .  |
   |     +  .  . (0)-A--F-(1)(2)-B  .  +  .  +  .  | (0)-A--F-(1)(2)-B  .  .  +  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |__.  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  .  .  .  .  .  .  .  .  |
   |  .  +  .  .  E  D (.) C  G (3) .  +  .  +  .  .  E  D (.) C  G (3) .  .  +  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |  |  |  |  |  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  o  o  o  o  o  o  o  o  o  o  .  .  .  |
   +=============================================================================+
                                         Vcc a0 a1 a2 a3 a4 a5 b0 b1 b2




      +-----------------------------------------+
	  | +-----------------+ +-----------------+ |
	  | |  _   _   _   _  | |  _   _   _   _  | |
	  | | | | | | | | | | | | | | | | | | | | | |
	  | | |-| |-| |-| |-| | | |-| |-| |-| |-| | |
	  | | |_|.|_|.|_|.|_|.| | |_|.|_|.|_|.|_|.| |
	  | +-----------------+ +-----------------+ |
	  |                                         |
	  |      [  ] [  ] [Ad] [7 ] [8 ] [9 ] [F ] |
	  |                                         |
	  |      [  ] [  ] [- ] [4 ] [5 ] [6 ] [E ] |
	  |                                         |
	  |      [  ] [  ] [+ ] [1 ] [2 ] [3 ] [D ] |
	  |                                         |
	  |                     [0 ] [A ] [B ] [C ] |
	  |                                         |
      +-----------------------------------------+
	       _   _   _   _       _   _   _   _     
	      | | | | | | | |     | | | | | | | |    
	      |-| |-| |-| |-|     |-| |-| |-| |-|    
	      |_|.|_|.|_|.|_|.    |_|.|_|.|_|.|_|.   


  b7 a7 a6 b5 b4 b3                   b7 a7 a6 b5 b4 b3
   +=====================================================+
   |  .  .  .  .  .  .  .  .  .  .  .  o  o  o  o  o  o  |
   |  .  .  .                 +--+--+-(0)-A--F-(1)(2)-B--+--+--+
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .                 +  -  -  E  D (.) C  G (3) -  -  +
   |  o  o  o  o  o  .  .  .  .  .  .  o  o  o  o  o  o  |
   +=====================================================+
  b6 a4 a5 b0 b1 b2                   b6 a4 a5 b0 b1 b2


*/

#define SEG_A_P1	(1<<7)
#define SEG_B_P1	0x00
#define SEG_C_P1	0x00
#define SEG_D_P1	(1<<4)
#define SEG_E_P1	0x00
#define SEG_F_P1	(1<<6)
#define SEG_G_P1	0x00
#define SEG_d_P1	(1<<5)
#define DIGIT_0_P1	0x00
#define DIGIT_1_P1	0x00
#define DIGIT_2_P1	0x00
#define DIGIT_3_P1	0x00

#define SEG_A_P2	0x00
#define SEG_B_P2	(1<<3)
#define SEG_C_P2	(1<<0)
#define SEG_D_P2	0x00
#define SEG_E_P2	(1<<6)
#define SEG_F_P2	0x00
#define SEG_G_P2	(1<<1)
#define SEG_d_P2	0x00
#define DIGIT_0_P2	(1<<7)
#define DIGIT_1_P2	(1<<5)
#define DIGIT_2_P2	(1<<4)
#define DIGIT_3_P2	(1<<2)

#define SEGS_BIT_P1(v) \
   (((v & (1<<7)) ? 7 : 0) +\
    ((v & (1<<6)) ? 6 : 0) +\
	((v & (1<<5)) ? 5 : 0) +\
	((v & (1<<4)) ? 4 : 0) +\
	((v & (1<<3)) ? 3 : 0) +\
	((v & (1<<2)) ? 2 : 0) +\
	((v & (1<<1)) ? 1 : 0) +\
	((v & (1<<0)) ? 0 : 0))

#define SEGS_BIT_P2(v) \
   (((v & (1<<7)) ? (7+8) : 0) +\
    ((v & (1<<6)) ? (6+8) : 0) +\
	((v & (1<<5)) ? (5+8) : 0) +\
	((v & (1<<4)) ? (4+8) : 0) +\
	((v & (1<<3)) ? (3+8) : 0) +\
	((v & (1<<2)) ? (2+8) : 0) +\
	((v & (1<<1)) ? (1+8) : 0) +\
	((v & (1<<0)) ? (0+8) : 0))


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
#define SEGS_1 (SEG_A_P1|SEG_B_P1|SEG_C_P1|SEG_D_P1|SEG_E_P1|SEG_F_P1|SEG_G_P1|SEG_d_P1)
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
//______________ abcdefg
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

#define LTR_A 0x77 // 0111 0111
#define LTR_b 0x1f // 0001 1111
#define LTR_C 0x4e // 0100 1110
#define LTR_c 0x0d // 0000 1101
#define LTR_d 0x3d // 0011 1101
#define LTR_E 0x4f // 0100 1111
#define LTR_e 0x6f // 0110 1111
#define LTR_F 0x47 // 0100 0111
#define LTR_G 0x5e // 0101 1110
#define LTR_g 0x7b // 0111 1011	//(as 9)
#define LTR_H 0x37 // 0011 0111	//(dup)
#define LTR_h 0x17 // 0001 0111
#define LTR_I 0x06 // 0000 0110
#define LTR_i 0x02 // 0000 0010
#define LTR_J 0x3c // 0011 1100
#define LTR_K 0x37 // 0011 0111	//(as H)
#define LTR_k 0x0f // 0000 1111	
#define LTR_L 0x0e // 0000 1110
#define LTR_m 0x54 // 0101 0100
#define LTR_N 0x76 // 0111 0110
#define LTR_n 0x15 // 0001 0101
#define LTR_o 0x1d // 0001 1101
#define LTR_P 0x67 // 0110 0111
#define LTR_q 0x73 // 0111 0011
#define LTR_r 0x05 // 0000 0101
#define LTR_S 0x5b // 0101 1011	//(as 5)
#define LTR_t 0x0f // 0000 1111
#define LTR_U 0x3e // 0011 1110
#define LTR_u 0x1c // 0001 1100
#define LTR_N 0x76 // 0111 0110
#define LTR_v 0x1c // 0001 1100	//(as u)
//_____________  abcdefg
#define LTR_w 0x2a // 0010 1010
#define LTR_X 0x37 // 0011 0111	//(as H)
#define LTR_y 0x3b // 0011 1011
#define LTR_Z 0x6d // 0110 1101	//(as 2)



#define LTRml 0x66 // 0110 0110
#define LTRmr 0x72 // 0111 0010
#define LTR__ 0x00 // 0000 0000
#define LTRmn 0x01 // 0000 0001

// port io values for individual digits / letters
// 1st byte cycles to stay
// 2nd byte port 1 value
// 3rd byte port 2 value
static const uint8_t digit2ports[][3] = { 
	SEGS_PORT(LTR_0), SEGS_PORT(LTR_1), SEGS_PORT(LTR_2), SEGS_PORT(LTR_3),
	SEGS_PORT(LTR_4), SEGS_PORT(LTR_5), SEGS_PORT(LTR_6), SEGS_PORT(LTR_7),
	SEGS_PORT(LTR_8), SEGS_PORT(LTR_9), SEGS_PORT(LTR_A), SEGS_PORT(LTR_b), 
	SEGS_PORT(LTR_C), SEGS_PORT(LTR_d), SEGS_PORT(LTR_E), SEGS_PORT(LTR_F), 

	SEGS_PORT(LTR_g), SEGS_PORT(LTR_h), SEGS_PORT(LTR_I), SEGS_PORT(LTR_J), 
	SEGS_PORT(LTR_k), SEGS_PORT(LTR_L), SEGS_PORT(LTR_m), SEGS_PORT(LTR_n), 
	SEGS_PORT(LTR_o), SEGS_PORT(LTR_P), SEGS_PORT(LTR_q), SEGS_PORT(LTR_r), 
	SEGS_PORT(LTR_S), SEGS_PORT(LTR_t), SEGS_PORT(LTR_u), SEGS_PORT(LTR_v), 

	SEGS_PORT(LTR_w), SEGS_PORT(LTR_X), SEGS_PORT(LTR_y), SEGS_PORT(LTR_Z), 
	SEGS_PORT(BLANK), SEGS_PORT(LTRmn), 

};

// digits / letters we are using
enum {
	POS_0, POS_1, POS_2, POS_3, POS_4, POS_5, POS_6, POS_7,
	POS_8, POS_9, POS_A, POS_b, POS_C, POS_d, POS_E, POS_F, 

	POS_g, POS_h, POS_I, POS_J, POS_k, POS_L, POS_m, POS_n,
	POS_o, POS_P, POS_q, POS_r, POS_S, POS_t, POS_u, POS_v, 

	POS_w, POS_x, POS_y, POS_z, POS__, POSmn

};

void led_putc(uint8_t c);
void led_dot(uint8_t pos);
void seg2port(uint8_t c);
uint8_t led_pos=0;


//______________________________________________________________________
void led_puts(const char *str) {
    while(*str!=0)
        led_putc((uint8_t) *str++);
}

//______________________________________________________________________
static char nibble_to_char(uint8_t nibble) {
    if (nibble < 0xA)
        return nibble + '0';
    return nibble - 0xA + 'a';
}

//______________________________________________________________________
void led_putdec32(uint32_t d) {
	uint8_t buf[12]="";
	uint8_t i=0;

	if (d) {
		while (d) {
			buf[i++] = d % 10 + '0';
			d /= 10;
		}//while
	}//if
	else {
		buf[i++] = '0';
	}//else

	while (i) {
		i--;
		led_putc(buf[i]);
	}//while
}

//______________________________________________________________________
void led_puthex8(uint8_t h) {
    led_putc(nibble_to_char((h & 0xF0)>>4));
    led_putc(nibble_to_char(h & 0x0F));
}

//______________________________________________________________________
void led_puthex32(uint32_t h) {
    led_puthex8((h & 0xFF000000) >> 24);
    led_puthex8((h & 0x00FF0000) >> 16);
    led_puthex8((h & 0x0000FF00) >> 8);
    led_puthex8(h & 0xFF);
}

//______________________________________________________________________
void led_puthex16(uint16_t h) {
    led_puthex8((h & 0xFF00) >> 8);
    led_puthex8(h & 0xFF);
}



static const char hwkey_map[] = {
	0x19, 0x20, 0x1f, 0x70, 0x60, 0x18, 0x17,
	0x00, 0x30, 0x1e, 0x11, 0x14, 0x16, 0x15,
	0x00, 0x00, 0x40, 0x1a, 0x1b, 0x50, 0x1c,
	0x00, 0x00, 0x00, 0x10, 0x12, 0x1d, 0x13, };

static volatile uint8_t ticks = 0;
static volatile uint8_t clicks = 1;
static volatile uint8_t hwkey = 0x00;

static volatile uint16_t rands = 0x1234;

union u_data {
	uint16_t w;
	struct {
		uint8_t l;
		uint8_t h;
	} b;
};



/*
		mnprtxs  a  b  c  d  ei j  
		4, 8, 8, 4, 4, 6, 5, 4, 8, 

      +-----------------------------------------+
	  | +-----------------+ +-----------------+ |
	  | |  _   _   _   _  | |  _   _   _   _  | |
	  | | | | | | | | | | | | | | | | | | | | | |
	  | | |-| |-| |-| |-| | | |-| |-| |-| |-| | |
	  | | |_|.|_|.|_|.|_|.| | |_|.|_|.|_|.|_|.| |
	  | +-----------------+ +-----------------+ |
	  |                mnp  rtx  s    j         |
	  |           [  ] [7 ] [8 ] [9 ] [F ] [Ad] | Address
	  |                          b/w  ei     << |
	  |           [  ] [4 ] [5 ] [6 ] [E ] [Md] | Mode (addressing mode / monitor mode)
	  |                               d    . >> |
	  |                [1 ] [2 ] [3 ] [D ] [Rg] | Register
	  |                     a    b    c         |
	  |                [0 ] [A ] [B ] [C ] [+ ] |
	  |                                         |
      +-----------------------------------------+


*/

//______________________________________________________________________
char *dec2str16(int16_t d, uint8_t w) {
	static char buf[7]="";
	char *cp = buf + 6;
	char fill = '_';

	if (w >= 10) {
		fill = '0';
		w -= 10;
	}//if

	*cp = '\0';

	if (d >= 0) {
		*buf = ' ';
	}//if
	else {
		*buf = '-';
		d = -d;
	}//else

	char *wp = cp-w;

	if (d) {
		while (d) {
			*(--cp) = d % 10 + '0';
			d /= 10;
		}//while
	}//if
	else {
		*(--cp) = '0';
	}//else

	if (*buf == '-') *(--cp) = '-';
	while (wp<cp) *(--cp) = fill;

	return cp;
}

//________________________________________________________________________________

#define ST_ADDR_ENTRY		BIT0
#define ST_ASSEMBLER		BIT1
#define ST_LED				(BIT2|BIT3)
#define ST_IN_RAM			BIT4
#define ST_RAM_CHANGED		BIT5
#define ST_WRITE     		BIT6
#define ST_LMC	     		BIT7

uint8_t g_state=0;
uint16_t blinks=0;

//______________________________________________________________________
uint8_t get_key(uint8_t *c) {
	*c = 0;
	if (hwkey) { 
		*c = hwkey;
		hwkey = 0;
		if (!(g_state&ST_LED)) g_state |= 0X04;
	}//if
	return *c;
}


//________________________________________________________________________________
void led_clear() {
	uint8_t i;
	for (i=0;i<16;i++) seg2port(POS__);
}

// storage for multiplexing, 5 bytes output x 8 digits x 2 sets
// loads data into output[] for immediate led multiplexing

uint8_t output[5 * 8 * 2];
uint8_t  pos=0, stays=0x40; 
//__________________________________________________
void seg2port(uint8_t c) {

	static const uint8_t digit_map[][4] = {
		{ DIGIT_0_P1, DIGIT_1_P1, DIGIT_2_P1, DIGIT_3_P1, },
		{ DIGIT_0_P2, DIGIT_1_P2, DIGIT_2_P2, DIGIT_3_P2, },
		};

	if (led_pos>=16) led_pos = 0;

	uint8_t *pp = output + (led_pos+1) * 5;
	uint8_t i, dec=0;

	if (c&0x80) { // add decimal point on demand
		dec = SEG_d_P1;
		c &= 0x7f;
	}//if

	const uint8_t *cp = digit2ports[c];

	*--pp = *cp++;				// load stay time

	for (i=0;i<2;i++) {
		*--pp = (*cp | dec) | digit_map[i][led_pos&0x03];
		*--pp = (*cp & dec) | ~digit_map[i][led_pos&0x03];
		if (led_pos&0x04) *pp = ~(*pp);
		dec = 0x00;
		cp++;
	}//for
	led_pos++;

	// 5 bytes port value; p2out, p2dir, p1out, p1dir, stay

}

//__________________________________________________
uint8_t paradiso_loop() {

	//_______ these are for keyboard scanning
	static uint8_t key_row=7, key_col=7;
	static const uint8_t seg_a_d[] = { 
		SEGS_BIT_P1(SEG_A_P1) + SEGS_BIT_P2(SEG_A_P2),
		SEGS_BIT_P1(SEG_B_P1) + SEGS_BIT_P2(SEG_B_P2),
		SEGS_BIT_P1(SEG_C_P1) + SEGS_BIT_P2(SEG_C_P2),
		SEGS_BIT_P1(SEG_D_P1) + SEGS_BIT_P2(SEG_D_P2),
		SEGS_BIT_P1(SEG_E_P1) + SEGS_BIT_P2(SEG_E_P2),
		SEGS_BIT_P1(SEG_F_P1) + SEGS_BIT_P2(SEG_F_P2),
		SEGS_BIT_P1(SEG_G_P1) + SEGS_BIT_P2(SEG_G_P2),
		SEGS_BIT_P1(SEG_d_P1) + SEGS_BIT_P2(SEG_d_P2), };

	//________ stays... lsb = cycles to stay on, msb = cycles to stay off (dark)
	//         0x43 means stay on for 3 cycles, then stay off for 4 cycles
	if (stays & 0x0f) { 
		stays--; 
		//__________ use off time to check for keys
		if (!(stays&0xf0)) {		
			// no msb, that means we are in the off cycles
			//____ do scan on another column
			key_col--;

			uint8_t *tp = (uint8_t*) 0x27;
			tp += (seg_a_d[key_row]&0x08);
			uint8_t bit = 1<<(seg_a_d[key_row]&0x07);
			*tp &= ~bit; tp -= 6; 	// REN
			*tp &= ~bit; tp++; 		// OUT
			*tp |= bit;				// DIR

			uint8_t volatile *cp = (uint8_t*) 0x20;
			cp += (seg_a_d[key_col]&0x08);

			uint8_t bitc = 1<<(seg_a_d[key_col]&0x07);
			if (!(*cp&bitc)) {
				stays = 0x0e;	// key pressed, mark stay w/ 0x0e so we will visit again
				key_col++;		// and have the same table (row / col)
			}//if
			else {
				if (stays == 0x0d) {		
					// key released (was pressed before)
					// translate to more managable code
					hwkey = (key_row<<4) | key_col;
					rands ^= hwkey;
					hwkey -= 0x10;
					hwkey = hwkey_map[(hwkey&0x0f)*7+(hwkey>>4)];
				}//if
			}//else

			*tp &= ~bit; tp--;
			*tp |= bit;	tp += 6;
			*tp |= bit;
			if (!key_col) {		// last col done, advance row
				key_row--;
				if (key_row) {
					key_col = key_row;
				}//if
				else {
					key_row = key_col = 7;
				}//else
			}//if
			//
		}//if
		return 0; 
	}
	stays >>= 4;
	P2DIR = 0; P2OUT = 0; 
	P1DIR &= ~USED_1; P1OUT &= ~USED_1;
	if (stays) { 
		stays--; 
		//_______________ set ports up for key scanning
		P2OUT |= SEGS_2;
		P2REN |= SEGS_2;
		//P2REN |= USED_2;
		P1OUT |= SEGS_1;
		P1REN |= SEGS_1;
		return 0; 
	}//if

	//___________ return if we are blinking
	if (!(g_state&ST_LED)) stays = 0x80;

	/*
    a7 b3 b0 a4 b6 a6 b1 a5
    (A) B  C  D  E  F  G  d ..0 (7)
     A (B) C  D  E  F  G  d ..1 (6)
     A  B (C) D  E  F  G  d ..2 (5)
     A  B  C (D) E  F  G  d ..3 (4)
     A  B  C  D (E) F  G  d ..4 (3)
     A  B  C  D  E (F) G  d ..5 (2)
     A  B  C  D  E  F (G) d ..6 (1)
     A  B  C  D  E  F  G (d)..7 (0)
	 0  1  2  3  4  5  6  7

     b7 a7 a6 b5 b4 b3
     (0)-A--F-(1)(2)-B
      E  D (.) C  G (3)
     b6 a4 a5 b0 b1 b2
	*/

	P2REN = 0;
	P1REN &= ~USED_1;
	P2DIR = 0; 
	P1DIR &= ~SEGS_1; 
	P2OUT = 0; 
	P1OUT &= ~SEGS_1;

	if (!stays) {
		uint8_t *ioptr = output + (pos*5);
		if (g_state&BIT3) ioptr += 40;

		P2OUT |= *ioptr++;
		P2DIR |= *ioptr++;
		P1OUT |= (*ioptr++ & USED_1);
		P1DIR |= (*ioptr++ & USED_1);
		stays = *ioptr;
		//______ dimmer
		if ((clicks&0x20) && (blinks&(1<<(pos+(g_state&0x08)))))
			stays = (stays&0xf0)<<1 | (stays&0x0f)>>2;
		// uncomment to compensate for mix-match led pairs
		//else if (pos >= 4) stays += 8;
	}//if

	pos++;
	pos &= 0x07;

	return 0;

}

//________________________________________________________________________________
void led_dot(uint8_t pos) {
	uint8_t *sp = output;
	sp += pos*5+2;

	if (pos&0x04)
		*sp &= ~SEG_d_P1;
	else
		*sp |= SEG_d_P1;
	sp++;
	*sp |= SEG_d_P1;
}

//________________________________________________________________________________
void led_putc(uint8_t c) {
	static uint8_t hide=0;

	switch (c) {
		case '.':
			if (led_pos) led_dot(led_pos-1);
			return;
		case '_': 
			c = POS__; 
			break;
		case '(': case ')': 
			hide ^= 1; 
			return;
		case '-': 
			c = POSmn; break;
		case '>': 
			led_clear();
		case '\n': case '\r': 
			led_pos = 16;
			return;
		default: 
			if (c>='A' && c<='Z') c -= 'A' - 10;
			else if (c>='a' && c<='z') c -= 'a' - 10;
			else if (c>='0' && c<='9') c -= '0';
			else return;
			break;
	}//switch

	if (led_pos<16 && !hide) seg2port(c);
}

//________________________________________________________________________________
uint16_t read_val(uint8_t hex) {
	uint16_t res=0;
	led_pos = 0;
	hex = hex ? 16 : 10;
	while (1) {
		uint8_t key = 0;
		while (!get_key(&key));
		if (key == 0x50) break;
		res *= hex;
		if (key < (hex+0x10)) {
			key -= 0x10;
			res += key;
			led_pos = 4;
			if (hex == 16)
				led_puthex16(res);
			else
				led_puts(dec2str16(res, 4));
		}//if
	}//while
	return res;
}


//________________________________________________________________________________
void flash_write(uint16_t ptr, char *src, uint8_t cnt) {
	volatile char *flash = (char*) ptr;
	FCTL2 = FWKEY+FSSEL0+FN1;
	FCTL1 = FWKEY+ERASE;
	FCTL3 = FWKEY;
	*flash = 0;

	FCTL1 = FWKEY+WRT; 
	uint8_t i;
	for (i=0;i<cnt;i++) *flash++ = *src++;
	FCTL1 = FWKEY;
	FCTL3 = FWKEY+LOCK; 
}

// little man code starts at 0xf000
// adder example
/*
void __attribute__ ((section (".rodata_lmc0"))) lmc_0() {
	asm(
	".word  901, 306, 901, 106, 902,   0,   0,   0,   0,  0\n"
	);
}

// counter example

void __attribute__ ((section (".rodata_lmc1"))) lmc_1() {
	asm(
	".word  511, 313, 902, 513, 111, 902, 313, 212, 810, 603\n"
	".word    0,   1,  10,   4,   0,   0,   0,   0,   0,   0\n"
	);
}

*/

//________________________________________________________________________________
void little_man_run(void) {
	uint16_t *ram = (uint16_t*) 0x0280;
	uint16_t *sp = ram;

	//".byte  511, 313, 902, 513, 111, 902, 313, 212, 810, 603\n"
	//".byte    0,   1,  10,   4,   0,   0,   0,   0,   0,   0\n"


	uint8_t neg=0, op=0;
	uint16_t calc=0;
	while (1) {
		led_putc('\n');
		clicks = 50;		// we do a operation per some clicks
		ticks = 0;
		led_pos = 0;
		led_puts(dec2str16(*sp, 13));
		//led_puthex16(*sp);
		op = *sp%100;
		switch (*sp/100) {
			case 1:	// ADD add
				calc += *(ram+op);
				break;
			case 2: // SUB subtract
				neg = 0;
				if (*(ram+op) > calc)
					neg = 1;
				else
					calc -= *(ram+op);
				break;
			case 3: // STA store 
				*(ram+op) = calc;
				break;
			case 5: // LDA load
				calc = *(ram+op);
				break;
			case 6: // BRA branch
				sp = ram + op - 1;
				break;
			case 7: // BRZ branch if zero
				if (!calc) sp = ram + op - 1;
				break;
			case 8: // BRP branch if positive
				if (!neg) sp = ram + op - 1;
				break;
			case 9: // INP/OUT input 901,902
				led_putc(' ');
				if (op == 2) {
					led_pos = 4;
					led_puts(dec2str16(calc, 4));
				}//if
				else {
					calc = read_val(0);
					led_clear();
					clicks = 0;		// wait no more
				}//else
				break;
		}//switch
		if (!op) break;
		sp++;
		while (clicks) asm(" nop");
	}//while
	led_putc('\n');

	while (!get_key(&neg)) asm(" nop");
}

#define ___use_cal_clk(x)	\
BCSCTL1 = CALBC1_##x##MHZ;	\
DCOCTL  = CALDCO_##x##MHZ;

#define __use_cal_clk(x)	___use_cal_clk(x)
//________________________________________________________________________________
int main(void) {
	WDTCTL = WDTPW + WDTHOLD;
	__use_cal_clk(MHZ);

	/*
	BCSCTL3 |= LFXT1S_2;
	//WDTCTL = WDT_ADLY_250;  // like 250ms / 13 * 32, or around 2/3 of a second
	WDTCTL = WDT_ADLY_1_9;  // _1000, _250, _16, _1_9
	IE1 |= WDTIE;
	*/
	// use 2nd time for led scanning
	// ticks  8000000/1024 = 7812.5 ticks/sec
	// clicks 8000000/1024/2048 = 3.814 clicks/sec
	//TA1CCR0  = 1024;		// for 8Mhz, will have 8k ticks/sec, 
	TA1CCR0  = 800;		// 10khz
	TA1CCTL0 = CCIE;
	TA1CTL = TASSEL_2 + MC_1;                  // SMCLK, upmode

	//

	P1SEL &= 0x0f;
	P2SEL  = 0;

	_BIS_SR(GIE);

	/*
	ram        at 0x0200..monitor use
	edit buffer   0x0300..0x3ff (256 bytes)
	monitor    at 0xc000..0xcfff
	prog banks at 0xd000+ in 64 bytes increments
				  0xd040....
	              0x1000 data flash 0
	              0x1040 data flash 1
	              0x1080 data flash 2
	              0x10a0 data flash 3 (don't touch, calibrated data)

	user banks 0xe000-0xe100 (256) maps 0x0280-0x0380
	*/
	uint16_t addr=0;

	union u_data data[3];
	uint8_t  first=1, c=0, n=0;
	int8_t   adv=0;
	uint16_t *cp=0;


	uint8_t prog=0;
	uint8_t digit_cnt=0;

	/* code to dectect press-hold-reset if we need it
	while (clicks) asm("nop");
	if (stays == 0x0e) {
		__blink_debug(0);
	}//if
	*/
	
	g_state |= ST_ADDR_ENTRY;

	while (1) {
		/*
		  |mnp  rtx  s    j         | |                              |
		  |[7 ] [8 ] [9 ] [F ] [Ad] | |[51] [71] [61] [11] [31] [21]i|
		  |          b/w  ei     << | |                              |
		  |[4 ] [5 ] [6 ] [E ] [Md] | |[41] [52] [72] [62] [32] [22]n|
		  |               d    . >> | |                              |
		  |[1 ] [2 ] [3 ] [D ] [Rg] | |     [42] [54] [74] [64] [33]r|
		  |     a    b    c         | |                              |
		  |[0 ] [A ] [B ] [C4] [+ ] | |     [44] [43] [53] [73] [63]N|

			key code returning
			0x1? 0-16 keys
			0x20 i address input
			0x30 n mode cycling
			0x40 r register cycling
			0x50 N enter
			0x60 u1
			0x70 u2
		*/
		if (c || get_key(&c)) {
			if (first) {
				first = 0;
				led_puts("\nWelcome to Paradiso Sweetener\n");
			}//if

			if (c >= 0x20)
				digit_cnt = 0;
			else
				digit_cnt++;


			switch (c>>4) {
				case 0x03:	// (n) cycle addressing mode (may be use + - during edit)
					if (g_state & ST_ADDR_ENTRY) {
						// done address entry, direct assembler
						c = 0x50;
						n = 0x30;
						continue;
					}//if
					break;
				case 0x04:	// (r) increments register (could use + - during edit)
					if (g_state & ST_ADDR_ENTRY) {
						if ((addr&0x0f00) == 0x0f00) addr &= 0xf000;
						else addr += 0x0100;
					}//if
					else {
						addr -= 2;
					}//else
					break;
				case 0x02:		// (i) address
					g_state |= ST_LMC;
					if (g_state & ST_ADDR_ENTRY) {
						g_state ^= ST_ASSEMBLER;
					}//if
					else {
						g_state &= ~ST_ASSEMBLER;
						if (g_state & ST_IN_RAM) {		//____ release buffer
							if (g_state & ST_RAM_CHANGED)
								flash_write((0xe0 + prog)<<8, (char *) 0x0280, 0xf0);
						}//if
						g_state &= ~(ST_IN_RAM|ST_RAM_CHANGED);
					}//else
					addr = 0xf000;
					g_state |= ST_ADDR_ENTRY;
					blinks = 0x000f;
					cp = &addr;
					break;
				/*
				case 0xff: 
					lmc_0(); 
					lmc_1(); 
				*/
				case 0x05:	// enter
					blinks = 0;
					if (g_state & ST_ADDR_ENTRY) {
						//________________ done w/ address mode
						if (!(g_state & ST_ASSEMBLER) ||
							((g_state & ST_LMC) && !(g_state & ST_IN_RAM))) {
							//____________ load flash to ram for edit, if within range
							//             for lmc, need to load into flash to execute
							prog = addr >> 8;
							if (prog >= 0xe0) {
								prog -= 0xe0;
								g_state |= ST_IN_RAM;
								uint16_t *ram = (uint16_t*) 0x0280;
								addr &= 0xff00;
								do {
									*ram++ = *((uint16_t*) addr);
									addr += 2;
								} while ((uint16_t) ram < 0x0380);
								addr = 0x0280;
							}//if
						}//if
						if (g_state & ST_ASSEMBLER) {	// run mode
							led_clear();
							if (g_state & ST_LMC) {		// little man goes to work
								little_man_run();
								n = 0x20;
								//continue;
							}//if
						}//if
						g_state &= ~(ST_ADDR_ENTRY|ST_ASSEMBLER);
						cp = 0;
						break;
					}//if
						if (g_state & ST_WRITE) {
							//led_putc('z'); led_putc('\n');
							*((uint16_t*) addr) = data[0].w;
							*((uint16_t*) addr+1) = data[1].w;
							*((uint16_t*) addr+2) = data[2].w;
							g_state &= ~ST_WRITE;
							g_state |= ST_RAM_CHANGED;
						}//if
						if (g_state&ST_ADDR_ENTRY)
							g_state ^= ST_ADDR_ENTRY;
						else
							addr += 2 + adv;
						adv = 0;
						if (c != ' ') led_putc('\n');
						if (g_state&BIT3) g_state -= BIT2;
						cp = 0;		// this should make us read again
					break;
				case 0x0b:	// (.) led on-on-off
					led_clear();
					if ((g_state&ST_LED) == BIT3) {
						g_state &= ~ST_LED;
					}//if
					else {
						g_state += BIT2;
					}//else
					break;
				case 0x01:
					c &= 0x0f;		// hex, get the real hex

					if (!cp && !(g_state&(ST_ADDR_ENTRY|ST_ASSEMBLER))) {	
						// load data to work on if not present
						cp = &data[0].w;
					}//if
					if (cp) {
						*cp %= 100;
						*cp *= 10;
						*cp += c;
						g_state |= ST_WRITE;
					}//if
				default:
					break;
			}//switch
			led_puts("\r[2K\r");

				if (!cp) {
					data[0].w = *((uint16_t*) addr);
					data[1].w = *((uint16_t*) (addr+2));
					data[2].w = *((uint16_t*) (addr+4));
				}//if

			led_clear();
			led_pos = 0;

			if (g_state & ST_IN_RAM) {
				if (g_state & ST_LMC) {
					led_puts(dec2str16((addr-0x0280)>>1, 12));
				}//if
				else {
					led_puthex16(addr + ((0xe0 + prog)<<8)-0x0280);
				}//else
			}//if
			else {
				led_puthex16(addr);
			}//else
			led_putc(' ');

			if (g_state & ST_ADDR_ENTRY) { 
				if (g_state & ST_ASSEMBLER) led_puts("_run");
				else led_puts("_mod");
				c = 0; continue; 
			}//if
			 
			static const char op_lmc[][4] = 
				{ "hlt", "add", "sub", "sta", "---", "lda", "bra", "brz", "brp", "inp", "out", };
			if (g_state & ST_LMC) {
				//_________________ little man
				if (g_state & ST_ASSEMBLER) {
					led_pos = 0;
					led_putc('_');
					led_putc('_');
				}//if

				uint8_t idx = data[0].w/100;
				uint8_t op  = data[0].w%100;
				if (!idx && op) idx = 4;
				if (idx>9) idx = 4;
				if (idx == 9 && op == 2) idx++;
				led_puts(op_lmc[idx]);
				led_putc('.');
				led_puts(dec2str16(data[0].w%1000, 13));
				led_dot(1);
			}//if

			c = n ? n : 0;
			n = 0;
		}//if
	}//while


}

#ifdef __led_H
//________________________________________________________________________________
interrupt(PORT1_VECTOR) PORT1_ISR(void) {
	led_port1_isr();
}

//________________________________________________________________________________
interrupt(TIMER0_A0_VECTOR) TIMER0_A0_ISR(void) {
	led_timera0_isr();
}
#endif

//________________________________________________________________________________
//interrupt(TIMER1_A0_VECTOR) Timer1_A0_iSR(void) {
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
	ticks++;
	//if (ticks > 2048) {
	if (ticks > 100) {
		ticks = 0;
		clicks--;
	}//if
	paradiso_loop();
}
