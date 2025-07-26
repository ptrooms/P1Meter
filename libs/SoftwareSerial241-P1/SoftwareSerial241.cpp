/*

SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
adapted to P1 messageing with activates P1active between '/' and '!' readed data
P1 is the smart electricity meter that output data on its serial port at speed 
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.
Extended for P1  (c) 2021 Peter Ooms.  All rights reserved.

Changes to core version:
	int getCycleCountIram() function (in header) added as WAITIram replacement for the ESP classed version
	bool m_P1active()  true if readRx is in the midst of reading a P1/DSMR telegram
	

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <Arduino.h>

// The Arduino standard GPIO routines are not enough,
// must use some from the Espressif SDK as well
extern "C" {
#include "gpio.h"
}
#include <eagle_soc.h>     // required as we use GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS to preven re-interrupt

// #include <SoftwareSerial.h>
// #include </home/pafoxp/code-P1meter/SoftwareSerial_2.4.1/SoftwareSerial.h>
#include <SoftwareSerial241.h>

#define MAX_PIN 15

#define BYTE_MAXWAIT_1 7100    // maximum cycles per bit which floats between 6930-7012

#define GET_CYCLE_COUNT  ESP.getCycleCount()       // get the esp cycle counter


// As the Arduino attachInterrupt has no parameter, lists of objects
// and callbacks corresponding to each possible GPIO pins have to be defined
SoftwareSerial *ObjList[MAX_PIN+3];

void ICACHE_RAM_ATTR sws_isr_0() { ObjList[0]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_1() { ObjList[1]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_2() { ObjList[2]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_3() { ObjList[3]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_4() { ObjList[4]->rxRead2(); };	// gpio4/D2 is used to bitbang secondary RX/WL
void ICACHE_RAM_ATTR sws_isr_5() { ObjList[5]->rxRead(); };
// Pin 6 to 11 can not be used
void ICACHE_RAM_ATTR sws_isr_12() { ObjList[12]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_13() { ObjList[13]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_14() { ObjList[14]->rxRead(); };   // gpio14/D5 is used to bitbang primary P1
void ICACHE_RAM_ATTR sws_isr_15() { ObjList[15]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_16() { ObjList[16]->rxTriggerBit(); };  // use gpio4  for bittiming
void ICACHE_RAM_ATTR sws_isr_17() { ObjList[17]->rxTriggerBit(); };  // use gpio14 for bittiming

// void ICACHE_RAM_ATTR rxTriggerBit_ISR(void); // store the ISR test routine in cache

static void (*ISRList[MAX_PIN+3])() = {
      sws_isr_0,
      sws_isr_1,
      sws_isr_2,
      sws_isr_3,
      sws_isr_4,
      sws_isr_5,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      sws_isr_12,
      sws_isr_13,
      sws_isr_14,
      sws_isr_15,
      sws_isr_16,
      sws_isr_17
};

/* 
   Serial 3 & 4
*/
SoftwareSerial::SoftwareSerial(int receivePin, int transmitPin, int inverse_logic, unsigned int buffSize) {    // v58: method of CHANGE ISR
   m_rxValid = m_txValid = m_txEnableValid = false;
   // Serial.println((String) "\n\r >>>>>> Allocated overloaded inverse_logic=" +  inverse_logic);  // print does not work here
   // SoftwareSerial(receivePin,transmitPin, true, buffSize);  // will cause crash
   m_rxValid = m_txValid = m_txEnableValid = false;
   m_buffer = NULL;
   m_invert = inverse_logic;
   m_overflow = false;
   m_rxEnabled = false;
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation between '/' and '!'
   // m_bitWait = 498;                       // 2021-04-30 14:07:35 initialise to control bittiming (not used)
   // m_bitWait = 509;     // before we added cycletime table
   m_bitWait = 511;     

   if (receivePin == 4 || receivePin == 14 ) {
      m_rxPin = receivePin;
      m_buffSize = buffSize;
      m_buffer = (uint8_t*)malloc(m_buffSize);     // https://cplusplus.com/reference/cstdlib/malloc/
                                       // https://www.guru99.com/difference-between-malloc-and-calloc.html
                                       // malloc allocates a single block of uninitialized memory
                                       // calloc allocates multiple blocks of memory and initializes them to zero.
                                       // 10*4*1024 = 61.6% (used 50448 bytes from 81920 bytes)
                                       // *1 = heap =20472   *2 = heap 15872 *3 = heap 11024      
      
      m_buffer_timePos  = M_TIME_START;
      m_buffer_time = (unsigned long*)calloc(M_TIME_ENTRIES, sizeof(unsigned long));               // v58d_ss241 ytimer positions
      m_buffer_time[M_TIME_START]  = GET_CYCLE_COUNT;   // initialise
      m_buffer_bits = (unsigned long*)calloc((m_buffSize)*1, sizeof(unsigned long));    // v58d_ss241 2-10 bit-transitions 
      // m_buffer_bitValue = 0;
      m_buffer_bitValue = M_BIT_CYCLE_VALUE  // initialise this 

      // --> if (m_buffer != NULL && m_buffer_time[0] == 0UL) {  causes boot loop

      if (m_buffer != NULL) {          // check we have memory // v58d_ss241
         m_rxValid = true;
         m_inPos = m_outPos = 0;
         pinMode(m_rxPin, INPUT);

         if (receivePin ==  4) ObjList[16] = this;
         if (receivePin == 14) ObjList[17] = this;
         enableRx(true);
      } else {
         Serial.println((String) "Serial Buffer allocation error.");
      }
   }
   if (isValidGPIOpin(transmitPin) || transmitPin == 16) {
      m_txValid = true;
      m_txPin = transmitPin;
      pinMode(m_txPin, OUTPUT);
      digitalWrite(m_txPin, !m_invert);
   }
   // Default speed
   SoftwareSerial::begin(9600);
}


