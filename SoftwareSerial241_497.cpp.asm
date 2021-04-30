
/home/pafoxp/code-P1Meter/.pio/build/p1meter-production/lib4c2/SoftwareSerial241-P1/SoftwareSerial241.cpp.o:     file format elf32-xtensa-le


Disassembly of section .text._ZN6Stream9readBytesEPhj:

00000000 <Stream::readBytes(unsigned char*, unsigned int)>:
   0:	0258      	l32i.n	a5, a2, 0
   2:	f0c112        	addi	a1, a1, -16
   5:	6558      	l32i.n	a5, a5, 24
   7:	3109      	s32i.n	a0, a1, 12
   9:	0005c0        	callx0	a5
   c:	3108      	l32i.n	a0, a1, 12
   e:	10c112        	addi	a1, a1, 16
  11:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial4readEv:

00000000 <SoftwareSerial::read()-0x4>:
   0:	000000        	ill
	...

00000004 <SoftwareSerial::read()>:
   4:	f0c112        	addi	a1, a1, -16
   7:	21c9      	s32i.n	a12, a1, 8
   9:	02cd      	mov.n	a12, a2
   b:	1c0222        	l8ui	a2, a2, 28
   e:	3109      	s32i.n	a0, a1, 12
  10:	11d9      	s32i.n	a13, a1, 4
  12:	e29c      	beqz.n	a2, 34 <SoftwareSerial::read()+0x30>
  14:	dc38      	l32i.n	a3, a12, 52
  16:	cc28      	l32i.n	a2, a12, 48
  18:	181237        	beq	a2, a3, 34 <SoftwareSerial::read()+0x30>
  1b:	fc28      	l32i.n	a2, a12, 60
  1d:	223a      	add.n	a2, a2, a3
  1f:	0002d2        	l8ui	a13, a2, 0
  22:	231b      	addi.n	a2, a3, 1
  24:	ec38      	l32i.n	a3, a12, 56
  26:	fff601        	l32r	a0, 0 <SoftwareSerial::read()-0x4>
  29:	0000c0        	callx0	a0
  2c:	dc29      	s32i.n	a2, a12, 52
  2e:	0d2d      	mov.n	a2, a13
  30:	000086        	j	36 <SoftwareSerial::read()+0x32>
  33:	f27c00        	rems	a7, a12, a0
  36:	3108      	l32i.n	a0, a1, 12
  38:	21c8      	l32i.n	a12, a1, 8
  3a:	11d8      	l32i.n	a13, a1, 4
  3c:	10c112        	addi	a1, a1, 16
  3f:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial8P1activeEv:

00000000 <SoftwareSerial::P1active()>:
   0:	210222        	l8ui	a2, a2, 33
   3:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial9availableEv:

00000000 <SoftwareSerial::available()>:
   0:	210242        	l8ui	a4, a2, 33
   3:	023d      	mov.n	a3, a2
   5:	020c      	movi.n	a2, 0
   7:	159427        	bne	a4, a2, 20 <SoftwareSerial::available()+0x20>
   a:	1c0352        	l8ui	a5, a3, 28
   d:	042d      	mov.n	a2, a4
   f:	d58c      	beqz.n	a5, 20 <SoftwareSerial::available()+0x20>
  11:	c348      	l32i.n	a4, a3, 48
  13:	d328      	l32i.n	a2, a3, 52
  15:	c02420        	sub	a2, a4, a2
  18:	0042d6        	bgez	a2, 20 <SoftwareSerial::available()+0x20>
  1b:	e338      	l32i.n	a3, a3, 56
  1d:	802230        	add	a2, a2, a3
  20:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial5flushEv:

00000000 <SoftwareSerial::flush()>:
   0:	030c      	movi.n	a3, 0
   2:	214232        	s8i	a3, a2, 33
   5:	030c      	movi.n	a3, 0
   7:	d239      	s32i.n	a3, a2, 52
   9:	c239      	s32i.n	a3, a2, 48
   b:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial4peekEv:

00000000 <SoftwareSerial::peek()>:
   0:	1c0232        	l8ui	a3, a2, 28
   3:	e38c      	beqz.n	a3, 15 <SoftwareSerial::peek()+0x15>
   5:	d238      	l32i.n	a3, a2, 52
   7:	c248      	l32i.n	a4, a2, 48
   9:	081437        	beq	a4, a3, 15 <SoftwareSerial::peek()+0x15>
   c:	f228      	l32i.n	a2, a2, 60
   e:	323a      	add.n	a3, a2, a3
  10:	000322        	l8ui	a2, a3, 0
  13:	f00d      	ret.n
  15:	f27c      	movi.n	a2, -1
  17:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial5writeEh:

00000000 <SoftwareSerial::write(unsigned char)-0x2c>:
	...

