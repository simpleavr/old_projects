
// 
//
// original methods found in 
// http://ghmicro.com/index.php?option=com_content&view=article&id=33:nokia-7110-lcd&catid=18:18f1320&Itemid=2
// it was originally for pic devices but easy to change to msp430, i take the control part
// thanks futz
// nokia 5110 initilization method taken from 43oh.com forum
// http://www.43oh.com/forum/viewtopic.php?f=8&t=1325#p8702
// thanks DanAndDusty
//


#ifndef __NOKIA_LCD
#define __NOKIA_LCD

#ifdef LCD_7110

#define CS      	BIT2
#define DATACHAR    BIT1
#define SCLK    	BIT5
#define SDATA   	BIT6
#define RST     	BIT4
//#define DATAIN      BIT2

#define LCD_WIDTH 	96
#define LCD_HEIGHT	64	// actually 65

#define UPSIDE_DOWN

#else

/*

#define SCK     BIT1		// to SCK     (BIT5 -> BIT1)
#define SDI     BIT6		// to SDO     (BIT7 -> BIT6)
#define SDO     BIT2		// to SDI     (BIT6 -> BIT2)
#define SEL     BIT7		// to nSEL    (BIT3 -> BIT7)

    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .
    .   .   .   .   .   .   +   - |Sel Rst D/C Dat Clk Led  .   .	Nokia 5110 module
    .   .   .   .   .   .   o   o | o   o   o   o   o   o   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .
    .   .   .   .   .  2.2 2.1 2.0|1.5 1.4 1.3 1.2 1.1 1.0  +   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   o   o   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | .   o   .   o   .   x   .   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .  2.3 2.4 2.5|1.6 1.7 Rst Tst 2.7 2.6  -   .   .
    .   .   .   .   .   .   .   . | .   .   .   .   .   .   .   .   .
    .   .   .   .   .   .   .   . | o   o   .   .   .   o   o   .
    .   .   .   .   .   .   .   .   .   .   .   .   .   +--[ 1k ]---+
    .   .   .   .   .   .   .   .   .   .   .   .   .   .   +---||--+
                      /Sel Dat (-) (+) Clk  .  DaI          o   474 o
					    o   o   o   o   o           o       Audio 

                       DaI (-) Clk (+) Dat Sel

                  /Sel Dat (+) Clk (-) DaI
                   DaI (-) Clk (+) Dat Sel\
                  /Sel Dat (-) (+) Clk  .  DaI
                   DaI  .  Clk (+) (-) Dat Sel\


*/
//#define CS      	BIT1
//#define RST     	BIT2
//#define DATACHAR  BIT3
//#define SDATA   	BIT4
//#define SCLK    	BIT5

//#define DATAIN  	BIT6
#define CS      	BIT5
#define RST     	BIT4
#define DATACHAR    BIT3
#define SDATA   	BIT2
#define SCLK    	BIT1

#define LCD_WIDTH 	84
#define LCD_HEIGHT	48

#endif

//______________________________________________________________________
void lcd_send(uint8_t cmd) {
    uint8_t x=8;
    P1OUT &= ~CS;

	while (x--) {
        P1OUT &= ~SCLK;
        P1OUT &= ~SDATA;
		if (cmd & 0x80)
            P1OUT |= SDATA;
        P1OUT |= SCLK;
		//if (!is_data || (x&0x01) || !(lcd_ctrl&LCD_BIG) )
        cmd <<= 1;
    }//while
    P1OUT |= CS;
}
        
//______________________________________________________________________
void lcd_goto_yx(uint8_t row, uint8_t col) {

	P1OUT &= ~DATACHAR;
#ifdef LCD_7110
	lcd_send(0xb0+row);   // page address
	col += 18;
	lcd_send(0x10+(col>>4));       // column address
	lcd_send(0x00+(col&0x0f));
#else
	lcd_send(0x40+row);
	lcd_send(0x80+col);
#endif
	P1OUT |= DATACHAR;
}

//______________________________________________________________________
void lcd_row_cls(uint8_t row){
    uint8_t x;
	P1OUT &= ~DATACHAR;
#ifdef LCD_7110
	lcd_send(0xb0+row);       //set page address
	lcd_send(0x11);       //set column address
	lcd_send(0x02);
#else
	lcd_send(0x80);
	lcd_send(0x40+row);
#endif
	P1OUT |= DATACHAR;
    for(x=0;x<LCD_WIDTH;x++) lcd_send(0x00);
}

//______________________________________________________________________
void lcd_cls(void) {
    uint8_t i;
    for (i=0;i<(LCD_HEIGHT/8);i++) lcd_row_cls(i);
}

//______________________________________________________________________
void lcd_init(void){
#ifdef RST
    P1OUT |= RST;
    P1DIR |= CS|RST|DATACHAR|SDATA|SCLK;
#else
    P1DIR |= CS|DATACHAR|SDATA|SCLK;
#endif
    __delay_cycles(200*USEC);
    P1OUT |= DATACHAR;
    P1OUT |= CS;

#ifdef RST
    __delay_cycles(200*USEC);
    P1OUT &=~RST;
    __delay_cycles(200*USEC);
    P1OUT |= RST;
#endif
    __delay_cycles(200*USEC);
	P1OUT &=~DATACHAR;

#ifdef LCD_7110

    lcd_send(0xa6);   //Display: Normal   
    lcd_send(0xA3);   //LCD Bias Settings: 1/7 (0xA2 for 1/9)
    lcd_send(0xA0);   //ADC Selection: Reverse
    lcd_send(0xA1);   //ADC Selection: Reverse
#ifdef UPSIDE_DOWN
    lcd_send(0xC8); //Common Output: Upside Down
#else
    lcd_send(0xC0); //Common Output: Normal Direction
#endif
    lcd_send(0x22);   //Set the V5 output Voltage
	lcd_send(0x81);   //set Electronic Volume - brightness
    lcd_send(0x2f);   
    lcd_send(0x2E);   //Power Controller Set:
    lcd_send(0x2F);   //Power Controller Set: Voltage follower circuit: ON

    lcd_send(0xE3);   //Non-OPeration Command
    lcd_send(0x40);   //Set the start line

    lcd_send(0xAF);   //LCD On
    lcd_send(0xA4);   //Display All Points: NORMAL

#else

	lcd_send(0x21);		// LCD Extended Commands
	lcd_send(0xBF);		// Set LCD Vop (Contrast)
	lcd_send(0x06);		// Set the Temp coefficient
	lcd_send(0x13);		// LCD Bias mode 1:48
	lcd_send(0x20);		// LCD in Normal mode (non inversed)
	lcd_send(0x08);		// ** TODO ** Unsure of this command here
	lcd_send(0x0C);		// ** TODO ** Unsure of this command here

#endif
	P1OUT |= DATACHAR;
    //lcd_cls();
}
 
#endif
