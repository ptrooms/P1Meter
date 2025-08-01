/*
SoftwareSerial.h

SoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

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

#ifndef SoftwareSerial_h
#define SoftwareSerial_h
#include <inttypes.h>
#include <Stream.h>

/*
   Timer control by experience
*/

// #define USE_RXREAD58 58          // use this gpio implementation        cop/dup=549, prod=419 else=469
// #define USE_RXREAD59 59          // use this gpio implementation        cop/dup=549, prod=419 else=469
// #define USE_RXREAD60 60          // PROD/419 TEST/519
// #define USE_RXREAD61 61        // use this gpio implementation     61b 419
// else USE_RXREAD2 2                  // w.i.p

#if defined(COP_MODE) || defined(DUP_MODE)
   #define USE_RXREAD58 58             // v63a enabled
   // #define USE_RXREAD60 60          // v63a disabled
   // -----------------------------------------------------------------vvvv conditional settings

   #ifdef USE_RXREAD58
      #define USE_RXREAD USE_RXREAD58
      #ifndef BITWAIT1
         #define BITWAIT1 579 // v63a rxread58  479-579-769 
      #endif
   #elif defined(USE_RXREAD59)
      #define USE_RXREAD USE_RXREAD59
      #ifndef BITWAIT1
         #define BITWAIT1 509 // rx60=519 // rx60=524 // rx60=549      // v62a rxread59 524 t_wait=5974
      #endif
   #elif defined(USE_RXREAD60)
      #define USE_RXREAD USE_RXREAD60
      #ifndef BITWAIT1
         // #define BITWAIT1 504 // rx60=519 // rx60=524 // rx60=549      // v62a rxread59 524 t_wait=5974
         #define BITWAIT1 594 // rx60=594 rx60=519 // rx60=524 // rx60=549      // v62a rxread59 524 t_wait=5974
      #endif
   #endif

#elif defined(PROD_MODE) 
   #define USE_RXREAD58 58          // v63a enabled
   // #define USE_RXREAD59
   // #define USE_RXREAD60       // v63a disabled

   // -----------------------------------------------------------------vvvv conditional settings
   #ifdef USE_RXREAD58
      #define USE_RXREAD USE_RXREAD58
      #ifndef BITWAIT1
         // #define BITWAIT1 500    // v58 original
         // #define BITWAIT1 445       // v63a fixed (<-- wdt) 335-445-555 (--> errors) t=
         // #define BITWAIT1 525       // v63: 31jul25 14u07 v63a 01aug25 13u47 (without lockout)
         // #define BITWAIT1 511          // v63a:02aug25 14u07 v63a  using ETS_INTR_LOCK() / ETS_INTR_UNLOCK();
         #define BITWAIT1 502          // v63a:02aug25 14u07 v63a  using ETS_INTR_LOCK() / ETS_INTR_UNLOCK(); & bittime
         // #define BITWAIT1 536       // v63a: 461-536 using cli() / sei()
      #endif
   #elif defined(USE_RXREAD59)
      #define USE_RXREAD USE_RXREAD59
      #ifndef BITWAIT1
         // #define BITWAIT1 509       // v59 rxread59 509
         #define BITWAIT1 522       // v62a 524 v59 rxread59 509
      #endif
   #elif defined(USE_RXREAD60)
      #define USE_RXREAD USE_RXREAD60
      #ifndef BITWAIT1
         // #define BITWAIT1 419       // v61b rxread59 418 t_wait=6074
         // #define BITWAIT1 625       // v63 rxrea60 625 t_wait=5872 70,474µSec for 8 bits. t16 table: 6937=83,2µSec + lead=4,2=
         #define BITWAIT1 630       // v63 rxrea60 625 t_wait=5872 70,474µSec for 8 bits. t16 table: 6937=83,2µSec + lead=4,2=
         // 01aug25 : bitwait=630 mqtt=2902 faults:  Miss=323, Crc=299, Rcvr=815, Rp1=24, Yld=3, lT2=0
      #endif
   #endif