0000002c <SoftwareSerial::write(unsigned char)>:
  2c:	d0c112        	addi	a1, a1, -48
  2f:	a1c9      	s32i.n	a12, a1, 40
  31:	02cd      	mov.n	a12, a2
  33:	1e0222        	l8ui	a2, a2, 30
  36:	91d9      	s32i.n	a13, a1, 36
  38:	81e9      	s32i.n	a14, a1, 32
  3a:	b109      	s32i.n	a0, a1, 44
  3c:	71f9      	s32i.n	a15, a1, 28
  3e:	0e0c      	movi.n	a14, 0
  40:	74d030        	extui	a13, a3, 0, 8
  43:	0292e7        	bne	a2, a14, 49 <SoftwareSerial::write(unsigned char)+0x1d>
  46:	003ac6        	j	135 <SoftwareSerial::write(unsigned char)+0x109>
  49:	200c22        	l8ui	a2, a12, 32
  4c:	0512e7        	beq	a2, a14, 55 <SoftwareSerial::write(unsigned char)+0x29>
  4f:	ffa022        	movi	a2, 255
  52:	30dd20        	xor	a13, a13, a2
  55:	2c0c22        	l8ui	a2, a12, 44
  58:	428c      	beqz.n	a2, 60 <SoftwareSerial::write(unsigned char)+0x34>
  5a:	ffe901        	l32r	a0, 0 <SoftwareSerial::write(unsigned char)-0x2c>
  5d:	0000c0        	callx0	a0
  60:	1f0c22        	l8ui	a2, a12, 31
  63:	928c      	beqz.n	a2, 70 <SoftwareSerial::write(unsigned char)+0x44>
  65:	180c22        	l8ui	a2, a12, 24
  68:	130c      	movi.n	a3, 1
  6a:	ffe601        	l32r	a0, 4 <SoftwareSerial::write(unsigned char)-0x28>
  6d:	0000c0        	callx0	a0
  70:	140c22        	l8ui	a2, a12, 20
  73:	130c      	movi.n	a3, 1
  75:	9ce8      	l32i.n	a14, a12, 36
  77:	ffe401        	l32r	a0, 8 <SoftwareSerial::write(unsigned char)-0x24>
  7a:	0000c0        	callx0	a0
  7d:	002020        	esync
  80:	03eaf0        	rsr.ccount	a15
  83:	140c22        	l8ui	a2, a12, 20
  86:	030c      	movi.n	a3, 0
  88:	ffe101        	l32r	a0, c <SoftwareSerial::write(unsigned char)-0x20>
  8b:	0000c0        	callx0	a0
  8e:	000386        	j	a0 <SoftwareSerial::write(unsigned char)+0x74>
  91:	2c0c22        	l8ui	a2, a12, 44
  94:	008256        	bnez	a2, a0 <SoftwareSerial::write(unsigned char)+0x74>
  97:	01a022        	movi	a2, 1
  9a:	ffdd01        	l32r	a0, 10 <SoftwareSerial::write(unsigned char)-0x1c>
  9d:	0000c0        	callx0	a0
  a0:	002020        	esync
  a3:	03ea20        	rsr.ccount	a2
  a6:	c022f0        	sub	a2, a2, a15
  a9:	e432e7        	bltu	a2, a14, 91 <SoftwareSerial::write(unsigned char)+0x65>
  ac:	9c28      	l32i.n	a2, a12, 36
  ae:	08a042        	movi	a4, 8
  b1:	80ee20        	add	a14, a14, a2
  b4:	140c22        	l8ui	a2, a12, 20
  b7:	0430d0        	extui	a3, a13, 0, 1
  ba:	0149      	s32i.n	a4, a1, 0
  bc:	ffd601        	l32r	a0, 14 <SoftwareSerial::write(unsigned char)-0x18>
  bf:	0000c0        	callx0	a0
  c2:	000406        	j	d6 <SoftwareSerial::write(unsigned char)+0xaa>
  c5:	2c0c22        	l8ui	a2, a12, 44
  c8:	d2cc      	bnez.n	a2, d9 <SoftwareSerial::write(unsigned char)+0xad>
  ca:	01a022        	movi	a2, 1
  cd:	006142        	s32i	a4, a1, 0
  d0:	ffd201        	l32r	a0, 18 <SoftwareSerial::write(unsigned char)-0x14>
  d3:	0000c0        	callx0	a0
  d6:	002142        	l32i	a4, a1, 0
  d9:	002020        	esync
  dc:	03ea20        	rsr.ccount	a2
  df:	c022f0        	sub	a2, a2, a15
  e2:	df32e7        	bltu	a2, a14, c5 <SoftwareSerial::write(unsigned char)+0x99>
  e5:	9c28      	l32i.n	a2, a12, 36
  e7:	440b      	addi.n	a4, a4, -1
  e9:	ee2a      	add.n	a14, a14, a2
  eb:	41d1d0        	srli	a13, a13, 1
  ee:	fc2456        	bnez	a4, b4 <SoftwareSerial::write(unsigned char)+0x88>
  f1:	140c22        	l8ui	a2, a12, 20
  f4:	130c      	movi.n	a3, 1
  f6:	ffc901        	l32r	a0, 1c <SoftwareSerial::write(unsigned char)-0x10>
  f9:	0000c0        	callx0	a0
  fc:	000306        	j	10c <SoftwareSerial::write(unsigned char)+0xe0>
  ff:	2c0c22        	l8ui	a2, a12, 44
 102:	62cc      	bnez.n	a2, 10c <SoftwareSerial::write(unsigned char)+0xe0>
 104:	120c      	movi.n	a2, 1
 106:	ffc601        	l32r	a0, 20 <SoftwareSerial::write(unsigned char)-0xc>
 109:	0000c0        	callx0	a0
 10c:	002020        	esync
 10f:	03ea20        	rsr.ccount	a2
 112:	c022f0        	sub	a2, a2, a15
 115:	e632e7        	bltu	a2, a14, ff <SoftwareSerial::write(unsigned char)+0xd3>
 118:	1f0c22        	l8ui	a2, a12, 31
 11b:	928c      	beqz.n	a2, 128 <SoftwareSerial::write(unsigned char)+0xfc>
 11d:	180c22        	l8ui	a2, a12, 24
 120:	030c      	movi.n	a3, 0
 122:	ffc001        	l32r	a0, 24 <SoftwareSerial::write(unsigned char)-0x8>
 125:	0000c0        	callx0	a0
 128:	2c0c22        	l8ui	a2, a12, 44
 12b:	1e0c      	movi.n	a14, 1
 12d:	428c      	beqz.n	a2, 135 <SoftwareSerial::write(unsigned char)+0x109>
 12f:	ffbe01        	l32r	a0, 28 <SoftwareSerial::write(unsigned char)-0x4>
 132:	0000c0        	callx0	a0
 135:	b108      	l32i.n	a0, a1, 44
 137:	0e2d      	mov.n	a2, a14
 139:	a1c8      	l32i.n	a12, a1, 40
 13b:	91d8      	l32i.n	a13, a1, 36
 13d:	81e8      	l32i.n	a14, a1, 32
 13f:	71f8      	l32i.n	a15, a1, 28
 141:	30c112        	addi	a1, a1, 48
 144:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial14isValidGPIOpinEi:

