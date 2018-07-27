/*
    ezrfm.c

	giftware, as is, no warranty, restricted to hobby use
	you cannot use this software for commercial gains w/o my permission

	if u are building software and releasing it based on this,
	u should retain this message in your source and cite it's origin
	otherwise just take whatever code snipplets you need

	credits
	i had used information and/or code snipplets from these sources

	HopeRF, RFM12B manufacturor http://www.hoperf.com/rf_fsk/rfm12b.htm
	blog.strobitics.com RFM12B info http://blog.strobotics.com.au/tutorials/rfm12-stuff
	jeelabs on the RFM12B, http://jeelabs.org/2009/04/29/rfm12-vs-rfm12b
	software uart, ti's example http://www.ti.com/lit/zip/slac080
	njc's uart tutorial http://www.msp430launchpad.com/2010/08/half-duplex-software-uart-on-launchpad.html
	hodgepig's LaunchPad Stakulator http://blog.hodgepig.org/2010/09/09/573

	simpleavr@gmail.com
	www.simpleavr.com

	Nov 2010 c chung

	source test build on ubuntu linux 10.04 w/ msg430-gcc toolchains
	source test build on windows 7 w/ TI CCS IDE (select gcc compatible option)

*/


#define F_CPU 8000000UL
#include <signal.h>
#include "msp430x20x2.h"

#include <stdint.h>
#include <stdlib.h>

#define TXD BIT1 // TXD on P1.1
#define RXD BIT2 // RXD on P1.2

#define BAUDRATE 9600

#define BIT_TIME        (F_CPU / BAUDRATE)
#define HALF_BIT_TIME   (BIT_TIME / 2)

#define USEC			(F_CPU/1000000)


#ifdef MSP430
//______________________________________________________________________
static void __inline__ brief_pause(register uint16_t n) {
    __asm__ __volatile__ (
                "1: \n"
                " dec      %[n] \n"
                " jne      1b \n"
        : [n] "+r"(n));

}
#define _delay_us(n)	brief_pause(n*USEC/2)

#else

#define _delay_us(n)	__delay_cycles(USEC)
//typedef unsigned char	uint8_t;
//typedef unsigned int	uint16_t;

#endif


static volatile uint8_t  bitCount;			// Bit count, used when transmitting byte
static volatile uint16_t tx_byte;			// Value sent over UART when uart_putc() is called
static volatile uint16_t rx_byte;			// Value recieved once hasRecieved is set

static volatile uint8_t isReceiving = 0;	// Status for when the device is receiving
static volatile uint8_t hasReceived = 0;	// Lets the program know when a byte is received

#include "rfm12.h"

#define uart_in()	(hasReceived)
//______________________________________________________________________
void uart_init(void) {
    P1SEL |= TXD;
    P1DIR |= TXD;
    P1IES |= RXD;	// RXD Hi/lo edge interrupt
    P1IFG &= ~RXD;	// Clear RXD (flag) before enabling interrupt
    P1IE  |= RXD;	// Enable RXD interrupt
}

//______________________________________________________________________
uint8_t uart_getc() {
	hasReceived = 0;
	return rx_byte;
}