/*
   Serial 1 & 2
*/
SoftwareSerial::SoftwareSerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) {   // v58: method of FALL/RISE ISR
   // Serial.println((String) "\n\r >>>>>> Allocated non overloaded inverse_logic=" +  inverse_logic); // print does not work here
   m_rxValid = m_txValid = m_txEnableValid = false;
   m_buffer = NULL;
   m_invert = inverse_logic;
   m_overflow = false;
   m_rxEnabled = false;
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation between '/' and '!'
   // m_bitWait = 498;                       // 2021-04-30 14:07:35 initialise to control bittiming (not used)
   m_bitWait = 509;                       // v58: production & copy , 498 is bottom
   if (isValidGPIOpin(receivePin)) {
      m_rxPin = receivePin;
      m_buffSize = buffSize;
      m_buffer = (uint8_t*)malloc(m_buffSize);     // https://cplusplus.com/reference/cstdlib/malloc/
                                       // https://www.guru99.com/difference-between-malloc-and-calloc.html
                                       // malloc allocates a single block of uninitialized memory
                                       // calloc allocates multiple blocks of memory and initializes them to zero.
                                       // 10*4*1024 = 61.6% (used 50448 bytes from 81920 bytes)
                                       // *1 = heap =20472   *2 = heap 15872 *3 = heap 11024      
      
      m_buffer_timePos  = M_TIME_START;
      m_buffer_time = (unsigned long*)calloc(M_TIME_ENTRIES, sizeof(unsigned long));               // v58d_ss241 ytimer positions
      m_buffer_time[M_TIME_START]  = GET_CYCLE_COUNT;   // initialise
      m_buffer_bits = (unsigned long*)calloc((m_buffSize)*1, sizeof(unsigned long));    // v58d_ss241 2-10 bit-transitions 
      // m_buffer_bitValue = 0;
      m_buffer_bitValue = M_BIT_CYCLE_VALUE  // initialise this 

      // --> if (m_buffer != NULL && m_buffer_time[0] == 0UL) {  causes boot loop

      if (m_buffer != NULL) {          // check we have memory // v58d_ss241
         m_rxValid = true;
         m_inPos = m_outPos = 0;
         pinMode(m_rxPin, INPUT);
         ObjList[m_rxPin] = this;
         enableRx(true);
      } else {
         Serial.println((String) "Serial Buffer allocation error.");
      }
   }
   if (isValidGPIOpin(transmitPin) || transmitPin == 16) {
      m_txValid = true;
      m_txPin = transmitPin;
      pinMode(m_txPin, OUTPUT);
      digitalWrite(m_txPin, !m_invert);
   }
   // Default speed
   SoftwareSerial::begin(9600);
}

