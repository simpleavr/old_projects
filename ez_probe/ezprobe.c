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

#define S_LOGIC		(1<<0)
#define S_HIGH		(1<<1)
#define S_OUTPUT  	(1<<2)
#define S_PWM     	(1<<3)
#define S_HIRES   	(1<<6)
#define S_HOLD    	(1<<7)

volatile uint8_t state = 0;
volatile uint8_t ifreq = 1;
volatile uint8_t ofreq = 4;
volatile uint8_t pwm = 1;
volatile uint8_t singles = 0;

volatile uint8_t ticks=0, tacs=0;
//______________________________________________________________________
void sample(void) {

	P1DIR &= ~P_PROBE;						// probe pin as input
	P1SEL &= ~P_PROBE;						// turn off alternate function on pin

	ADC10AE0  |= P_PROBE;					// probe pin in adc
	ADC10CTL0 |= ENC + ADC10SC;             // enable, start adc conversion
	__bis_SR_register(CPUOFF | GIE);        // enter lpm0 sleep for adc read
	ADC10AE0 &= ~P_PROBE;					// adc done, turn off probe pin as adc

	//______ setup timer
	TACTL = TASSEL_2|ID_1|SCS|MC_2|TACLR;	// continous mode
	uint8_t adc = ADC10MEM>>2;				// we only need 8 bit
	if ((adc) < 0x30 || (adc) >= 0xd0) {	// that's what we consider non-floating

		P1SEL |= P_PROBE;					// turn probe pin as timer capture pin
		CCTL1 = CAP | CCIS_1 | CM_2;		// capture falling edges

		uint16_t trips = 0;
		// timer will overflow in 
		//  8mhz =  8,000,000 / 0x10000 = 1/122 sec (/2 clk = 1/61)
		// 12mhz = 12,000,000 / 0x10000 = 1/183 sec (/2 clk = 1/91)
		while (!(TACTL & TAIFG)) {
			if (CCTL1 & CCIFG) {	// tripped
				CCTL1 &= ~CCIFG;
				trips++;
			}//if
		}//while

		CCTL1 = 0;
		P1SEL &= ~P_PROBE;

		//trips >>= 1;		// divide by 2 as we counted both edges
		if (trips > 640) {
			trips /= 91;
			trips *= 100;
		}//if
		else {
			trips *= 100;
			trips /= 91;		// now trips in 100ms units
		}//else

// 100, 500, 1k, 5k, 10k, 50k, 100k, 500k
// in 100ms we got
// 1 ..  1-  4 trips
// 2 ..  5-  9 trips
// 3 .. 10- 49 trips
// 5 .. 50-100 trips
		ifreq = 0;
		if (trips == 1) {
			singles++;
			if (singles > 8) singles = 8;
		}//if
		else {
			while (trips) {
				ifreq++;
				if (trips >= 5) ifreq++;
				trips /= 10;
			}//while
			if (ifreq > 8) ifreq = 8;
		}//else
		state |= S_LOGIC;				// pin is accepted with logic level
		if (ifreq) {
			/* alternate logic
			uint16_t high=1, low=0;
			while (high && P1IN&P_PROBE);			// wait for logic low
			high = 0;
			while (high<=0xffff && !(P1IN&P_PROBE)) high++;	// count highs
			while (low<=0xffff  && P1IN&P_PROBE) 	low++;	// count lows
			if (high >= low) state |= S_HIGH;
			else             state &= ~S_HIGH;
			*/
			state |= S_HIGH;
		}//if
		else {
			if (adc >= 0xd0) state |= S_HIGH;
			else             state &= ~S_HIGH;
		}//else
		//if (adc >= 0xd0) state |= S_HIGH;
		//else             state &= ~S_HIGH;

	}//if
	else {
		ifreq = 0;
		singles = 0;
		while (!(TACTL & TAIFG));	// floating input, wait it out
		state &= ~(S_LOGIC|S_HIGH);	// assume pin is floating 1st
	}//else
	TACTL = 0;

}

