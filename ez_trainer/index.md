## EZ430Trainer


**A Retro Style Basic Computer.**


`[December, 2012]` Initial write-up.

`[Dec 9th, 2012]` entered as november / december project of month contest in 43oh.com.

 http://forum.43oh.com/topic/2839-open-nov-dec-2012-43oh-project-of-the-month-contest/


 (c) chris chung 2012.12.10


 <img src="images/ez_trainer.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/>



(*) `this project can only be built with msp430gcc ,cygwin or linux`, i had made attempt to build it under CCS but it proved to me very difficult as there are assembly elements and memory mapping issues. there is, however a reduced c source file to build just the LMC machine.)



### Description

The EZ430trainer system turns the low-cost TI LaunchPad into a microcontroller trainer via simple hardware and software enrichments.

The EZ430trainer is made up of a

- TI LaunchPad, w/ msp430g2553 mcu
- A "Paradiso UI Shield", adding basic I/O capabilites.
- A "Eztrainer Monitor" firmware, providing monitor and assembler capabilites.

The intention of the EZ430trainer is to bring in a "low-cost" computer training system that resembles the 1st generation hobby based microprocessor systems that surfaced in the mid-70's. These systems offer the best computer training as users interacts w/ the CPU almost directly. Programming are done via entering machine code directly or via simple assemblers. Users need to understand very well the cpu architecture in order to work on such systems.

Parts / Bill of Materials

- TI Launchpad V1.5 kit (w/ msp430g2553 mcu)
- 4 digit 7 segment LED display Common Anode
- 4 digit 7 segment LED display Common Cathode
- 20 x tactile button
- buzzer 3V (Optional)


the LED display is commonly available in ebay, look for those 0.4" or 0.36" 4 digit displays, you will want to make sure it's common cathode and the pin layout matches (most do)


```
       ___a__
      |      |        (0) A  F (1)(2) B
     f|      | b      -+--+--+--+--+--+
       ___g__        |                 |
     e|      | c     |Pin1             |
      |      |        -+--+--+--+--+--+
       ___d__          E  D  .  C  G (3)
```

### Paradiso UI Shield Schematic


```

               MSP430G2553 on TI Launchpad
             -----------------
         /|\|                 |
          | |                 |
          --|RST              |
            |                 |     
            |  as digit 0 P2.2|-----+---------------+  +---------------+ led modules
            |  as digit 1 P2.4|-----| _   _   _   _ |  | _   _   _   _ | 1x common cathode
            |  as digit 2 P2.5|-----||_| |_| |_| |_||  ||_| |_| |-| |_|| 1x common anode
            |  as digit 3 P2.7|-----||_|.|_|.|_|.|_||  ||_|.|_|.|_|.|_||
            |                 |     +---------------+  +---------------+
            |                 |        | segment a to g + dot........
            |                 |       /                                 ....\
            |                 |      /                                       \
            |   segment A P1.7|-----+-----+-----+-----+-----+-----+-----+-----+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment B P2.3|-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment c P1.7|-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment D P2.0|-o o-+-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ | _=_ |
            |   segment E P2.6|-o o-+-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ | _=_ |
            |   segment F P1.6|-o o-+-o o-+-o o-+-o o-+
            |                 | _=_ | _=_ | _=_ |
            |   segment G P2.7|-o o-+-o o-+-o o-+
            |                 | _=_ |
            |   segment H P1.5|-o o-+           (not all buttons populated)
            |                 |
             -----------------
 
	mapping of buttons. Ex. A-F .. buon across segment A and F pins

                [A-F] [A-.] [A-G] [B-A] [D-A] [C-A]

                [A-E] [B-F] [B-.] [B-G] [D-B] [C-B]

                      [B-E] [D-F] [D-.] [D-G] [C-D]

                      [D-E] [C-E] [C-F] [C-.] [C-G]

          +-----------------------------------------+
          | +-----------------+ +-----------------+ |
          | |  _   _   _   _  | |  _   _   _   _  | |
          | | |_| |_| |_| |_| | | |_| |_| |_| |_| | |
          | | |_|.|_|.|_|.|_|.| | |_|.|_|.|_|.|_|.| |
          | +-----------------+ +-----------------+ |
          |                mnp  rtx  s    j         |
          |           [  ] [7 ] [8 ] [9 ] [F ] [Ad] | Address
          |                          b/w  ei     << |
          |           [  ] [4 ] [5 ] [6 ] [E ] [Md] | Mode / Last
          |                               d    . >> |
          |                [1 ] [2 ] [3 ] [D ] [Rg] | Register / Next
          |                     a    b    c         |
          |                [0 ] [A ] [B ] [C ] [+ ] | Advance / Enter
          |                                         |
          +-----------------------------------------+
```
### Paradiso UI Shield features