SoftwareSerial::~SoftwareSerial() {    // P1meter never called as we keep the buffers alive in main/loop()
   enableRx(false);
   if (m_rxValid)
      ObjList[m_rxPin] = NULL;
   if (m_buffer)
      free(m_buffer);
   if (m_buffer_time)          // v58d_ss241
      free(m_buffer_time);
   if (m_buffer_bits)          // v59b
      free(m_buffer_bits);

}

bool SoftwareSerial::isValidGPIOpin(int pin) {
   return (pin >= 0 && pin <= 5) || (pin >= 12 && pin <= MAX_PIN);
}

void SoftwareSerial::begin(long speed) {
   // Use getCycleCount() loop to get as exact timing as possible
   Serial.print((String) ":B" + (portActive() ? "!" : "@") + m_rxPin);      /// v58b/v59 diagnose
   this->begin(speed,SERIAL_RECORDTYPE_PORT);   // do normal serial
   
   /*

   m_bitTime = ESP.getCpuFreqMHz()*1000000/speed;	// for 115k2=80000000/115200 = 694
   // m_wait = m_bitTime + m_bitTime/3 - 500;      // calculate timer waits midship done in ISR
                                                   // 497-501-505 // 425 115k2@80MHz 
   m_highSpeed = speed > 9600;
   
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation between '/' and '!'
   if (!m_rxEnabled)
     enableRx(true);
   */
}

/*
   Simulate input: 
   0 = none, use physical
   1 = produce P1/power telegram
   2 = produce WL/heat telegram
*/