//______________________________________________________________________
void uart_putc(uint8_t c) {
    tx_byte = c;

    while (isReceiving);			// Wait for RX completion

    CCTL0 = OUT;					// TXD Idle as Mark
    TACTL = TASSEL_2 + MC_2;		// SMCLK, continuous mode

    bitCount = 10;					// Load Bit counter, 8 bits + ST/SP

    CCR0 = TAR;						// Initialize compare register
    CCR0 += BIT_TIME;				// Set time till first bit

    tx_byte |= 0x100;				// Add stop bit to tx_byte (which is logical 1)
    tx_byte = tx_byte << 1;			// Add start bit (which is logical 0)
    CCTL0 = CCIS0 + OUTMOD0 + CCIE;	// set signal, intial value, enable interrupts

    while (CCTL0 & CCIE);			// Wait for completion
}
//______________________________________________________________________
#ifdef MSP430
interrupt(PORT1_VECTOR) PORT1_ISR(void) {
#else
#pragma vector=PORT1_VECTOR
__interrupt void Port1_Vector(void) {
#endif
    isReceiving = 1;

    P1IE &= ~RXD;	// Disable RXD interrupt
    P1IFG &= ~RXD;	// Clear RXD IFG (interrupt flag)

    TACTL = TASSEL_2 + MC_2;	// SMCLK, continuous mode
    CCR0 = TAR;					// Initialize compare register
    //CCR0 += HALF_BIT_TIME;	// Set time till first bit, not working for me
    CCR0 += BIT_TIME;			// this works better for me
    CCTL0 = OUTMOD1 + CCIE;		// Disable TX and enable interrupts

    rx_byte = 0;				// Initialize rx_byte
    bitCount = 9;				// Load Bit counter, 8 bits + start bit
}

//______________________________________________________________________
#ifdef MSP430
interrupt(TIMERA0_VECTOR) TIMERA0_ISR(void) {
#else
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A(void) {
#endif
	CCR0 += BIT_TIME;
    if (!isReceiving) {
        if (bitCount == 0) {
            //TACTL = TASSEL_2;	// SMCLK, timer off (for power consumption)
            CCTL0 &= ~CCIE ;	// Disable interrupt
        }//if
        else {
			if (bitCount < 6) CCR0 -= 12;
            CCTL0 |= OUTMOD2;	// Set TX bit to 0
            if (tx_byte & 0x01)
                CCTL0 &= ~OUTMOD2;
            tx_byte = tx_byte >> 1;
            bitCount--;
        }//else
    }//if
    else {
        if (bitCount == 0) {
            TACTL = TASSEL_2;	// SMCLK, timer off (for power consumption)
            CCTL0 &= ~CCIE ;	// Disable interrupt

            isReceiving = 0;

            P1IFG &= ~RXD;		// clear RXD IFG (interrupt flag)
            P1IE |= RXD;		// enabled RXD interrupt

            if ((rx_byte & 0x201) == 0x200) {
				// the start and stop bits are correct
                rx_byte >>= 1; 		// Remove start bit
                rx_byte &= 0xff;	// Remove stop bit
                hasReceived = 1;
            }//if
        }//if
        else {
            if ((P1IN & RXD) == RXD)	// If bit is set?
                rx_byte |= 0x400;		// Set the value in the rx_byte
            rx_byte = rx_byte >> 1;		// Shift the bits down
            bitCount--;
        }//else
    }//else
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


//______________________________________________________________________
void notify(uint16_t freq, uint8_t level) {
	uart_putc(freq&0xff);
	uart_putc(freq>>8);
	uart_putc(level);
}

//______________________________________________________________________
int main (void) {

    WDTCTL = WDTPW + WDTHOLD; 
	BCSCTL1 = 0x8d;			// around 8Mhz
    DCOCTL  = (3<<5)|22;

	P1SEL = P2SEL = 0x00;
	P1DIR = P2DIR = 0x00;
	P1OUT = P2OUT = 0x00;
	uart_init();
	_BIS_SR(GIE);		// enable interrupt

	// adc setup
	ADC10CTL0 = SREF_1 + REFON + REF2_5V + ADC10SHT_2 + ADC10ON + ADC10IE; 
	ADC10CTL1 = INCH_0 + ADC10DIV_3;	// adc channel 0
	ADC10AE0 |= (1<<0);					// use p1.0

	_led_output();
	_delay_us(8000);		// allow time for RFM12B to settle
	rf12_port_init();
	rf12_init(M915);

	_delay_us(8000);
	rf12_wrt_cmd(0x82C9);	// enable receiver
	rf12_reset_recv();		// reset fifo

	uint16_t freq = 0x36;
	//uint8_t  catch = 0;
	uint8_t  send = 0;
	uint8_t  skip = 16;

	//rf12_wrt_cmd(0x94A0|3);
	while (1) {
		if (uart_in()) {
			uint8_t cmd = uart_getc();
			uint8_t parm = cmd & 0x0f;
			cmd >>= 4;
			switch (cmd) {
				// we can add commands here in the future
				case 0:	// turn on/off data streaming
					send = parm ? 1 : 0;
					break;
				case 2:	// steps to skip, 16 is typical
					skip = 1<<parm;
					break;
				case 3:	// re-configure rfm12b module on selected band
					rf12_init(parm<<4);	// 0x10-0x30 433,868,915
					_delay_us(8000);
					rf12_wrt_cmd(0x82C9);		
					rf12_reset_recv();
					_delay_us(8000);
				default: break;
			}//switch
			//notify(0, cmd, parm);
			//_________ acknowledge command by blinking
			_led_on(); _delay_us(2000); _led_off();
		}//if

		if (!send) continue;		// no need to do anything

		//______________ we are sampling on every 7.5k x 16 steps = 120kHz
		rf12_wrt_cmd(0xA000|freq);
		rf12_wrt_cmd(0xb000);

		ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
		__bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit

		// from google the range for arssi is around 0.7v to 2.2v
		// we need a cutoff, otherwise the uart will be overwhemed by noise (weak signals)
		// CUTOFF_AT is minimum adc voltage to be eligible to send to host, at 0.1v scale
		// if we put 8 meaing we want to cut off at 0.8v
		#define CUTOFF_AT	8
		#define CUTOFF		((1024*CUTOFF_AT)/25)

		// this is not really scientific but works to show relative signal strengh
		// we want to limit our resolution to under 256 for now
		if ((ADC10MEM > CUTOFF) && send)
			notify(freq, ((ADC10MEM-CUTOFF)>>3)&0xff);
		freq += skip;		// skip steps, we are not that fast

		// rfm12 has requency range from 36 to 3903
		if (freq >= 3903) {
			freq = 36;
			if (send) notify(0, 0);
			//catch = 0;
		}//if
	}//while
}