00000000 <SoftwareSerial::isValidGPIOpin(int)>:
   0:	f4c322        	addi	a2, a3, -12
   3:	140c      	movi.n	a4, 1
   5:	0142b6        	bltui	a2, 4, a <SoftwareSerial::isValidGPIOpin(int)+0xa>
   8:	040c      	movi.n	a4, 0
   a:	120c      	movi.n	a2, 1
   c:	0163b6        	bltui	a3, 6, 11 <SoftwareSerial::isValidGPIOpin(int)+0x11>
   f:	020c      	movi.n	a2, 0
  11:	202420        	or	a2, a4, a2
  14:	042020        	extui	a2, a2, 0, 1
  17:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial8baudRateEv:

00000000 <SoftwareSerial::baudRate()-0x10>:
   0:	000000        	ill
   3:	424000        	xorb	b4, b0, b0
   6:	0f          	.byte 0xf
	...

00000010 <SoftwareSerial::baudRate()>:
  10:	f0c112        	addi	a1, a1, -16
  13:	21c9      	s32i.n	a12, a1, 8
  15:	20c220        	or	a12, a2, a2
  18:	fffa21        	l32r	a2, 0 <SoftwareSerial::baudRate()-0x10>
  1b:	036102        	s32i	a0, a1, 12
  1e:	fffa01        	l32r	a0, 8 <SoftwareSerial::baudRate()-0x8>
  21:	0000c0        	callx0	a0
  24:	fff831        	l32r	a3, 4 <SoftwareSerial::baudRate()-0xc>
  27:	822230        	mull	a2, a2, a3
  2a:	9c38      	l32i.n	a3, a12, 36
  2c:	fff801        	l32r	a0, c <SoftwareSerial::baudRate()-0x4>
  2f:	0000c0        	callx0	a0
  32:	3108      	l32i.n	a0, a1, 12
  34:	21c8      	l32i.n	a12, a1, 8
  36:	10c112        	addi	a1, a1, 16
  39:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial20setTransmitEnablePinEi:

00000000 <SoftwareSerial::setTransmitEnablePin(int)-0x8>:
	...