#else   
   #define USE_RXREAD58 58
   #define USE_RXREAD USE_RXREAD58
   #ifndef BITWAIT1
      #define BITWAIT1 469       // else test, fixed wait = 500
   #endif
#endif

//30jul25 COP_MODE RXREAD60 BITWAIT1 524

// check test
// #undef BITWAIT1
// #define BITWAIT1 509
// #define BITWAIT1 419    // v61b with rxread59 PROD
   // v58: m_bitWait = 498;   rxread58
   // v59: m_bitWait = 509;   rxRead59 59a/59b
   // v60: m_bitWait = 519;   rxRead60
   // v60: m_bitWait = 435;   rxRead61
   // v61b --> 419

/*
   test diagnostics v61+ in rxRead60
*/


// This class is compatible with the corresponding AVR one,
// the constructor however has an optional rx buffer size.
// Speed up to 115200 can be used.

#define SERIAL_RECORDTYPE_PORT 0    // read physical serialport for reading data
#define SERIAL_RECORDTYPE_P1   1    // use simulated data in P1 energy format (default)
#define SERIAL_RECORDTYPE_WL   2    // use simulated data in WL heat format
#define SERIAL_RECORDTYPE_P1_B 3    // and do bittiming for P1
#define SERIAL_RECORDTYPE_WL_B 4    // and do bittiming for WL


#ifndef M_TIME_NAMES
   #define M_TIME_NAMES           // Indicate our name
   #define M_TIME_ENTRIES    20  // Numbeer of entries in M_TIME table
   #define M_TIME_START       0   // cyclenumber start of Ovject
   #define M_TIME_RX_START    1   // cyclenumber start of void SoftwareSerial::enableRx Attach
   #define M_TIME_RX_END      2   // cyclenumber end of void SoftwareSerial::enableRx Detach
   #define M_TIME_BEGIN_START 3   // cyclenumber start of  SoftwareSerial::begin
   #define M_TIME_BEGIN_END   4   // cyclenumber end of  SoftwareSerial::begin
   #define M_TIME_AVAIL_START 5   // cyclenumber start of void SoftwareSerial::enableRx Attach
   #define M_TIME_AVAIL_END   6   // cyclenumber start of void SoftwareSerial::enableRx Attach
   #define M_TIME_BIT_START   7   // v60a Cycle ISR START
   #define M_TIME_BIT_STOP    8   // v60a Cycle ISR AFTER STOPBIT
   #define M_TIME_BIT_START1  9   // v60a Cycle ISR Nominal end Actual End (inclusdng get cycle)
   #define M_TIME_BIT_STOPT  10   // v60a Cycle ISR Nominal to save 
   #define M_TIME_BIT_STOP1  10   // v60a Cycle ISR Nominal end
   #define M_TIME_BIT_END1   11   // v60a Cycle ISR END
   #define M_TIME_BIT_END2   12   // v60a Cycle ISR Nominal end
#endif

#define M_BIT_CYCLE_VALUE (getCycleCountIram() % 4096);     // get distance

class SoftwareSerial : public Stream
{
public:
      SoftwareSerial(int receivePin, int transmitPin, bool inverse_logic = false, unsigned int buffSize = 64);
      SoftwareSerial(int receivePin, int transmitPin, int  inverse_logic = 10   , unsigned int buffSize = 64);
   ~SoftwareSerial();   // called when destroy (reaching end of scope, or calling delete to a pointer to) the instance of the object.

   void begin(long speed);             // will call/do  SERIAL_RECORDTYPE_PORT
   void begin(long speed, int);        // elect type of port 0-physical/ 1-simulatedP1 / 2-simulatatedWL
   long baudRate();
   void setTransmitEnablePin(int transmitEnablePin);