- 8 x 7 segment display
- 22 tactile switch positions w/ 20 switch filled
- Prototyping area
- requires software mutliplexing to drive segments
- requires software matrix scanning for key inputs
- has no led driving / current limiting capabilities

### EZ430trainer Monitor features


- (1) LMC Little Man Computer emulator built-in

- (2) One-pass assembler generating msp430 machine code

- (3) Direct machine code entry for msp430

- Correct support for pseudo-op codes, ex. RET implemented as MOV @SP+,PC

- Interface w/ Paradiso UI Shield and UART terminal

- Program storage starts at flash 0xe000 at 256 byte blocks

- Program edit at ram 0x0280 for each 256 byte blocks

- Basic IO routines

  * number printing

  * string printing

  * wait key w/

  * timer at 1/100 sec intervals



### Memory Space

```
          +------------------------------------------------+
          |0x0000-0x0007   Special Func Registers          |
          |0x0008-0x00ff    8-bit Periphercals             |
          |0x0100-0x01ff   16-bit Periphercals             |
          |0x0200        + 512 byte RAM -------------------|
          |  0200-  027f | Monitor Use                     |
          |  0280-  037f | Program Edit Buffer             |
          |  0380-  03ff | Monitor Use, Stack              |
          |       0x03ff-+---------------------------------|
          | ....                                           |
          |0x1000        + Data Flash ---------------------|
          |  1000-  103f | Data Flash Bank 0               |
          |  1040-  107f | Data Flash Bank 0               |
          |  1080-  10bf | Data Flash Bank 0               |
          |  10c0-  10ff | Data Flash Bank 0 (Factory Data)|
          |       0x10ff +---------------------------------|
          | ....                                           |
          |0xc000        + Program Flash ------------------|
          |  c000-  deff-| Monitor Code                    |
          |  dd00-  dfff-| Bios code / dd00 is entry point |
          |  e000-  efff-| 16 banks of User Code           |
          |  f000-  ffbf-| 16 banks of LMC code            |
          |  ffc0-  ffff | Interrupt Vectors               |
          |       0xffff +---------------------------------|
          +------------------------------------------------+
```
<iframe width="560" height="315" src="https://www.youtube.com/embed/BDqhotMs" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
<br><br>

### EZ430Trainer Monitor Power Up

```
          We 1st need to familiarize w/ the EZ430Trainer keypad

                  |                mnp  rtx  s    j         |
                  |           [U0] [7 ] [8 ] [9 ] [F ] [Ad] | Address
                  |           [U1] [4 ] [5 ] [6 ] [E ] [Md] | Mode / Last
                  |                [1 ] [2 ] [3 ] [D ] [Rg] | Register / Next
                  |                [0 ] [A ] [B ] [C ] [+ ] | Advance / Enter


          [Ad] press to enter address or toggle monitor operation mode
          [Md] switch between machine code and assembler
                   not used in LMC mode
                   when in Monitor mode, step back to last program bank
          [Rg] cycle thru registers 2 to 15 (f) when in assembler mode
                   when in LMC mode, step back to last instruction
                   when in Monitor mode, advance to the next program bank
          [+ ] advance to the next instruction of word
                   when in LMC mode, advance to next instruction
                   when in Monitor mode, edit or run current "program bank"
          [0 ]..[F ]
                   generate hex value entry
                   when in assembly mode, keys [7 ]..[F ] cycles thru assembler op-codes

          After power-on, press any key to bring the EZ430Trainer Monitor to live
          You will be greeted by the following LED message

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
          (Any key)                 E000  run  only upon power on

          pressing [Ad] will cycle thru one of the four monitor options

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
                                    E000  run  for msp430 program run
          [Ad]                      E000  mod  for msp430 program modification
          [Ad]                      F000  run  for LMC program run
          [Ad]                      F000  mod  for LMC program modification
          [Ad]                      E000  run  back to msp430 program run


          pressing [Rg] or [Md] key will advance / retreate the program bank number

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
                                    E000  run
          [Rg][Rg]                  E200  run  advance to bank 2
          [Md]                      E100  run  retreat to bank F

          pressing [0 ]..[F ] will enter absolute address into the monitor

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
                                    E000  mod
          [0 ][2 ][8 ][0 ]          0280  mod  address now points to 0x280

          pressing [+ ] key will commence the current operation

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
                                    E000  run
          [+ ]                      ???? ????  runs msp430 program in bank 0
          .... or
                                    E000  mod
          [+ ]                      ???? ????  modifies msp430 program in bank 0

```
### Entering and running a LMC program

