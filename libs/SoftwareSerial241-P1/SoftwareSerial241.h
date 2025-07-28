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
   void rxRead59();		// BitBang routine v58 
   void rxRead60();		// BitBang routine v60
   void rxTriggerBit(); // use bittiming every flank change allocates a time

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
   volatile bool m_P1active;                 // Ptro 28mar21 to support P1 messageing, volatile v59b used in ISR
   bool m_port_state;               // v59 contains status of (in)activated ISR
   volatile bool m_overflow;        // volatile v59b , used in ISR
   volatile unsigned long m_bitTime;  // volatile v60a, used in ISR
   // volatile unsigned long m_bitWait;         // introduced to control bittiming
   bool m_highSpeed;
   volatile unsigned int m_inPos, m_outPos;     // volatile v59b
   volatile int m_buffSize;
   uint8_t *m_buffer;            // note this is a pointer to unt8_t array (aka bytes) index by m_inPos, m_outPos;

   unsigned long *m_buffer_bits;         // 4096-value time-array  index m_inPos, m_outPos;
   unsigned int m_buffer_bits_inPos;        // position in buffer
   unsigned int m_buffer_bits_outPos;        // position in buffer
   
   int          m_buffer_bitValue;       // Last time detected index by 
   
   unsigned long *m_buffer_time;         // time-array
   unsigned int m_buffer_timePos;        // position in buffer
   
      
   // unsigned long m_wait = m_bitTime + m_bitTime/3 - 500;
   // 497-501-505 // 425 115k2@80MHz 
   volatile unsigned long m_wait; 


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
