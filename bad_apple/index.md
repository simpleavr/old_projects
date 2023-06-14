
## Bad Apple


`[November, 2010]` Initial write-up.

`[Dec 1st, 2011]` entered as november / december [project of month contest in 43oh.com](www.43oh.com/forum/viewtopic.php?p=13840#p13840)


**Jukebox Like Device Plays Bad Apple Meme.**


<iframe width="560" height="315" src="https://www.youtube.com/embed/N" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
<br><br>

 <img src="images/bad_apple.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/>


### Description


- Reproduction of the internet meme "bad apple" sound and sight, in a self contained device
- Ti launchpad / msp430 low end device demo
- msp430 spi usage, bit-bang style, on two devices
- Lcd module access demonstrated
- Sd / mmc low level access demonstrated
- Polyphonic tone generation via pwm demonstrated



### Parts / Bill of Materials


- This is what you will need to construct this project
- A ti launchpad (or most 2k+ msp430 devices + programmer)
- Nokia 7110 or 5110 lcd module, preferrably in breakout
- 1k-10k resistor (whatever comes in handy)
- 0.1uF or 0.22uF capacitor (again, whatever comes in handy)
- A mmc / sd memory card and card adaptor, 2Mb will do
- A 170 tie-points mini breadboard (or pref board)
- A headphone adaptor

### Schematic



```
//               MSP430F20x2/3       ----------o--------------o Audio out
//             -----------------     |         |              o 
//         /|\|              XIN|-   |        ---             |
//          | |                 |   .-.       --- 10-500nF    |
//          --|RST          XOUT|-  | | 1-10K  |              |
//            |                 |   |_|        |---------------
//      LED <-|P1.0             |    |        ___                               
//            |                 |    |        ///    SD / MMC card      /|\
//            |    pwm out  P2.6|-----           ---------------------   |
//            |                 |               |                Vcc 4|---
//            |    as CS    P1.7|-------------->|1 CS, select device  |
//            |    as MOSI  P1.6|<--------------|7 MISO, data out     |
//            |                 |   ----------->|2 MOSI, data in      |
//            |                 |   | --------->|5 CLK, spi clock     |
//            |                 |   | |         |                Gnd 3|---
//            |                 |   | |          ---------------------   |
//            |                 |   | |                                 ---
//            |                 |   | |           Nokia 5110 breadout   ///
//            |                 |   | |   ----------------------------- 
//            |    as SCLK  P1.1|---|-o->|7 SCLK, spi clock            |
//            |    as MISO  P1.2|---o--->|6 MOSI, data in              |
//            |    as D/C   P1.3|------->|5 D/C, data or control       |
//            |    as RST   P1.4|------->|4 RST, reset, low active     |
//            |    as SCE   P1.5|------->|3 SCE, select , low active   | /|\
//            |                 |    ----|2 Gnd                        |  |
//            |                 |    |   |                        Vcc 1|---
//                                  ---   -----------------------------
//                                  ///
//
//        SD (back side)                          Micro-SD (back side)
//        __________________                      __________________
//       /  1 2 3 4 5 6 7 8 | 1. CS      .. SEL  /  1 2 3 4 5 6 7 8 | 1. NC            
//       |9                 | 2. DATAIN  .. SDO  |                  | 2. CS      .. SEL
//       |                  | 3. GND     .. Gnd  |                  | 3. DATAIN  .. SDO
//       |  S O - + C   I   | 4. 3.3V    .. Vcc  |    S O + C - I   | 4. 3.3V    .. Vcc
//       |                  | 5. CLK     .. Clk  |                  | 5. CLK     .. Clk
//       |                  | 6. GND/NC  .. Gnd  |                  | 6. GND/NC
//       |                  | 7. DATAOUT .. SDI  |                  | 7. DATAOUT .. SDI
//       |                  | 8. DAT1            |                  | 8. DAT1
//       |                  | 9. DAT2            |                  | 9. DAT2
```


The circuit is consist of three modules.


- A sd / mmc module that reads content from sd / mmc card
- A lcd module that show video on nokia 7110 or 5110 lcds
- A polyphonic pwm output implementation, 8 channels are used in this project
- The sd / mmc module and the lcd module shares some pins
- Both sd / mmc module and the lcd module are isolated and can be included in other projects
- The core part of the code implements pwm sound and controls timing for content reads and displays


### SD / MMC Module



The sd / mmc module talks to a sd memory card via spi interface. We are using low level raw device access for simpicity reasons and the fact that we do not have enough program space for a proper file system. The same resource constraint also force us to use a non 512 byte read sector. Instead our read block is only 16 bytes long. This can be adjusted to 64 bytes for a bigger device (430g2452/2553 for example). Bit-banging is used for spi for freedom on layout / pin selection and comptability between low-end and high-end devices.


The sd card contains the video and sound data to reproduce "bad apple". It is read on the fly to create the sound and sight show. Data content for both the video and sound can be created via the supplied zcontent.pl script. Input for content includes an avi video file and a midi file. This script can be used to generated alternate contents for most avi and midi files.



### Polyphonic PWM Out Implementation



This is almost a direct copy from a pic project i found, www.pic24.ru [link](http://www.pic24.ru/doku.php/en/osa/articles/pk2_osa_piano)


The pwm output uses timera with a base / sampling frequency of about 16khz. Timer is setup w/ pin toggling. At each sampling frequency triggered interrupt the following is done;

- Loop thru all sound channels and sum them up, divide and setup CCR1 for mixed frequency
- For each channel, frequency is sine wave adjusted via a 64 phased sine table

Channel sound / midi contents are kind of serially encoded, read from the sd card as a sequence of bytes. For example, the following shows 2 sequences / steps of channel changes



```
 0x78, 0x03, 0x1b, 0x27,
 0x3c, 0x0f, 0x00, 0x00, 0x2a, 0x33,
```

- The 1st byte signals the duration of this step. i.e. 0x78 time units before next step.
- The 2nd byte shows which channel needs change. i.e. 0x03 = b00000011 = channel 0 and 1.
- The next 2 bytes is the "note" for the two channels, i.e. ch 0 = 0x1b, ch 1 = 0x27.
- After setting the above channel frequencies, core loop waits until 0x78 time units expire.
- During the wait, at about 16khz frequency, interrupt handler will apply channel freqeuncy plus sine table phase adjustments to timera. this realizes the pwm output.
- When the next step kicks in, the new duration is 0x3c time units
- The 2nd byte in this step shows 0x0f = b00001111, ie, channel 0 to 3 needs to be adjusted.
- The next 4 bytes turns off channel 0 and 1, sets channel 2 and 3 to 0x2a and 0x33 respectively.


### Video LCD Show Implementation



It was originally for pic devices but easy to change to msp430, i take the control part link also nokia 5110 initilization method taken from 43oh.com forum link i originally stage the project w/ 7110 lcd and later turn to a 5110 for simplicity. The 7110 display did show more detail.


The video show is achieved by writing to the lcd successively. Contents are kind or run-length encoded from bmp files, which in turn was extracted / converted from an avi source. Pixel output to lcd is done frame by frame with a software timer delay. The lcd control logic is structured such that 6 rows of 84 bytes are written via spi successively. At the break of each row, we also check to see if we need to update the sound channels. This provides better sound / sight synchronization over doing it at the end of each frame.


Video contents are read from the sd card at offset 0. For each word of data, the highest 3 bits represent what the word is for. A word can carry

- If bit 15 is set, run-length of "black" pixels follows. i.e. 0x8234 means the next 0x0234 pixels are "black"
- If bit 14 is set, run-length of "white" pixels follows. i.e. 0x4234 means the next 0x0234 pixels are "black"
- If bit 12 is set, the lower 8 bit represents the next 8 pixels. i.e. 0x203c means the next 8 pixels are 00111100.
- The encoding is not optimized for space but rather selected for run-time efficiency. we do have a lot of flash space in a typical sd card.


### Construction



The initial project was done on a mini breadboard. With the small number of parts in this project one can easily do the layout directly from the schematic. There are a few notes though.


- For the nokia 5110 breakout, the spi pin mapping is selected so that the breakout board aligns nicely w/ the launchpad.
- The sd card adapter shares two spi pins (CLK and DATA) with the nokia lcd, you just need two additional wire for CS (chip select) and DATA OUT. that is apart from power.
- The low pass audio filter is constructed very casually. i just took the resistor / capacitor that lies closest to me.
- Alternatively, you can construct this project by putting together everything on a perfboard w/ a bit of soldering work.

Lazy no-shield breadboarding, tie-points wired, ready to mount main components


 <img src="images/bad_apple_01.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/>


Assembled


 <img src="images/bad_apple_02.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/>


Sd card adaptor used to host micro-sd card


 <img src="images/bad_apple_03.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/>


Front view as how the clip is shown


 <img src="images/bad_apple_04.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/>


### Firmware



The firmware was done under ubuntu 11.10 w/ mspgcc4 + mspdebug as programmer. It is also test built under windows 7 with cygwin environment, mspgcc4 is also used and a MSP430Flasher.exe (command line programmer released by TI) is used to program the launchpad. It should build also w/ CCS as i had compiler directives to switch code portions. However i did not verify, you are on your own if you are on CCS or IAR.


The are additional details / options that you can tweak, they are explained in the source code. They are not listed here as you only need them if you built the project.



### Content Generation


Data content for both the video and sound can be created via the supplied zcontent.pL script. Input for content includes an avi video file and a midi file. This script can be used to generated alternate contents for most avi and midi files.


The followings are needed for content generation

- We need an avi file for video
- We need a midi file for audio, prefer about 8 channel simple paino instrument
- Supporing utility / tools
- Should be done in a linuxish machine or w/ cygwin
- unixish bin-utils, etc. we need "sort" for sure
- Perl, the internet duct tape to tie things together
- Certian perl modules, like "Image::BMP", "POSIX"
- midicsv and whatever he requires, this is for midi extraction
- ffmpeg to extract bitmaps from video
- dd utility (part of linux / cygwin) to write content binary to sd / mmc card

This is what u get

- Creates sound.h which contains sound data
- Creates content.h that contains video data
- out.img raw binary image to be flash to sd / mmc card


### Downloads



Source files

~~Click to download bad_apple.c~~

~~Click to download common.h~~

~~Click to download nokia_lcd.h~~

~~Click to download mmc.h~~

~~Click to download param_5110.h~~

~~Click to download param_7110.h~~


Content generation script

~~Click to download zcontent.pl~~


Firmware binary for mspdebug

~~Click to download bad_apple.elf~~


Firmware binary for ti MSP430Flasher.exe / ccs, etc

~~Click to download bad_apple.hex~~


Sd card raw content (for use w/ dd)

~~Click to download out_5110.img~~

~~Click to download out_7110.img~~


Content source, compressed package includes midi and avi

~~Click to download bad_apple_midi_avi.tgz~~


All source files now reside on [my repository](https://github.com/simpleavr/old_projects/tree/master/bad_apple)


