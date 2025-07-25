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


class SoftwareSerial : public Stream
{
public:
   SoftwareSerial(int receivePin, int transmitPin, bool inverse_logic = false, unsigned int buffSize = 64);
   ~SoftwareSerial();   // called when destroy (reaching end of scope, or calling delete to a pointer to) the instance of the object.

   void begin(long speed);
   long baudRate();
   void setTransmitEnablePin(int transmitEnablePin);

   bool overflow();
   int peek() override;

   virtual size_t write(uint8_t byte) override;
   virtual int read() override;
   virtual int available() override;
   virtual void flush() override;
   virtual bool P1active();          // defined class used during P1 serilisation
   operator bool() {return m_rxValid || m_txValid;}

   // Disable or enable interrupts on the rx pin
   void enableRx(bool on);

   void rxRead();		// witp1active detection beween / and !
   void rxRead2();		// without p1active detection beween / and !

   // AVR compatibility methods
   bool listen() { enableRx(true); return true; }
   void end() { stopListening(); }
   bool isListening() { return m_rxEnabled; }
   bool stopListening() { enableRx(false); return true; }

   inline uint32_t getCycleCountIram();

   using Print::write;

private:
   bool isValidGPIOpin(int pin);
   // Member variables
   int m_rxPin, m_txPin, m_txEnablePin;
   bool m_rxValid, m_rxEnabled;
   bool m_txValid, m_txEnableValid;
   bool m_invert;
   bool m_P1active;                 // Ptro 28mar21 to support P1 messageing
   bool m_overflow;
   unsigned long m_bitTime;
   volatile unsigned long m_bitWait;         // introduced to control bittiming
   bool m_highSpeed;
   unsigned int m_inPos, m_outPos;
   int m_buffSize;
   uint8_t *m_buffer;


};

uint32_t ICACHE_RAM_ATTR SoftwareSerial::getCycleCountIram()
{
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount":"=a" (ccount));
    return ccount;
}

// If only one tx or rx wanted then use this as parameter for the unused pin
#define SW_SERIAL_UNUSED_PIN -1


#endif