//______________________________________________________________________
void main(void) {

    WDTCTL = WDT_MDLY_8;			// wdt timer mode (8ms/12)

	BCSCTL1 = CALBC1_12MHZ;
	DCOCTL  = CALDCO_12MHZ;

	P1SEL = 0x00;
	P2SEL = 0x00;
	P1DIR = P_LEDS;
	P2DIR = P_LO;
	//________________ we need wdt to control led brightness
	IE1 |= WDTIE;					// enable wdt interrupt
	_BIS_SR(GIE);					// enable interrupt

	//________________ cycle leds shows initilization
	uint8_t i=4;
	while (i) {
		if (ticks>100) {
			ticks = 0;
			ifreq <<= 1;
			i--;
		}//if
	}//while

	// adc setup
	ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; 
	ADC10CTL1 = ADC_CH + ADC10DIV_3;


	// button pull-resistor enable
	//P1REN |= P_BUTTON;			

	uint16_t use_cnt = 60000;
	while (1) {
		// button sensing is suspensive, once pressed we wait for release
		if (!(P1IN & P_BUTTON)) {
			ticks = 0, tacs = 0;
			while (!(P1IN & P_BUTTON)) {
				if (tacs >= 2) state |= S_HOLD;
			}//while
			if (ticks > 10) {			// register only if held for 30ms+ (debounce)
				if (!(state & S_HOLD)) {
					if (state & S_OUTPUT) {	
						if (state & S_PWM) {	// pwm mode, alter pwm
							pwm++;
							if (pwm>8) pwm = 0;
							state &= ~S_OUTPUT;
							state |= S_HOLD;	// fake long press to start ouput
						}//if
						else {					// output mode, alter frequency
							ofreq++;
							if (ofreq>8) ofreq = 0;
							state &= ~S_OUTPUT;
							state |= S_HOLD;	// fake long press to start ouput
						}//else
					}//if
				}//if
				if (state & S_HOLD) {	// toggle mode probe->freq->pwm
					if (state & S_OUTPUT) {
						if (state & S_PWM)
							state &= ~(S_OUTPUT|S_PWM);	// back to probe
						else
							state |= S_PWM;				// advance to pwm
					}//if
					else {
						state |= S_OUTPUT;				// advance to frequency
					}//else
					if (state & S_OUTPUT) {
						state &= ~(S_LOGIC|S_HIGH);
						//________ probe output on
						P1DIR |= P_PROBE;
						P1SEL |= P_PROBE;
						uint8_t i = ofreq;
						use_cnt = 60000;
						while (i--) {
							use_cnt /= i&0x01 ? 5 : 2;
						}//while
						TACTL = TASSEL_2|ID_1|MC_1|TACLR;	// div2, up mode
						CCR0  = use_cnt-1;
						if (ofreq > 3)
							CCR1  = ((pwm%9) * use_cnt) / 8;
						else
							CCR1  = (pwm%9) * (use_cnt / 8);
						CCTL1 = OUTMOD_7;					// set / reset
					}//if
					else {
						//________ probe output off
						TACTL = 0;
						CCTL0 = 0;
						CCTL1 = 0;
						P1DIR &= ~P_PROBE;
						P1SEL &= ~P_PROBE;
					}//else
					state &= ~S_HOLD;
				}//if
			}//if
		}//if
		if (!(state & S_OUTPUT)) {
			sample();
		}//if
	}//while
}

//______________________________________________________________________
#ifdef MSP430
interrupt(ADC10_VECTOR) ADC10_ISR(void) {
#else
#pragma vector=ADC10_VECTOR
__interrupt void Adc10_Vector(void) {
#endif
	__bic_SR_register_on_exit(CPUOFF);	// Clear CPUOFF bit from 0(SR)
}

static uint8_t bar[] = { 0x00, 0x04, 0x0c, 0x1c, 0x3c, 0x04, 0x0c, 0x1c, 0x3c, };
//______________________________________________________________________
#ifdef MSP430
interrupt(WDT_VECTOR) WDT_ISR(void) {
#else
#pragma vector=WDT_VECTOR
__interrupt void Wdt_Vector(void) {
#endif
	//___________ wdt interrupt
	//            it's used to control led brightness as we don't have resistor
	//            for most leds
	P1OUT &= ~P_LEDS;
	P2OUT = 0x00;
	if (state & S_OUTPUT) {
		if (!(state & S_PWM) || (tacs & 0x01)) 
			P1OUT |= P_IND;
	}//if
	if (!(ticks&0x0f)) {
		//_______ 1/16 duty cycle to turn led on, :( we have no resistors for these
		if (state & S_LOGIC) {
			if (!ifreq || (tacs & 0x01)) {	// blink for pulsing
				if (state & S_HIGH) P1OUT |= P_HI;
				else                P2OUT |= P_LO;
			}//if
		}//if
		if (state & S_HOLD) {
			//P1OUT |= P_RANGE;
			P1OUT |= P_HI;
			P2OUT |= P_LO;
		}//if
		else {
			uint8_t val;
			if (state & S_OUTPUT) {
				if (state & S_PWM) val = pwm;
				else               val = ofreq;
			}//if
			else {
				if (ifreq)
					val = ifreq;
				else
					val = singles;
			}//else
			if ((val <= 4) || (tacs & 0x01))	// blink for high range bar display
			P1OUT |= bar[val];
		}//else
	}//else
	if (!ticks) tacs++;
	ticks++;
}

