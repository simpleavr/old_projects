/******************************************************************************
  MSP430F2012 ezprobe

	giftware, as is, no warranty, restricted to hobby use
	you cannot use this software for commercial gains w/o my permission

	if u are building software releasing it based on this,
	u should retain this message in your source and cite it's origin
	otherwise just take whatever code snipplets you need

  Description: turns a ez430 dongle into a logic probe tool
  . supply from circuit via usb connector
  . 3 operating modes rotating between logic read, pulse output, pwm output
  . long button press (about 1.5 sec) rotates through the 3 operating modes
  . p1.0 original green led as mode indicator, off - probe, on - output, blink - pwm
  logic probe
  . logic probe red - hi, green - low, none - floating
  . logic probe red / green blinks on continous pulse reads > 100hz
  . 4 yellow leds shows detected frequencies in 8 steps, blinking yellows indicate hi-range (i.e. step 5-8)
  . shows detected pulse frequences for 100hz+, 500hz+, 1khz+, 5khz+, 10khz+, 50khz+, 100khz+, 500khz+
  . for non-continous single pulse bursts, the red / green leds stays on and subsequent pulse counts are displayed incrementally on the leds, will count up to 8 pulse
  continous pulse output, frequency setting
  . indicated by p1.0 original green led on
  . 4 yellow leds shows output pulse frequencies in 9 steps, blinking yellows indicate hi-range (i.e. step 5-8)
  . pulse frequences output for 100hz, 500hz, 1khz, 5khz, 10khz, 50khz, 100khz, 500khz, 1mhz
  . short button press rotates the 9 different frequency settings.
  continous pulse output, pwm setting
  . indicated by p1.0 original green led blinking
  . same as previous operation mode, except pwm values are show (and be setup) instead of frequency
  . 4 yellow leds shows output pwm percentages in 9 steps, blinking yellows indicate hi-range (i.e. step 5-8)
  . pwm percentages for 0%, 12.5%, 25%, 37.5%, 50%, 62.5%, 75%, 87.5%, 100%
  . short button press rotates the 9 different pwm settings.


  
         MSP430F2012 target board


          Gnd o----+    o Vcc
         P2.6 o    |    o P1.0
         P2.7 o-|>-+-<|-o P1.1     P2.7 is green, P1.1 is red
          Clk o    +-<|-o P1.2     P1.2 to P1.5 are yellow leds
           IO o    +-<|-o P1.3
         P1.7 o-+  +-<|-o P1.4
         P1.6 o |  +-<|-o P1.5
              | |
              / /
             / o P1.7 to USB D+ (for button)
            o    P1.6 to USB D- (for nail)

  USB 
   +-------------+
   |-==-==-==-==-+
   |  1  2  3  4 |
   +-------------+
     5v D- D+ Gnd

  to simplify the ezprobe construction, we are using / re-purposing the usb connector for io
  this way we can have a very simple single external connector
  to handle power supply, menu button and probe input

  . although the ez430 dongle is design to take 5v usb power and turn it to 3.6v, it works reliably on 3v supply
  . the ez430 dongle register itself as a full speed usb device it has a pull-up on D+
  . we make use of this fact to use the same connection as out button
  . the usb D- is left floating and we use the same connection as probe input / ouput

  c. chung
  www.simpleavr.com
  december 2010
  built with msp430-gcc
******************************************************************************/

#include "signal.h"
#include <msp430x20x2.h>

#define MODE_PROBE	0
#define MODE_SIGGEN	1
#define MODE_PWM	2
#define MODE_PWMHI	3

#define ADC_CH		INCH_6
#define P_BUTTON	(1<<7)
#define P_PROBE		(1<<6)
#define P_LO 		(1<<7)
#define P_HI  		(1<<1)
#define P_IND 		(1<<0)
#define P_RANGE		(0x0f<<2)

#define P_LEDS	(P_HI|P_RANGE|P_IND)

#define S_OUTPUT  	(1<<0)
#define S_HOLD    	(1<<1)

//______________________________________________________________________
void main(void) {

    WDTCTL = WDTPW + WDTHOLD; 

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;

	P1SEL = 0x00;
	P2SEL = 0x00;
	P1DIR = P_LEDS;
	P2DIR = P_LO;
	P1OUT = 0;
	P2OUT = 0;
	 
	P1REN |= P_BUTTON;
	P1OUT |= P_BUTTON;

	uint8_t state = 0;
	uint8_t pwm = 0;
	uint8_t leds = 0;
	uint16_t ticks = 0;
	uint8_t bar[] = { 0x00, 0x04, 0x0c, 0x1c, 0x3c, 0x04, 0x0c, 0x1c, 0x3c, };
	while (1) {
		// button sensing is suspensive, once pressed we wait for release
		if (!(P1IN & P_BUTTON)) {
			uint16_t button = 0;
			while (!(P1IN & P_BUTTON)) {
				volatile uint8_t foobar=1; 
				while (foobar) foobar++;
				if (button == 200) P1OUT |= P_IND;
				button++;
			}//while
			if (button > 10) {			// register only if held for 30ms+ (debounce)
				if (button > 200) {	// toggle pwm on-off
					state ^= S_OUTPUT;
					P1OUT &= ~P_IND;
				}//if
				if (state & S_OUTPUT) {
					//________ probe output on
					P1DIR |= P_PROBE;
					P1SEL |= P_PROBE;
					// we need 20ms pulse, at 1mhz, need to count to 20000
					TACTL = TASSEL_2|MC_1|TACLR;	// up mode
					CCR0  = 20000-1;
					// pwm range from 5% to 10% for servo
					CCR1  = 0;
					CCTL1 = OUTMOD_7;					// set / reset
				}//if
				else {
					//________ probe output off
					TACTL = 0;
					CCTL0 = 0;
					CCTL1 = 0;
					P1DIR &= ~P_PROBE;
					P1SEL &= ~P_PROBE;
					leds = 0;
				}//else
			}//if
		}//if
		if ((state&S_OUTPUT) && !(ticks&0x3fff)) {
			pwm++;
			if (pwm > 8) pwm = 0;
			CCR1  = ((1000 * pwm) / 8) + 1000;
			leds  = bar[pwm&0x03];
			if (pwm > 3) leds |= P_HI;
		}//if
		P1OUT |= P_IND;
		if ((ticks & 0x00ff) == 0x00) {
			P1OUT |= leds;
			P2OUT |= P_LO;
		}//if
		if ((ticks & 0x00ff) == 0x10) {
			P1OUT &= ~leds;
			P2OUT &= ~P_LO;
		}//if
		ticks++;
	}//while
}

