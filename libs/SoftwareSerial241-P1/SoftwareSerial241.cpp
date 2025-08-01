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

// As the Arduino attachInterrupt has no parameter, lists of objects
// and callbacks corresponding to each possible GPIO pins have to be defined
SoftwareSerial *ObjList[MAX_PIN+1];

void ICACHE_RAM_ATTR sws_isr_0() { ObjList[0]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_1() { ObjList[1]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_2() { ObjList[2]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_3() { ObjList[3]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_4() { ObjList[4]->rxRead2(); };		// gpio4 is used for read secondary p1
void ICACHE_RAM_ATTR sws_isr_5() { ObjList[5]->rxRead(); };
// Pin 6 to 11 can not be used
void ICACHE_RAM_ATTR sws_isr_12() { ObjList[12]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_13() { ObjList[13]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_14() { ObjList[14]->rxRead(); };   // gpio14 is used for read primary p1
void ICACHE_RAM_ATTR sws_isr_15() { ObjList[15]->rxRead(); };

static void (*ISRList[MAX_PIN+1])() = {
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
      sws_isr_15
};

SoftwareSerial::SoftwareSerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) {
   m_rxValid = m_txValid = m_txEnableValid = false;
   m_buffer = NULL;
   m_invert = inverse_logic;
   m_overflow = false;
   m_rxEnabled = false;
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation between '/' and '!'
   m_bitWait = 498;                       // 2021-04-30 14:07:35 initialise to control bittiming (not used)
   if (isValidGPIOpin(receivePin)) {
      m_rxPin = receivePin;
      m_buffSize = buffSize;
      m_buffer = (uint8_t*)malloc(m_buffSize);
      if (m_buffer != NULL) {
         m_rxValid = true;
         m_inPos = m_outPos = 0;
         pinMode(m_rxPin, INPUT);
         ObjList[m_rxPin] = this;
         enableRx(true);
      }
   }
   if (isValidGPIOpin(transmitPin) || transmitPin == 16) {
      m_txValid = true;
      m_txPin = transmitPin;
      pinMode(m_txPin, OUTPUT);
      digitalWrite(m_txPin, !m_invert);
   }
   // Default speed
   begin(9600);
}

SoftwareSerial::~SoftwareSerial() {
   enableRx(false);
   if (m_rxValid)
      ObjList[m_rxPin] = NULL;
   if (m_buffer)
      free(m_buffer);
}

bool SoftwareSerial::isValidGPIOpin(int pin) {
   return (pin >= 0 && pin <= 5) || (pin >= 12 && pin <= MAX_PIN);
}

void SoftwareSerial::begin(long speed) {
   // Use getCycleCount() loop to get as exact timing as possible
   m_bitTime = ESP.getCpuFreqMHz()*1000000/speed;	// for 115k2=80000000/115200 = 694
   m_highSpeed = speed > 9600;
   m_P1active = false;                    // 28mar21 added Ptro for P1 serialisation between '/' and '!'
   if (!m_rxEnabled)
     enableRx(true);
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
      if (on)
         attachInterrupt(m_rxPin, ISRList[m_rxPin], m_invert ? RISING : FALLING);
      else
         detachInterrupt(m_rxPin);
      m_rxEnabled = on;
   }
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

int SoftwareSerial::available() {
   if (m_P1active) return 0;                     // Return 0 if P1 is active, buffer will be filled until receive '!'
   if (!m_rxValid) return 0;
   int avail = m_inPos - m_outPos;
   if (avail < 0) avail += m_buffSize;
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

// added wait test to prevent overrunning when Clocks are slower (10000 works ok, 2021-05-05 22:01:29: testing #7000)
// note: wait starts at approx 500
// getCycleCountIram = cycle counter, which increments with each clock cycle  (doc: v55d)
#define WAITIram4 { while (SoftwareSerial::getCycleCountIram()-start < wait && wait<7000); wait += m_bitTime; }
#define WAITIram5 { while (wait<7000 && SoftwareSerial::getCycleCountIram()-start < wait); wait += m_bitTime; }
// at 115k2 --> m_bittime = 694 cycles

void ICACHE_RAM_ATTR SoftwareSerial::rxRead() {
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);    // 26mar21 Ptro done at ISR start as per advice espressif //clear interrupt status

   // Advance the starting point for the samples but compensate for the
   // initial delay which occurs before the interrupt is delivered
   // unsigned long wait = m_bitTime + m_bitTime/3 - m_bitWait;	//corrupts	// 425 115k2@80MHz
   unsigned long wait = m_bitTime + m_bitTime/3 - 500;		// 497-501-505 // 425 115k2@80MHz
   // unsigned long wait = m_bitTime + m_bitTime/3 - 498;		// 501 // 425 115k2@80MHz

   // failed // unsigned long wait = 427UL;		// 501 // 425 115k2@80MHz   
   // unsigned long wait = m_bitWait;		// 425 115k2@80MHz // goes stuck
   // unsigned long wait = 425; // harcoded too fast as cycles for the calculation time are omitted
   unsigned long start = getCycleCountIram();         // cycle counter, which increments with each clock cycle  (doc: v55d)
   uint8_t rec = 0;
   for (int i = 0; i < 8; i++) {
     WAITIram4; // while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
     rec >>= 1;
     if (digitalRead(m_rxPin))
       rec |= 0x80;
     else                     // v52 balance isr rxread always doing or operation
       rec |= 0x00;
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
#ifdef TEST_MODE
   if (next != m_outPos && wait < 7000) {  // abort if wait exceeded the expected readtime (test=OK, in production=NOK will cause more timeouts)
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

void ICACHE_RAM_ATTR SoftwareSerial::rxRead2() {
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);    // 26mar21 Ptro done at ISR start as per advice espressif //clear interrupt status

   // Advance the starting point for the samples but compensate for the
   // initial delay which occurs before the interrupt is delivered
   // unsigned long wait = m_bitTime + m_bitTime/3 - m_bitWait;	//corrupts	// 425 115k2@80MHz
   // unsigned long wait = m_bitTime + m_bitTime/3 - 498;		// 501 // 425 115k2@80MHz
   unsigned long wait = m_bitTime + m_bitTime/3 -    m_bitWait;		// 501 // 425 115k2@80MHz
   // unsigned long wait = m_bitWait;		// 425 115k2@80MHz // goes stuck
   // unsigned long wait = 425; // harcoded too fast as cycles for the calculation time are omitted
   unsigned long start = getCycleCountIram();
   uint8_t rec = 0;
   for (int i = 0; i < 8; i++) {
     WAITIram4; // while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
     rec >>= 1;
     if (digitalRead(m_rxPin))
       rec |= 0x80;
     else                     // v52 balance isr rxread2 always doing or operation
       rec |= 0x00;
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
   if (next != m_outPos && wait < 7000) {  // abort if wait exceeded the expected readtime (test=OK, in production=NOK will cause more timeouts)
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