00000008 <SoftwareSerial::setTransmitEnablePin(int)>:
   8:	f0c112        	addi	a1, a1, -16
   b:	21c9      	s32i.n	a12, a1, 8
   d:	3109      	s32i.n	a0, a1, 12
   f:	02cd      	mov.n	a12, a2
  11:	f4c322        	addi	a2, a3, -12
  14:	0a42b6        	bltui	a2, 4, 22 <SoftwareSerial::setTransmitEnablePin(int)+0x1a>
  17:	0763b6        	bltui	a3, 6, 22 <SoftwareSerial::setTransmitEnablePin(int)+0x1a>
  1a:	020c      	movi.n	a2, 0
  1c:	1f4c22        	s8i	a2, a12, 31
  1f:	000746        	j	40 <SoftwareSerial::setTransmitEnablePin(int)+0x38>
  22:	120c      	movi.n	a2, 1
  24:	1f4c22        	s8i	a2, a12, 31
  27:	6c39      	s32i.n	a3, a12, 24
  29:	742030        	extui	a2, a3, 0, 8
  2c:	01a032        	movi	a3, 1
  2f:	fff401        	l32r	a0, 0 <SoftwareSerial::setTransmitEnablePin(int)-0x8>
  32:	0000c0        	callx0	a0
  35:	180c22        	l8ui	a2, a12, 24
  38:	030c      	movi.n	a3, 0
  3a:	fff201        	l32r	a0, 4 <SoftwareSerial::setTransmitEnablePin(int)-0x4>
  3d:	0000c0        	callx0	a0
  40:	3108      	l32i.n	a0, a1, 12
  42:	21c8      	l32i.n	a12, a1, 8
  44:	10c112        	addi	a1, a1, 16
  47:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial8enableRxEb:

00000000 <SoftwareSerial::enableRx(bool)-0xc>:
	...

0000000c <SoftwareSerial::enableRx(bool)>:
   c:	f0c112        	addi	a1, a1, -16
   f:	21c9      	s32i.n	a12, a1, 8
  11:	02cd      	mov.n	a12, a2
  13:	1c0222        	l8ui	a2, a2, 28
  16:	11d9      	s32i.n	a13, a1, 4
  18:	3109      	s32i.n	a0, a1, 12
  1a:	74d030        	extui	a13, a3, 0, 8
  1d:	f2ac      	beqz.n	a2, 50 <SoftwareSerial::enableRx(bool)+0x44>
  1f:	4c28      	l32i.n	a2, a12, 16
  21:	fd9c      	beqz.n	a13, 44 <SoftwareSerial::enableRx(bool)+0x38>
  23:	fff731        	l32r	a3, 0 <SoftwareSerial::enableRx(bool)-0xc>
  26:	200c62        	l8ui	a6, a12, 32
  29:	a03230        	addx4	a3, a2, a3
  2c:	150c      	movi.n	a5, 1
  2e:	002332        	l32i	a3, a3, 0
  31:	02a042        	movi	a4, 2
  34:	742020        	extui	a2, a2, 0, 8
  37:	934560        	movnez	a4, a5, a6
  3a:	fff201        	l32r	a0, 4 <SoftwareSerial::enableRx(bool)-0x8>
  3d:	0000c0        	callx0	a0
  40:	000246        	j	4d <SoftwareSerial::enableRx(bool)+0x41>
  43:	202000        	or	a2, a0, a0
  46:	f00174        	lsi	f7, a1, 0x3c0
  49:	ff          	.byte 0xff
  4a:	0000c0        	callx0	a0
  4d:	1d4cd2        	s8i	a13, a12, 29
  50:	3108      	l32i.n	a0, a1, 12
  52:	21c8      	l32i.n	a12, a1, 8
  54:	11d8      	l32i.n	a13, a1, 4
  56:	10c112        	addi	a1, a1, 16
  59:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial5beginEl:

00000000 <SoftwareSerial::begin(long)-0x18>:
   0:	000000        	ill
   3:	424000        	xorb	b4, b0, b0
   6:	0f          	.byte 0xf
   7:	258000        	extui	a8, a0, 16, 3
	...

00000018 <SoftwareSerial::begin(long)>:
  18:	f0c112        	addi	a1, a1, -16
  1b:	21c9      	s32i.n	a12, a1, 8
  1d:	02cd      	mov.n	a12, a2
  1f:	fff821        	l32r	a2, 0 <SoftwareSerial::begin(long)-0x18>
  22:	036102        	s32i	a0, a1, 12
  25:	0161d2        	s32i	a13, a1, 4
  28:	03dd      	mov.n	a13, a3
  2a:	fff801        	l32r	a0, c <SoftwareSerial::begin(long)-0xc>
  2d:	0000c0        	callx0	a0
  30:	fff531        	l32r	a3, 4 <SoftwareSerial::begin(long)-0x14>
  33:	822230        	mull	a2, a2, a3
  36:	0d3d      	mov.n	a3, a13
  38:	fff601        	l32r	a0, 10 <SoftwareSerial::begin(long)-0x8>
  3b:	0000c0        	callx0	a0
  3e:	fff241        	l32r	a4, 8 <SoftwareSerial::begin(long)-0x10>
  41:	9c29      	s32i.n	a2, a12, 36
  43:	130c      	movi.n	a3, 1
  45:	020c      	movi.n	a2, 0
  47:	0124d7        	blt	a4, a13, 4c <SoftwareSerial::begin(long)+0x34>
  4a:	023d      	mov.n	a3, a2
  4c:	214c22        	s8i	a2, a12, 33
  4f:	1d0c22        	l8ui	a2, a12, 29
  52:	2c4c32        	s8i	a3, a12, 44
  55:	82cc      	bnez.n	a2, 61 <SoftwareSerial::begin(long)+0x49>
  57:	0c2d      	mov.n	a2, a12
  59:	130c      	movi.n	a3, 1
  5b:	ffee01        	l32r	a0, 14 <SoftwareSerial::begin(long)-0x4>
  5e:	0000c0        	callx0	a0
  61:	3108      	l32i.n	a0, a1, 12
  63:	21c8      	l32i.n	a12, a1, 8
  65:	11d8      	l32i.n	a13, a1, 4
  67:	10c112        	addi	a1, a1, 16
  6a:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerialD2Ev:

00000000 <SoftwareSerial::~SoftwareSerial()-0x10>:
	...

00000010 <SoftwareSerial::~SoftwareSerial()>:
  10:	f0c112        	addi	a1, a1, -16
  13:	21c9      	s32i.n	a12, a1, 8
  15:	02cd      	mov.n	a12, a2
  17:	fffa21        	l32r	a2, 0 <SoftwareSerial::~SoftwareSerial()-0x10>
  1a:	030c      	movi.n	a3, 0
  1c:	0c29      	s32i.n	a2, a12, 0
  1e:	0c2d      	mov.n	a2, a12
  20:	3109      	s32i.n	a0, a1, 12
  22:	fff901        	l32r	a0, 8 <SoftwareSerial::~SoftwareSerial()-0x8>
  25:	0000c0        	callx0	a0
  28:	1c0c22        	l8ui	a2, a12, 28
  2b:	b28c      	beqz.n	a2, 3a <SoftwareSerial::~SoftwareSerial()+0x2a>
  2d:	4c38      	l32i.n	a3, a12, 16
  2f:	fff521        	l32r	a2, 4 <SoftwareSerial::~SoftwareSerial()-0xc>
  32:	a02320        	addx4	a2, a3, a2
  35:	030c      	movi.n	a3, 0
  37:	006232        	s32i	a3, a2, 0
  3a:	fc28      	l32i.n	a2, a12, 60
  3c:	428c      	beqz.n	a2, 44 <SoftwareSerial::~SoftwareSerial()+0x34>
  3e:	fff301        	l32r	a0, c <SoftwareSerial::~SoftwareSerial()-0x4>
  41:	0000c0        	callx0	a0
  44:	3108      	l32i.n	a0, a1, 12
  46:	21c8      	l32i.n	a12, a1, 8
  48:	10c112        	addi	a1, a1, 16
  4b:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerialC2Eiibj:

00000000 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x24>:
	...
   8:	002580        	lsi	f8, a5, 0
	...

00000024 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)>:
  24:	f0c112        	addi	a1, a1, -16
  27:	21c9      	s32i.n	a12, a1, 8
  29:	01e9      	s32i.n	a14, a1, 0
  2b:	02cd      	mov.n	a12, a2
  2d:	03ed      	mov.n	a14, a3
  2f:	e8a332        	movi	a3, 0x3e8
  32:	2c39      	s32i.n	a3, a12, 8
  34:	fff331        	l32r	a3, 0 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x24>
  37:	070c      	movi.n	a7, 0
  39:	0c39      	s32i.n	a3, a12, 0
  3b:	a9a132        	movi	a3, 0x1a9
  3e:	11d9      	s32i.n	a13, a1, 4
  40:	ac39      	s32i.n	a3, a12, 40
  42:	3109      	s32i.n	a0, a1, 12
  44:	1c79      	s32i.n	a7, a12, 4
  46:	1f4c72        	s8i	a7, a12, 31
  49:	1e4c72        	s8i	a7, a12, 30
  4c:	1c4c72        	s8i	a7, a12, 28
  4f:	fc79      	s32i.n	a7, a12, 60
  51:	204c52        	s8i	a5, a12, 32
  54:	224c72        	s8i	a7, a12, 34
  57:	1d4c72        	s8i	a7, a12, 29
  5a:	214c72        	s8i	a7, a12, 33
  5d:	f4ce32        	addi	a3, a14, -12
  60:	04dd      	mov.n	a13, a4
  62:	062d      	mov.n	a2, a6
  64:	0243b6        	bltui	a3, 4, 6a <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)+0x46>
  67:	356ef6        	bgeui	a14, 6, a0 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)+0x7c>
  6a:	4ce9      	s32i.n	a14, a12, 16
  6c:	ec29      	s32i.n	a2, a12, 56
  6e:	ffe701        	l32r	a0, c <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x18>
  71:	0000c0        	callx0	a0
  74:	fc29      	s32i.n	a2, a12, 60
  76:	62ac      	beqz.n	a2, a0 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)+0x7c>
  78:	030c      	movi.n	a3, 0
  7a:	120c      	movi.n	a2, 1
  7c:	1c4c22        	s8i	a2, a12, 28
  7f:	dc39      	s32i.n	a3, a12, 52
  81:	cc39      	s32i.n	a3, a12, 48
  83:	7420e0        	extui	a2, a14, 0, 8
  86:	ffe201        	l32r	a0, 10 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x14>
  89:	0000c0        	callx0	a0
  8c:	4c38      	l32i.n	a3, a12, 16
  8e:	ffdd21        	l32r	a2, 4 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x20>
  91:	a02320        	addx4	a2, a3, a2
  94:	02c9      	s32i.n	a12, a2, 0
  96:	130c      	movi.n	a3, 1
  98:	0c2d      	mov.n	a2, a12
  9a:	ffde01        	l32r	a0, 14 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x10>
  9d:	0000c0        	callx0	a0
  a0:	f4cd22        	addi	a2, a13, -12
  a3:	1352b6        	bltui	a2, 5, ba <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)+0x96>
  a6:	106db6        	bltui	a13, 6, ba <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)+0x96>
  a9:	ffd731        	l32r	a3, 8 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x1c>
  ac:	0c2d      	mov.n	a2, a12
  ae:	ffda01        	l32r	a0, 18 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0xc>
  b1:	0000c0        	callx0	a0
  b4:	000a46        	j	e1 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)+0xbd>
  b7:	000000        	ill
  ba:	1e0c      	movi.n	a14, 1
  bc:	7420d0        	extui	a2, a13, 0, 8
  bf:	1e4ce2        	s8i	a14, a12, 30
  c2:	5cd9      	s32i.n	a13, a12, 20
  c4:	130c      	movi.n	a3, 1
  c6:	ffd501        	l32r	a0, 1c <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x8>
  c9:	0000c0        	callx0	a0
  cc:	200c22        	l8ui	a2, a12, 32
  cf:	3032e0        	xor	a3, a2, a14
  d2:	140c22        	l8ui	a2, a12, 20
  d5:	743030        	extui	a3, a3, 0, 8
  d8:	ffd201        	l32r	a0, 20 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)-0x4>
  db:	0000c0        	callx0	a0
  de:	fff1c6        	j	a9 <SoftwareSerial::SoftwareSerial(int, int, bool, unsigned int)+0x85>
  e1:	3108      	l32i.n	a0, a1, 12
  e3:	21c8      	l32i.n	a12, a1, 8
  e5:	11d8      	l32i.n	a13, a1, 4
  e7:	01e8      	l32i.n	a14, a1, 0
  e9:	10c112        	addi	a1, a1, 16
  ec:	f00d      	ret.n