```
          There are 16 blocks of 256 byte flash memory to store and run LMC programs
          LMC instructions are entered as 3-digit decimal codes
          [+ ] and [Rg] keys can be used to navigate code positions

          It is time to try out a little example program to demonstrate
          the system operations and ensure that everything is in order

          In this example, we will load two values into two mailox (via the accumulator)
          add them together and display the result values. We will do
          that via the LMC program editor

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
                                    F900  mod  select F??? modify function
          [+ ]                      00?? ????  modifies LMC program in bank 9
          [9][0][1]                 00IN P901  00..step,INP..op-code,901..machine code
          [3][0][6]                 01ST A306  sta 6 (store to bucket 6)
          [9][0][1]                 02IN P901  inp   (input value from keypad into accumulator)
          [1][0][6]                 03AD D106  add 6 (add bucket 6 to accumulator)
          [9][0][2]                 04OU T902  out   (output accumulator to display)
          [0][0][0]                 05HL T000  hlt   (stop cpu)
                  for navigation
          [Rg]                      04OU T902  back one step
          [+ ]                      05HL T000  forward one step


          Once you entered a LMC program you can run it via the monitor. You can also run the "canned"
          example LMC programs that comes w/ the system build.

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
                                    F900  run  select F??? modify function
          [+ ]                      901        using the above program example, wait for input
          [1][0][+ ]                901    10  enter value 10 into accumulator
                                    306        LED shows next instruction briefy
                                    901        program prompt for input again
          [2][0][+ ]                901    20  enter value 20 into accumulator
                                    106        LED shows next instruction briefy
                                    902    30  next instruction is to show result, 30
                                    000    30  next instruction is halt, program ends
                  press [Ad] key will return to monitor
```
### Entering and running a msp430 Program via assembler Op-code

```
          Press Keys                Displays   Notes
          ----------------------------------------------------------------
          [Ad]                      e000 mod   select address input
          [Md]                      ef00 mod   step back to bank f (bank 15)
          [+]                       ef00 ????  confirm for program edit
          [Md]                      ???? ????  change mode to assembler

          [7]++                     Mov  ????  press [7]s until "Mov" shows
          [+]                       Mov  r7r7  confirm "Mov" op
          [1][2][3][4]              Mov  1234  numeric entry, hex
          [+]                       Mov  1234  confirm 1st operand as 0x1234
          [Rg]++                    r8         press [Rg]s until r8 shows
          [+]                       ???? ????  confirm 2nd operand as r8
                                  Mov #0x1234,R8
          [A]++                     Add  ????  press [A]s until "Add" shows
          [+]                       Add  r7r8  confirm "Add" op
          [1][1][1][1]              Add  1111  numeric entry, hex
          [+]                       Add  1111  confirm 1st operand as 0x1111
          [Rg]++                    r8         press [Rg]s until r8 shows
          [+]                       ???? ????  confirm 2nd operand as r8
                                  Add #0x1111,R8
          [7]++                     Mov  ????  press [7]s until "Mov" shows
          [+]                       Mov  r7r7  confirm "Mov" op
          [Rg]++                    Mov  r8r7  press [Rg]s until r8 shows
          [+]                       Mov  r8r7  confirm 1st operand as r8
          [0][2][8][0]              0280       numeric entry, hex
          [+]                       ???? ????  confirm 2nd operand as 0x0280
                                  Mov R8,&0x0280
          [8]++                     Ret  ????  press [8]s until "Ret" shows
                                  Ret


          there, we had just enter a program via the assembler, now is the time to try it


          Press Keys                Displays   Notes
          ----------------------------------------------------------------
          [Ad]..                    e000 run   select address input, cycle to run
          [Md]                      ef00 mod   step back to bank f (bank 15)
          [+]                       ef00 run   run program at bank f
                                  Program executed, examine result
          [Ad]..                    e000 mod   toggle to "mod" option
          [0][2][8][0]              0280       enter hex address to examine
          [+]                       0280 2345  confirm address, show result

          Congratulations, your 1st assembler program had executed correctly,
          here's the program source listing

          Mov #0x1234,R9        ;move 0x1234 into register R8
          Add #0x1111,R9        ;add value of R9 to R10
          Mov R10,&0x0280       ;move (copy) value of R10 to address location 0x0280
          Ret

          To re-capture the process in more detail, here is what happened

          . by pressing [Ad], we enter the "address input" mode.
                . the default program space of 0xe000 are shown.
                . you can enter any direct address via the numeric keys (0..F).
                . or you can press [Rg] register key to advance to the next program bank.
                . there are 16 program banks of 256 bytes starting from address 0xe000,0xe100,..
                . press [+] will commit the address entry in hex entry mode (machine code)
                . press [Md] "mode" key will commit the entry and place system into assembler mode
                . subsequent presses of [Ad] while in address mode will toggle the "run" flag

          . in hex mode, the display will show a 4-digit address plus it's 16bit content
                . ex. e000 4039 indicated the content of address 0xe000 is 0x4039
                . to change the content in the current address space, use (0..F) and [+] keys

          . in assembler mode, the monitor dis-assemble the instruction at the current address location
                . the dis-assmebled op-code and associated operand(s) are shown
                . if there is not enough room to show the 2nd operand, you can press [0] to show it.
                . ex Mov #0123,R7 is displayed as "Mov  0123", "R7       " alternatively via [0] key
```
### Bios

