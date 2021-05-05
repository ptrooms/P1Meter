## Purpose
This file show the compiel differences and explain why changing the bittimne
calculation in SoftwareSerial241/V31 will change intstructions cycles.

## The disassembly libs/SoftwareSerial241-P1/SoftwareSerial241.cpp  v31 2021-04-310
-- decoding, to check intruction path length for various combinations, done via:
    ~/.platformio/packages/toolchain-xtensa32/bin/xtensa-esp32-elf-objdump -C -d
    ~/code-P1Meter/.pio/build/p1meter-test-OTA/lib4c2/SoftwareSerial241-P1/SoftwareSerial241.cpp.o


## source libs/SoftwareSerial241-P1/SoftwareSerial241.cpp v31 2021-04-310
    rxRead SoftwareSerial241.cpp.asm disassembled code using fixed value calculation vs set direct
## void ICACHE_RAM_ATTR SoftwareSerial::rxRead() {
    .....
    .....
	s	  34:	0020c0        	memw
	s	  37:	0239      	s32i.n	a3, a2, 0
;; =================================================================================================
## unsigned long wait = m_bitTime + m_bitTime/3 - 498;  // stable code uysing harcoded value 498
		  39:	9c28      	l32i.n	a2, a12, s36							; load bitTime
	 	  3b:	330c      	movi.n	a3, 3									; dvide by 3
		  3d:	0eaed2      movi	a13, 0xfffffe0e							; move in 498 fixed value
		  40:	d2da      	add.n	a13, a2, a13							: add value
		
          42:	fff001      l32r	a0, 4 <SoftwareSerial::rxRead()-0x14>	; set start
		  45:	0000c0      callx0	a0										; calculate
		  48:	dd2a      	add.n	a13, a13, a2							; set wait result
;; -------------------------------------------------------------------------------------------------
## unsigned long wait = m_bitTime + m_bitTime/3 - m_bitWait; // using this will corrupt things
		  3b:	0e2d      	mov.n	a2, a14                                 ; load bitTime
		  3d:	330c      	movi.n	a3, 3									; divide by 3
		
          3f:	fff101      l32r	a0, 4 <SoftwareSerial::rxRead()-0x14>   ; set start
		  42:	0000c0      callx0	a0										; calculate
		  45:	acd8      	l32i.n	a13, a12, 40  							: get bitWait
		  47:	c022d0      sub	a2, a2, a13									; substract this value
		  4a:	d2ea      	add.n	a13, a2, a14                            ; set wait result
;; -------------------------------------------------------------------------------------------------
m_bitWait made volatile
## unsigned long wait = m_bitTime + m_bitTime/3 - m_bitWait; // serial read will fail with this method
	      3b:	0020c0      memw											; get/set volatile bitWait
		  3e:	acd8      	l32i.n	a13, a12, 40							; get bitWait varbl
		  40:	202ee0      or	a2, a14, a14								; load bitTime
		  43:	03a032      movi	a3, 3									; divide by 3
		
          46:	ffef01      l32r	a0, 4 <SoftwareSerial::rxRead()-0x14>	; set start 
		  49:	0000c0      callx0	a0										; calculate
		  4c:	c022d0      sub	a2, a2, a13									; subtract
		  4f:	d2ea      	add.n	a13, a2, a14							; set wait result
;; -------------------------------------------------------------------------------------------------
## unsigned long wait = m_bitWait; // mbitwait is fixed to 498 bu serial read will wail
		  35:	acd8      	l32i.n	a13, a12, 40;;							; set wait from bitWait
;; =================================================================================================
## unsigned long start = getCycleCountIram();
	s	  4a:	002020      esync
	s	  4d:	03ea30      rsr.ccount	a3
## uint8_t rec = 0;
## for (int i = 0; i < 8; i++) {
	s	  50:	8e0c      	movi.n	a14, 8
	s	  52:	0139      	s32i.n	a3, a1, 0
	s	  54:	0f0c      	movi.n	a15, 0
	s	  56:	000386      j	68 <SoftwareSerial::rxRead()+0x50>      
	s	  59:	2c0c22      l8ui	a2, a12, 44
	s	  5c:	008256      bnez	a2, 68 <SoftwareSerial::rxRead()+0x50>
	s	  5f:	01a022      movi	a2, 1
	s	  62:	ffe901      l32r	a0, 8 <SoftwareSerial::rxRead()-0x10>
	s	  65:	0000c0      callx0	a0 


## observed wait values and the result
We execute a number of trials to investigater the most reliable value for the Iram wait during rxRead.
the value from 497-502 between with 498 as sweetspot gives the best result:
--  about 1-2 readfailures per 100 telegram reads (each conataining 675 CRC checked bytes) @ 115K2.

Note the exact wait time is calculated (@80Mhz with 12nS): 

    Wait value (tested 2021-05-01)
        NOK 504 = 5,052 µS
        NO0	503 = 5,064
        OK1	502 = 5,076 = 424 clockcycles
    424 OK  501 = 5,088 = 425 clockcycles
    425 OK-	500 = 5,100 = 12⋅(10^-9)⋅(round(80000000/115200)+(round(round(80000000/115200)/3)−500) µS
    426	OK+	499 = 5,112 =                 = 426 cycles
    427	OK=	498 = 5,124 = 694 + 231 - 498 = 427 cycles
    428	OK*	497 = 5,136 = 694 + 231 - 497 = 428 cycles
    429	OK-	496 = 5,148 =                 = 429 cycles
            495 = 5,160
            494 = 5,172
            493 = 5,183
        OK1 492 = 5,196
        NOK 490 = 5,220 
        ERR	488 = 5,244 µS
        
        Note: A single bit @115k2 baud takes approx 8.6µS where we want to hit that (start)bit at approx 2/3th. A 8N1 byte starts with a startbit (High) followed by 8 data bits, no -parity and closed with 1 stop (low) bit and silcence. The next byte (hange cstate) will trigger the ISR routine.
        A single byte take about 86µS. A telegram of 675 bytes is completed in about 58,085mS