Disassembly of section .text._ZN14SoftwareSerial8overflowEv:

00000000 <SoftwareSerial::overflow()>:
   0:	220242        	l8ui	a4, a2, 34
   3:	030c      	movi.n	a3, 0
   5:	214232        	s8i	a3, a2, 33
   8:	224232        	s8i	a3, a2, 34
   b:	042d      	mov.n	a2, a4
   d:	f00d      	ret.n

Disassembly of section .iram.text:

00000000 <SoftwareSerial::rxRead()-0x18>:
   0:	000324        	lsi	f2, a3, 0
   3:	000060        	lsi	f6, a0, 0
	...

00000018 <SoftwareSerial::rxRead()>:
  18:	d0c112        	addi	a1, a1, -48
  1b:	a1c9      	s32i.n	a12, a1, 40
  1d:	02cd      	mov.n	a12, a2
  1f:	4228      	l32i.n	a2, a2, 16
  21:	130c      	movi.n	a3, 1
  23:	401200        	ssl	a2
  26:	a13300        	sll	a3, a3
  29:	fff521        	l32r	a2, 0 <SoftwareSerial::rxRead()-0x18>
  2c:	91d9      	s32i.n	a13, a1, 36
  2e:	b109      	s32i.n	a0, a1, 44
  30:	81e9      	s32i.n	a14, a1, 32
  32:	71f9      	s32i.n	a15, a1, 28
  34:	0020c0        	memw
  37:	0239      	s32i.n	a3, a2, 0
  39:	9c28      	l32i.n	a2, a12, 36
  3b:	330c      	movi.n	a3, 3
  3d:	0faed2        	movi	a13, 0xfffffe0f
  40:	d2da      	add.n	a13, a2, a13
  42:	fff001        	l32r	a0, 4 <SoftwareSerial::rxRead()-0x14>
  45:	0000c0        	callx0	a0
  48:	dd2a      	add.n	a13, a13, a2
  4a:	002020        	esync
  4d:	03ea30        	rsr.ccount	a3
  50:	8e0c      	movi.n	a14, 8
  52:	0139      	s32i.n	a3, a1, 0
  54:	0f0c      	movi.n	a15, 0
  56:	000386        	j	68 <SoftwareSerial::rxRead()+0x50>
  59:	2c0c22        	l8ui	a2, a12, 44
  5c:	008256        	bnez	a2, 68 <SoftwareSerial::rxRead()+0x50>
  5f:	01a022        	movi	a2, 1
  62:	ffe901        	l32r	a0, 8 <SoftwareSerial::rxRead()-0x10>
  65:	0000c0        	callx0	a0
  68:	002020        	esync
  6b:	03ea20        	rsr.ccount	a2
  6e:	0138      	l32i.n	a3, a1, 0
  70:	c02230        	sub	a2, a2, a3
  73:	e232d7        	bltu	a2, a13, 59 <SoftwareSerial::rxRead()+0x41>
  76:	9c28      	l32i.n	a2, a12, 36
  78:	41f1f0        	srli	a15, a15, 1
  7b:	dd2a      	add.n	a13, a13, a2
  7d:	100c22        	l8ui	a2, a12, 16
  80:	ffe301        	l32r	a0, c <SoftwareSerial::rxRead()-0xc>
  83:	0000c0        	callx0	a0
  86:	728c      	beqz.n	a2, 91 <SoftwareSerial::rxRead()+0x79>
  88:	80af22        	movi	a2, -128
  8b:	204f20        	or	a4, a15, a2
  8e:	74f040        	extui	a15, a4, 0, 8
  91:	ee0b      	addi.n	a14, a14, -1
  93:	fd1e56        	bnez	a14, 68 <SoftwareSerial::rxRead()+0x50>
  96:	200c22        	l8ui	a2, a12, 32
  99:	429c      	beqz.n	a2, b1 <SoftwareSerial::rxRead()+0x99>
  9b:	ffa022        	movi	a2, 255
  9e:	30ff20        	xor	a15, a15, a2
  a1:	000306        	j	b1 <SoftwareSerial::rxRead()+0x99>
  a4:	2c0c22        	l8ui	a2, a12, 44
  a7:	62cc      	bnez.n	a2, b1 <SoftwareSerial::rxRead()+0x99>
  a9:	120c      	movi.n	a2, 1
  ab:	ffd901        	l32r	a0, 10 <SoftwareSerial::rxRead()-0x8>
  ae:	0000c0        	callx0	a0
  b1:	002020        	esync
  b4:	03ea20        	rsr.ccount	a2
  b7:	0138      	l32i.n	a3, a1, 0
  b9:	c02230        	sub	a2, a2, a3
  bc:	e432d7        	bltu	a2, a13, a4 <SoftwareSerial::rxRead()+0x8c>
  bf:	ccd8      	l32i.n	a13, a12, 48
  c1:	ec38      	l32i.n	a3, a12, 56
  c3:	01cd22        	addi	a2, a13, 1
  c6:	ffd301        	l32r	a0, 14 <SoftwareSerial::rxRead()-0x4>
  c9:	0000c0        	callx0	a0
  cc:	dc38      	l32i.n	a3, a12, 52
  ce:	201237        	beq	a2, a3, f2 <SoftwareSerial::rxRead()+0xda>
  d1:	f32c      	movi.n	a3, 47
  d3:	059f37        	bne	a15, a3, dc <SoftwareSerial::rxRead()+0xc4>
  d6:	130c      	movi.n	a3, 1
  d8:	0001c6        	j	e3 <SoftwareSerial::rxRead()+0xcb>
  db:	132c00        	lsi	f0, a12, 76
  de:	049f37        	bne	a15, a3, e6 <SoftwareSerial::rxRead()+0xce>
  e1:	030c      	movi.n	a3, 0
  e3:	214c32        	s8i	a3, a12, 33
  e6:	fc38      	l32i.n	a3, a12, 60
  e8:	d3da      	add.n	a13, a3, a13
  ea:	004df2        	s8i	a15, a13, 0
  ed:	cc29      	s32i.n	a2, a12, 48
  ef:	000246        	j	fc <SoftwareSerial::rxRead()+0xe4>
  f2:	020c      	movi.n	a2, 0
  f4:	214c22        	s8i	a2, a12, 33
  f7:	120c      	movi.n	a2, 1
  f9:	224c22        	s8i	a2, a12, 34
  fc:	b108      	l32i.n	a0, a1, 44
  fe:	a1c8      	l32i.n	a12, a1, 40
 100:	91d8      	l32i.n	a13, a1, 36
 102:	81e8      	l32i.n	a14, a1, 32
 104:	71f8      	l32i.n	a15, a1, 28
 106:	30c112        	addi	a1, a1, 48
 109:	f00d      	ret.n
 10b:	000000        	ill
	...

