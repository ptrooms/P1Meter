#ifndef CRC16_H
#define CRC16_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

/*
 Calculate/progress CRC using Character array *buf of length=len
   shift calculate each byte to right
*/
unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
{
  //DebugCRC Serial.println((String) "\nCRC16 call length="+len+"\t processed=" );  // limit prints
  // Serial.print((String) "\n len="+len+"\t data=" );  // limit prints
	for (int pos = 0; pos < len; pos++)
	{
		crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc
     	// Serial.printf("x%02x",buf[pos]);
		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
	return crc;
}
#endif