void SoftwareSerial::begin(long speed, int recordtype) {
   const char * str1 = "/KFM5KAIFA-METER\r\n\r\n1-3:0.2.8(42)\r\n0-0:1.0.0(210420113523S)\r\n0-0:96.1.1(1234567890123456789012345678901234)\r\n1-0:1.8.1(012345.111*kWh)\r\n1-0:1.8.2(012345.222*kWh)\r\n1-0:2.8.1(000000.000*kWh)\r\n1-0:2.8.2(000000.000*kWh)\r\n0-0:96.14.0(0002)\r\n1-0:1.7.0(00.560*kW)\r\n1-0:2.7.0(00.000*kW)\r\n0-0:96.7.21(00003)\r\n0-0:96.7.9(00003)\r\n1-0:99.97.0(5)(0-0:96.7.19)(210407073103W)(0000001404*s)(181103114840W)(0000008223*s)(180911211118S)(0000003690*s)(160606105039S)(0000003280*s)(000101000001W)(2147483647*s)\r\n1-0:32.32.0(00000)\r\n1-0:32.36.0(00000)\r\n0-0:96.13.1()\r\n0-0:96.13.0()\r\n1-0:31.7.0(002*A)\r\n1-0:21.7.0(00.560*kW)\r\n1-0:22.7.0(00.000*kW)\r\n!078E validated CR , normal=078E\r\n\xFF";                 
   const char * str2 = "_/VALID-VI\\ 1-3:0.2.8(50) 0-0:1.1.0(250714103614W) 0-0:96.1.1(1000000000000000000000000000000000000000000000000000000000000000) 0-1:24.1.0(012) 0-1:96.1.0(20000000000000000000000000000000) 0-1:24.2.1(250714103600W)(12.000*GJ)!A5AE_E621_B\xff";
   // Serial.print((String) "\r\nBegin port" + m_rxPin  + " cycle" + GET_CYCLE_COUNT + "\r\n");
   m_buffer_time[M_TIME_BEGIN_START] = GET_CYCLE_COUNT;   // initialise
   m_bitTime = ESP.getCpuFreqMHz()*1000000/speed;	// for 115k2=80000000/115200 = 694
   m_highSpeed = speed > 9600;
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation between '/' and '!'
   Serial.print((String) (portActive() ? "!" : "@") ); // v58b/v59 diagnose
   if (recordtype == SERIAL_RECORDTYPE_PORT ) {
         Serial.print((String) "\b" +  (m_rxPin == 14 ? "p" : "w" ) );         // v59 print indication which port to activate
         if (!m_rxEnabled) enableRx(true);
         else Serial.print((String) "\b" +  (m_rxPin == 14 ? "P" : "W" ));    // v59 print indication thingy was already active
         /*
         // Serial.print((String) "\tset@"+ m_rxPin + "=>"); // v58b diagnose
         // note: plain use will calll somethign else and produeces
         this->begin(speed);   // do normal serial
         */
      } else {
         Serial.print((String) "\b\tuse@");  
         // char *  str = warning: deprecated conversion from string constant to 'char*'
         // corrected to const char * str
         // Serial.print((String) "\r\n using serial " + __FILE__ +  "\r\n" + str);         
         this->enableRx(false, recordtype);     // will set m_rxEnabled = false;
         m_outPos = 0;        // position to start of buffer
         // msg3.concat("
         // int str_len = str.length() + 1; 
         // // Prepare the character array (the buffer) 
         // char char_array[str_len];
         // Copy it over 
         // m_inPos = str.length() % m_buffSize;
         // str.toCharArray(m_buffer, m_inPos );
         
         // no match for call to '(String) (unsigned int&)'  
         // Serial.print((String) "\r\n using serial " + __FILE__  + ">") ;
         if (recordtype == SERIAL_RECORDTYPE_WL) {
               for (m_inPos = 0;  str2[m_inPos] != 0xff && (m_inPos % m_buffSize) < m_buffSize  ;m_inPos++ ) {
               m_buffer[m_inPos] = str2[m_inPos];  // move data
               // Serial.print((String) m_buffer[m_inPos]);
               }
         } else {
            for (m_inPos = 0;  str1[m_inPos] != 0xff && (m_inPos % m_buffSize) < m_buffSize  ;m_inPos++ ) {
               m_buffer[m_inPos] = str1[m_inPos];  // move data
               // Serial.print((String) m_buffer[m_inPos]);
            }
         }

         m_inPos++;  // position after  last one
         // Serial.print((String) "<\r\n Serialsize=" + m_inPos + " + \r\n");
         Serial.print((String)  m_rxPin + (portActive() ? "I" : "i" ) + ":"  + recordtype + " "); /// v59 diagnose
         if (false) Serial.println(" Dummy print line" + __LINE__ );   // v59 just checking if this influences stability

   }
   m_buffer_time[M_TIME_BEGIN_END] = GET_CYCLE_COUNT;   // initialise
}

long SoftwareSerial::baudRate() {
   return ESP.getCpuFreqMHz()*1000000/m_bitTime;
}

void SoftwareSerial::setTransmitEnablePin(int transmitEnablePin) {
  if (isValidGPIOpin(transmitEnablePin)) {
     m_txEnableValid = true;
     m_txEnablePin = transmitEnablePin;
     pinMode(m_txEnablePin, OUTPUT);
     digitalWrite(m_txEnablePin, LOW);
  } else {
     m_txEnableValid = false;
  }
}