   bool overflow();
   int peek();
   // unsigned long peek(int);
   unsigned long peekBit(int);            // v59b return request Time of inserted entry
   unsigned long peekBitPos();            // v59b return last entry in BitTime table
   unsigned long peekTime(int);           // v59b return Time where driver is
   int           peekByte(int);           // v59b return this Byte in Buffer

   virtual size_t write(uint8_t byte) override;
   virtual int read() override;
   virtual int available() override;
   virtual void flush() override;
   virtual bool P1active();          // check if driver is active beween header-/ and trailer byte-!
   virtual bool portActive();        // check if port is active (m_port_state)
   operator bool() {return m_rxValid || m_txValid;}

   // Disable or enable interrupts on the rx pin
   void enableRx(bool on);
   void enableRx(bool on, int recordtype);      // v58 to use/do/selec type of data 0-physical/ 1-simulatedP1 / 2-simulatatedWL

   void rxRead();		   // v59b volatile (to be user) BitBang P1 with    p1active detection beween / and !
   void rxRead2();		// BitBang Wl without p1active detection beween / and !
   void rxRead58();		// BitBang routine v58 
   void rxRead59();		// BitBang routine v59 
   void rxRead60();		// BitBang routine v60
   void rxRead61();		// BitBang routine v61
   void rxTriggerBit(); // use bittiming every flank change allocates a time
   int m_use_rxRead;      // holds the USE_RXREAD ISR number to be used
   
   // AVR compatibility methods
   bool listen() { enableRx(true); return true; }
   void end() { stopListening(); }
   bool isListening() { return m_rxEnabled; }
   bool stopListening() { enableRx(false); return true; }
   volatile unsigned long m_bitWait;         // introduced to control bittiming
   volatile int preset_bitWait;              // v58 to set m_bitwait up/down
   inline uint32_t getCycleCountIram();

   using Print::write;

private:
   bool isValidGPIOpin(int pin);
   // Member variables
   int m_rxPin, m_txPin, m_txEnablePin;
   bool m_rxValid, m_rxEnabled;
   bool m_txValid, m_txEnableValid;
   bool m_invert;
   #ifdef RXREAD58
      bool m_P1active;                 // Ptro 28mar21 to support P1 messageing 
   #else
      volatile bool m_P1active;        // Ptro 28mar21 to support P1 messageing, volatile v59b used in ISR
   #endif
   bool m_port_state;               // v59 contains status of (in)activated ISR
   bool m_overflow;        // volatile v59b , used in ISR
   volatile unsigned long m_bitTime;  // volatile v60a, used in ISR
   // volatile unsigned long m_bitWait;         // introduced to control bittiming
   bool m_highSpeed;
   #ifdef RXREAD58
      unsigned int m_inPos, m_outPos;
   #else
      volatile unsigned int m_inPos, m_outPos;
   #endif
   int m_buffSize;
   uint8_t *m_buffer;            // note this is a pointer to unt8_t array (aka bytes) index by m_inPos, m_outPos;

   unsigned long *m_buffer_bits;         // 4096-value time-array  index m_inPos, m_outPos;
   unsigned int m_buffer_bits_inPos;        // position in buffer
   unsigned int m_buffer_bits_outPos;        // position in buffer
   
   int          m_buffer_bitValue;       // Last time detected index by 
   
   unsigned long *m_buffer_time;         // time-array
   unsigned int m_buffer_timePos;        // position in buffer
   
      
   // unsigned long m_wait = m_bitTime + m_bitTime/3 - 500;
   // 497-501-505 // 425 115k2@80MHz 
   unsigned long m_wait; 

   bool m_d4_isr_state;        // v61b track ISR state

};

// ??  ESP.getCycleCount()

uint32_t ICACHE_RAM_ATTR SoftwareSerial::getCycleCountIram()
{
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount":"=a" (ccount));
    return ccount;
}

// If only one tx or rx wanted then use this as parameter for the unused pin
#define SW_SERIAL_UNUSED_PIN -1


#endif
