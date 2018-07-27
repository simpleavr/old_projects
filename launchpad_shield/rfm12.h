/*

   +=====================================================+
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .nSEL . SDI . VDD .  .  .  |
   |  .  .  .  .  .  +>|+  o-----o  o  o||o  o  o  .  .  |
   |  .  .  .  .  .  .  .  .  .  . SCK . GND .  .  .  .  |
   |  .  .  .  .  .  +--+--+--+--+--+--+                 |
   |                |- 26 27 CK IO 17 16|                |
   |                |+ 10 11 12 13 14 15|                |
   |  .  .  .  .  .  +--+--+--+--+--+--+                 |
   |  .  .  .  .  .  .  .  .  .  .nIRQ .  .  .  .  .  .  |
   |  .  .  .  .  .  +--B--+  .  o  o  o||x  x  o  .  .  |
   |  .  .  .  .  .  .  .  .  . SDO .DATA .  .  .  .  .  |
   |  .  .  .  .  .  .  . Rx Tx  .  .  .  .  .  .  .  .  |
   +=====================================================+
                         Red Blue -> nokia ca-42

	pinout for RFM12B, rectangle is the crystal

               SDO + o--+     o + nSEL
              nIRQ + o  |     o + SCK
     FSK/DATA/nFFS + o  |     o + SDI
    DCLK/CFIL/FFIT + x  |     x + nINT/VDI
               CLK + x--+     o + GND 
              nRES + x        o + VDD
               GND + o        o + ANT 

RFM12B Map
SEL  - P2.7
SDI  - P1.6
SCK  - P1.7
SDO  - P1.3
DATA - P1.5
IRQ  - P1.4
ARRSI- P1.0

*/

#define SEL  	P2
#define SELN 	7
#define SDI  	P1
#define SDIN 	6
#define SCK  	P1
#define SCKN 	7
#define SDO  	P1
#define SDON 	3
#define DATA  	P1
#define DATAN	5
#define IRQ  	P1
#define IRQN	4

#define BUT		P1
#define BUTN	1

#define M433	0x0010
#define M868	0x0020
#define M915	0x0030
#define ELEF	0x00C0

#define __join(x, y)	 x ## y

#ifdef AVR

#define _hi(a)        	 __join(PORT, a) |= (1<<a##N)
#define _low(a)        	 __join(PORT, a) &= ~(1<<a##N)
#define _output(a)       __join(DDR, a)  |= (1<<a##N)
#define _input(a)        __join(DDR, a)  &= ~(1<<a##N)
#define _is_hi(a)        (__join(PIN, a) & (1<<a##N))
#define _wait_low(a)     while (__join(PIN, a)&(1<<a##N))

#define _led_output()       DDRB  |= (1<<4)|(1<<5)|(1<<6)
#define _led_off()          PORTB &= ~((1<<4)|(1<<5)|(1<<6))
#define _ledg_on()          PORTB |= (1<<6)
#define _ledr_on()          PORTB |= (1<<4)
#define _button_output()    PORTD |= (1<<5)
#define _button_up()        (PIND&(1<<5))


#else

#define _hi(a)        	 __join(a, OUT) |= (1<<a##N)
#define _low(a)        	 __join(a, OUT) &= ~(1<<a##N)
#define _output(a)       __join(a, DIR) |= (1<<a##N)
#define _input(a)        __join(a, DIR) &= ~(1<<a##N)
#define _is_hi(a)        (__join(a, IN) & (1<<a##N))
#define _wait_low(a)     while (__join(a, IN)&(1<<a##N))

#define _led_output()    P2DIR |= (1<<6)
#define _led_off()       P2OUT &= ~(1<<6)
#define _led_on()        P2OUT |= (1<<6)

#endif

//________________________________________________________
#define RF_CFG			0x8000
#define RF_CFG_TX		(1<<7)
#define RF_CFG_RX		(1<<6)
#define RF_CFG_433		(1<<4)
#define RF_CFG_868		(1<<5)
#define RF_CFG_915		((1<<5)|(1<<4))
#define RF_CFG_10PF		3
#define RF_CFG_12PF		7

//________________________________________________________
#define RF_PWR			0x8200
#define RF_PWR_RX		(1<<7)
#define RF_PWR_BBB		(1<<6)
#define RF_PWR_TX 		(1<<5)
#define RF_PWR_SYN		(1<<4)
#define RF_PWR_OSC		(1<<3)
#define RF_PWR_BAT		(1<<2)
#define RF_PWR_WAKE		(1<<1)
#define RF_PWR_NCLK		(1<<0)

