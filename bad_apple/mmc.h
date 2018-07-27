
//
// low level mmc / sd card i found from internet 
// two sources, 2nd source tell me how to do sd (different from mmc)
// both kind of mentioned they derived from elm-chan's code
// thanks internet / google.
// i changed indentation style and add small block sector init / read / write
// i am using 430g2231 mostly 2k devices that has only 128byte ram
//
#ifndef __MMC_H
#define __MMC_H
//       SD
//        __________________
//       /  1 2 3 4 5 6 7 8 | 1. CS      .. SEL
//       |9                 | 2. DATAIN  .. SDO
//       |                  | 3. GND     .. Gnd
//       |  S O - + C   I   | 4. 3.3V    .. Vcc
//       |                  | 5. CLK     .. Clk
//       |                  | 6. GND/NC
//       |                  | 7. DATAOUT .. SDI
//       |                  | 8. DAT1
//       |                  | 9. DAT2
//
//       Micro-SD
//        __________________
//       /  1 2 3 4 5 6 7 8 | 1. NC            
//       |                  | 2. CS      .. SEL
//       |                  | 3. DATAIN  .. SDO
//       |    S O + C - I   | 4. 3.3V    .. Vcc
//       |                  | 5. CLK     .. Clk
//       |                  | 6. GND/NC  .. Gnd
//       |                  | 7. DATAOUT .. SDI
//       |                  | 8. DAT1
//       |                  | 9. DAT2


#ifdef LCD_7110

#define SCK     BIT5		
#define SDI     BIT7		
#define SDO     BIT6		
#define SEL     BIT3		

#define COM_OUT	P1OUT
#define CLK_OUT	P1OUT

#define spi_port_init()	\
	P1OUT |= (SEL);		\
	P1DIR |= (SEL|SDO|SCK)

#else

#define SCK     BIT1		// to SCK     (BIT5 -> BIT1)
#define SDI     BIT6		// to SDO     (BIT7 -> BIT6)
#define SDO     BIT2		// to SDI     (BIT6 -> BIT2)
#define SEL     BIT7		// to nSEL    (BIT3 -> BIT7)

#define COM_IN 	P1IN
#define COM_OUT	P1OUT
#define CLK_OUT	P1OUT

#define spi_port_init()	\
	P1OUT |= (SEL);		\
	P1DIR |= (SEL|SDO|SCK)

#endif

struct u16bytes {
	uint8_t low;
	uint8_t high;
};

union u16convert {
	uint16_t value;
	struct u16bytes bytes;
};

struct u32bytes {
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
	uint8_t byte4;
};

struct u32words {
	uint16_t low;
	uint16_t high;
};

union u32convert {
	uint32_t value;
	struct u32words words;
	struct u32bytes bytes;
};

#ifdef MMC_BLOCK_SIZE
	#define BLOCK_SIZE	MMC_BLOCK_SIZE
#else
	#define BLOCK_SIZE	512
#endif

//______________________________________________________________________
uint8_t xspi_byte(uint8_t cmd) {
	uint8_t i;
	uint8_t res=0;

	for (i=0;i<8;i++) {
		if (cmd & 0x80)
			COM_OUT |= (SDO);
		else
			COM_OUT &= ~(SDO);
		// looks like we don't need delays
		//__delay_cycles(1*USEC);
		CLK_OUT |= (SCK);
		res <<= 1;
		if (COM_IN & (SDI)) res |= 0x01;
		CLK_OUT &= ~(SCK);
		cmd <<= 1;
	}//for
	return (res);
}

#define CS_ASSERT   { COM_OUT &= ~(SEL); }
#define CS_DEASSERT { COM_OUT |= (SEL);  }
//______________________________________________________________________
uint8_t spi_byte(uint8_t cmd) {
	uint8_t i=8;
	uint8_t res=0;

	while (i--) {
		CLK_OUT &= ~(SCK);
		COM_OUT &= ~(SDO);
		if (cmd & 0x80) COM_OUT |= (SDO);
		CLK_OUT |= (SCK);
		cmd <<= 1;
		res <<= 1;
		if (COM_IN & (SDI)) res |= 0x01;
	}//for
	return (res);
}



/* ------------- SD CARD LOW LEVEL ACCESS ------------- */
#define GO_IDLE_STATE            0
#define SEND_OP_COND             1
#define SEND_CSD                 9
#define STOP_TRANSMISSION        12
#define SEND_STATUS              13
#define SET_BLOCK_LEN            16
#define READ_SINGLE_BLOCK        17
#define READ_MULTIPLE_BLOCKS     18
#define WRITE_SINGLE_BLOCK       24
#define WRITE_MULTIPLE_BLOCKS    25
#define ERASE_BLOCK_START_ADDR   32
#define ERASE_BLOCK_END_ADDR     33
#define ERASE_SELECTED_BLOCKS    38
#define CRC_ON_OFF               59

/* sends a command with parameters to the sd card */
void mmc_send_command(uint8_t cmd, uint32_t param) {
	union u32convert r;
	r.value = param;

	CS_ASSERT; 
	spi_byte(0xff);

	spi_byte(cmd | 0x40);	// send command

	spi_byte(r.bytes.byte4);	// send parameter, one byte at a time
	spi_byte(r.bytes.byte3);

	spi_byte(r.bytes.byte2);
	spi_byte(r.bytes.byte1);

	uint8_t n=0;
	if (cmd == 0) n=0x95;			
	if (cmd == 8) n=0x87;			/* CRC for CMD8(0x1AA) */
	spi_byte(n);			// CRC for first command, after that ignore
	spi_byte(0xff);			// ignore return
}