void SoftwareSerial::enableRx(bool on) {
   if (m_rxValid) {
      if (on) {
         attachInterrupt(m_rxPin, ISRList[m_rxPin], m_invert ? RISING : FALLING);
         m_buffer_time[M_TIME_RX_START] = GET_CYCLE_COUNT;   // initialise
         m_port_state = true;    // v59
      } else {
         detachInterrupt(m_rxPin);
         m_buffer_time[M_TIME_RX_END] = GET_CYCLE_COUNT;   // initialise
         m_port_state = false;    // v59
      }         
      m_rxEnabled = on;
      /*
      if (on)  // diagnose v58
        Serial.print((String) "\r\n Serial " + m_rxPin +  " On  " + (m_invert ? "RISING" : "FALLING") + " " );
      else        
        Serial.print((String) " Serial " + m_rxPin +  " OFF " + (m_invert ? "RISING" : "FALLING") + "\r\n" );
      
         Serial.print((String) " Serial " + m_rxPin 
                     + ", enableRx=" + (m_rxEnabled ? "ON" : "OFF")
                     + ", m_bitWait=" + m_bitWait 
                     + ", flank=" + (m_invert ? "RISING" : "FALLING") + "\r\n" );
      */
   }
}

void SoftwareSerial::enableRx(bool on, int recordtype) {
   m_port_state = false;    // v59
   if (m_rxValid) {
      if (on) {
         if (recordtype == SERIAL_RECORDTYPE_P1_B || recordtype == SERIAL_RECORDTYPE_WL_B) {
            if      (m_rxPin ==  4) attachInterrupt(m_rxPin, ISRList[16], CHANGE);
            else if (m_rxPin == 14) attachInterrupt(m_rxPin, ISRList[17], CHANGE);
         } else {
                                    attachInterrupt(m_rxPin, ISRList[m_rxPin], m_invert ? RISING : FALLING);
         }
         m_port_state = true;    // v59
      }
      m_buffer_time[M_TIME_RX_START] = GET_CYCLE_COUNT;   // initialise
   } else {
      detachInterrupt(m_rxPin);
      m_buffer_time[M_TIME_RX_END] = GET_CYCLE_COUNT;   // initialise
   }         
   m_rxEnabled = on;
   /*
   if (on)  // diagnose v58
     Serial.print((String) "\r\n Serial " + m_rxPin +  " On  " + (m_invert ? "RISING" : "FALLING") + " " );
   else        
     Serial.print((String) " Serial " + m_rxPin +  " OFF " + (m_invert ? "RISING" : "FALLING") + "\r\n" );
   
      Serial.print((String) " Serial " + m_rxPin 
                  + ", enableRx=" + (m_rxEnabled ? "ON" : "OFF")
                  + ", m_bitWait=" + m_bitWait 
                  + ", flank=" + (m_invert ? "RISING" : "FALLING") + "\r\n" );
   */
}


int SoftwareSerial::read() {
   if (!m_rxValid || (m_inPos == m_outPos)) return -1;
   uint8_t ch = m_buffer[m_outPos];
   m_outPos = (m_outPos+1) % m_buffSize;
   return ch;
}


bool SoftwareSerial::P1active() {                // When / is read on serial P1Active, at ! this is stopped
   if (m_P1active) return true;                  // 28mar21 added Ptro for P1 serialisation (to checkout)
   return false;
}

bool SoftwareSerial::portActive() {                // v59 indicate portstatus
   if (m_port_state) return true;
   return false;
}

int SoftwareSerial::available() {
   if (m_P1active) return 0;                     // Return 0 if P1 is active, buffer will be filled until receive '!'
   if (!m_rxValid) return 0;
   m_buffer_time[M_TIME_AVAIL_START] = GET_CYCLE_COUNT;   // initialise
   int avail = m_inPos - m_outPos;
   if (avail < 0) avail += m_buffSize;
   m_buffer_time[M_TIME_AVAIL_END] = GET_CYCLE_COUNT;   // initialise
   return avail;
}