00000110 <sws_isr_15()>:
 110:	ffff21        	l32r	a2, 10c <SoftwareSerial::rxRead()+0xf4>
 113:	f0c112        	addi	a1, a1, -16
 116:	f228      	l32i.n	a2, a2, 60
 118:	3109      	s32i.n	a0, a1, 12
 11a:	000005        	call0	11c <sws_isr_15()+0xc>
 11d:	3108      	l32i.n	a0, a1, 12
 11f:	10c112        	addi	a1, a1, 16
 122:	f00d      	ret.n
 124:	000000        	ill
	...

00000128 <sws_isr_14()>:
 128:	ffff21        	l32r	a2, 124 <sws_isr_15()+0x14>
 12b:	f0c112        	addi	a1, a1, -16
 12e:	e228      	l32i.n	a2, a2, 56
 130:	3109      	s32i.n	a0, a1, 12
 132:	000005        	call0	134 <sws_isr_14()+0xc>
 135:	3108      	l32i.n	a0, a1, 12
 137:	10c112        	addi	a1, a1, 16
 13a:	f00d      	ret.n
 13c:	000000        	ill
	...

00000140 <sws_isr_13()>:
 140:	ffff21        	l32r	a2, 13c <sws_isr_14()+0x14>
 143:	f0c112        	addi	a1, a1, -16
 146:	d228      	l32i.n	a2, a2, 52
 148:	3109      	s32i.n	a0, a1, 12
 14a:	000005        	call0	14c <sws_isr_13()+0xc>
 14d:	3108      	l32i.n	a0, a1, 12
 14f:	10c112        	addi	a1, a1, 16
 152:	f00d      	ret.n
 154:	000000        	ill
	...