//_______________________________ wait until not busy
uint8_t mmc_get(void) {
	uint16_t i = 0xffff;
	uint8_t  b = 0xff;

	while ((b == 0xff) && (--i)) b = spi_byte(0xff);
	return b;

}

//_______________________________  wait for a 0xfe
uint8_t mmc_datatoken(void) {
	uint16_t i = 0xffff;
	uint8_t  b = 0xff;

	while ((b != 0xfe) && (--i)) b = spi_byte(0xff);
	return b;
}

//_______________________________ 
uint8_t send_cmd(uint8_t command, uint32_t param) {
	mmc_send_command(command, param);
	return mmc_get();
}


//_______________________________ release sd card
void mmc_release(void) {
	uint8_t i;
	//__________ at least 8 final clocks
	for(i=0;i<10;i++) spi_byte(0xff);	

	CS_DEASSERT;
}

//____________________________ reads a block from the SD card
int mmc_readsector(uint32_t lba, uint8_t *buffer) {

	// send command and sector
	mmc_send_command(READ_SINGLE_BLOCK, lba*BLOCK_SIZE);

	if (mmc_datatoken() != 0xfe) {	// wait for start of block token
		mmc_release();	// error
		return -1;
	}//if

	uint16_t i;
	for (i=0;i<BLOCK_SIZE;i++)
	    *buffer++ = spi_byte(0xff);

	spi_byte(0xff);					// ignore checksum
	spi_byte(0xff);					// ignore checksum

	mmc_release();

	return 0;
}

//____________________________ write a block
//         if buffer == NULL, then it clears the sector
unsigned int mmc_writesector(uint32_t lba, uint8_t *buffer) {
	uint16_t i;
	uint8_t r;
	CS_ASSERT;
	//_______________ send command and sector
	mmc_send_command(WRITE_SINGLE_BLOCK, lba*BLOCK_SIZE);
	mmc_datatoken();	// get response
	spi_byte(0xfe);		// send start block token
	
	//_______________ if given a buffer, write it
	if (buffer) {
		for (i=0;i<BLOCK_SIZE;i++)	// write sector data
			spi_byte(*buffer++);
    }//if
	else { 
		//___________ if not given a buffer, clear it
    	for (i=0;i<BLOCK_SIZE;i++)	// write sector data
			spi_byte(0x00);
    }//else

	spi_byte(0xff);	// ignore checksum
	spi_byte(0xff);	// ignore checksum

	r = spi_byte(0xff);
    
	//_________________ check for error
	if ((r & 0x1f) != 0x05) return r;
    
	//_________________ wait for SD card to complete writing and become idle
	i = 0xffff;
	//_________________ wait for card to finish writing
	while (!spi_byte(0xff) && --i) ;	
    
	mmc_release();	// cleanup

	if (!i) return -1;	// timeout error

	return 0;
}

#define rcvr_spi() spi_byte(0xff)
//________ Initialize a mmc/sd card (alternate, for mmc only)
//         rename and use this init() if u only deal w/ mmc, save some bytes
uint8_t _mmc_init(void) {
	int i;

	spi_port_init();

	for(i=0;i<10;i++)			// send 80 clocks
		rcvr_spi();

	if (send_cmd(GO_IDLE_STATE,0) != 1) {
	   mmc_release();
	   return 1;
	}//if

	// wait for initialization to finish (gets a 0 back)
	do {
	     if (send_cmd(SEND_OP_COND,0) == 0) break;
	} while (i--);
	
	if (!i)	return 2; // timed out above
	
	mmc_send_command(SET_BLOCK_LEN, BLOCK_SIZE);	//set block size to 512
	mmc_release();
	
	return 0;
}

//________ Initialize a mmc/sd card
uint8_t mmc_init(void) {
	spi_port_init();

	uint8_t n, f, ocr[4];
	int i=0;

	for (n = 20; n; n--) rcvr_spi();	// 80 dummy clocks

	CS_ASSERT;			// CS = H
	f = 0;
	if (send_cmd(0, 0) == 1) {			// Enter Idle state
		i = 0x0fff;
		if (send_cmd(8, 0x1AA) == 1) {	// SDC Ver2+
			//
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
			if (ocr[2] == 0x01 && ocr[3] == 0xaa) {	// The card can work at vdd range of 2.7-3.6V
				do {
					if (send_cmd(55, 0) <= 1 && send_cmd(41, 1UL << 30) == 0) {
						break;	// A41 with HCS bit
					}//if
				} while (--i);
				if (i && send_cmd(58, 0) == 0) {	// Check CCS bit
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					f = (ocr[0] & 0x40) ? 3 : 1;
				}//if
			}//if
		}//if
		else {							// SDC Ver1 or MMC
			if (send_cmd(55, 0) <= 1 && send_cmd(41, 0) <= 1) f = 4;	// SDC?
			do {
				if (!f && send_cmd(1, 0) == 0) break;								// 1
				if ( f && send_cmd(55, 0) <= 1 && send_cmd(41, 0) == 0) break;	// A41
			} while (i--);
			if (i) f = 1;
		}//else
		if (f == 1 && send_cmd(16, BLOCK_SIZE) != 0)	// Select R/W block length
			f = 0;
	}//if
	CS_DEASSERT;
	rcvr_spi();

	if (!(f&0x01)) return -1;

	return 0;
}

#endif