//________________________________________________________
// 12 bit freq 900 + F * 7.5Khz (36..3903)
#define RF_FREQ			0xa000

//________________________________________________________
// baud bit6..0 BR=10000000/29/(R+1)/(1+cs*7)
#define RF_BAUD			0xc600
#define RF_BAUD_CS		(1<<7)		// checksum

//________________________________________________________
// receiver control
#define RF_RCV			0x9000
#define RF_RCV_VDI		(1<<10)	`	// vdi output

#define RF_RCV_VDI_FAST	0
#define RF_RCV_VDI_MID 	(1<<8)
#define RF_RCV_VDI_SLOW	(2<<8)
#define RF_RCV_VDI_CONT	(3<<8)

#define RF_RCV_400KHZ	(1<<5)
#define RF_RCV_340KHZ	(2<<5)
#define RF_RCV_270KHZ	(3<<5)
#define RF_RCV_200KHZ	(4<<5)
#define RF_RCV_134KHZ	(5<<5)
#define RF_RCV_67KHZ	(6<<5)

#define RF_RCV_0DB		0		// strongest
#define RF_RCV_6DB		(1<<3)
#define RF_RCV_14DB		(2<<3)
#define RF_RCV_20DB		(3<<3)

#define RF_RCV_RSSI_0	0		// highest
#define RF_RCV_RSSI_1	1
#define RF_RCV_RSSI_2	2
#define RF_RCV_RSSI_3	3
#define RF_RCV_RSSI_4	4
#define RF_RCV_RSSI_5	5

//________________________________________________________
#define RF_FIL			0xc22c
#define RF_FIL_ALOCK	(1<<7)
#define RF_FIL_FASTM	(1<<6)
#define RF_FIL_ANALOG	(1<<4)
#define RF_FIL_QDQ0		0
#define RF_FIL_QDQ1		1
#define RF_FIL_QDQ2		2
#define RF_FIL_QDQ3		3

//________________________________________________________
#define RF_FIFO				0xca00
#define RF_FIFO_INT8		(8<<4)
#define RF_FIFO_1BSYNC		(1<<3)	// sync byte, 2b 0x2d,0xd4, 1b 0xd4
#define RF_FIFO_FILL_CONT	(1<<2)	// fifo always fill, otherwise fill sync word
#define RF_FIFO_FILL		(1<<1)	// enable fifo fill
#define RF_FIFO_NO_RESET	(1<<0)	// disable hi sensitivity reset

//________________________________________________________
// replace the default 0xd4 sync pattern
#define RF_SYNC				0xce00

//________________________________________________________
#define RF_READ				0xb000

//________________________________________________________
#define RF_AFC				0xc400
#define RF_AFC_MCU			0	// control by mcu
#define RF_AFC_ONCE			(1<<6)
#define RF_AFC_VDI 			(2<<6)	// keep offset when vdi high
#define RF_AFC_IND 			(3<<6)	// independent of vdi

#define RF_AFC_RANGE0		0		// no restriction
#define RF_AFC_RANGE1		(1<<4)	// 15
#define RF_AFC_RANGE2		(2<<4)	// 7
#define RF_AFC_RANGE3		(3<<4)	// 3 most restricted

#define RF_AFC_OFFSET		(1<<3)	// store offset
#define RF_AFC_ACCURATE		(1<<2)	// high accuracy mode
#define RF_AFC_OUTPUT		(1<<1)	// enable output register
#define RF_AFC_ENABLE		(1<<0)	// enable afc
// 0x--83 keep_offset+out+afc, 0x--9b keep_offset+offset+afc+range1

//________________________________________________________
#define RF_TX				0x9800
#define RF_TX_POLARITY		(1<<8)
#define RF_TX_DEV015KHZ		0
#define RF_TX_DEV030KHZ		(1<<4)
#define RF_TX_DEV045KHZ		(2<<4)
#define RF_TX_DEV060KHZ		(3<<4)
#define RF_TX_DEV075KHZ		(4<<4)
#define RF_TX_DEV090KHZ		(5<<4)
#define RF_TX_DEV105KHZ		(6<<4)
#define RF_TX_DEV120KHZ		(7<<4)
#define RF_TX_DEV135KHZ		(8<<4)
#define RF_TX_DEV150KHZ		(9<<4)
#define RF_TX_DEV165KHZ		(10<<4)
#define RF_TX_DEV180KHZ		(11<<4)
#define RF_TX_DEV195KHZ		(12<<4)
#define RF_TX_DEV210KHZ		(13<<4)
#define RF_TX_DEV225KHZ		(14<<4)
#define RF_TX_DEV240KHZ		(15<<4)