```
        The system bios is compiled in at memory address 0xdd00
        applications can call bios functions via op-code "call 0xdd00"
        when calling the bios;
        . command number must be placed at r12h (high byte)
        . 1st parameter be placed at r12l (low byte)
        . additional / optional parameter be placed at r13 (full word)

        r12 (r12 high byte) contains command number (0..255)
        r12 (r12 low byte)  contains optional parameter
        r13                 optional parameter

        the following is the list of bios functions available

        r12h r12l (xx bits are not used)
        -------------------------------------------------------------------------------
        0x00 0bxxxxCPPP led clear / position, C..clear, PPP..position
        0x01 0bxxxxxxxx led put string, r13 contains ponter to string
        0x02 0bxxxxxxPP led show hex value, PP..position (x2)
        0x03 0bxxxxxZPP led show decimal value, Z..leading zero, PP..position (x2)
        0x04 0bDDDDDDDD led show dots, DDDDDDDD.. one or more dot positions to light up
        0x05 (char)     led put char, (*) note not all ascii values can be displayed
        0x06 (token)    led put token (internal character), see led.h for mapping
        0x10 (time)     wait for key press or time expiry
                        time in units of 1/100 secs. put 100 for 1 sec wait time
                        if time = 0, wait forever
                        returns r12h=time remaining, r12l=key pressed (0) means no key pressed
        0x11 (seed)     not accurate random number request, r12=<random word>
        0x20 0bxxxxxxxx read decimal number, r12=<entered word>

    a typical call to bios via assembler looks like this

        mov     #HELLO, r13
        mov     #0x0100, r12
        call    #0xdd00
        HELLO:  .ascii "hello"
```
### Assembler Advance Details