00000158 <sws_isr_12()>:
 158:	ffff21        	l32r	a2, 154 <sws_isr_13()+0x14>
 15b:	f0c112        	addi	a1, a1, -16
 15e:	c228      	l32i.n	a2, a2, 48
 160:	3109      	s32i.n	a0, a1, 12
 162:	000005        	call0	164 <sws_isr_12()+0xc>
 165:	3108      	l32i.n	a0, a1, 12
 167:	10c112        	addi	a1, a1, 16
 16a:	f00d      	ret.n
 16c:	000000        	ill
	...

00000170 <sws_isr_5()>:
 170:	ffff21        	l32r	a2, 16c <sws_isr_12()+0x14>
 173:	f0c112        	addi	a1, a1, -16
 176:	5228      	l32i.n	a2, a2, 20
 178:	3109      	s32i.n	a0, a1, 12
 17a:	000005        	call0	17c <sws_isr_5()+0xc>
 17d:	3108      	l32i.n	a0, a1, 12
 17f:	10c112        	addi	a1, a1, 16
 182:	f00d      	ret.n
 184:	000000        	ill
	...

00000188 <sws_isr_4()>:
 188:	ffff21        	l32r	a2, 184 <sws_isr_5()+0x14>
 18b:	f0c112        	addi	a1, a1, -16
 18e:	4228      	l32i.n	a2, a2, 16
 190:	3109      	s32i.n	a0, a1, 12
 192:	000005        	call0	194 <sws_isr_4()+0xc>
 195:	3108      	l32i.n	a0, a1, 12
 197:	10c112        	addi	a1, a1, 16
 19a:	f00d      	ret.n
 19c:	000000        	ill
	...

000001a0 <sws_isr_3()>:
 1a0:	ffff21        	l32r	a2, 19c <sws_isr_4()+0x14>
 1a3:	f0c112        	addi	a1, a1, -16
 1a6:	3228      	l32i.n	a2, a2, 12
 1a8:	3109      	s32i.n	a0, a1, 12
 1aa:	000005        	call0	1ac <sws_isr_3()+0xc>
 1ad:	3108      	l32i.n	a0, a1, 12
 1af:	10c112        	addi	a1, a1, 16
 1b2:	f00d      	ret.n
 1b4:	000000        	ill
	...

000001b8 <sws_isr_2()>:
 1b8:	ffff21        	l32r	a2, 1b4 <sws_isr_3()+0x14>
 1bb:	f0c112        	addi	a1, a1, -16
 1be:	2228      	l32i.n	a2, a2, 8
 1c0:	3109      	s32i.n	a0, a1, 12
 1c2:	000005        	call0	1c4 <sws_isr_2()+0xc>
 1c5:	3108      	l32i.n	a0, a1, 12
 1c7:	10c112        	addi	a1, a1, 16
 1ca:	f00d      	ret.n
 1cc:	000000        	ill
	...

000001d0 <sws_isr_1()>:
 1d0:	ffff21        	l32r	a2, 1cc <sws_isr_2()+0x14>
 1d3:	f0c112        	addi	a1, a1, -16
 1d6:	1228      	l32i.n	a2, a2, 4
 1d8:	3109      	s32i.n	a0, a1, 12
 1da:	000005        	call0	1dc <sws_isr_1()+0xc>
 1dd:	3108      	l32i.n	a0, a1, 12
 1df:	10c112        	addi	a1, a1, 16
 1e2:	f00d      	ret.n
 1e4:	000000        	ill
	...

000001e8 <sws_isr_0()>:
 1e8:	ffff21        	l32r	a2, 1e4 <sws_isr_1()+0x14>
 1eb:	f0c112        	addi	a1, a1, -16
 1ee:	0228      	l32i.n	a2, a2, 0
 1f0:	3109      	s32i.n	a0, a1, 12
 1f2:	000005        	call0	1f4 <sws_isr_0()+0xc>
 1f5:	3108      	l32i.n	a0, a1, 12
 1f7:	10c112        	addi	a1, a1, 16
 1fa:	f00d      	ret.n