#define WAIT { while (ESP.getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; }
#define WAITIram { while (SoftwareSerial::getCycleCountIram()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; }
// at 115k2 --> m_bittime = 694 cycles

size_t SoftwareSerial::write(uint8_t b) {
   if (!m_txValid) return 0;

   if (m_invert) b = ~b;
   if (m_highSpeed)
     // Disable interrupts in order to get a clean transmit
     cli();
   if (m_txEnableValid) digitalWrite(m_txEnablePin, HIGH);
   unsigned long wait = m_bitTime;
   digitalWrite(m_txPin, HIGH);
   unsigned long start = ESP.getCycleCount();
    // Start bit;
   digitalWrite(m_txPin, LOW);
   WAIT;
   for (int i = 0; i < 8; i++) {
     digitalWrite(m_txPin, (b & 1) ? HIGH : LOW);
     WAIT;
     b >>= 1;
   }
   // Stop bit
   digitalWrite(m_txPin, HIGH);
   WAIT;
   if (m_txEnableValid) digitalWrite(m_txEnablePin, LOW);
   if (m_highSpeed)
     // re-Enable interrupts
    sei();
   return 1;
}

void SoftwareSerial::flush() {
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation P1active finished
   m_inPos = m_outPos = 0;
}

bool SoftwareSerial::overflow() {
   bool res = m_overflow;
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation P1active finished
   m_overflow = false;
   return res;
}

int SoftwareSerial::peek() {
   if (!m_rxValid || (m_inPos == m_outPos)) return -1;
   return m_buffer[m_outPos];
}

/*
   return request Time entry where driver is see for names M_TIME_NAMES
*/
unsigned long SoftwareSerial::peekTime(int buffer_time_Pos ) {
   // if (!m_rxValid || (m_inPos == m_outPos)) return -1;
   return m_buffer_time[buffer_time_Pos];
}

/*
   Return requested BitTime entry
*/
unsigned long SoftwareSerial::peekBit(int buffer_bit_Pos ) {
   // if (!m_rxValid || (m_inPos == m_outPos)) return -1;
   return m_buffer_bits[buffer_bit_Pos];
}

/*
   Return requested BitTime entry
*/
int SoftwareSerial::peekByte(int buffer_byte_Pos ) {
   // if (!m_rxValid || (m_inPos == m_outPos)) return -1;
   return m_buffer[buffer_byte_Pos];
}


/*
   return number of entries in BitTime table
*/
unsigned long SoftwareSerial::peekBitPos() {
   return m_inPos;
}



// added wait test to prevent overrunning when Clocks are slower (10000 works ok, 2021-05-05 22:01:29: testing #BYTE_MAXWAIT_1)
// note: wait starts at approx 500
// getCycleCountIram = cycle counter, which increments with each clock cycle  (doc: v55d)
// BYTE_MAXWAIT_1 was 7000, changed in v59b to 7100
#define WAITIram4 { while (SoftwareSerial::getCycleCountIram()-start < wait && wait<BYTE_MAXWAIT_1); wait += m_bitTime; }
#define WAITIram4w { while (SoftwareSerial::getCycleCountIram()-start < m_wait && m_wait<BYTE_MAXWAIT_1); m_wait += m_bitTime; }
#define WAITIram5 { while (wait<BYTE_MAXWAIT_1 && SoftwareSerial::getCycleCountIram()-start < wait); wait += m_bitTime; }
// at 115k2 --> m_bittime = 694 cycles

/*
  Register cycle count in m_buffer_bits[m_buffer_bits_inPos]
*/
void ICACHE_RAM_ATTR SoftwareSerial::rxTriggerBit() {          
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);    // 26mar21 Ptro done at ISR start as per advice espressif //clear interrupt status
   int next = (m_inPos+1) % m_buffSize;
   if (next != m_outPos) {  // this works best in production
      m_buffer_bits[m_inPos] = M_BIT_CYCLE_VALUE;
      m_inPos = next;
   }
}