```
        In the last section, we know how to enter a simple assembly program. First, we select the opcode,
		then we enter the zero, one or two operands required for the particular opcode. 
		They are different distinct input / edit stages.

        The monitor tracks at what stage the input / edit is occuring and the key press are processed different at that context.

        At opcode edit stage, only keys A to F, 7, 8, 9 and 6 are used. With the exception of '6', 
		each key cycles thru a list of msp430 assembler opcode when pressed. 
		The key '6' toggles between byte and word operands for those operations that observes it. (i.e. mov vs. mov.b)

        7 (m,n,p) ..  mov, nop, pop, push
        8 (r+) .....  ret, reti, rla, rlc, rra, rrc, tst, xor
        9 (s) ......  sbc, setc, setn, setz, sub, subc, swpb, sxt
        A (a) ......  adc, add, addc, and
        B (b) ......  bic, bis, bit, br
        C (c) ......  call, clr, clrc, clrn, clrz, cmp
        D (d) ......  dadc, dadd, dec, decd, dint
        E (e,i) ....  eint, inc, incd, inv
        F (j) ......  jc, jeq, jge, jl, jmp, jn, jnc, jne

        The key / opcode mapping is decide in part by the 1st letter of opcode and in part for usability. Example;

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
          [A]                       adc  r7r7  1st 'A' shows adc, adc blinks
          [A]                       add  r7r7  2nd 'A' shows add, add blinks
          [6]                       addb r7r7  '6' toggles to byte mode, now it's add.b
          [+]                       addb r7r7  confirm "add" operation

        At operand edit stage, the pressing numeric keys 0..F allows for direct input of vales, 
		if register operand is desired, the [Rg] key should be pressed. 
		Pressing [Rg] continously cycles thru the available registers.
        Suppose we want to complete that last commnd to make "add.b #0x0123, r10";

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
          ....
          [+]                       addb r7r7  confirm "add" operation
          [0][1][2][3]              addb 0123  1st operand entered as 0x0123
          [+]                       addb 0123  confirm 1st operand
          [Rg]                      r7         2nd operand [Rg] for register
                (*note now display moves to 2nd page)
          [Rg][Rg][Rg]              ra         press [Rg] 3 times to advance to R10/Ra
                (*note R10 register is represented as Ra)

        For some two operand commands, they may not be enough room to display everthing, in such cases, 
		the monitor will advance to a second page of display. 
		User can use the '0' key to toggle page 1 and page 2 of the command.

        Indirect and other addressing modes, 
		are handles with the [Ad] key and is used as a "modifier" when entering operands.
        Suppose instead of "add #0x0123, r10" we want to do "add @0x0123, @r10";

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
          ....
          [+]                       addb r7r7  confirm "add" operation
          [0][1][2][3]              addb 0123  1st operand entered as 0x0123
          [Ad]                      addb 0123. toggle addressing mode
          [+]                       addb 0123. confirm 1st operand
          [Rg]                      r7         2nd operand [Rg] for register
          [Rg][Rg][Rg]              ra         press [Rg] 3 times to advance to R10/Ra
          [Ad]                      ra.        toggle addressing mode
                (*note a "dot" after the operand indicates indirect addressing)

        Indirect address w/ increment was implemented but eventually was taken out as it's seldom used 
		and is really confusing on our tiny display. Readers interested can study the source code to redo it.

        Jumps, has special treatments. Jumps always uses plus or minus relative addressing. 
		When entering relative position of jumps, we use the [<<] and [>>] keys, 
		which are the same as the [Md] and [Rg] keys.

        Suppose we want to do "jz $-10"

          Press Keys                Displays   Notes
          ----------------------------------------------------------------
          ....
          [F]...                    jz   -2    press [F] until we reach jz
          [Rg]..                    jz   -10   press [Rg] 4 times until we get -10
                (*note key [Md] goes +2 each pressing)
```
### Example Assembler Programs

```
        0xe000 display example
                prints "paradiso", "ui boost"
                wait for key, prints key code
                exit if same key pressed twice

        0xe100 simple counter example
                decimal up counts
                any key exits

        0xe200 simple clock example
                store hhmm as bcd in r10 (rA)
                sec in r11 (rB)
                use wait key 100ms to advance seconds
                hh:mm:ss in r10,r11,r12 (rA,rB,rC)
                ss >= 60.. reset seconds advance mm
                mm >= 60.. reset seconds advance hh
                hh >= 24.. reset hh
                bios show hhmm + sec
                upper-right key exits
                other key will prompt to enter time

        0xe300 lunar lander example
                r7 - height, r8 - velocity, fuel not implemented
                shows height + vertical velocity
                crash when height = 0 and velocity > 20
                upper-right key exits
                other key burns rocket

        0xe400 xxxxx xxxx xxxxxxx example
                16 xxxxxx per level
                3 xxxxx
                xxxxx xxxxxxxxxx implemented
                xxxxx xxxxxx quicker after each level
                upper-right key xxxxx
                lower-right key xxxxxx
                other key cycles xxxxxx digit

                * note.. this program is bigget than 256 bytes,
                  don't put anything in bank 5 (0xe5000)

        There are also two LMC pre-load programs (adder + counter) in the system.
```


### Little Man Computer


<iframe width="560" height="315" src="https://www.youtube.com/embed/HNK9imVcz_E" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
<br><br>

### Lunar Lander


<iframe width="560" height="315" src="https://www.youtube.com/embed/TbD8qBQw4qk" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
<br><br>

### Related Files


~~D55524_5x10.zip is the production ready seeedstudio compliance file package~~

~~ez430trainer.tgz are source files in tgz format ,tarred, gzipped~~

(*) ~~this project can only be built with msp430gcc ,cygwin or linux~~

source code now reside in [my github repository](https://github.com/simpleavr/old_projects/tree/master/ez_trainer)


lmc.c contains only the LMC machine (compiles on mps430gcc and CCSV4), for CCS, create a lmc project and rename lmc.c to main.c to build