#define RF_TX_PWR0			0		// highest
#define RF_TX_PWR1			1
#define RF_TX_PWR2			2
#define RF_TX_PWR3			3
#define RF_TX_PWR4			4
#define RF_TX_PWR5			5
#define RF_TX_PWR6			6
#define RF_TX_PWR7			7

//________________________________________________________
#define RF_PLL				0xcc12
#define RF_PLLCLK_DEF		0
#define RF_PLLCLK_33		(1<<5)
#define RF_PLLCLK_25		(1<<6)
#define RF_PLL_DELAY		(1<<3)
#define RF_PLL_NODITHER		(1<<2)
#define RF_PLLBW_HIGH		1

//________________________________________________________
#define RF_WRITE			0xb800

//________________________________________________________
#define RF_WAKEUP			0xe000
#define RF_LOWDUTY			0xc800	// duty cycle bit7..1, enable bit0
#define RF_LOWBAT			0xc000	// low battery
#define RF_STATUS			0x0000	// read status

void     rf12_init(uint8_t band);
void     rf12_port_init(void);
uint16_t rf12_wrt_cmd(uint16_t aCmd);
void     rf12_send(uint8_t aByte);
uint8_t  rf12_recv(void);
void     rf12_reset_recv(void);

//______________________________________________________________________
void rf12_init(uint8_t band) {

	rf12_wrt_cmd(0x8000|band|ELEF|RF_CFG_12PF);
	rf12_wrt_cmd(0x8239);//!er,!ebb,ET,ES,EX,!eb,!ew,DC
	rf12_wrt_cmd(0xA640);//A140=430.8MHz
	rf12_wrt_cmd(0xC647);//4.8kbps
	rf12_wrt_cmd(0x94A0);//VDI,FAST,134kHz,0dBm,-103dBm
	rf12_wrt_cmd(0xC2AC);//AL,!ml,DIG,DQD4
	rf12_wrt_cmd(0xCA81);//FIFO8,SYNC,!ff,DR
	rf12_wrt_cmd(0xCED4);//SYNC=2DD4;
	rf12_wrt_cmd(0xC483);//@PWR,NO RSTRIC,!st,!fi,OE,EN
	//rf12_wrt_cmd(0xC49B);//@PWR,RSTRIC 15,st,!fi,OE,EN
	rf12_wrt_cmd(0x9850);//!mp,9810=30kHz,MAX OUT
	rf12_wrt_cmd(0xCC67);//OB1,OB0,!lpx,!ddy,DDIT,BW0
	rf12_wrt_cmd(0xE000);//NOT USE
	rf12_wrt_cmd(0xC800);//NOT USE
	rf12_wrt_cmd(0xC000);//1.66MHz,2.2V
	
}

//______________________________________________________________________
void rf12_port_init(void) {
	_hi(SEL);
	_hi(SDI);
	_low(SCK);
	_output(SEL);
	_output(SDI);
	_input(SDO);
	_output(SCK);

	_output(DATA);
	_hi(DATA);
	_input(IRQ);

}

//______________________________________________________________________
uint16_t rf12_wrt_cmd(uint16_t cmd) {
	uint8_t  i;
	uint16_t res=0;

	_low(SCK);
	_low(SEL);
	for (i=0;i<16;i++) {
		if (cmd & 0x8000)
			_hi(SDI);
		else
			_low(SDI);
		_hi(SCK);
		res <<= 1;
		if (_is_hi(SDO)) res |= 0x0001;
		_low(SCK);
		cmd <<= 1;
	}//for
	_low(SCK);
	_hi(SEL);
	return (res);
}
//______________________________________________________________________
void rf12_send(uint8_t byte) {
	_wait_low(IRQ);
	rf12_wrt_cmd(0xB800 + byte);
}
//______________________________________________________________________
uint8_t rf12_recv(void) {
	uint16_t fifo_data;

	_wait_low(IRQ);
	rf12_wrt_cmd(0x0000);
	fifo_data = rf12_wrt_cmd(0xB000);
	return (fifo_data & 0x00FF);
}
//______________________________________________________________________
void rf12_reset_recv(void) {
	rf12_wrt_cmd(0xCA81);		// disable fifo
	rf12_wrt_cmd(0xCA83);		// enable fifo
}