/*
  Do BitBang, store Byte in m_buffer[m_inPos] and check for P1 and
*/

void ICACHE_RAM_ATTR SoftwareSerial::rxRead() {
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);    // 26mar21 Ptro done at ISR start as per advice espressif //clear interrupt status

   // Advance the starting point for the samples but compensate for the
   // initial delay which occurs before the interrupt is delivered
   // unsigned long wait = m_bitTime + m_bitTime/3 - m_bitWait;	//corrupts	// 425 115k2@80MHz

      // unsigned long m_wait = m_bitTime + m_bitTime/3 - 500;		// 497-501-505 // 425 115k2@80MHz /
      unsigned long m_wait = m_bitTime + m_bitTime/3 - m_bitWait;		// 497-501-505 // 425 115k2@80MHz /
      // stored as m_wait

   // unsigned long wait = m_bitTime + m_bitTime/3 - 498;		// 501 // 425 115k2@80MHz

   // failed // unsigned long wait = 427UL;		// 501 // 425 115k2@80MHz   
   // unsigned long wait = m_bitWait;		// 425 115k2@80MHz // goes stuck
   // unsigned long wait = 425; // harcoded too fast as cycles for the calculation time are omitted
   unsigned long start = getCycleCountIram();         // cycle counter, which increments with each clock cycle  (doc: v55d)
   m_buffer_bits[m_inPos] = start;                    // v59_first try to get timnng
   uint8_t rec = 0;
   for (int i = 0; i < 8; i++) {
     WAITIram4w; // while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
     rec >>= 1;
     if (digitalRead(m_rxPin))
       rec |= 0x80;
   //   else                     // v52 balance isr rxread always doing or operation
   //     rec |= 0x00;
   }

   if (m_invert) rec = ~rec;
   // Stop bit , time betweeen bytes should not be needed to time as we have processed the databits (ISR is RISING or FALLING start bit, )
   // wait = wait - 400; // try to play with this time
   // wait = wait - (m_bitTime + m_bitTime/3 - 498) ; // no need to fully wait for end of stopbit and this finish the interrupt more quickly
   // wait = wait - 100;   // below 100 in production leads to more errors. In test (serial more reliable) value can lower than 400)
   //note: normal stopbit is LOW, inverted this (shoudl) shift to HIGH which may influence operations
   WAITIram4w; // stopbit:  while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
   
   // Store the received value in the buffer unless we have an overflow
   int next = (m_inPos+1) % m_buffSize;
   #ifdef TEST_MODE
      if (next != m_outPos && m_wait < BYTE_MAXWAIT_1) {  // abort if wait exceeded the expected readtime (test=OK, in production=NOK will cause more timeouts)
   #else
      if (next != m_outPos) {  // this works best in production
   #endif
      if (rec == '/') m_P1active = true ;   // 26mar21 Ptro P1 messageing has started by header
      if (rec == '!') m_P1active = false ; // 26mar21 Ptro P1 messageing has ended due valid trailer
      m_buffer[m_inPos] = rec;
      m_inPos = next;
   } else {
      m_P1active = false;                   // 26mar21 Ptro P1 messageing has ended due overflow
      m_overflow = true;
   }
   
   // Must clear this bit in the interrupt register,
   // it gets set even when interrupts are disabled
   // 26mar21 Ptro done at start: GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);
   // Serial.print("-"); // this blocks and make the routine inoperable
}

/*
  Do BitBang, store Byte in m_buffer[m_inPos]
*/
void ICACHE_RAM_ATTR SoftwareSerial::rxRead2() {
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);    // 26mar21 Ptro done at ISR start as per advice espressif //clear interrupt status

   // Advance the starting point for the samples but compensate for the
   // initial delay which occurs before the interrupt is delivered
   // unsigned long wait = m_bitTime + m_bitTime/3 - m_bitWait;	//corrupts	// 425 115k2@80MHz
   // unsigned long wait = m_bitTime + m_bitTime/3 - 498;		// 501 // 425 115k2@80MHz
   unsigned long wait = m_bitTime + m_bitTime/3 -  m_bitWait;	// v58 m_bitwait = 498 goes ok // 501 // 425 115k2@80MHz
   // unsigned long wait = m_bitWait;		// 425 115k2@80MHz // goes stuck
   // unsigned long wait = 425; // harcoded too fast as cycles for the calculation time are omitted
   unsigned long start = getCycleCountIram();
   /* ---------------------------------------------------------------------------------------------------------
    - timing:
               80Mhz --. approx 12.5 ns          per cycle or 80 cycles per microsecond. 
               115200 bit timing is approx: 8,6µS --> ~      694 cycles
               10bits = 86µSec -->                         6.940 cycles
               1024bytes ==>  ~  89,5mS minimal        7.111.111 cycles ==> 
          scoped USB: 875bytes: 245mS                 21.016.666
               interline time =   8-9mS                  650.666 cyles (19 bytes take=1.6mS = 133.333 cycles)
               startbit-down  =   8,6µSec                    700 cycles (sustains a rise -7.3µS) = 
                databits 8x:  =  69,2µSec                  5.750 cycles     
                stopbit up    =   8,6µSec                    700  
                     total    =  86,4µSec                  7.100
             protection limit =                            7.000 cycles = 84µSec
   ---------------------------------------------------------------------------------------------------------- */   
   m_buffer_bits[m_inPos] = start;                    // v59_first try to get timnng
   uint8_t rec = 0;     /// 236:	01a0d2    	            movi	a13, 1  
   for (int i = 0; i < 8; i++) {    /// 233:	08a0f2     	movi	a15, 8  
     WAITIram4; // while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
     rec >>= 1;         ///  2bd:	41d1d0               	srli	a13, a13, 1   
     if (digitalRead(m_rxPin))
       rec |= 0x80;     /// 239:	81af52               	movi	a5, -127   
     else                     // v52 balance isr rxread2 always doing or operation
       rec |= 0x00;     /// 237:	250c                	   movi.n	a5, 0      , note: x01 will optimize
   }
   if (m_invert) rec = ~rec;
   // Stop bit , time betweeen bytes should not be needed to time as we have processed the databits (ISR is RISING or FALLING start bit, )
   // wait = wait - 400; // try to play with this time
   // wait = wait - (m_bitTime + m_bitTime/3 - 498) ; // no need to fully wait for end of stopbit and this finish the interrupt more quickly
   // wait = wait - 100;   // below 100 in production leads to more errors. In test (serial more reliable) value can lower than 400)
   //note: normal stopbit is LOW, inverted this (shoudl) shift to HIGH which may influence operations
   WAITIram4; // stopbit:  while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
   
   // Store the received value in the buffer unless we have an overflow
   int next = (m_inPos+1) % m_buffSize;
   // rec = 'z'; // nty sure why we define this as
   #ifdef TEST_MODE
      if (next != m_outPos && wait < BYTE_MAXWAIT_1) {  // abort if wait exceeded the expected readtime (test=OK, in production=NOK will cause more timeouts)
   #else
      if (next != m_outPos) {  // this works best in production
   #endif
      if (rec == '/') m_P1active = true ;   // 26mar21 Ptro P1 messageing has started by header
                           ///  optimized compilation rubles things up
      if (rec == '!') m_P1active = false ; // 26mar21 Ptro P1 messageing has ended due valid trailer
      m_buffer[m_inPos] = rec;
      m_inPos = next;
   } else {
      m_P1active = false;                   // 26mar21 Ptro P1 messageing has ended due overflow
      m_overflow = true;
   }
   // Must clear this bit in the interrupt register,
   // it gets set even when interrupts are disabled
   // 26mar21 Ptro done at start: GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);
   // Serial.print("-"); // this blocks and make the routine inoperable
}


