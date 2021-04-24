

// W A R N I N G : Note check port and nodemcu port before uploading using Arduino Framework
// see/for git: https://github.com/ptrooms/P1Meter
// Update on V20 to check if this works in Visual Studio Code 

// --> use NodeMCU1.0 ESP-12E with LwIp V2 lower memory, BuiltIn led (GPIO)16

#define P1_Override_Settings    // set in PlatoformIO ; override wifi/server settings
// #define TEST_MODE    // set in PlatformIO; use our file defined confidential wifi/server/mqtt settings
#define UseP1SoftSerialLIB   //  use the old but faster SoftwareSerial structure based on 2.4.0 and use P1serial verion to listen /header & !finish
#define RX2TX2LOOPBACK false  // OFF , ON:return Rx2Tx2 (loopback test) & TX2 = WaterTriggerread
#define P1_STATIC_IP          // if define we use Fixed-IP, else (undefined) we use dhcp

#ifdef TEST_MODE
  #warning This is the TEST version, be informed
  #define P1_VERSION_TYPE "t1"      // "t1" for ident nodemcu-xx and other identification to seperate from production
  #define DEF_PROG_VERSION 11275.241 // current version (displayed in mqtt record)
      // #define TEST_CALCULATE_TIMINGS    // experiment calculate in setup-() ome instruction sequences for cycle/uSec timing.
      // #define TEST_PRINTF_FLOAT       // Test and verify vcorrectness of printing (and support) of prinf("num= %4.f.5 ", floa 
#else
  #warning This is the PRODUCTION version, be warned
  #define P1_VERSION_TYPE "p1"      // "p1" production
  #define DEF_PROG_VERSION 2127.241 //  current version (displayed in mqtt record)
#endif
// #define ARDUINO_<PROCESSOR-DESCRIPTOR>_<BOARDNAME>
// tbd: extern "C" {#include "user_interface.h"}  and: long chipId = system_get_chip_id();

// *
// * * * * * L O G  B O O K
// *
// Compiled on Arduino:
//  - fixed Ip adress to eliminate iterference (if defined P1_STATIC_IP )
    // Set your Static IP address IPAddress local_IP(192, 168, 1, 125);
    // Set your Gateway IP address IPAddress gateway(192, 168, 1, 1);
    // Subnet IPAddress subnet(255, 255, 255, 0);
    // DNS1 IPAddress primaryDNS(192.168.1.8);   //optional
    // DNS2 IPAddress secondaryDNS(192.168.1.1); //optional
// ## [V21.25]    
//  - Using SDK 2.4.1 to investigate stability
// ## [V21.22]
//  19apr21 23u39 added ESP.wdtFeed(); in loop ()
//  - 19apr21 15u22 - added diagnostic information sdk version & emempry use etc.etc.
//  - 15apr21 14u14
//    Sketch uses 298836 bytes (28%) of program storage space. Maximum is 1044464 bytes.
//    Global variables use 40276 bytes (49%) of dynamic memory, leaving 41644 bytes for local variables. Maximum is 81920 bytes.
// 	- 15apr21 00u02 changed to platformio 1.6.0 (uses arduino 2.4.0) to check if this stabilize
// 	- 15arp21 00u02 platformio 1.7.0 (uses arduino 2.4.1) wdt reset after 500-800 reads
// 	- removed delay in local-yields
// ## [V21.21]
// - 14apr21 01u50 only output to mqtt if it is connected  via "if (client.connected())" 
// - 13apr21 18u38 V21 added progress line-counter tio research where WDT hits....
// 	--- (+9sec after last mqtt)
// - 13apr21 18u38 V21 improved WDT as we call "mqtt client".loop() during speific yields,
// 	-- normal yield does support Wifi but NOT the (disconnected) Pubsubclient
// 	-- Beautified
// ## [V21.20]
// - 11apr21 16u29 V20: added JSON error message to topic /error/.. if P1 serial is not (properly) connected
// 	-- /error/t1 {"error":001 ,"msg":"serial not connected", "mqttCnt":28}
// - 11apr21 15u16 mqtt timeout (override) set from 15 to 60 seconds #define MQTT_KEEPALIVE = 60
// 	-- sometimes the mqtt server is very busy and we do not want a preliminary reboot
// - 11apr21 15u15 Added mqtt server address  in serial debug at startup
// 13apr21 V21.20/241 improved and added better yield that also calls the pubsub mqtt loop *(if connected)
// __ap21  migrated to github repo and CHANGED.md
// 08apr21 V21.20/241 adapted for PlatformIO AND Arduino 
//             -
// 07apr21 V21.19/241 adapted for PaltformIO
// 04apr21 V21.19/241 Hardware Library 2.4.1 and use Lwip 2.0 lower memory, very stable
/*
                Sketch uses 297412 bytes (28%) of program storage space. Maximum is 1044464 bytes.
                Global variables use 39676 bytes (48%) of dynamic memory, leaving 42244 bytes for local variables. Maximum is 81920 bytes.
*/
// 03apr21 V21.18/274 Lower mempry no features
// 03apr21 V21.18/274 -68 bytes: Refactored incude libraries UseP1SoftSerialLIB will include <SoftwareSerial241>
//                - which has the P1active() function between / & ! durign serial receive
/*
                  Executable segment sizes:
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP 
                  BSS    : 28240 )         - zeroed variables      (global, static) in RAM/HEAP 
                  IROM   : 299984          - code in flash         (default or ICACHE_FLASH_ATTR) 
                  IRAM   : 29036   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
                  Sketch uses 334836 bytes (32%) of program storage space. Maximum is 1044464 bytes.
                  RODATA : 4472  ) / 81920 - constants             (global, static) in RAM/HEAP 
                  Global variables use 34056 bytes (41%) of dynamic memory, leaving 47864 bytes for local variables. Maximum is 81920 bytes.

*/
// 03apr21 V21.18/274 +332 bytes: sprintf causes corruption, use snprinf when using multiple formatted valaues 
//                - removed unused librabries ESP8266mDNS.h ESP8266HTTPClient.h WiFiUdp.h OneWire.h
/*                 
 *                Executable segment sizes:
                  BSS    : 28248 )         - zeroed variables      (global, static) in RAM/HEAP 
                  IRAM   : 29036   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
                  IROM   : 300016          - code in flash         (default or ICACHE_FLASH_ATTR) 
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP 
                  RODATA : 4508  ) / 81920 - constants             (global, static) in RAM/HEAP 
                  Sketch uses 334904 bytes (32%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 34100 bytes (41%) of dynamic memory, leaving 47820 bytes for local variables. Maximum is 81920 bytes.
 */
// 03apr21 V21.18/274 +4bytes: Progress message changed to 
//                - M#@t=15 @201, TimeP1="000309" (e#=0)    Gpio5:1, Water#:0, Hot#:0, WaterState:0, Trg#:0, DbC#:0, Int#:344 .
//                = mqttcnt/secs  P1-readed hhmmss  CrcE    state    count     Ledl    sensor        #number Debounce Interval
/*
                  Executable segment sizes:
                  IROM   : 299712          - code in flash         (default or ICACHE_FLASH_ATTR) 
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP 
                  RODATA : 4484  ) / 81920 - constants             (global, static) in RAM/HEAP 
                  BSS    : 28240 )         - zeroed variables      (global, static) in RAM/HEAP 
                  IRAM   : 29036   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
                  Sketch uses 334576 bytes (32%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 34068 bytes (41%) of dynamic memory, leaving 47852 bytes for local variables. Maximum is 81920 bytes.
*/ 
// 03apr21 V21.17/274 mqtt buffer increated from 360 to 480
//                    - if serial data contain a space, it is also counted as invalid character (field e#=xxx)
//                    - buffer too small, leading to JSON failures
//                    - when dynamically adding "Force, Ttrigger and PullUp fields causing "}" deletion overflow
/*
                  Executable segment sizes:
                  BSS    : 28248 )         - zeroed variables      (global, static) in RAM/HEAP 
                  RODATA : 4496  ) / 81920 - constants             (global, static) in RAM/HEAP 
                  IRAM   : 29036   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP 
                  IROM   : 299696          - code in flash         (default or ICACHE_FLASH_ATTR) 
                  Sketch uses 334572 bytes (32%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 34088 bytes (41%) of dynamic memory, leaving 47832 bytes for local variables. Maximum is 81920 bytes.
 */
// 03apr21 V21.17/274 changed BLUE-LED state will invert during watertrigger (will reset at end by thermostate)
//                    - Normally this wil show  & relfect the  (stable) Thermostate ON/OFF state
//                    - during mqtt & active serial P1 read/processiong it will blink for that  (short) duration
//                    - During Watertapping, the BLUE_LED will be pulsed by the attached interrupts (hence twice per liter).
// 03apr21 V21.17/274 streamlined source and added loop-() check timer 1 second interval to limit WDT reset risks
/*
                  Executable segment sizes:
                  IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
                  IROM   : 299376          - code in flash         (default or ICACHE_FLASH_ATTR)
                  RODATA : 4436  ) / 81920 - constants             (global, static) in RAM/HEAP
                  BSS    : 28240 )         - zeroed variables      (global, static) in RAM/HEAP
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP
                  Sketch uses 334172 bytes (31%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 34020 bytes (41%) of dynamic memory, leaving 47900 bytes for local variables. Maximum is 81920 bytes.
*/
// 02apr21 V21.16/274 testOnly chekc ISR P1serial timings  Header/Trailer (hardly a cycle and near to a µSec)
// 02apr21 V21.15/274 Using My changed 240 to display serial progress
// 02apr21 V21.14/274 Only insert a date from P1Meter data in JSON mqtt if this fully nummeric else use previous (whioch could be "000000"
// 02apr21 V21.14/274 Added runtime in Seconds at Timestamp to ease PD
/*
                  Executable segment sizes:
                  RODATA : 4292  ) / 81920 - constants             (global, static) in RAM/HEAP
                  IROM   : 299024          - code in flash         (default or ICACHE_FLASH_ATTR)
                  BSS    : 28232 )         - zeroed variables      (global, static) in RAM/HEAP
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP
                  IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
                  Sketch uses 333676 bytes (31%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 33868 bytes (41%) of dynamic memory, leaving 48052 bytes for local variables. Maximum is 81920 bytes.
*/
// 01apr21 V21.14/274 Making CRC work with esp8266 is a change of 1 top 15 as as softserial often goes in missing characters
/*
                  Executable segment sizes:
                  IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
                  RODATA : 4284  ) / 81920 - constants             (global, static) in RAM/HEAP
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP
                  IROM   : 298928          - code in flash         (default or ICACHE_FLASH_ATTR)
                  BSS    : 28232 )         - zeroed variables      (global, static) in RAM/HEAP
                  Sketch uses 333572 bytes (31%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 33860 bytes (41%) of dynamic memory, leaving 48060 bytes for local variables. Maximum is 81920 bytes.
*/
//                    - extended mqtt record with Json P1crc state 0=NotOk, 1=Valid CRC
//                    - mqtt log record corrected "powerConsumptionLowTariff" exchanged with "powerConsumptionHighTariff"
//                    - hardly no alternative that accepting that almost every tranaction has an error, as long
//                      we can read the values in number fields, this is OK.
//                    - cleaned comments and debugging to check why CRC16 constantly fails.
/*
                  Executable segment sizes:
                  BSS    : 28224 )         - zeroed variables      (global, static) in RAM/HEAP
                  IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
                  DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP
                  IROM   : 298944          - code in flash         (default or ICACHE_FLASH_ATTR)
                  RODATA : 4332  ) / 81920 - constants             (global, static) in RAM/HEAP
                  Sketch uses 333636 bytes (31%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 33900 bytes (41%) of dynamic memory, leaving 48020 bytes for local variables. Maximum is 81920 bytes.
*/
// 30mar21 v21.13/274 added yield time control (p1TriggerTime) to prevent we get a wdt reset while waiting for serial input
/*
                  Executable segment sizes:
                  BSS    : 28736 )         - zeroed variables      (global, static) in RAM/HEAP
                  RODATA : 4404  ) / 81920 - constants             (global, static) in RAM/HEAP
                  DATA   : 1348  )         - initialized variables (global, static) in RAM/HEAP
                  IROM   : 299568          - code in flash         (default or ICACHE_FLASH_ATTR)
                  IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
                  Sketch uses 334336 bytes (32%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 34488 bytes (42%) of dynamic memory, leaving 47432 bytes for local variables. Maximum is 81920 bytes.
*/
// 30mar21 v11.13/274 added CRC , using/following [https://github.com/jantenhove/P1-Meter-ESP8266]
/*
                  Executable segment sizes:
                  RODATA : 4272  ) / 81920 - constants             (global, static) in RAM/HEAP
                  IROM   : 299088          - code in flash         (default or ICACHE_FLASH_ATTR)
                  IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
                  DATA   : 1340  )         - initialized variables (global, static) in RAM/HEAP
                  BSS    : 28096 )         - zeroed variables      (global, static) in RAM/HEAP
                  Sketch uses 333716 bytes (31%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 33708 bytes (41%) of dynamic memory, leaving 48212 bytes for local variables. Maximum is 81920 bytes.
*/

// 30mar21 v11.13/274 started with fixing RX2 port on GPIO2 output and GPIO4 Input, rememeber we have two class instances
//                    - added yield() after reading P1 available serial data....
/*
                  Executable segment sizes:
                  IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
                  BSS    : 28080 )         - zeroed variables      (global, static) in RAM/HEAP
                  RODATA : 4244  ) / 81920 - constants             (global, static) in RAM/HEAP
                  DATA   : 1340  )         - initialized variables (global, static) in RAM/HEAP
                  IROM   : 298976          - code in flash         (default or ICACHE_FLASH_ATTR)
                  Sketch uses 333576 bytes (31%) of program storage space. Maximum is 1044464 bytes.
                  Global variables use 33664 bytes (41%) of dynamic memory, leaving 48256 bytes for local variables. Maximum is 81920 bytes.
*/
// 30mar21 v21.12/274 If mqtt is not connected, continue to support the primary/core thermostatic function
//                    - the routines are implemented in doCritical(): processGpio() & handleOTA()
//                    - the mqtt client.connect routine is added with extra code to increased wait loops
//                      stating at 2 tot 30 seconds this to prevent that unnecessary load.
//                    - Note: extensive printing can significantly impact total performance.
//                      remember: the esp8266 is a single task machine that operates in a loop
//    sudo cat /dev/ttyS4 | stdbuf -o0 grep -a -E '(started|stopped|wdt)' | stdbuf -o0 ts | stdbuf -o0  grep -a -A 1 -B 1 -E '(wdt)'
//    mosquitto_pub -t nodemcu-p1/switch/port1 -m D

// The next may be usefull in determining which Core version is (not) to be used.
//  #if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1) || defined(ARDUINO_ESP8266_RELEASE_2_4_2) || defined(ARDUINO_ESP8266_RELEASE_2_5_0) || defined(ARDUINO_ESP8266_RELEASE_2_5_1) || defined(ARDUINO_ESP8266_RELEASE_2_5_2)
//   #warning "Arduino ESP8266 Core versions before 2.7.1 are not supported"
//   #endif
/*
   Executable segment sizes:
   IROM   : 298880          - code in flash         (default or ICACHE_FLASH_ATTR)
   IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
   RODATA : 4208  ) / 81920 - constants             (global, static) in RAM/HEAP
   BSS    : 28088 )         - zeroed variables      (global, static) in RAM/HEAP
   DATA   : 1340  )         - initialized variables (global, static) in RAM/HEAP
   Sketch uses 333444 bytes (31%) of program storage space. Maximum is 1044464 bytes.
   Global variables use 33636 bytes (41%) of dynamic memory, leaving 48284 bytes for local variables. Maximum is 81920 bytes.
*/
// 30mar21 v21.12/274 source code added to distinquick Test from Production (TEST_MODE absent)
// 28mar21 v21.11/274 added ESP.wdtFeed() after yield() to feed the WDT timer.
// 27mar21 V21.11/274 If (inverted) ledlight1 is active (waterheating on), accumulate the watercnt also as "HotCnt"
//               added loop timers: overall 120mSec,
//               approx 2Sec is for 10Sec/P1 intervals minus margins 1-2 seconds,
//               which leaves about 6 seconds for other items.
//               added check not to process P1serial if we are already finished
//               in case serial reads a zerolength byte, innore this
//               improved/validation of timestamp. Without P1Meter we use millis() to calculate timestamp
//               During forced mode (to retain) temprature functions, Mqtt display "Force:cnt"
// 26mar21 V21.10 Changed library /home/pafoxp/Arduino/libraries/SoftwareSerial-2.4.0/SoftwareSerial240.cpp
//                  void ICACHE_RAM_ATTR SoftwareSerial::rxRead() {
//                     GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);  // 26mar21 Ptro done at start as per advice espressif
// 26mar21 V21.10 clearing interrupt for our WATERSENSOR_READ at start of routine
//
// 26mar21 V21.09 Corrected Debug print inactive/active state at command "D"
// 26mar21 V21.09 Stable version, improved reliability by fluctuating input pull-up to assist state.
// 23mar21 V21.08 hardware set to use external pullup, internal pullup  WaterSensor disabled
//                in LoopBack testmode, the WaterSensor readstate is forewarded to GPIO2 (TX2)
//                Introduced "W" to reWire ISR watersensor interrupt WaterTrigger1 (<>Watertrigger)
//                Introduced "w" to switch ON (<>OFF) internal pullup for watersensor pin
// 23mar21 V21.08 revise debounce WaterTrigger via ISR, add "Z" to zeroise counters
// 21mar21 V21.07 Arrange and use old SoftwareSerial code  (of hw 2.4.0) to accomodate rs232
//   allocated dedicated softeware serial librbary version 3.3.1. as (was) used in hardwaresetup 2.4.0
//   - newer librbaries are to slow and not useful at baudrate > 57600
//   - for this: allocate SoftwareSerial-2.4.0 with name SoftwareSerial240.h/.cpp
//   - code require the use for "#define UseP1SoftSerialLIB" to disinquish from new layout
//   - new serial layout as of version 2.6.0 uses a setup that's more compatible with hardware serial library.
//   P1 serial databuffer increased 600 bytes
// 20mar21 V21.06  prepare watertap meter T1MeterT.5.ino
//   structured D0-D8 Arduino port schema
//   added myserial2 (1200 baud for Kamstrup byte for byte interface)
// 29jun20 V13.05 renamed to T1MeterT.4.ino
// T1MeterT.4, external p1 renamed to t1
// 29jun20 V13.05 copied from /home/pafoxp/code-P1meter/P1Meter1.4 but for test only
// ----------------------------------------------------------------------------------------
// 29jun20 V13.05 testing OTA which failed at laptop
// 10jun20 V13.04 P1 timeout processing, allow do logic wihout P1 connected, i/I-nterval
// 19nov19 V13.03 tiem string with leading zeroes is sometimes/somewhere not acceptable as json-number, reverted to valu number
// 19nov19 V13.03 reverted back to 2.4.0 (2.5.2 instable rs232), add timestring is now substringed from timedate, eases conversion
// 19nov19 V13.03 l/L activates off/on Mqtt logging of kaifka meter
// 28okt18 V13.03 2.5.2 LwIP V2: solve double ,, in json output , add mqtt count
// 28okt18 V13.02 2.4.0 LwIP V2: S/W-inter time in "0-0:1.0.0(181028223640W)" message
// 13sep18 V13.01 2.4.0 LwIP V2: Default P1 follow mode
// 04sep18 V1.3 Compile with esp8266 expressif 2.3.0
// 27aug18 V1.3 MQTT pubsize expanded from 256 to 320 (as we added a field)
// 27aug18 V1.3 Added temperature device 6 (livingroom Whites=GND, Green=Signal, Orange=3.3V) 28aa93bb13130138 as T0 (??)
// 27aug18 V1.3 Thermistor replaces by LDR in Livingroom (Blue/white wire)
// 01aug18 V1.3 Added 4 devices DS18B20 JSON published as T2, T3, T4, T5
// 24jul18 V1.3 DS18B20 temperature reading which is JSON published as T1
// 14jul18 V1.2 MQTT subscription for commands mosquitto_pub -h 192.168.1.8 -p 1883 -t "nodemcu-p1/switch/port1" -m "L"
//  :: Reset, noLogmqtt, doDebugserialout, doP1mqtt, 1-heatD8/2-off/3-leave/2-followD7
// 25jun2018 00u00 - added Digital read D6 {LedLight1} for hotwater LedLight sensor
// 24jun2018 00u00 - added analog read A0 (Ajson: {analogRead} for temperature sensor
// D7 is input {ThermoState} On/low  , D8 is (planned) heat control {Hlight} On/high
// P1 meter report: {CurrentPowerConsumption} {powerConsumptionHighTariff} {powerConsumptionLowTariff}
//
// updated to rebooot in case millis interval if no messages were send

// added for Visual Studio Code , check & include local Hardware paths /home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266
//                                Note we momentariluy use Arduino IE, so this section mat require update
// #include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.3.0/cores/esp8266/stdlib_noniso.h>
// #include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.2/variants/nodemcu/pins_arduino.h>
// #include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.2/cores/esp8266/stdlib_noniso.h>

//

// Note changing libarieslooks to be useless as only 2.4.1 seem to resist WDT reset and is 99% (Serial) reliable
//      but we levae the oportunity to centrally control which specifx libary will be included/
//      any library must be made available with(in) Arduino/esp8266 Boardmanager
// #define libuse242  // 301136 bytes, not very reliable, serial produces not many successes
#define libuse241     // 297428/prod, 297412/test bytes, best version offering reliability and NO spontaneous wdt resets
// #define libuse250  // TBI 322320 bytes, not very trustfull, serial is interrupt and causes soft errors, not sure about wdt
// #define libuse252  // unreliable (in the past, not sure how i operates with adapted P1 softserial), UseNewSoftSerialLIB
// #define libuse241  // 297428 bytes, very stable, fast compile (note we use the softserial 2.4.1. adapted to p1), 
// #define libuse274  // unstable, the esp remains for at 10 to 35 minutes active and then goes into wdt, UseNewSoftSerialLIB
// #define libuse240  // very goed library, fast compile to use UseNewSoftSerialLIB

// - - - > following was for test experimenting with libbary use directived. Can be deleted.
// test #include </home/pafoxp/code-P1meter/SoftwareSerial_2.4.0/SoftwareSerial.h>
// #ifdef UseP1SoftSerialLIB   // do we use an old library
// #undef UseP1SoftSerialLIB   // clear setting for now until included
// #ifndef __SoftwareSerial_h
// #define UseP1SoftSerialLIB
// // #include </home/pafoxp/code-P1meter/SoftwareSerial_2.4.0/SoftwareSerial.h>
// #define UseP1SoftSerialLIB   // old librbary included an indicate this
// #endif  // __SoftwareSerial_h
// #endif  // UseP1SoftSerialLIB

// Note 2.4.0 is to be used , works most of time OK, sometimes errors at higher baudrate
#ifdef libuse240
#undef UseNewSoftSerialLIB      // use the the standard  localised library

#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.0/cores/esp8266/Arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.0/variants/nodemcu/pins_arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.0/cores/esp8266/stdlib_noniso.h>
#endif

// Note 2.4.1  very stable and reliable softserial and now wdt resets
#ifdef libuse241
#undef UseNewSoftSerialLIB      // use the the standard  localised library
// #include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/Arduino.h>
// #include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/variants/nodemcu/pins_arduino.h>
// #include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/stdlib_noniso.h>
#endif

// Note 2.4.2  TBI
#ifdef libuse242
#undef UseNewSoftSerialLIB      // use the the standard  localised library
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.2/cores/esp8266/Arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.2/variants/nodemcu/pins_arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.2/cores/esp8266/stdlib_noniso.h>
#endif


// Note 2.5.0  to be checked
#ifdef libuse250
#undef UseNewSoftSerialLIB      // use the the standard  localised library
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.5.0/cores/esp8266/Arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.5.0/variants/nodemcu/pins_arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.5.0/cores/esp8266/stdlib_noniso.h>
#endif


// Note 2.5.2 is  not to be used, was unstable on softserial
#ifdef libuse252
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/Arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/variants/nodemcu/pins_arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.5.2/cores/esp8266/stdlib_noniso.h>
#endif

// 2.6.0. give strange Git and Software sofserial errors, do not use
#ifdef libuse260
  #ifndef UseP1SoftSerialLIB   // do we use an old library
  #define UseNewSoftSerialLIB   // softwareserial format made equal to hardware version
  #endif
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.6.0/cores/esp8266/Arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.6.0/variants/nodemcu/pins_arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.6.0/cores/esp8266/stdlib_noniso.h>
#endif

#ifdef libuse274              // not very stable, we must use the (old 2.4.1 now) P1 adapted softserial
  #ifndef UseP1SoftSerialLIB   // do we use an old library
  #define UseNewSoftSerialLIB   // softwareserial format made equal to hardware version
  #endif
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/cores/esp8266/Arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/variants/nodemcu/pins_arduino.h>
#include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/cores/esp8266/stdlib_noniso.h>
#endif

#ifndef P1_VERSION_TYPE         // if not defined, fallback to test version
#define P1_VERSION_TYPE "t1"    // "t1" test "p1" production
#endif


// communication mode settings
#ifdef TEST_MODE
bool bSERIAL_INVERT = false;  // P1 meter require - here - inverted serial levels (TRUE) for RS232
#else
bool bSERIAL_INVERT = true;  // P1 meter require - here - inverted serial levels (TRUE) for RS232
#endif

#define bSERIAL2_INVERT false // GJ meter is as far as we  know normal  serial (FALSE) RS232
// #define bSERIAL2_INVERT true // GJ meter is as far as we  know normal (FALSE) RS232


// hardware PIN settings, change accordingly , total esp8266 pins 17
#define BLUE_LED         D0  // pin GPIO16 output to BLUE signal indcator
#define WATERSENSOR_READ D1  // pin GPIO5  input  for watermeter input (require 330mS debouncing)
#define SERIAL_RX2       D2  // pin GPIO14 input  for SoftwareSerial RX  GJ
#define DS18B20_SENSOR   D3  // pin GPIO0 Pin where DS18B20 sensors are mounted (High@Boot)
#define SERIAL_TX2       D4  // pin GPIO2 Secondary TX GJ  passive High@Boot (parallel 3k3 with 270R-opto)
#define SERIAL_RX        D5  // pin GPIO14 input  for SoftwareSerial RX P1 Port
#define LIGHT_READ       D6  // pin GPIO12 input  for LDR read (passive lowR = LOW)
#define THERMOSTAT_READ  D7  // pin GPIO13 input  for button read in (via relay)
#define THERMOSTAT_WRITE D8  // pin GPIO15 output for button write out VALVE (via relay)
#define ANALOG_IN        A0  // pin gpio17 ADC    for Lightlevel LDR 
// D9   = 2;
// D10  = 2;

const int  prog_Version = DEF_PROG_VERSION;  // added ptro 2021 version

#ifndef ARDUINO_ESP8266_RELEASE
  // note: file [/home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/core_version.h] contains 
  #include <core_version.h>
  // see also file: [/home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/core_esp8266_main.cpp]
#endif  
// const const char *core_version_P1Meter_Release = ARDUINO_ESP8266_RELEASE ; 

// Standard includes for Wifi, OTA, ds18b20 temperature
#include <ESP8266WiFi.h>           // standard, from standard Arduino/esp8266 platform
#ifdef P1_Override_Settings      // include our P1_Override_Settings
  #include "P1OverrideSettings.h"   // which contains our privacy/site related setting 
  // #warning Using override settings
#else
  #warning Using default settings, please use override for settings
  #ifdef TEST_MODE                  // Note: we use the override file at compile
    const char *ssid = "Production ssid";    // "Pafo SSID4"    //  T E  S T   setting
    const char *password = "wifipassword";
    const char *hostName = "nodemcu" P1_VERSION_TYPE;          // our hostname for OTA nodemcut1
    const char *mqttServer = "192.168.1.45";                          // our mqtt server adress (desk top test)
    const char *mqttClientName = "nodemcu-" P1_VERSION_TYPE;          // Note our ClientId for mqtt
    const char *mqttTopic = "/T1meter/" P1_VERSION_TYPE;          // mqtt topic s is /T1Meter/p1
    const char *mqttLogTopic = "/log/" P1_VERSION_TYPE;          // mqtt topic is /log/p1
    const char *mqttErrorTopic = "/error/" P1_VERSION_TYPE;          // mqtt topic is /error/p1
    const char *mqttPower = "/T1meter/" P1_VERSION_TYPE "power";  // mqtt topic is /T1meter/P1power
    const int   mqttPort = 1883;
    #ifdef P1_STATIC_IP
      IPAddress local_IP(192, 168, 1, 35);
      IPAddress gateway(192, 168, 1, 1);
      IPAddress subnet(255, 255, 255, 0);
      IPAddress primaryDNS(192, 168, 1, 8);   //optional
      IPAddress secondaryDNS(192, 168, 1, 1); //optional
    #endif
  #else
    const char *ssid = "Test ssid";    // "Pafo SSID5"    //  P R O D U C T I O N  setting
    const char *password = "wifipassword";
    const char *hostName = "nodemcu"        P1_VERSION_TYPE;          // our hostname for OTA nodemcut1
    const char *mqttServer = "192.168.1.254";                         // our mqtt server address
    const char *mqttClientName = "nodemcu-" P1_VERSION_TYPE;          // Note our ClientId
    const char *mqttTopic = "/P1meter/"     P1_VERSION_TYPE;          // mqtt topic is /P1Meter/p1
    const char *mqttLogTopic = "/log/"      P1_VERSION_TYPE;          // mqtt topic is /log/p1
    const char *mqttErrorTopic = "/error/"  P1_VERSION_TYPE;          // mqtt topic is /error/p1
    const char *mqttPower = "/P1meter/"     P1_VERSION_TYPE "power";  // mqtt topic is /P1meter/P1power
    const int   mqttPort = 1883;
    #ifdef P1_STATIC_IP
      IPAddress local_IP(192, 168, 1, 35);
      IPAddress gateway(192, 168, 1, 1);
      IPAddress subnet(255, 255, 255, 0);
      IPAddress primaryDNS(192, 168, 1, 8);   //optional
      IPAddress secondaryDNS(192, 168, 1, 1); //optional
    #endif    
  #endif  // TEST_MODE 
#endif  // P1_Override_Settings

// administrative timers for loop, check to limit  risks for WDT resets
unsigned long currentMillis = 0; // millis() mainloop
unsigned long currentMicros = 0; // micros() mainloop
unsigned long startMicros = 0;   // micros()

// used to research and find position of wdt resets
unsigned long test_WdtTime = 0;   // time the mainloop
unsigned long loopCnt = 0;        // countthe loop
unsigned int  serialLoopCnt = 0;        // countthe loop


// control the informative led within the loop
unsigned long previousBlinkMillis = 0; // used to shortblink the BLUELED, at serialinput this is set to high value
const long    intervalBlinkMillis = 250;

// CRC16/arc redundancy check (data including between / and ! of P1 Telegram )
unsigned int currentCRC = 0;    // add CRC routine to calculate CRC of data
#include "CRC16.h"              // CRC-16/ARC

// start regular program soruce segment
#include <PubSubClient.h>
#include <eagle_soc.h>     // required as we use GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS to preven re-interrupt

#ifndef UseP1SoftSerialLIB   // if not yet included, include standard defined softserial library
#include <SoftwareSerial.h>           // "standard" version , the newer ones  after 2.4.1 are not very reliable.
#else
#include <SoftwareSerial241.h>        // Version 2.4.1. adapted for and to P1 communication which set P1Active() method
#undef UseNewSoftSerialLIB            // softwareserial class format made equal to hardware version
// #include </home/pafoxp/code-P1meter/SoftwareSerial_2.4.0/SoftwareSerial.h>
// #include </home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/libraries/SoftwareSerial_2.4.0/SoftwareSerial240.h>
#endif

/* leave to check test developement
   // #include <SoftwareSerial.h>   // testRX2 usinbg oldversion
    // cannot use the duplicate
    In file included from /home/pafoxp/code-P1meter/T1Meter1.13/T1Meter1.13.ino:213:0:
    /home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.7.4/libraries/SoftwareSerial/src/SoftwareSerial.h:87:7: error: redefinition of 'class SoftwareSerial'
     class SoftwareSerial : public Stream {
           ^
    In file included from /home/pafoxp/code-P1meter/T1Meter1.13/T1Meter1.13.ino:207:0:
    /home/pafoxp/Arduino/libraries/SoftwareSerial-2.4.0/SoftwareSerial240.h:34:7: error: previous definition of 'class SoftwareSerial'
     class SoftwareSerial : public Stream
            ^
*/

// Standard includes for Wifi, OTA, ds18b20 temperature
// #include <ESP8266WiFi.h>           // standard, from standard Arduino/esp8266 platform

// #include <ESP8266mDNS.h>        // standard, ESP8266 Multicast DNS req. ESP8266WiFi AND needed for dynamic OTA
// #include <ESP8266HTTPClient.h>   // not needed
// #include <WiFiUdp.h>             // not needed

#include <ArduinoOTA.h>           // standard, will also automgically include mDNS & UDP

// ================================================================ Temperature processing ===============
// #include <OneWire.h>           // <DallasTemperature.h will ask/include this themselves
#include <DallasTemperature.h>    // DS18B20 temporatur Onewire     
//------------------------------------------------------------------------------------------------ DS18B20
#define ONE_WIRE_BUS DS18B20_SENSOR       // GPIO0 Pin to which is attached a temperature sensor 
#define ONE_WIRE_MAX_DEV 15               // The maximum number of devices on a single wire
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
int numberOfDsb18b20Devices;              // Number of temperature devices found
DeviceAddress devAddr[ONE_WIRE_MAX_DEV];  // An array device temperature sensors
float tempDev[ONE_WIRE_MAX_DEV];          // Saving the last measurement of temperature
float tempDevLast[ONE_WIRE_MAX_DEV];      // Previous temperature measurement
long lastTempReadTime;                    // The last measurement
const int durationTemp = 5000;            // The frequency of temperature measurement
// -----------------------------------------
// int nowTempDs18b201 = 0;               // actual read
// char temperatureString1[6];            // formatted value 99,99-99,99 sensor1

// ==========================================================================================
//Optional add/change THIS SETTING TO 768 IN pubsubclient.h to allow long MQTT messages
//#define MQTT_MAX_PACKET_SIZE 768

// ==========================================================================================
// instructions to interact with the program:
//  pafoxp@ubuntuO380:~$ mosquitto_sub -v -R -h 192.168.1.8 -t "/energy/#
//  pafoxp@ubuntuO380:~$ mosquitto_pub -h 192.168.1.8 -t "nodemcu-t1/switch/port1" -m "0" =Off/1=On/2=Follow/3=NotUsed-Skip
//  port1 commands: i/I make P1 timeout critica, l/L logging, R-estart, P/p-ublish-P1

// array (not used) to facilitate character ruler (if any)
const char *decArray  = "000000000111111111122222222223333333333444444444455555555556666666666777777777788888888888";
const char *numArray  = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
const char *charArray = "abcdefghi1....+....2....+....3....+....4....+....5....+....6....+....7....+....8....+....9";

// Interval is how long we wait for lockup and data was not processed
// add const if this should never change
int intervalP1    = 12000;    // mSecs : P1 30 --> 12 seconds response time interval of meter is 10 seconds
int intervalP1cnt =   360;    // count * : = of maximum 72 minutes (360*12)  will increase at each success until 144minutes
bool forceCheckData = false;  // force check on data in order to output some essential mqtt (Thermostat & Temprature) functions.
bool firstRun = true;         // do not use forceCheckData (which might be activated during setup-() or first loop-() )

// Tracks the time since last event fired between serial reads
unsigned long previousMillis = 0;      // this controls the functional loop
unsigned long previousP1_Millis = 0;   // this controls the inner P1_serial loop
unsigned long previousLoop_Millis = 0; // this tracks the main loop-() < 1000mSec

//for debugging, outputs info to serial port and mqtt
bool allowOtherActivities = true; // allow other activities if not reading serial P1 port
bool p1SerialActive   = false;   // start program with inactive P1 port
bool p1SerialFinish   = false;   // transaction finished (at end of !xxxx )

long p1TriggerDebounce = 1000;   //  1000 mSeconds between yields while tapping water, which may bounce
long p1TriggerTime    = 0;       // initialise for Timestamp when P1 is active with serial communication

int  telegramError    = false;   // indicate the P1 Telegram contains non-printable data ((e=...)
// -------------------------------------------------------------------------------------------------- DEBUG output
#ifdef TEST_MODE
bool outputOnSerial  = true;    // "D" debug default output in Testmode
#else
bool outputOnSerial  = false;   // "D" No debug default output in production
#endif
// bool outputOnSerial  = true;    // "D" debug default output in Testmode
// ---------------------------------------------------------------------------------------------------

bool useWaterTrigger1 = false;   // 'W" Use standard WaterTrigger or (on) WaterTrigger1 ISR routine,
bool useWaterPullUp   = false;   // 'w' Use external (default) or  internal pullup for Wattersensor readpin
bool loopbackRx2Tx2   = RX2TX2LOOPBACK; // 'T' Testloopback RX2 to TX2 (OFF, ON is also WaterState to TX2 port)
bool outputMqttLog    = false;   // "L" ptro false -> true , output to /log/t0
bool outputMqttPower  = true;    // "P" ptro false -> true , output to /energy/t0current

// Vars to store meter readings & statistics
bool mqttP1Published = false;        //flag to cgheck if we have published
long mqttCnt = 0;                    // updated with each publish as of 19nov19
bool validCRCFound = false;          // Set by DdecodeTelegram if Message CRC = valid and send with Mqtt
// P1smartmeter timestate messages
long currentTime   =  210402;        // Meter reading Electrics - datetime  0-0:1.0.0(180611210402S)
char currentTimeS[] = "210401";      // same in String format time, will be overriden by Telegrams
const char dummy1[] = {0x0000};      // prevent overwrite

long powerConsumptionLowTariff = 0;  // Meter reading Electrics - consumption low tariff in watt hours
long powerConsumptionHighTariff = 0; // Meter reading Electrics - consumption high tariff  in watt hours
long powerProductionLowTariff = 0;   // Meter reading Electrics - return low tariff  in watt hours
long powerProductionHighTariff = 0;  // Meter reading Electrics - return high tariff  in watt hours
long CurrentPowerConsumption = 0;    // Meter reading Electrics - Actual consumption in watts
long CurrentPowerProduction = 0;     // Meter reading Electrics - Actual return in watts
long GasConsumption = 0;             // Meter reading Gas in m3

long OldPowerConsumptionLowTariff = 0;  // previous reading
long OldPowerConsumptionHighTariff = 0; // previous reading
long OldPowerProductionLowTariff = 0;   // previous reading
long OldPowerProductionHighTariff = 0;  // previous reading
long OldGasConsumption = 0;

bool thermostatReadState  = LOW;      // assume Input is active THERMOSTAT_READ
bool thermostatWriteState = LOW;      // Set initial command output THERMOSTAT_WRITE
bool lightReadState       = LOW;      // assume Input is active HEATING Light read

void WaterTrigger_ISR(void) ICACHE_RAM_ATTR;  // store the ISR prod routine in cache
void WaterTrigger1_ISR(void) ICACHE_RAM_ATTR; // store the ISR test routine in cache
volatile long waterTriggerCnt   = 0;   // initialize trigger count
long waterDebounceCnt  = 0;   // administrate usage  for report
bool waterReadState    = LOW; // switchsetting
long waterReadDebounce = 200; // 200mS debounce
long waterReadCounter  = 0;   // nothing to read ye  for regular water counter
long waterReadHotCounter  = 0;// nothing to read yet for HotWaterCounter
volatile long waterTriggerTime  = 0;   // initialise for ISR WaterTrigger()
volatile bool waterTriggerState = LOW; // switchsetting during trigger

// used for analog read ADC port
int nowValueAdc = 0;      // actual read
int pastValueAdc = 0;     // previous read
int filteredValueAdc = 0; // average between previous and and published


//Infrastructure stuff
#define MQTTCOMMANDLENGTH 80          // define our receiving buffer
#define MQTTBUFFERLENGTH 480          // define our sending mqtt buffer
#define MAXLINELENGTH 768           // full P1 serial message: the 589 byte message takes about 0.440 Seconds
/*  #define MAXLINELENGTH 768            // shorten to see if this helps
    Executable segment sizes:
    BSS    : 28080 )         - zeroed variables      (global, static) in RAM/HEAP
    IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
    IROM   : 298592          - code in flash         (default or ICACHE_FLASH_ATTR)
    DATA   : 1340  )         - initialized variables (global, static) in RAM/HEAP
    RODATA : 4200  ) / 81920 - constants             (global, static) in RAM/HEAP
    Sketch uses 333148 bytes (31%) of program storage space. Maximum is 1044464 bytes.
    Global variables use 33620 bytes (41%) of dynamic memory, leaving 48300 bytes for local variables. Maximum is 81920 bytes.
*/
/*  #define MAXLINELENGTH 64            // shorten to see if this helps
    Executable segment sizes:
    IROM   : 298592          - code in flash         (default or ICACHE_FLASH_ATTR)
    DATA   : 1340  )         - initialized variables (global, static) in RAM/HEAP
    BSS    : 27504 )         - zeroed variables      (global, static) in RAM/HEAP
    IRAM   : 29016   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...)
    RODATA : 4200  ) / 81920 - constants             (global, static) in RAM/HEAP
    Sketch uses 333148 bytes (31%) of program storage space. Maximum is 1044464 bytes.
    Global variables use 33044 bytes (40%) of dynamic memory, leaving 48876 bytes for local variables. Maximum is 81920 bytes.
*/


// Telegram datarecord and arrays   // corrupted op 5e regel (data#93 e.v.) positie 27: 0-0:96.1.1(453030323730303x3030x4313xx235313x)
char dummy2[17];       // add some spare bytes
char telegram[MAXLINELENGTH];       // telegram maxsize 64bytes for P1 meter
char telegramLast[3];               // used to catch P1meter line termination bracket
bool telegramP1header = false;      // used to trigger/signal Header window /KFM5KAIFA-METER
//DebugCRC int  testTelegramPos  = 0;          // where are we
//DebugCRC char testTelegram[MAXLINELENGTH];   // use to copy over processed telegram // ptro 31mar21

// RX2 buffer on RX/TX Gpio4/2 , use a small buffer as GJ meter read are on request
#define MAXLINELENGTH2 64
char telegram2[MAXLINELENGTH2];     // telegram maxsize 64bytes for P1 meterMAXLINELENGTH
char telegramLast2[3];              // overflow used to catch line termination bracket

// Callback mqtt value which is used to received data
const char* submqtt_topic = "nodemcu-" P1_VERSION_TYPE "/switch/port1";  // to control Port D8, heating relay
int new_ThermostatState = 2;       // 0=Off, 1=On, 2=Follow, 3=NotUsed-Skip
void callbackMqtt(char* topic, byte* payload, unsigned int length);   // pubsubscribe
char mqttReceivedCommand[MQTTCOMMANDLENGTH] = "";      // same in String format time, will be overriden by Telegrams

// Note Software serial uses "attachinterrupt" FALLING on pin to calculate BAUD timing
// class: SoftwareSerial(int receivePin, int transmitPin, bool inverse_logic = false, unsigned int buffSize = 64);
#ifdef UseNewSoftSerialLIB
//  2.5.2+ (untstable): swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
SoftwareSerial mySerial;      // declare our classes for serial1 (P1 115200 8N1 inverted baud
SoftwareSerial mySerial2;     // declare our classes for serial2 (GJ 1200 8N1 baud)
#else
/// @param baud the TX/RX bitrate
/// @param config sets databits, parity, and stop bit count
/// @param rxPin -1 or default: either no RX pin, or keeps the rxPin set in the ctor
/// @param txPin -1 or default: either no TX pin (onewire), or keeps the txPin set in the ctor
/// @param invert true: uses invert line level logic
/// @param bufCapacity the capacity for the received bytes buffer
/// @param isrBufCapacity 0: derived from bufCapacity (used for/with asynchronous)
// 274 rubbish // SoftwareSerial mySerial(SERIAL_RX, -1, true, MAXLINELENGTH); // (RX, TX. inverted, buffer)

SoftwareSerial mySerial(SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH); // (RX, TX. inverted, buffer)
SoftwareSerial mySerial2(SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2); // (RX, TX, noninverted, buffer)
#endif

// Wifi
WiFiClient espClient;             // declare and name our Internet conenction
PubSubClient client(espClient);   // Use this connection client


/* Setup
  - allow/include printf support
  - define initialise gpio pins
  - prepare serial connections
  - start wifi
  - handle/setup OTA
  - handle/callback Mqtt conenction commands (topic nodemcu-...)
  - initialise temperature sensors DS18B20
  - (TESTMODE) do timing calculations to research exact timing of routines
  - attach waterpulse interrupt
*/



void setup()
{
  asm(".global _printf_float");            // include floating point support
  pinMode(BLUE_LED, OUTPUT);               // Declare Pin mode Builtin LED Blue (nodemcu-E12: GPIO16)
  // pinMode(THERMOSTAT_READ, INPUT);      // Declare Pin mode INPUT read always low
  pinMode(THERMOSTAT_READ, INPUT_PULLUP);  // Declare Pin mode INPUT with pullup to Read room thermostate
  pinMode(LIGHT_READ, INPUT_PULLUP);       // Declare Pin mode INPUT with pullup to read HotWater valve

  pinMode(WATERSENSOR_READ, INPUT_PULLUP); // Woggle the Input pin to read Waterpulsed infrared
  pinMode(WATERSENSOR_READ, INPUT);        // Declare Pin mode INPUT with no pullup

  pinMode(THERMOSTAT_WRITE, OUTPUT);       // Declare Pin mode OUTPUT to control Heat-Valve

  digitalWrite(BLUE_LED, LOW);             //Turn the LED ON  (active-low)

  // Thermostats initialisation
  thermostatReadState   = digitalRead(THERMOSTAT_READ); // read current room state
  thermostatWriteState  = thermostatReadState;          // move this to output 
  digitalWrite(THERMOSTAT_WRITE, thermostatWriteState); // ThermostatWriteout valve (TBD: only when required)

  // Lightread initialisation
  lightReadState   = digitalRead(LIGHT_READ); // read Hotwater valve state

  // prepare wdt
  ESP.wdtDisable();
  // ESP.wdtEnable(WDTO_8S); // 8 seconds 0/15/30/60/120/250/500MS 1/2/4/8S
  ESP.wdtEnable(22000); // allow two passses missing

  Serial.begin(115200);
  Serial.println("Booting");              // message to serial log

  /*//DebugCRC print a test mesaage tov erify if CRC is functional, Yes it is
    // check display CRC ode to determine the type to use
    char testCRCmessage[10]="123456789";
    currentCRC=CRC16(0x0000,(unsigned char *) testCRCmessage, 9);
    Serial.println((String) "");
    Serial.printf("\ntestCRC 1234567890=%x \n",currentCRC);
    see []
    "A"=0x30C0  , "B"=0x3180 , "123456789"=0xBB3D (0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38  0x39 )
    CRC-16/ARC   0xBB3D  0xBB3D  0x8005  0x0000  true  true  0x0000
    */

  #ifdef P1_STATIC_IP
    // Configures static IP address
    // WiFi.config(IPAddress(192,168,1,125), IPAddress(192,168,1,1), IPAddress(255,255,255,0));    
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.print((String)"STA Failed to configure: ");
    } else {
      Serial.print((String)"STA Using fixed adresses: ");
    }
    Serial.println(local_IP );
  #endif

  Serial.println("Settingup WifiSTAtion.");  // wait 5 seconds before retry
  WiFi.mode(WIFI_STA);            // Client mode

  Serial.println((String) "Connecting to " + ssid);  // wait 5 seconds before retry
  WiFi.begin(ssid, password);     // login to AP
    
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.print((String)"Connection to "+ ssid +", Failing ..");  // wait 5 seconds before retry
    for (int i = 0; i < 9; i++) {                       // while flashing led
      bool blueLedState = digitalRead(BLUE_LED);
      digitalWrite(BLUE_LED, !blueLedState);
      delay(250);
      digitalWrite(BLUE_LED, blueLedState);
      delay(250);
    }
    Serial.print((String) "Connection to "+ ssid +", Faild, restartting in 5 seconds..");  // wait 5 seconds before retry
    delay(2000);
    ESP.restart();
  }


  /*
    #ifdef UseNewSoftSerialLIB
      // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
      mySerial.begin  (115200,SWSERIAL_8N1,SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH,0); // Note: Prod use require invert
      mySerial2.begin (  1200,SWSERIAL_8N1,SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2,0);
      // newserial: mySerial.begin(115200, SWSERIAL_8N1, SERIAL_RX, -1, true , MAXLINELENGTH,0);  // define softserial  GPIO14
      // Serial.swap();       // use the alternative serial pins
      // Serial.begin(115200);   // use use UART0 (TX = GPIO1, RX = GPIO3) hardware
    #else
      mySerial.begin(115200);    // P1 meter port 115200 baud
      mySerial2.begin( 1200);   // GJ meter port   1200 baud
    #endif
  */

  ArduinoOTA.setHostname(hostName);   // nodemcut1/p1

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  
  
//    #define P1_VERSION_TYPE "t1"      // "t1" for ident nodemcu-xx and other identification to seperate from production
  // #define DEF_PROG_VERSION 1123.240
  ArduinoOTA.begin();
  Serial.println("Ready version: "+ (String)P1_VERSION_TYPE + "-" + (String)DEF_PROG_VERSION+ "." );
  Serial.println ((String)"\nArduino esp8266 core: "+ ARDUINO_ESP8266_RELEASE);  // from <core.version>
  // DNO:  Serial.println ((String)"LWIP_VERSION_MAJOR: "+ LWIP_VERSION_MAJOR);
  Serial.print   ("IP address: ");
    Serial.println(WiFi.localIP());
  Serial.println ("ESP8266-ResetReason: "+  String(ESP.getResetReason()));
  Serial.println ("ESP8266-free-space: "+   String(ESP.getFreeSketchSpace()));
  Serial.println ("ESP8266-sketch-size: "+  String(ESP.getSketchSize()));
  Serial.println ("ESP8266-sketch-md5: "+   String(ESP.getSketchMD5()));
  Serial.println ("ESP8266-chip-size: "+    String(ESP.getFlashChipRealSize()));
  Serial.println ("ESP8266-sdk-version: "+  String(ESP.getSdkVersion()));
  Serial.println ("ESP8266-getChipId: "+    ESP.getChipId()); 

  WiFi.printDiag(Serial);   // print data  
  client.setServer(mqttServer, mqttPort);

  digitalWrite(BLUE_LED, LOW);   //Turn the LED ON and ready to go for process
  mqttP1Published = false; //flag to track if we have published....

  // client.setServer(mqtt_server,1883);   // for pubsubscribe // already set in mqttPort
  // client.subscribe(submqtt_topic);       // put pubsubscribe in reconnect section
  memset(mqttReceivedCommand, 0, sizeof(mqttReceivedCommand));  // initialise
  client.setCallback(callbackMqtt);             // for pubsubscribe

  // 1531562100: New connection from 192.168.1.35 on port 1883.
  // 1531562100: New client connected from 192.168.1.35 as nodemcu-t1 (c1, k15).
  // 1531562100: No will message specified.
  // 1531562100: Sending CONNACK to nodemcu-t1 (0, 0)
  // 1531562100: Received PUBLISH from nodemcu-t1 (d0, q0, r0, m0, '/energy/t1', ... (178 bytes))

  //Setup DS18b20 temperature sensor
  SetupDS18B20();

  Serial.print("ESP P1 Monitor interval "); // report initiali state
  Serial.print(intervalP1cnt );
  Serial.print(" of ");
  Serial.print(intervalP1 );
  Serial.print(" mSecs,");

#ifdef TEST_PRINTF_FLOAT
  // Test float functions
  Serial.printf ("Test FloatingPoint support\n", mqttCnt);
  // asm(".global _printf_float"); // setin in Setup()
  Serial.printf ("\tmqttCnt Value4.1f = %4.1f\n", mqttCnt);
  Serial.printf ("\tmqttCnt ValueUu = %u\n", mqttCnt);
  Serial.printf ("\t9.0453 Value4.1f = %4.1f\n", 9.0453);   
  Serial.printf ("\t9.12 Value4.1f = %4.1f\n", 9.0); 
  Serial.printf ("\t9.12340 Value5.1f = %5.2f\n", 9.0); 
  Serial.printf ("\t 9.0 Value4.1f = %6.1f\n", 9.0);
  Serial.printf ("\t1234.567 Value6.1f = %6.6f\n", 1234.567);
#endif

#ifdef TEST_CALCULATE_TIMINGS
  Serial.print((String)"m-bitime 115K2=" + (ESP.getCpuFreqMHz() * 1000000 / 115200)); // = 694 cycles for 115K2@80Mhz

  // insert a small loop to stabilize
  for (int i = 0; i < 1000; i++) {
    int test = 0;
  }

  // #define WAIT { while (ESP.getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; }
  // Checktiming for test only
  unsigned long m_bitTime = ESP.getCpuFreqMHz() * 1000000 / 115200;
  unsigned long wait = m_bitTime + m_bitTime / 3 - 500;  // 425 115k2@80MHz
  unsigned long start = ESP.getCycleCount();

  unsigned long startMicro1 = micros();
  unsigned long startCycle1 = ESP.getCycleCount();
  unsigned long   endCycle1 = ESP.getCycleCount();
  unsigned long   endMicro1 = micros();
  unsigned long  diffCycle1 =  endCycle1 - startCycle1;
  unsigned long  diffMicro1 =  endMicro1 - startMicro1;

  for (int i = 0; i < 1000; i++) {
    int test = 0;
  }

  // check-test native ISR routine
  cli();   // hold interrupts
  unsigned long startMicro2 = micros();
  unsigned long startCycle2 = ESP.getCycleCount();
  unsigned long wait2 = m_bitTime + m_bitTime / 3 - 500; // 425 115k2@80MHz
  // unsigned long wait = 425; // harcoded
  unsigned long start2 = ESP.getCycleCount();
  uint8_t rec2 = 0;
  unsigned long   endCycle2 = ESP.getCycleCount();
  unsigned long   endMicro2 = micros();
  unsigned long  diffCycle2 =  endCycle2 - startCycle2 ;
  unsigned long  diffMicro2 =  endMicro2 - startMicro2 ;
  cli();   // resume interrupts
  for (int i = 0; i < 1000; i++) {
    int test = 0;
  }

  // test  timing for logic test only
  bool t_P1active1;
  uint8_t t_rec1 = 0;
  unsigned long startMicro3 = micros();
  unsigned long startCycle3 = ESP.getCycleCount();
  if (t_rec1 == '/') t_P1active1 = true ;   // 26mar21 Ptro P1 messageing has started by header
  if (t_rec1 == '!') t_P1active1 = false ; // 26mar21 Ptro P1 messageing has ended due valid trailer
  unsigned long   endCycle3 = ESP.getCycleCount();
  unsigned long   endMicro3 = micros();
  unsigned long  diffCycle3 =  endCycle3 - startCycle3;
  unsigned long  diffMicro3 =  endMicro3 - startMicro3;

  for (int i = 0; i < 1000; i++) {
    int test = 0;
  }

  // test  timing for printing a byte
  unsigned long startMicro4 = micros();
  unsigned long startCycle4 = ESP.getCycleCount();
  Serial.print(".");
  unsigned long   endCycle4 = ESP.getCycleCount();
  unsigned long   endMicro4 = micros();
  unsigned long  diffCycle4 =  endCycle4 - startCycle4;
  unsigned long  diffMicro4 =  endMicro4 - startMicro4;

  for (int i = 0; i < 1000; i++) {
    int test = 0;
  }

  // check timing for testing P1 Header/Trailer and set State active
  bool t_P1active5;
  uint8_t t_rec5 = '/';
  unsigned long startMicro5 = micros();
  unsigned long startCycle5 = ESP.getCycleCount();
  if (t_rec5 == '/') t_P1active5 = true ;   // 26mar21 Ptro P1 messageing has started by header
  if (t_rec5 == '/') t_P1active5 = false ; // 26mar21 Ptro P1 messageing has ended due valid trailer
  /* // test for loop duration 1 GetCycle 1-2 µSec
    for (int i=0; i < 1000; i++) {
        int test=0;
    }
  */
  unsigned long   endCycle5 = ESP.getCycleCount();
  unsigned long   endMicro5 = micros();
  unsigned long  diffCycle5 =  endCycle5 - startCycle5;
  unsigned long  diffMicro5 =  endMicro5 - startMicro5;

  for (int i = 0; i < 1000; i++) {
    int test = 0;
  }

  Serial.println(" Timing for native as reference ");
  Serial.print((String) "startMicro1=" + startMicro1 +
               ", startCycle1=" + startCycle1 +
               ", endCycle1=" +   endCycle1 +
               ", endMicro1=" +   endMicro1 +
               ", diffCycle1=" +  diffCycle1 +
               ", diffMicro1=" +  diffMicro1 +
               ".\r\n");
  Serial.println("EOI.");

  Serial.println(" Timing for ISR Softserial initiation ");
  Serial.print((String) "startMicro2=" + startMicro2 +
               ", startCycle2=" + startCycle2 +
               ", endCycle2=" +   endCycle2 +
               ", endMicro2=" +   endMicro2 +
               ", diffCycle2=" +  diffCycle2 +
               ", diffMicro2=" +  diffMicro2 +
               ", wait (cycles)=" +       wait2 +
               ".\r\n");
  Serial.println("EOI.");

  Serial.println(" Timing instructions for P1 test hdr/trl in ISR Softserial");
  Serial.print((String) "startMicro3=" + startMicro3 +
               ", startCycle3=" + startCycle3 +
               ", endCycle3=" +   endCycle3 +
               ", endMicro3=" +   endMicro3 +
               ", diffCycle3=" +  diffCycle3 +
               ", diffMicro3=" +  diffMicro3 +
               ".\r\n");
  Serial.println("EOI3");

  Serial.println(" Timing instructions for Print a single byte");
  Serial.print((String) "startMicro4=" + startMicro4 +
               ", startCycle4=" + startCycle4 +
               ", endCycle4=" +   endCycle4 +
               ", endMicro4=" +   endMicro4 +
               ", diffCycle4=" +  diffCycle4 +
               ", diffMicro4=" +  diffMicro4 +
               ".\r\n");
  Serial.println("EOI4");

  Serial.println(" Timing instructions for P1 set hdr&trl in ISR Softserial");
  Serial.print((String) "startMicro5=" + startMicro5 +
               ", startCycle5=" + startCycle5 +
               ", endCycle5=" +   endCycle5 +
               ", endMicro5=" +   endMicro5 +
               ", diffCycle5=" +  diffCycle5 +
               ", diffMicro5=" +  diffMicro5 +
               ".\r\n");
  Serial.println("EOI5\n");

  Serial.println((String)"P1 ItE logic: " +
                 (diffCycle3 - diffCycle1) + "Cycles ," +
                 (diffMicro3 - diffMicro1) + "uSecs " +
                 ".");

  Serial.println((String)"Test for P1 Hdr/Trl: " +
                 (diffCycle5 - diffCycle3) + "Cycles ," +
                 (diffMicro5 - diffMicro3) + "uSecs " +
                 ".");

  Serial.println((String)"Test & Set P1active: " +
                 (diffCycle5 - diffCycle1) + "Cycles ," +
                 (diffMicro5 - diffMicro1) + "uSecs " +
                 ".\n");

#endif  // calculate timings

  // intervalP1cnt =  10;        // initialise timeout count
  // intervalP1  = 30000;        // 30.000 mSec
  previousMillis    = millis();           // initialise previous interval
  previousP1_Millis = previousMillis;  // initialise previous interval
  // attachInterrupt(WATERSENSOR_READ, WaterTrigger_ISR, CHANGE); // trigger at every change
  attachWaterInterrupt();

  waterTriggerTime = 0;  // ensure and assum no trigger yet
  test_WdtTime = 0;  // set first loop timer
  loopCnt = 0;              // set loopcount to 0

//  WiFi.printDiag(Serial);   // print data
}

/* 
  Mainloop:
    - supervise timers to prevent wdt reset
    - manage serialconnection during P1 intervals
    - handle/decode P1 readings
    - monitor water pulses (, specifically wheel debounce)
    - process secondary serial
    - prevent WDT reset (mqtt_local_yield)
    - watchdog/force for timeouts (P1 serial too long not connected)
    - Handle OTA updates

*/

//  WiFi.printDiag(Serial) 
// Serial.printDiag(Wifi);

// WiFi.printDiag(Serial);
 
void loop()
{
   // declare global timers loop(s)
  currentMillis = millis(); // Get snapshot of runtime
  currentMicros = micros(); // get current cycle time
  unsigned long debounce_time = currentMicros - waterTriggerTime;    // µSec - waterTriggerTime in micro seconds set by ISR waterTrigger

  // if (test_WdtTime < currentMillis and !outputOnSerial )  {  // print progress 
  if (test_WdtTime < currentMillis ) {
    loopCnt++ ;
    Serial.print((String)"-"+(loopCnt%99)+" \b");
    test_WdtTime = currentMillis + 1000;  // next interval
    if (serialLoopCnt >= 0 && serialLoopCnt <= 5 ) serialLoopCnt++;
  }

  // Following will trackdown loops
  if (currentMillis > previousLoop_Millis + 1000) { // exceeding one second  ?, warn user
    Serial.printf("\r\nLoop %6.3f exceeded at prev %6.3f !!yielding\n", ((float) currentMicros / 1000000), ((float) previousLoop_Millis / 1000));
    // yield();
    mqtt_local_yield();  // do a local yield with 50mS delay that also feeds Pubsubclient to prevent wdt
  }
  previousLoop_Millis = currentMillis;  // registrate our loop time

  // note do not use wifi.connected test in loop... it is not needed nor reuired per
  // https://stackoverflow.com/questions/49205073/soft-wdt-reset-esp8266-nodemcu
  if (!client.connected())     // this handles the mosquitto connection
  {
    reconnect();               // reconnect Mqtt
    mqttP1Published = false;   // flag to check if we have published procssed data
    delay(1000);               // delay processing to prevent overflow on error messages
    Serial.println("\n #!!# ESP P1 reconnected."); // print message line
    // WiFi.printDiag(Serial);   // print data ESP8266WiFiClass::printDiag(Print& p)
  }

  mqtt_local_yield();      //   client.loop(); // handle mqtt

  if (allowOtherActivities) {     // are we outside P1 Telegram processing (which require serial time resources)

    if (!p1SerialActive) {      // P1 (was) not yet active, start primary softserial to wait for P1
      if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + " serial started at " + currentMillis);
      serialLoopCnt = 0;
      p1SerialActive = !p1SerialActive ; // indicate we have started
      p1SerialFinish = false; // and let transaction finish
      mySerial2.end();          // Stop- if any - GJ communication
      mySerial2.flush();        // Clear GJ buffer
      telegramError = 0;        // start with no errors

      // Start secondary serial connection if not yet active
#ifdef UseNewSoftSerialLIB
      // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
      mySerial.begin  (115200, SWSERIAL_8N1, SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH, 0); // Note: Prod use require invert
      // mySerial2.begin (  1200,SWSERIAL_8N1,SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2,0);
#else
      mySerial.begin(115200);    // P1 meter port 115200 baud
      mySerial2.begin( 1200);    // GJ meter port   1200 baud     // required during test without P1
#endif

    } else {

      if (p1SerialFinish) {     // P1 transaction completed, we can start GJ serial operation at Serial2
        if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + " serial stopped at " + currentMillis);
        // if (!outputOnSerial) Serial.print((String) "\t stopped:" + micros() + " ("+ (micros()-currentMicros) +")" + "\t");
        if (!outputOnSerial) Serial.printf("\t stopped: %6.6f (%4.0f)__\b\b\t", ((float)micros() / 1000000), ((float)micros() - startMicros));
        serialLoopCnt = 10; // debug end printing progress during readTelegram
        p1SerialFinish = !p1SerialFinish;   // reverse this
        p1SerialActive = true;  // ensure next loop sertial remains off
        mySerial.end();    // P1 meter port deactivated
        mySerial.flush();  // Clear P1 buffer

#ifdef UseNewSoftSerialLIB
        // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
        // mySerial.begin  (115200,SWSERIAL_8N1,SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH,0); // Note: Prod use require invert
        mySerial2.begin (  1200, SWSERIAL_8N1, SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2, 0);
#else
        // mySerial.begin(115200);    // P1 meter port 115200 baud
        mySerial2.begin( 1200);    // GJ meter port   1200 baud
#endif
        previousP1_Millis = currentMillis;  // indicate time we have stopped.
        previousMillis    = currentMillis;  // initialise our start counter for the overall proces.
      } else {

        // Serial are closed and not yet (re)started
        // check timer to set p1SerialActive to false in order to commence P1 data
        if ( (unsigned long)(currentMillis - previousP1_Millis) >= (intervalP1 / 2) && !p1SerialFinish) {    // terminate serial2 and restart serial1 halfway
          p1SerialActive = false;   // activate serial in next round
          p1SerialFinish = false;  // and let it finish
          previousP1_Millis = currentMillis; // Use the snapshot to set track time to prepare next P1 serial event

        } // check  initiate serial read
      } // else p1SerialFinish
    } // else !!p1SerialActive

    ESP.wdtFeed();  // as advised by forums

    // Check for WaterPulse trigger
    if ( (waterTriggerCnt == 1) && (debounce_time > waterReadDebounce * 100)) {
      // we are in hold read interrupt mode , re-establish interrupts
      waterDebounceCnt++;                // administrate usage  for report
      attachWaterInterrupt();
      waterTriggerTime = currentMicros;  // reset our trigger counter
      waterTriggerCnt  = 2;              // Leave at 2 so the ISR routine can increase it to 3
    } else
      // check if we must add a pulse counter
      if ( (waterTriggerTime != 0) && (debounce_time > waterReadDebounce * 1000) ) { // debounce_time (mSec
        waterTriggerTime = 0;                        // reset counter for next trigger
        // waterTriggerCnt  = 0;                     // reset ISR counter
        if (waterReadState != waterTriggerState) {   // switchsetting) {  // read switch state
          waterReadState = waterTriggerState ;       // stabilize switch
          if (!waterReadState) waterReadCounter++;   // count this as pulse if switch went LOW
          if (!waterReadState && !lightReadState) waterReadHotCounter++; // count this as pulse if switch went LOW
          // accomodate/enforce stability
          if ( waterReadState) pinMode(WATERSENSOR_READ, INPUT_PULLUP); // Improve voltage our external pull-up
          if (!waterReadState) pinMode(WATERSENSOR_READ, INPUT);        // Weaken our external pull-up
        } // if waterReadState
      } // else waterTriggerCnt = 1

      if (mqttReceivedCommand[0] != '\x00') {
          ProcessMqttCommand(mqttReceivedCommand,2); // process incoming Mqtt command (if any)
          memset(mqttReceivedCommand,0,sizeof(mqttReceivedCommand));
      }

    // if (loopbackRx2Tx2) Serial.print("Rx2 "); // print message line
    readTelegram2();    // read GJ gpio4-input (if available), write gpio2 output

  }   // if allow other activities

  // readTelegram2();    // read GJ gpio4-input (if available), write gpio2 output (now done in closed setting)

  // (p1SerialActive && !p1SerialFinish) , process Telegram data
  readTelegram();     // read P1 gpio14 (if serial data available)
 
  mqtt_local_yield();  // do a local yield with 50mS delay that also feeds Pubsubclient to prevent wdt
  ESP.wdtFeed();  // as advised by forums

  p1TriggerTime = millis();   // indicate our mainloop yield

  // check and prevail deadlock timeout
  if   ((unsigned long)(currentMillis - previousMillis) >= intervalP1) {  // is our deadlock check interval passed  12secs ?
    //  if (millis() > 600000) {  // autonomous restart every 5 minutes
    intervalP1cnt--;            // decrease reliability
    if ( intervalP1cnt < 1) {   // if we multiple or frequent misses, restart esp266 to survive
      if (outputMqttLog && client.connected()) client.publish(mqttLogTopic, "ESP P1 not connected" );  // report we have survived this interval
      Serial.print("\n #!!# ESP P1 not connected, cnt="); // print message line
      Serial.println( intervalP1cnt );
      mqttP1Published = false;
    } else {

      // report to error mqtt, //V20 candidate for a callable error routine
      char mqttOutput[128];
      String mqttMsg = "{";  // start of Json
        mqttMsg.concat("\"error\":001 ,\"msg\":\"P1 serial not connected\""); 
        // String mqttMsg = "Error, "; // build mqtt frame 
        // mqttMsg.concat("serial not connected ");  
        // mqttMsg,concat("interval="); // build mqtt frame 
        // char mqttCntS[8] = ""; ltoa(mqttCnt,mqttCntS,10); mqttMsg.concat(mqttCntS));
        mqttMsg.concat((String)", \"mqttCnt\":"+(mqttCnt+1));    // +1 to reflect the actual mqtt message
      mqttMsg.concat("}");  // end of json
      mqttMsg.toCharArray(mqttOutput, 128);
      if (client.connected()) {
        client.publish(mqttErrorTopic, mqttOutput); // report to error topic
        if (outputMqttLog) client.publish(mqttLogTopic, "ESP P1 Active interval checking" );  // report we have survived this interval
      }

      // Alway print this serious timeout failure
      // if (outputOnSerial) {
      Serial.print("\n#!!# ESP P1 Active interval checking ");
      Serial.print(intervalP1cnt );
      Serial.print(", timeP:");
      Serial.print(previousMillis);
      Serial.print(", time2:");
      Serial.print(currentMillis); // print message line
      Serial.println("."); // print message line
      mySerial2.println((String)"t=" + currentMillis + " ." );  // testoutput
      // }

      forceCheckData = true;  // enforce mqtt publish
      validCRCFound  = false; // Enforce we (might) have a CRC error
      processGpio();          // Do regular functions of the system
      publishP1ToMqtt();      // PUBLISH this mqtt
      forceCheckData = false;

      mqttP1Published = true;
    } // else millis exceeded intervalP1 hile enforce mqtt for tempsensors

    // If no MQTT poublished in this timout interval, whatever the reason, execute a restart to reset
    if (!mqttP1Published) {  // in case no publish energy yet , try restart
      Serial.println("ESP timeout.restart"); // print message line
      if (client.connected()) client.publish(mqttLogTopic, "ESP timeout.restart" );
      ESP.restart();     // if no/yet data published force restart
    }

    // if (outputMqttLog) client.publish(mqttLogTopic, "ESP P1 Active interval checking" );  // report we have survived this interval
    previousMillis = currentMillis; // Use the snapshot to set track time until next loop event
    mqttP1Published = false;        // assume we have not yet published procssed data
  }

  ArduinoOTA.handle();             // check if we must service on the Air update

}

/* 
  Reconnect lost mqtt connection
*/
void reconnect() {                 // mqtt read usage doc https://pubsubclient.knolleary.net/api
  // Loop until we're reconnected
  int mqttConnectDelay    = 2000;     // start reconnect and ease things increasingly to 30000 if this continues to fail
  while (!client.connected())   {
    Serial.print((String)"Attempt MQTT connection to " + mqttClientName + " ..." );
    if (client.connect(mqttClientName))     {
      Serial.print((String)"(re)connected to " + mqttServer);
      // Serial.print(client.state());
    } else {
      Serial.print((String)"failed to " + mqttServer);
      // Serial.print(" try again in 5 seconds...");
      // Wait 2 seconds before retrying
      // delay(2000);
      // delay(mqttConnectDelay);    // delay next try

      delay(1000);                                         // initial delay
      for (int i = 0; i < mqttConnectDelay; i = i + 1000) { // mSecs conenction delay before doing critical function
        for (int b = 0; b < 4; b++) {                    // delay 1 second and blink led fast
          digitalWrite(BLUE_LED, !digitalRead(BLUE_LED));  // invert BLUE LED as warning
          delay(250);
        }
        doCritical();  // do critical process to maintain basic Thermostat  & OTA functions
      }
      if (mqttConnectDelay <= 27000) {
        mqttConnectDelay = mqttConnectDelay + 3000; // increase retry
        Serial.print((String) "... try again in " + (mqttConnectDelay / 1000) + " seconds...\r\n");
      }
    }
    Serial.print((String)", connect-rc=" + client.state() + " \r\n");
  }
  client.subscribe(submqtt_topic);
  Serial.print((String)"; subscribed to >" + submqtt_topic) ;
  // Serial.print(submqtt_topic);
  Serial.println("< .");
}

/* 
    Do the critical core functions: reading thermostat and setting the heating
    We must ensure the thermostater is supervised, even if mqtt or network is not connected.
*/
void doCritical() {
  // takes 18mS without sensors if printing in processGpio() is left on, without its 2-4mS
  // with sensors this Gpio process took -78 mS
  bool save_outputOnSerial = outputOnSerial;
  outputOnSerial = false;                  // disable frequent prints
  // unsigned long startMillis = millis(); // not used
  ArduinoOTA.handle();  // check if we must service on the Air update
  processGpio();        // Do regular functions of the system
  // signed long durationMillis =  startMillis - millis();
  outputOnSerial = save_outputOnSerial;    // re-establish serial print
  // Serial.print((String)"Gpio process took " + durationMillis + " mS.\r\n");
}


/* 
  Process  P1 serial data buffer and sendout result at P1 trailer
*/
void readTelegram() {

#ifdef TEST_MODE
    // Serial.print("+");
    // Serial.print(serialLoopCnt);
    // if (serialLoopCnt >= 3 && serialLoopCnt <= 4) Serial.print("+");
    // if (serialLoopCnt == 3) Serial.print(".");
    if (serialLoopCnt == 4) Serial.print("+");  //. after the fourth count, things goes intermittently to wdt
    if (serialLoopCnt == 5) Serial.print("#");
#endif


  unsigned long p1Debounce_time = millis() - p1TriggerTime; // mSec - waterTriggerTime in micro seconds set by ISR waterTrigger
  // if (p1SerialActive && p1Debounce_time > p1TriggerDebounce ) { // debounce_time (mSec
  if (p1Debounce_time > p1TriggerDebounce ) { // debounce_time (mSec
    // yield();                // allow time to others
    mqtt_local_yield();
    if (outputOnSerial) {   // indicate on console
      Serial.print((String) " !!>" + millis() + " yield due " + p1TriggerTime + "< exceeded !!" ); // length processed previous line
    }
    p1TriggerTime = millis();
  }


  if (!mySerial.available()) return ;  // quick return if no data

#ifdef UseP1SoftSerialLIB              // Note this serial version will P1Active while reading between / and !
  if ( mySerial.P1active()) {
    Serial.print("XxX");
    return ;  // quick return if serial is receiving, fucntion of SoftwareSerial for P1
  }
#endif  

  if (outputOnSerial)    {
    if ( !telegramP1header) Serial.print((String) P1_VERSION_TYPE + " DataRx started at " + millis() + "s=s123..\b\b\b\b\b") ; // print message line
    // if (  telegramP1header) Serial.print("\n\rxP= "); // print message line if message is broken in buffer
  }
  startMicros = micros();  // Exact time we started
  // if (!outputOnSerial) Serial.print((String) "\rDataCnt "+ (mqttCnt+1) +" started at " + micros());
  if (!outputOnSerial) Serial.printf("\r Cycle: %2.9f DataCnt: %u started at %6.6f \b\b\t", ((float)ESP.getCycleCount()/80000000), (mqttCnt + 1), ((float) startMicros / 1000000));

  if (outputMqttLog && client.connected()) client.publish(mqttLogTopic, mqttClientName );

  int lenTelegram = 0;
  memset(telegram, 0, sizeof(telegram));   // initialitelegram buffer-array to 0
  memset(telegramLast, 0, sizeof(telegramLast));     // initialise 3 byte array to 0

  //DebugCRC memset(testTelegram, 0, sizeof(testTelegram));   // initialise 640byte telegram array to 0 to test/check single array

  // Serial.println("Debug: startbuf..");

  while (mySerial.available())     {      // If serial data in buffer
    // Serial.print((String) "xa\b"); no need to display serial available this goes ok
    
    /*
        if (outputOnSerial) {         // do we want/need to print the Telegram
          Serial.print("\nDataP1>");
        }
    */
    // https://www.arduino.cc/reference/en/language/functions/communication/stream/streamreadbytesuntil/
    // note Serial.setTimeout() sets the maximum and defaults to 1000 milliseconds.
    // The function returns the characters up to the last character before the supplied terminator.

    int len = mySerial.readBytesUntil('\n', telegram, MAXLINELENGTH - 2); // read a max of  64bytes-2 per line, termination is not supplied
    // Serial.print((String) "yb\b"); no need to display RX progess this goes ok
    

    if (len > 1) {      // correct dual Carrigage returns, Windows formatted testdata on Linux
      // decrease len as the dual 0x0d 0x0d is from streamed testdata
      if (telegram[len-2] == '\x0d' && telegram[len-1] == '\x0d' ) len-- ;  
    }
    // debug this
    // if (len > 1)  Serial.print((String)" La="+ (int)telegram[len-2]);
    // if (len > 0)  Serial.print((String)" Lb="+ (int)telegram[len-1]);
    // if (len == 0) Serial.print((String)" Lc="+ (int)telegram[len]);
    
    if (outputOnSerial) {         // do we want/need to print the Telegram
      Serial.print((String)"\nlT=" + (len-1) + " \t[");
      for (int cnt = 0; cnt < len; cnt++) {
          
        if (telegram[cnt] == '\x0D') {
            Serial.print("_");
        } else {
            // Serial.print(telegram[cnt - 1]);
            Serial.print(telegram[cnt]);
        }
          // Serial.printf("%02x",telegram[cnt-1]);   // hexadecimal formatted
      }
      Serial.print((String)"]");
    }

    /* //debugCRC
            yield();        // ptro added 31mar21 tr/to prevent wdt reset
            // print hexadecimal of this read.
             Serial.printf("readbytesuntil=%u :", len);
             for (int cnt = 0; cnt < len; cnt++) {
                 Serial.printf("%02x",telegram[cnt-1]);
             }
             Serial.println(". ");
    */
    // len == 0 ? lenTelegram = -1 : lenTelegram += len;   // if len = 0 indicate an error report

    //debugCRC if ( len == 0 && outputOnSerial) Serial.print(" <0> "); // debug print processing serial data
    //debugCRC if (outputOnSerial) Serial.printf("{%02X %02X %02X}",telegram[len-2],telegram[len-1],telegram[len]);

    lenTelegram = lenTelegram + (len - 1); // accumulate actual number of characters (excluding cr/lf)
    if ( len == 0) lenTelegram = -1; // if len = 0 indicate for report


    //DebugCRC int lenSave = len;                     // the the original value
    /* // debugCRC
       if (outputOnSerial) Serial.print(" <"); // debug print processing serial data
       if (outputOnSerial) Serial.print(len) ; // debug print processing serial data
       if (outputOnSerial) Serial.print(">"); // debug print processing serial data
    */

    if (len > 0 )  {  // len=0 indicates serial error, a length of 1 means the zerobyte (0x00-0d-0a) string.

      /* //debugCRC
               // Extract last character into a printable string to  //debugCRC
               telegramLast[0] = telegram[len - 2];  // 3byte-telegram readible lastbyte  bracket-2 null-1 cr-0
               telegramLast[1] = '\n';               // 3byte telegram lastbyte termination string
               telegramLast[2] = 0;                  // 3vyte-telegram lastbyte terminaion variable

               // Make Telegram printable , Note this may could overwrite the running buffer
               telegram[len + 1] = '\n';    // string set lastcharacter to \n
               telegram[len + 2] = 0;       // variable next character before null

               // sprintf("%02X", theNumber); // take alot of memory
               if (outputOnSerial) Serial.print((String)"\n\r1:"+String(telegramLast[0],HEX)+"="+telegramLast[0]+"  >"); // Hex debug print line last character
      */

      if (decodeTelegram(len + 1)) {        // process data did we have a 'true' end of message condition including terminating CR

        /*  //debugCRC
            // print/display readed length
            if (outputOnSerial) {           // this line is alway printed.
                Serial.print(" [");         // debug print transaction length
                Serial.print(lenTelegram);  // length processed previous line
                Serial.print("] #OK# ");    // display we have a successful end of data
                // Serial.println("\r\n");
            }

        */
        yield();                    // do background processing required for wifi etc.
        ESP.wdtFeed();              // feed the hungry timer
        Serial.print((String) "zc");  //here
        p1TriggerTime = millis();   // indicate we have a yield

        processGpio() ; // do read and process regular data pins and finish  this with mqtt
        
        publishP1ToMqtt();      // PUBLISH this mqtt
        // Note: after this we could do the serial2 process for a period of about 6-8seconds ......
        // this is arranged in the mainloop

        if (intervalP1cnt < 1140) intervalP1cnt++ ; // increase survived read count

        p1SerialFinish = true; // indicate mainloop we can stop P1 serial for a while
        mySerial.flush();      // flush the buffer (if any)

      }
      /* //debugCRC
           else {                              // data not yet finished or in transit
               if (outputOnSerial) {           // this line is alway printed.
                   Serial.print(" [");         // debug print transaction length
                   Serial.print((String) lenTelegram + "," + lenSave);  // length processed previous line
                   Serial.print("] ");         // single of with println: double lines spacing
                   // Serial.println("\r\n");
               }
         } // else
      */

    } // if len > 1
  } // while serial available
  // Serial.println("startend..");
} // void



/* 
  process the secondary serial input port, to be used  in future for 1200Baud GJ meter
*/
void readTelegram2() {
  // if (loopbackRx2Tx2) Serial.print("Rx2 "); // print message line
#ifdef UseP1SoftSerialLIB
  if (mySerial.P1active()) return ;   // return if P1 is active
#endif  

  int lenTelegram2 = 0;
  /*
    if (mySerial2.P1active())   {               // P2 blocked
      // if (loopbackRx2Tx2) Serial.print("RxA "); // print message line
      if (outputOnSerial)    {
        Serial.print("\n\r Rx2 blocked, mySerial2.P1active"); // print message line
      }
    }
  */

  // 30mar21: no data available .....
  if (mySerial2.available())   {
    if (outputOnSerial)    {
      Serial.print("\n\r Rx2N= "); // print message line
    }

    memset(telegram2, 0, sizeof(telegram2));            // initialise telegram array to 0
    memset(telegramLast2, 0, sizeof(telegramLast2));    // initialise array to 0

    while (mySerial2.available())     {
      int len = mySerial2.readBytesUntil('\n', telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
      // len == 0 ? lenTelegram = -1 : lenTelegram += len;   // if len = 0 indicate for report
      lenTelegram2 = lenTelegram2 + len;
      if ( len == 0) lenTelegram2 = -1; // if len = 0 indicate for report

      if (len > 0)  {
        // remember  last character
        telegramLast2[0] = telegram2[len - 2];  // 3byte-telegram readible lastbyte  bracket-2 null-1 cr-0
        telegramLast2[1] = '\n';                // 3byte telegram lastbyte termination string
        telegramLast2[2] = 0;                   // 3byte-telegram lastbyte terminaion variable
        telegram2[len + 1] = '\n';    // string set lastcharacter to \n
        telegram2[len + 2] = 0;       // variable next character before null

        if (outputOnSerial) Serial.print("2>")     ; // debug print processing serial data
        if (outputOnSerial) Serial.print(telegram2); // debug print processing serial data
        if (loopbackRx2Tx2) mySerial2.println(telegram2); // echo back

        yield();  // do background processing required for wifi etc.
        ESP.wdtFeed(); // feed the hungry timer

      }
    } // while data
    if (outputOnSerial) {
      Serial.print(" [");         // debug print transaction length
      Serial.print(lenTelegram2);
      Serial.print("].");
      Serial.println("");
    }

  }  // if available
} // void


/* 
  Handle GPIO pins Adc, Thermostat and Waterheating light
*/
void processGpio() {    // Do regular functions of the system
  /*
  int  tmpb = processAnalogRead();                              // read analog pin (+previous/2) to smooth, return adc
  bool tmpc = processLightRead(digitalRead(LIGHT_READ));        // process LedLightstatus read pin D6
  bool tmpa = processThermostat(digitalRead(THERMOSTAT_READ)) ; // process thermostat switch  D7-in, D8-out
  */
  processAnalogRead();                              // read analog pin (+previous/2) to smooth, return adc
  processLightRead(digitalRead(LIGHT_READ));        // process LedLightstatus read pin D6
  processThermostat(digitalRead(THERMOSTAT_READ)) ; // process thermostat switch  D7-in, D8-out

  GetTemperatures();      // from DS18B20 tempsensors

  // debug pin status 
  if (outputOnSerial) {
    bool tmpd = digitalRead(WATERSENSOR_READ); // read watersensor pin
    Serial.print((String) "\nM#@t=" + (mqttCnt+1) + " @" + (millis() / 1000) + ", TimeP1="  + currentTimeS);  // as - including any errors -  extracted from P1 read
    if (validCRCFound) Serial.print((String) " (E#="          + telegramError + ")"); // data contains errors
    else               Serial.print((String) " (e#="          + telegramError + ")"); // data contains errors;

    Serial.print( "\tGpio5:" );
    if ( tmpd) Serial.print( "1" );
    if (!tmpd) Serial.print( "0" );
    Serial.print( (String)", Water#:"   + waterReadCounter);
    Serial.print( (String)", Hot#:" + waterReadHotCounter);
    Serial.print( ", WaterState:" );
    if ( waterReadState) Serial.print( "1" );
    if (!waterReadState) Serial.print( "0" );
    Serial.print( (String) ", Trg#:" + waterTriggerCnt);    // print ISR call counter
    Serial.print( (String) ", DbC#:" + waterDebounceCnt);   // print Debounce counter
    Serial.print( (String) ", Int#:" + intervalP1cnt);      // print Interval checking counter
    Serial.println( " ." );
  }
  // publishP1ToMqtt();      // PUBLISH this mqtt
}

/* 
    Mqtt input received, callled whenever a MQTT subscription message arrives
*/
void callbackMqtt(char* topic, byte* payload, unsigned int length) {
  previousBlinkMillis = millis() + intervalBlinkMillis ; // end the shortblink (if any)

  // struggling with C++ forbids comparison between pointer and integer
  // char w1 = "" + (char)payload[0];
  // char w1[1];
  // w1[0] = '3';
  // String w1="";
  //  w1 = (char)payload[0];

  String mqttCommand = ""; // initialise as string
  int i = 0;
  for (i = 0; (i < length && i < sizeof(mqttReceivedCommand)); i++) { // read mqtt payload
      char receivedChar = (char)payload[i];
      mqttCommand += receivedChar;
      mqttReceivedCommand[i] = (char)payload[i];
      mqttReceivedCommand[i+1] = '\x00';
  }

  if (outputOnSerial) {
    Serial.print("\rCallback Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    // char w1 = (char)payload[0];
    Serial.println((String)(mqttCommand) + ", len=" + i );
    // Serial.println((String)mqttReceivedCommand[0] + (int)mqttReceivedCommand[1]); 
    // yield();  // do background processing required for wifi etc.
  }
}


/* 
  Processing queued mqttCommand, received by callbackMqtt 
*/
void ProcessMqttCommand(char* payload, unsigned int length) {
  // int new_ThermostatState = 3;       // 0=Off, 1=On, 2=Follow, 3=NotUsed-Follow
  if ((char)payload[0] != '\x00' && (char)payload[1] == '\x00' )  {   // only single command supported for now
    if         ((char)payload[0] == '0') {
      new_ThermostatState = 0;                   // Heating on
      if (outputOnSerial) Serial.print("Thermostat will be switched off.");
    } else  if ((char)payload[0] == '1') {
      new_ThermostatState = 1;                   // Heating off
      if (outputOnSerial) Serial.print("Thermostat will be switched on.");
    } else  if ((char)payload[0] == '2') {
      new_ThermostatState = 2;                   // Heating follows input
      if (outputOnSerial) Serial.print("Thermostat will follow input.");
    } else  if ((char)payload[0] == '3') {
      new_ThermostatState = 3;                   // Heating left alone
      if (outputOnSerial) Serial.print("Thermostat routine disabled.");
    } else  if ((char)payload[0] == 'R') {                                            // Restart
      if (outputOnSerial) Serial.print("Restart Request Received.");
      ESP.restart();
    } else  if ((char)payload[0] == 'D') {
      outputOnSerial  = !outputOnSerial ; // re/enable print logging
      Serial.print("Serial Debug ");
      if (!outputOnSerial)  Serial.print("Inactive\n\r");
      if (outputOnSerial) Serial.println("\nActive.");
    } else  if ((char)payload[0] == 'L') {
      outputMqttLog   = true ;         // Do not log
      if (outputOnSerial) {
        Serial.print("outputMqttLog now ON.");
      }

    } else  if ((char)payload[0] == 'l') {
      outputMqttLog   = false ;       // Do not publish Log
      if (outputOnSerial) {
        Serial.print("outputMqttLog now OFF");
      }

    } else  if ((char)payload[0] == 'T') {
      loopbackRx2Tx2   = !loopbackRx2Tx2 ; // loopback serial port
      // mySerial2.begin( 1200);    // GJ meter port   1200 baud
      // mySerial2.println("..echo.."); // echo back
      if (outputOnSerial) {
        Serial.print("RX2TX2 looptest=");
        if (loopbackRx2Tx2)  Serial.print("ON.");
        if (!loopbackRx2Tx2) Serial.print("OFF.");
      }

/* Does not operate, as serial isetup during setup
    } else  if ((char)payload[0] == 't') {
      bSERIAL_INVERT = !bSERIAL_INVERT ; // switch between invert/noninvert
      if (outputOnSerial) {
        Serial.print("P1rx=");
        if (bSERIAL_INVERT)  Serial.print("Positive P1 serial.");
        if (!bSERIAL_INVERT) Serial.print("Negative P1 serial.");
      }
*/

    } else  if ((char)payload[0] == 'W') {
      useWaterTrigger1 = !useWaterTrigger1;  // Rewrite ISR
      detachWaterInterrupt();
      // detachInterrupt(WATERSENSOR_READ); // disconnect ISR
      waterTriggerTime = micros();       // set our time
      waterTriggerCnt  = 1;              // indicate we are in detached mode
      if (outputOnSerial) {
        Serial.print("useWaterTrigger1=");
        if (useWaterTrigger1) {
          Serial.print("ON .");
          // attachInterrupt(WATERSENSOR_READ, WaterTrigger1_ISR, CHANGE); // trigger at every change
        } else {
          Serial.print("OFF .");
          // attachInterrupt(WATERSENSOR_READ,  WaterTrigger_ISR, CHANGE); // trigger at every change
        }
      }

    } else  if ((char)payload[0] == 'w') {
      useWaterPullUp = !useWaterPullUp;  // No/Yes use internal pullup
      if (outputOnSerial) {
        Serial.print("assist useWaterPullUp=");
        if (useWaterPullUp) {
          Serial.print("ON .");
          pinMode(WATERSENSOR_READ, INPUT_PULLUP);        // Use watersensor with internal pullup
        } else {
          Serial.print("OFF .");
          pinMode(WATERSENSOR_READ, INPUT);               // Do not use internal pullup
        }
      }

    } else  if ((char)payload[0] == 'Z') {
      waterReadCounter = 0 ;     // Zero the Water counters
      waterReadHotCounter = 0 ;  // Zero the WaterHot counter
      mqttCnt = 0;               // Zero mqttCnt
      waterTriggerTime  = 0;     // initialise debounce
      waterTriggerState = LOW;   // reset debounce
      waterReadState    = LOW;   // read watersensor pin
      if (outputOnSerial) {
        Serial.print("WaterCnt = 0, mqttCnt = 0, reset/Watersensor/timers");
      }
    } else  if ((char)payload[0] == 'I') {
      intervalP1cnt = 2880;              // make P1 Interval not critical
    } else  if ((char)payload[0] == 'i') {              // make P1  stepping down interval
      if (intervalP1cnt > 1000) intervalP1cnt = intervalP1cnt - 500;
      if (intervalP1cnt > 500)  intervalP1cnt = intervalP1cnt - 100;
      if (intervalP1cnt > 50)   intervalP1cnt = intervalP1cnt - 25;
      if (intervalP1cnt > 10)   intervalP1cnt = intervalP1cnt - 10;
      if (intervalP1cnt > 3)    intervalP1cnt = intervalP1cnt - 2;
      Serial.println((String)"#!!# ESP P1 timeout count intervalP1cnt=" + intervalP1cnt );
    } else  if ((char)payload[0] == 'P') {
      outputMqttPower = !outputMqttPower ;       // Do not publish P1 meter
      if (outputOnSerial) {
        Serial.print("outputMqttPower now ");
        if (outputMqttPower ) Serial.print("Active.");
        if (!outputMqttPower ) Serial.print("Inactive.");
      }
    } else  {
      if (outputOnSerial) Serial.print((String)"Invalid command:" + (char)payload[0] + "" );
    }

     if (outputOnSerial) Serial.println();   // ensure crlf
  }
}


/* 
    // -------------------------------------------------------------------------------
    /                        Publish full as JSONPATH record
    / -------------------------------------------------------------------------------
*/
void publishP1ToMqtt()    // this will go to Mosquitto
{

  mqttCnt++ ;             // update counter
  // Serial.println("Debug4 publishP1ToMqtt");
  if (CheckData() || forceCheckData ) {       // do we have/want some valid P1 meter reading
    // Serial.println("Debug CheckData=true");
    /*
      digitalWrite(BLUE_LED, HIGH);   //Turn the ESPLED off to signal a short
      previousBlinkMillis = millis();   // initialize start and wiat for interval blinks
      while (previousBlinkMillis < millis() + intervalBlinkMillis) {
      // delay(250);
      }
      digitalWrite(BLUE_LED, LOW);   //Turn the ESPLED on
    */

    // digitalWrite(BLUE_LED, HIGH);   //Turn the ESPLED off to signal a short
    digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // invert BLUE ked
    delay(250);
    // digitalWrite(BLUE_LED, LOW);   //Turn the ESPLED on
    digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // invert BLUE ked

  // Buffers
    char msgpub[MQTTBUFFERLENGTH];             // 20mar21 changed from 320 to 360  04apr21 to #define 480
    char output[MQTTBUFFERLENGTH];             // 20mar21 changed from 320 to 360, 04apr21 to #define 480

    String msg = "{"; // build mqtt frame 
    // msg.concat("\"currentTime\": %lu");                // P1 19nov19 17u12 remove superflous comma
    msg.concat("\"currentTime\":\"%s\"");                  // %s is string
    msg.concat(",\"CurrentPowerConsumption\":%lu");    // P1
    msg.concat(",\"ThermoState\":%u");                 // Johnson
    msg.concat(",\"AnalogRead\":%u");                  // adc
    msg.concat(",\"LedLight1\":%u");                   // Hot water
    msg.concat(",\"powerConsumptionLowTariff\":%lu");  // P1
    msg.concat(",\"powerConsumptionHighTariff\":%lu"); // P1
    msg.concat(",\"P1crc\":%u");                       // Validity CRC 0 or 1

    char temperatureString[7];
    msg.concat(",\"T0\":"); dtostrf(tempDev[5], 2, 2, temperatureString); msg.concat(temperatureString);  // T0
    msg.concat(",\"T1\":"); dtostrf(tempDev[0], 2, 2, temperatureString); msg.concat(temperatureString);  // T1
    msg.concat(",\"T2\":"); dtostrf(tempDev[1], 2, 2, temperatureString); msg.concat(temperatureString);  // T2
    msg.concat(",\"T3\":"); dtostrf(tempDev[2], 2, 2, temperatureString); msg.concat(temperatureString);  // T3
    msg.concat(",\"T4\":"); dtostrf(tempDev[3], 2, 2, temperatureString); msg.concat(temperatureString);  // T4
    msg.concat(",\"T5\":"); dtostrf(tempDev[4], 2, 2, temperatureString); msg.concat(temperatureString);  // T5
    msg.concat(",\"Heating\":%u");                     // Heating valve (out)
    msg.concat(",\"HeatMode\":%u");                    // Heating mode 0=Off, 1=On, 2=Follow, 3=NotUsed-Skip

    /*
      if (outputOnSerial) {
        Serial.print( "Sending Mqtt temp5of5: " );
        Serial.println( temperatureString );
      }

      // msg.concat("\"T0\": %2.2f,");                     // temperature probe T0 device 6 of 6 28d7b5459203022d so92
      msg.concat("\"T0\": %u,");                     // temperature probe T0 device 6 of 6 28d7b5459203022d so92
      msg.concat("\"T1\": %2.2f,");                     // temperature probe T1 device 1 of 6 28aa689d13130197 metal
      msg.concat("\"T2\": %2.2f,");                     // temperature probe T2 device 2 of 6 28aa18c2131301b8 metal
      msg.concat("\"T3\": %2.2f,");                     // temperature probe T3 device 3 of 6 28aa859c1313012e metal
      msg.concat("\"T4\": %2.2f,");                     // temperature probe T4 device 4 of 6 28aae5c913130166 metal
      msg.concat("\"T5\": %2.2f,");                     // temperature probe T5 device 5 of 6 28aa93bb13130138 metal
    */
    // msg.concat("\"powerProductionLowTariff\": %lu,");
    // msg.concat("\"powerProductionHighTariff\": %lu,");
    // msg.concat("\"CurrentPowerProduction\": %lu,");
    // msg.concat("\"GasConsumption\": %lu");

    // msg.concat(", \"Version\":1.3a }");
    msg.concat(", \"mqttCnt\":%u");                     // as of 19nov19 include our message counter

    msg.concat(", \"WaterSwitch\":%u");        // as of 19nov19 include Watersensor waterReadState
    if (loopbackRx2Tx2) {
      msg.concat(", \"WaterTst\":%u");       // as of 25mar21 Use this Json to indicate Testmode
      msg.concat(", \"WaterHotTst\":%u");    // as of 26mar21 Use this Json to indicate Testmode for HotWater
    } else {
      msg.concat(", \"WaterCnt\":%u");       // as of 25mar21 Use this Json to indicate Normal Mode
      msg.concat(", \"WaterHotCnt\":%u");    // as of 26mar21 Use this Json to indicate Normal Mode for HotWater
    }

    if (useWaterTrigger1) msg.concat(", \"Trigger1:\"1");  // show triggernumber
    if (useWaterPullUp)   msg.concat(", \"PullUp:\"1");    // show pullupmode

    if (forceCheckData) {
      // no clock time, use our runtine active
      // unsigned long allSeconds=millis()/1000;
      unsigned long allSeconds = currentMillis / 1000;  // take time of mailoop
      int runHours = (allSeconds / 3600) % 24;
      int secsRemaining = allSeconds % 3600;
      int runMinutes = secsRemaining / 60;
      int runSeconds = secsRemaining % 60;
      
      // sprintf(currentTimeS, "\"%02d%02d%02d\"", runHours, runMinutes, runSeconds); // error: is actually 8 bytes !!!!!
      snprintf(currentTimeS, sizeof(currentTimeS), "%02d%02d%02d", runHours, runMinutes, runSeconds);
      
      // +hoursRemaining+runMinutes+runSeconds;
      // char buf[21];
      // sprintf(buf,"Runtime%02d:%02d:%02d",runHours,runMinutes,runSeconds);
      /*
        Serial.println(buf);
        delay(1000);
      */
      msg.concat((String)", \"Force\":" + intervalP1cnt);  // No meter reads, indicate we are in forced bypassmode
      
    }

    msg.concat(", \"Version\":%u");                    // version to determine message layout

    msg.concat(" }"); // terminate JSON
    
    msg.toCharArray(msgpub, MQTTBUFFERLENGTH);         // 27aug18 changed from 256 to 320 to 360 to MQTTBUFFERLENGTH

// important note: sprinft corrupts and crashes esp8266, use snprinf whihc CAN handle multiple variables
//  sprintf(output, msgpub,           // construct data  http://www.cplusplus.com/reference/cstdio/sprintf/ , formats: http://www.cplusplus.com/reference/cstdio/printf/
    snprintf(output, sizeof(output), msgpub ,
            // currentTime,                // metertime difference 52 seconds can also use millis()
            currentTimeS,               // meter time in string format from timedate record
            // millis(),
            CurrentPowerConsumption,
            !thermostatReadState,       // input setting Switch press 1=ON low , Not active High 0=OFF
            filteredValueAdc,           // read analog value
            !lightReadState,            // Hotwater we want 1=ONlight , Not active 0=OFFlight
            powerConsumptionLowTariff,
            powerConsumptionHighTariff,
            validCRCFound,              // P1crc=0 or 1 for valid CRC
            // powerProductionLowTariff,
            // powerProductionHighTariff,
            // CurrentPowerProduction,
            /*
              tempDev[5],               // temperature probe T0 (device 6 of 6)
              tempDev[0],               // temperature probe T1
              tempDev[1],               // temperature probe T2
              tempDev[2],               // temperature probe T3
              tempDev[3],               // temperature probe T4
              tempDev[4],               // temperature probe T5
            */
            thermostatWriteState,       // output setting for heating 1=inSetHighPowerRelais, 0=OffnoPowertoRelais
            new_ThermostatState,        // Heating mode 0=Off, 1=On, 2=Follow, 3=NotUsed-Skip
            // GasConsumption;
            mqttCnt,                    // mqtt counter

            waterReadState,             // Watersensor counter 21mar21 0=LOW 1=HIGH
            waterReadCounter,           // Watersensor state 21mar21 number falldowns
            waterReadHotCounter,        // Watersensor state 26mar21 number during Hotwater

            prog_Version );             // (fixed) Version from program , see top

    if (client.connected()) {
      if (outputMqttPower) client.publish(mqttTopic, output);   // are we publishing data ? (on *mqttTopic = "/energy/p1")
      mqttP1Published = true;             // yes we have publised energy data
    }

    // digitalWrite(BLUE_LED, HIGH);   //Turn the ESPLED off
    digitalWrite(BLUE_LED, thermostatReadState);   //Turn the ESPLED according to input thermostate
  }
}

/* 
    If output = input then false changed
    bool thermostatChanged = processThermostat(LOW) || thermostatReadState = processThermostat(HIGH) || processThermostat(thermostatWriteState)
    true state was changed , false state was already as instructed

*/
bool processThermostat(bool myOperation)    // my operation is currently readed thermostate during call
{
  thermostatReadState = digitalRead(THERMOSTAT_READ); // read
  if (outputOnSerial) Serial.println("processThermostat:") ;

  // Process this one-to-one directly to output
  // new_ThermostatState = 0-off, 1-set, 2-follow, 3-leave

  switch (new_ThermostatState) {          // switch works on integers
    case 0: thermostatWriteState = LOW;        // heating switchOFF
      // Serial.print(", proposed thermostatWriteState=low, ") ;
      break ;
    case 1: thermostatWriteState = HIGH;        // heating switchON
      // Serial.print(", proposed thermostatWriteState=high, ") ;
      break ;
    case 2: thermostatWriteState  = !thermostatReadState;     // follow input
      break ;
    default: thermostatWriteState = thermostatReadState;
  }
  // Serial.println("processThermostatCase:") ;
  // if (thermostatReadState) digitalWrite(THERMOSTAT_WRITE, LOW );

  if (outputOnSerial) {

    if (thermostatReadState)            {
      Serial.print("ThermostateIn D7 high(OFF), ") ; // Display input Thermostate
    } else                              {
      Serial.print("ThermostateIn D7 low(ON), ") ;  // LOW means set, HIGH means unset
    }

    if         (new_ThermostatState == 0) {
      Serial.print("ThermoCommand=OFF "    ) ;        // Display processsing mode
    } else  if (new_ThermostatState == 1) {
      Serial.print("ThermoCommand=ON "     ) ;
    } else  if (new_ThermostatState == 2) {
      Serial.print("ThermoCommand=FOLLOW " ) ;
    } else                              {
      Serial.print("ThermoCommand=LEAVE "  ) ;
    }

    if (thermostatWriteState)           {
      Serial.print(" := ThermostateOut high(ON) ") ;  // Display result
    } else {
      Serial.print(" := ThermostateOut low(OFF) ") ;
    }

  }
  /*
    thermostatReadState = digitalRead(THERMOSTAT_READ); // read
    digitalWrite(THERMOSTAT_WRITE, myOperation );
    thermostatWriteState = myOperation
    if (thermostatReadState == myOperation ) return LOW;  // not actively changed
  */

  // if (outputOnSerial) Serial.print("processThermostat:");
  if (new_ThermostatState >= 0 && new_ThermostatState <= 2) {
    if (outputOnSerial) Serial.print(" set ") ;
    if ( thermostatWriteState) {
      digitalWrite(THERMOSTAT_WRITE, HIGH ); // leave to high, switch off relay that issues voltage to Valve
    }
    if (!thermostatWriteState) {
      digitalWrite(THERMOSTAT_WRITE, LOW );  // pull to ground switch on relay that issues voltage to Valve
    }
  } else {
    if (outputOnSerial) Serial.print(" kept") ;
  }

  thermostatWriteState = digitalRead(THERMOSTAT_WRITE); // Get status of output relay
  if (thermostatWriteState) {
    if (outputOnSerial) Serial.print(" high(ON) ") ;
  } else {
    if (outputOnSerial) Serial.print(" low(OFF) ") ;
  }

  if (thermostatWriteState != thermostatReadState) {    // request=result or nothing changed (note thermoritestate is 1L=ON, 0H=off for humans)
    if (outputOnSerial) Serial.println(" Same as requested.") ;
    return false ;
  }

  if (outputOnSerial) Serial.println(" Overruling the request.") ;
  return true ;  // was actively changed
}


/* 
   read analog ADC, return smoothed (averaged) lux value
*/

int processAnalogRead()   // read adc analog A0 pin and smooth it with previous read (addtwo /2)
{ // return filtervalue
  pastValueAdc = nowValueAdc;
  nowValueAdc = analogRead(ANALOG_IN);      // A0 pin
  // nowValueAdc = 123;
  filteredValueAdc = (pastValueAdc + nowValueAdc) / 2;  // smooth values by avarageing
  if (outputOnSerial and thermostatReadState) {  // debug
    Serial.print("\n\rAnalog sensor = ");
    Serial.print(nowValueAdc);
    Serial.print(" filtered Value = ");
    Serial.println(filteredValueAdc);
  }
  return filteredValueAdc;
}

// process Ledlightstatus indicating if Hot water is tapped, return state
bool processLightRead(bool myOperation)
{
  lightReadState   = digitalRead(LIGHT_READ); // read D6
  // Process this one-to-one directly to output
  if (outputOnSerial and lightReadState)  Serial.print("lightReadState D6 LOW...");  // debug
  if (outputOnSerial and !lightReadState) Serial.print("lightReadState D6 HIGH...");  // debug
  return lightReadState;  // return status
}


/* 
  process P1 data start / to ! Let us decode to see what the /KFM5KAIFA-METER meter is reading
*/
bool decodeTelegram(int len)    // done at every P1 line that ends in crlf
{
  //need to check for start
  validCRCFound = false;  // bool Global defined

  int startChar = FindCharInArrayRev(telegram, '/', len);  // 0-offset "/KFM5KAIFA-METER" , -1 not found
  int endChar   = FindCharInArrayRev(telegram, '!', len);  // 0-offset "!769A" , -1 not found
  bool endOfMessage = false;    // led on during long message transfer

  if (startChar >= 0) {        // We are at start/first line of Meter reading
    // DebugCRC telegramP1header = false;
    // DebugCRC testTelegramPos = 0;     // initialise array

    // initialise CRC
    // currentCRC=CRC16(0x0000,(unsigned char *) telegram+startChar, len-startChar);

    //DebugCRC if (outputOnSerial) Serial.printf("{.%02X%02X%02X%02X}",telegram[len-startChar-2],telegram[len-startChar-1],telegram[len-startChar],telegram[len-startChar+1]);

    //   xN= {52 0D 00}
    //     {.0D000A00} /KFM5KAIFA-METER [16,17] {00 0D 4B}{29 0D 45}
    //                 0123456789012345
    //                 1234567890123456

    // /KFM5KAIFA-METER.0
    // currentCRC=CRC16(currentCRC,(unsigned char *) "\x0d\x0a\x00\x0d\x0a", 5);    // our intersection messages are terminated with \n which is in fact cr-lf
    //DebugCRC   currentCRC=CRC16(0x0000,(unsigned char *) "/KFM5KAIFA-METER\x0d\x0a\x0d\x0a", 21);    // our intersection messages are terminated with \n which is in fact cr-lf

    // initialise CRC

    if (strncmp(telegram, "/KFM5KAIFA-METER", strlen("/KFM5KAIFA-METER")) == 0) telegramP1header = true;   // indicate we are in header mode
    currentCRC = CRC16(0x0000, (unsigned char *) telegram + startChar, len - startChar - 1); 
    if (telegram[startChar+len-2] == '\x0d') currentCRC = CRC16(currentCRC, (unsigned char *) "\x0a", 1); // add implied NL on header

    /*
    if ( len = 1 && telegram[len-1] == '\x0d') {
      Serial.print("DebugAddCrLf.."); 
      currentCRC = CRC16(currentCRC, (unsigned char *) "\x0a\x0d\x0a", 3); // add stripped header
    }
    */



    // digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off
    // digitalWrite(BLUE_LED, LOW);       // Turn the ESPLED on to inform user
    digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // invert BLUE ked
    allowOtherActivities = false;      // hold off other serial/interrupt acvities
    /*
         if (outputOnSerial)  {     // Printing /KFM5KAIFA-METER
             // Serial.println("soh>\n\r");
             if (len > 4) {
                 for (int cnt = startChar; cnt < len - (startChar + 2); cnt++) { // print P1 characters
                   // copy over the processed data
                   testTelegram[testTelegramPos]=telegram[cnt]              ; // v13 test 31mar21 ptro
                   if (testTelegramPos < MAXLINELENGTH-2) testTelegramPos++ ; // v13 test 31mar21 ptro
                // Serial.print("<");
                   Serial.print(telegram[cnt]);
                // Serial.print(">");
                 } // for
                 Serial.println("\r\n");
             }
             // Serial.println("\r\n");
         }
    */

  } else {

    if (endChar >= 0 && telegramP1header) {   // Are we reading the last line of the Meter and header passed

      currentCRC = CRC16(currentCRC, (unsigned char*)telegram + endChar, 1); // include trailer '!' into CRC
      char messageCRC[5];
      strncpy(messageCRC, telegram + endChar + 1, 4);
      messageCRC[4] = 0;  // make it an end of string
      validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);
      if (outputOnSerial)  Serial.printf("crc\t!%s (arc=%x)\n", messageCRC, currentCRC); // /t produces an error
      /* // DebugCRC not activated as most of the time we miss characters which make CRC Invalid
         if (outputOnSerial)  {
             if( validCRCFound) Serial.print((String)  "\nVALID CRC FOUND ! start=" + startChar + " end="+ endChar );
             if(!validCRCFound) Serial.print((String)"\nINVALID CRC FOUND ! start=" + startChar + " end="+ endChar );
             Serial.printf(", crc=%x msg=%s \n",currentCRC,messageCRC);
         }
      */
      // digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on
      // digitalWrite(BLUE_LED, HIGH);       //Turn the ESPLED off
      digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // revert BLUE led
      allowOtherActivities = true;    // hold off other conflicting acvities
      /*
        //DebugCRC
              if (outputOnSerial)     {
                // Serial.println("a\n<eof0\nb");
                for (int cnt = 0; cnt < len-2; cnt++) {

                 // copy over the processed data
                 testTelegram[testTelegramPos]=telegram[cnt]              ; // v13 test 31mar21 ptro
                 if (testTelegramPos < MAXLINELENGTH-2) testTelegramPos++ ; // v13 test 31mar21 ptro

                  Serial.print(telegram[cnt]);
                  // Serial.print(".");
                }


                if (mqttCnt < 10 && telegramError == 0) {
                  Serial.printf("\n\nString>%s< [%u, crc=%x msg=%s ]\n",testTelegram,testTelegramPos,currentCRC,messageCRC);
                  yield();
                  Serial.print("\n HEX:>");
                  for (int cnt=0; cnt <= testTelegramPos; cnt++) {
        ;                  Serial.printf("%02X",testTelegram[cnt]);
                  }
                  Serial.print("<HEX \n");
                }

                currentCRC = 0;

                //  Serial.println(" <eof1");
                //  Serial.println("\n\r"); // disabled at this  interferes with the cast EoT record
           } // else if (endChar >= 0 && telegramP1header)
      */

    } else {                        // We are reading between header and trailer
      currentCRC = CRC16(currentCRC, (unsigned char*)telegram, len - 1); // calculatate CRC upto/including 0x0D
      if (telegram[len - 2] == '\x0d') currentCRC = CRC16(currentCRC, (unsigned char *) "\x0a", 1); // add excluded \n
      if (outputOnSerial)
      {
        // Serial.print("<");
        if (len > 2) {  // (was > 2)
          for (int cnt = 0; cnt < len - 2; cnt++) {  // read max 64 bytes (was len-2)
            /*// DebugCRC
              //DebugCRC copy over the processed data
              testTelegram[testTelegramPos]=telegram[cnt]              ; // v13 test 31mar21 ptro
              if (testTelegramPos < MAXLINELENGTH-2) testTelegramPos++ ; // v13 test 31mar21 ptro
            */
            if (!isPrintable(telegram[cnt]) || telegram[cnt] == ' ' ) telegramError++; // check if data is readable

            /* //debugCRC
              {
                  Serial.print(telegram[cnt]);
              } else {
                  telegramError++;                  // count as error
                  // Serial.printf("\?%02\?",telegram[cnt]);
                  Serial.printf("\?%02\?",telegramLast2[0]);
                  // Serial.print((String)"?"+String(telegramLast[cnt],HEX)+"?");
              }
            */
          } // for loop
          //DebugCRC if (outputOnSerial) Serial.printf("{%02X}",telegram[len-2]);
        }
        /* //DebugCRC
          if (len <= 2) {
          Serial.print(charArray[len]);
          }
          // Serial.print(">\n\r");
          // Serial.print(">");
        */
      } // if output serial
    } // else if endChar >= 0 && telegramP1header
  } // startChar >= 0

  if (outputMqttLog && client.connected()) client.publish(mqttLogTopic, telegram );   // debug to mqtt log ?
  // if (outputMqttLog) client.publish(mqttLogTopic, telegramLast );

  /*
    if (!(telegram[len - 2] == ')') ) { // ignore if not ending on )
      telegram[0] = telegram[len - 2];
      telegram[1] = telegram[len - 2];
      telegram[2] = telegram[len - 2];
      client.publish(mqttLogTopic, telegram );   // log
      return endOfMessage;
    }
  */

  /*
     if (telegram[len - 3] == ')' ) client.publish(mqttLogTopic, "tele-3" );   // log
     if (telegram[len - 2] == ')' ) client.publish(mqttLogTopic, "tele-2" );   // log valid
     if (telegram[len - 1] == ')' ) client.publish(mqttLogTopic, "tele-1" );   // log
     if (telegram[len]     == ')' ) client.publish(mqttLogTopic, "tele=0" );   // log
     if (telegram[len + 1] == ')' ) client.publish(mqttLogTopic, "tele+1" );   // log
     if (telegram[len + 2] == ')' ) client.publish(mqttLogTopic, "tele+2" );   // log
     if (telegram[len + 3] == ')' ) client.publish(mqttLogTopic, "tele+3" );   // log

     if (telegram[22] == 'S'     ) client.publish(mqttLogTopic, "tele+S" );   // log
     if (telegram[22] == 'S'     ) client.publish(mqttLogTopic, telegram );   // log DateTime
  */

  // 11jun20 yield();  // do background Wifi processing else things will not work

  if ( startChar >= 0 or endChar >= 0 ) {   //  Do we have a start OR finish of meterread
    // if (outputMqttLog) client.publish(mqttLogTopic, telegramLast );   // log
    // client.publish(mqttLogTopic, telegram );   // log
    if (endChar >= 0 && telegramP1header ) {      // we have had a full Telegram
      endOfMessage = true;                      // assume end of message
      telegramP1header = false;                 // switch off header mode
      if (!isNumber(currentTimeS, 6)) endOfMessage = false; // Data error, reject state
      // if (loopbackRx2Tx2) endOfMessage = false; // testonly, in this case mqtt are forced in overall loop
    }
    return endOfMessage;
  }

  if ( !telegramP1header) return endOfMessage; // if not in header received mode then return
  // 11jun20 yield();  // do background Wifi processing else things will not work


  // used to catch P1meter line termination bracket
  // if (telegram[len - 3] != ')' ) client.publish(mqttErrorTopic, telegram ); // log invalid
  // if (telegram[len - 3] != ')' ) client.publish(mqttLogTopic, "tele-2" );   // log invalid

  if (   telegram[0] != '0' && telegram[0] != '1' && telegram[1] != '-' ) return endOfMessage; // if not start wih 0- or 1-

  // if ( !(telegram[len - 3] == ')' || telegram[len - 4] == ')' )) return endOfMessage; // if not terminated by bracket then return
  if ( !(telegram[len - 3] == ')')) return endOfMessage; // if not terminated by bracket then return

  // Serial.println((String)"DebugDecode6:"+ (int)telegram[len - 3] );   // with testdata, does not arrive here
  
  // Serial.println((String)"DebugDecode7 (=" + strncmp(telegram, "(", strlen("(")) + " )="+strncmp(telegram, ")", strlen(")"))) ;
  // Serial.println((String)"DebugDecode7 substr1-7(=" + telegram[0]+telegram[1]+telegram[2]+telegram[3]+telegram[4]+"<");
  //  c h e c k   t e s t   P 1   r e c o r d s
  // long val = 0;
  // long val2 = 0;

  // client.publish(mqttLogTopic, telegram );    // show what we have got

  //  0123456789012345678901234567890
  //  0-0:1.0.0(180611014816S) added ptro
  //  0-0:1.0.0(191119182821W)
  
  if (strncmp(telegram, "0-0:1.0.0(", strlen("0-0:1.0.0(")) == 0) {       // do we have a Date record ?
    if (telegram[22] == 'S' || telegram[22] == 'W') {  // check for Summer or Wintertime
      char resDate[16];     // maximum prefix of timestamp message
      memset(resDate, 0, sizeof(resDate));       // Pointer to the block of memory to fill.
      if (strncpy(resDate, telegram + 16, 6)) {  // start buffer+10+1 for 6 bytes
        if (isNumber(resDate, 6)) {            // only copy/use new date if this is full numeric (decimal might give trouble)
          currentTime  = (1 * atof(resDate)); // Convert string to double http://www.cplusplus.com/reference/cstdlib/atof/
          strncpy(currentTimeS, telegram + 16, 6);  // Ptro 27mar21: added as we like to have readible timestamp in message(s)
        }
        /*
          // currentTimeS = "000000";
          // char currentTimeS[] = "000000";      // array element local
          currentTimeS[] = "000000";      // array element local
          for (int i = 0; i >= 5; i++) {   // forward
          // strncpy(currentTimeS[i], telegram+16+i,1);  // copy 6 bytes as string
          // currentTimeS[i] = (int)"0";
          // if (((telegram[16+i] >= "0") && telegram[16+i] <= "9")) currentTimeS[i] = telegram[16+i];  // copy 6 bytes as string
          currentTimeS[i] = telegram[16+i];
          // if (!isNumber(currentTimeS, 6))
          } // replaces strncpy(currentTimeS, telegram + 16, 6);  // copy 6 bytes as string
        */
      }
    }
    InitialiseValues();         // Data record, ensure we start fresh with newly values in coming records
  }

  // yield();  // do background processing else things will not work

  // 1-0:1.8.1(000992.992*kWh)
  // 1-0:1.8.1 = Elektra verbruik laag tarief (DSMR v4.0)  
  if (strncmp(telegram, "1-0:1.8.1(", strlen("1-0:1.8.1(")) == 0) {        // total Watts Low
    powerConsumptionLowTariff = getValue(telegram, len);
    }

  // 1-0:1.8.2(000560.157*kWh)
  // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.8.2(", strlen("1-0:1.8.2(")) == 0)         // total Watts High
    powerConsumptionHighTariff = getValue(telegram, len);

  // 1-0:2.8.1(000348.890*kWh)
  // 1-0:2.8.1 = Elektra opbrengst laag tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.1(", strlen("1-0:2.8.1(")) == 0)         // total Production Low
    powerProductionLowTariff = getValue(telegram, len);

  // 1-0:2.8.2(000859.885*kWh)
  // 1-0:2.8.2 = Elektra opbrengst hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.2(", strlen("1-0:2.8.2(")) == 0)         // total Production High
    powerProductionHighTariff = getValue(telegram, len);

  // 1-0:1.7.0(00.424*kW) Actueel verbruik
  // 1-0:2.7.0(00.000*kW) Actuele teruglevering
  // 1-0:1.7.x = Electricity consumption actual usage (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.7.0(", strlen("1-0:1.7.0(")) == 0)        // Watts usage
    // client.publish(mqttLogTopic, telegram );
    CurrentPowerConsumption = getValue(telegram, len);


  if (strncmp(telegram, "1-0:2.7.0(", strlen("1-0:2.7.0(")) == 0)        // Watts produced
    CurrentPowerProduction = getValue(telegram, len);

  // 0-1:24.2.1(150531200000S)(00811.923*m3)
  // 0-1:24.2.1 = Gas (DSMR v4.0) on Kaifa MA105 meter
  if (strncmp(telegram, "0-1:24.2.1(", strlen("0-1:24.2.1(")) == 0)      // Gaz
    GasConsumption = getValue(telegram, len);
  return endOfMessage;
}


/* 
  get string to long value in numeric (...* strings)
*/
long getValue(char *buffer, int maxlen)
{
  // 1234567890123456789012345678901234567890   = 40    18061   000   // countlist 1
  // 01234567890123456789012345678901234567890  = 40                  // countlist 0
  //           123456789012                                           // countlist after first bracket
  // 0-0:1.0.0(180611014816S)                   = data record len=25, string length datalen 12
  // 0-1:24.2.1(150531200000S)(00811.923*m3)    = gaz date record which we do not have at our P1
  // 1-0:1.8.2(010707.720*kWh)                  = power consumption record len=26 datalen 10
  //  .........1.........2.........3.........4

  int s = FindCharInArrayRev(buffer, '(', maxlen - 2);  // search buffer fro bracket (s=25)
  if (s < 8) return 0;    // no valid string
  if (s > 32) s = 32;     // no bracket in range, try at end backward search

  int l = FindCharInArrayRev(buffer, '*', maxlen - 2) - s - 1;    // search end of * , s=start of (


  /*    // ignore this, we only process data starting at bracket ( en finished with * asterisk
    if (l < 0) {   // no asterisk in message, check for Summer or Winter Time date record
      // 11jun20 yield();  // do background processing else things will not work
      // l = FindCharInArrayRev(buffer, 'S', maxlen - 0) - s - 1;  // search S , s=start of (
      if (buffer[22] == 'S') l = 6; // length 7-digits does not work vaue too large
      if (buffer[22] == 'W') l = 6; // length 7-digits does not work vaue too large
  */

  if (l < 4)  return 0;    // length from (-* too short
  if (l > 12) return 0;   // length from (-* too long

  char res[16];                 // extract number starting at startbracket for a lengt of l
  memset(res, 0, sizeof(res));

  if (strncpy(res, buffer + s + 1, l))  // start buffer+10+1
  {
    if (isNumber(res, l))         // is this all numeric ?
    {
      return (1000 * atof(res));  // Convert Character String to Float mulitplied by 1000 (remove comma)
    }
  }
  return 0;
}


/* 
  generic subroutine to find a character in reverse , return position as offset to 0
*/
int FindCharInArrayRev(char array[], char c, int len)               // Find character >=0 found, -1 failed
{
  //           123456789012
  // 1234567890123456789012345678901234567890  = 40
  // 0-0:1.0.0(180611014816S)                  = sublen12 len=25 pos24=) post10=(=ret10, pos23=S=ret23   , maxlen==23 == return i=10
  // 0-1:24.2.1(150531200000S)(00811.923*m3)

  for (int i = len - 1; i >= 0; i--)   // backward
  {
    if (array[i] == c)
    {
      return i;
    }
  }
  return -1;
}

/* 
  Check if we are complete on data retrieval and construct and publish output to mqtt LOG topic
*/
bool CheckData()        // 
{

  if (firstRun)  {      // at start initialise and set olddata to new data
    SetOldValues();
    firstRun = false;
    return true;
  }

  if (outputMqttLog) {  // if we LOG status old values not yet set
    char msgpub[768];
    char output[768];
    String msg = "{";
    msg.concat("\"currentTime\": %lu,");              // %lu is unsigned long
    msg.concat("\"CurrentPowerConsumption\": %lu,");
    msg.concat("\"powerConsumptionLowTariff\": %lu,");
    msg.concat("\"powerConsumptionHighTariff\": %lu,");
    msg.concat("\"CurrentPowerProduction\": %lu,");
    msg.concat("\"powerProductionLowTariff\": %lu,");
    msg.concat("\"powerProductionHighTariff\": %lu,");
    msg.concat("\"GasConsumption\": %lu,");
    msg.concat("\"OldPowerConsumptionLowTariff\": %lu,");
    msg.concat("\"OldPowerConsumptionHighTariff\": %lu,");
    msg.concat("\"OldPowerProductionLowTariff\": %lu,");
    msg.concat("\"OldPowerProductionHighTariff\": %lu,");
    msg.concat("\"OldGasConsumption\": %lu");
    msg.concat("}");
    msg.toCharArray(msgpub, 768);
    // sprintf(output, msgpub, currentTime,

    // sprintf(output, msgpub, millis(),                // prefer to use snprinf which offers protection
    snprintf(output, sizeof(output), msgpub,
            millis(),
            CurrentPowerConsumption,
            powerConsumptionLowTariff, powerConsumptionHighTariff,
            CurrentPowerProduction,
            powerProductionLowTariff, powerProductionHighTariff,
            GasConsumption,
            OldPowerConsumptionLowTariff, OldPowerConsumptionHighTariff,
            OldPowerProductionLowTariff, OldPowerProductionHighTariff,
            OldGasConsumption);
    if (client.connected()) client.publish(mqttLogTopic, output);
  }

  
  // check overrating more then 25.200kWh per 10sec read (=70w/s => 80Amps) is likely die to misread, can require recycle to recover
  if ((powerConsumptionLowTariff  - OldPowerConsumptionLowTariff > 70) || powerConsumptionLowTariff < 1)   // ignore zero reads
  {
    
    // problem: test_mode alsways ends in powerConsumptionLowTariff=0
    // Serial.println((String)"Checkdata3.."+" new="+powerConsumptionLowTariff+" old="+OldPowerConsumptionLowTariff);   // problem returns and does not let publish

    OldPowerConsumptionLowTariff = powerConsumptionLowTariff ;
    if (powerConsumptionLowTariff < 1) OldPowerConsumptionLowTariff = 0 ;
    return false;
  }

  if ((powerConsumptionHighTariff - OldPowerConsumptionHighTariff > 70) || powerConsumptionHighTariff < 1)  // ignore zeroreads
  {
    OldPowerConsumptionHighTariff = powerConsumptionHighTariff ;
    if (powerConsumptionHighTariff < 1) OldPowerConsumptionHighTariff = 0 ;
    return false;
  }

  // following is totally not used at our place but can be used if/when we have solarpanels.

  if ((powerProductionLowTariff  - OldPowerProductionLowTariff    > 70) || powerProductionLowTariff < 0)   // production > 0
  {
    OldPowerProductionLowTariff  = powerProductionLowTariff ;
    if (powerProductionLowTariff < 0) OldPowerProductionLowTariff = 0 ;
    return false;
  }

  if ((powerProductionHighTariff - OldPowerProductionHighTariff > 70) || powerProductionHighTariff < 0)    // production > 0
  {
    OldPowerProductionHighTariff = powerProductionHighTariff ;
    if (powerProductionHighTariff < 0) OldPowerProductionHighTariff = 0 ;
    return false;
  }
  if ((CurrentPowerConsumption > 18400) || (CurrentPowerProduction < 0)) return false;  // return if outside 80Amp load


  /*
     // next Code abonded as we do not have "0-1:24.2.1" / Gaz records and cannot reliable proof/test

    if ((GasConsumption - OldGasConsumption > 1) || GasConsumption < 0)
    {
    return false;
    }
    SetOldValues();
  */

  if (outputMqttPower)    // output currrent power only , flatnumber
  {
    char msgpub[768];     // allocate a message buffer
    char output[768];     // use snprintf to format data
    String msg = "";      // initialise data
    msg.concat("CurrentPowerConsumption: %lu");       // format data
    msg.toCharArray(msgpub, 768);                     // move it to format buffwer
    sprintf(output, msgpub, CurrentPowerConsumption); // insert datavalue  (Note if using multiple values use snprint)
    if (client.connected()) client.publish(mqttPower, output);                // publish output
  }
  // Serial.println("Debug5, Checkdata..true");
  return true;
}

/* 
  Initise consumption variables to zero
*/
void InitialiseValues()  // executed at every telegram new run
{
  powerConsumptionLowTariff  = 0;
  powerConsumptionHighTariff = 0;
  powerProductionLowTariff   = 0;
  powerProductionHighTariff  = 0;
  GasConsumption             = 0 ;
}


void SetOldValues()     // executed at end succesful  telegrams
{
  OldPowerConsumptionLowTariff  = powerConsumptionLowTariff;
  OldPowerConsumptionHighTariff = powerConsumptionHighTariff;
  OldPowerProductionLowTariff   = powerProductionLowTariff;
  OldPowerProductionHighTariff  = powerProductionHighTariff;
  OldGasConsumption             = GasConsumption;
}

// check if data/field is numeric 0-9 & decimal-point
bool isNumber(char *res, int len)       // False/true if numeriv
{
  for (int i = 0; i < len; i++)
  {
    if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0))
    {
      return false;
    }
  }
  return true;
}

// =============================================== routines added for temperature processing
/* 
  Setting the temperature sensor
*/
void SetupDS18B20() {

  for (int i = 0; i < ONE_WIRE_MAX_DEV; i++) {    // search temperature sensors
    tempDev[i] = 0.00; //Initialize table to 0
  }

  DS18B20.begin();                                // initialise

  // if (outputOnSerial) {                          // debug will disable
  Serial.print("Parasite power is: ");
  if ( DS18B20.isParasitePowerMode() ) {
    Serial.print("ON");
  } else {
    Serial.print("OFF");
  }
  numberOfDsb18b20Devices = DS18B20.getDeviceCount();
  Serial.print( ", device count: " );
  Serial.println( numberOfDsb18b20Devices );
  //  }

  lastTempReadTime = millis();                        // last time interval we  did read
  DS18B20.requestTemperatures();

  // if (outputOnSerial) {
  // Loop through each device, print out address
  for (int i = 0; i < numberOfDsb18b20Devices; i++) {
    // Search the wire for address
    if ( DS18B20.getAddress(devAddr[i], i) ) {
      //devAddr[i] = tempDeviceAddress;
      Serial.print("Found Ds18b20 device ");
      Serial.print(i, DEC);
      Serial.print(" with address: " + GetAddressToString(devAddr[i]));
      // Serial.println();
      //Get resolution of DS18b20
      Serial.print(" at Resolution: ");
      Serial.print(DS18B20.getResolution( devAddr[i] ));
      //Read temperature from DS18b20
      float tempC = DS18B20.getTempC( devAddr[i] );
      Serial.print(", Temp C: ");
      Serial.println(tempC);
    } else {
      Serial.print("Found Temp ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  } // for
  // } // outputonserial
  // test if (numberOfDsb18b20Devices < 6) intervalP1cnt=-1; // check and do something if devices are missing.
}

/* 
  Read and get DS18B20 temperatures
*/
void GetTemperatures() {
  // String message = "Number of devices: ";
  // message += numberOfDsb18b20Devices;
  // message += "\r\n";
  char temperatureString[6];

  for (int i = 0; i < numberOfDsb18b20Devices; i++) {
    float tempC = DS18B20.getTempC( devAddr[i] ); //Measuring temperature in Celsius
    tempDev[i] = tempC; //Save the measured value to the array
  }

  DS18B20.setWaitForConversion(false); // No waiting for measurement
  DS18B20.requestTemperatures();       // Initiate the temperature measurement
  lastTempReadTime = millis();         // Remember the last time measurement

  if (outputOnSerial) {
    Serial.print( "Sending temperatures: " );
    for (int i = 0; i < numberOfDsb18b20Devices; i++) {
      // The dtostrf() function converts the double value passed in val into an ASCII representationthat will be stored under s.
      dtostrf(tempDev[i], 2, 2, temperatureString);
      //if ( i == 1 ) dtostrf(tempDev[i], 2, 2, temperatureString1);  // not longer used
      Serial.print( "\t" );
      Serial.print( temperatureString );
      // message += GetAddressToString( devAddr[i] )
    }
    Serial.println(" ");
    // message += "</table>\r\n";
  }
}


/*
  attach waterinterrupt
  Activate ISR water interurpt and choose between two operative versions W/w
  ISR read water sensor on default pin grpio4 and respect debouncetime 40mSec
*/
void attachWaterInterrupt() {   // 
  if (useWaterTrigger1) {
    attachInterrupt(WATERSENSOR_READ, WaterTrigger1_ISR, CHANGE); // establish trigger
    if (outputOnSerial) Serial.println((String)"\nSet Gpio" + WATERSENSOR_READ + " to second WaterTrigger1_ISR routine");
  } else {
    attachInterrupt(WATERSENSOR_READ, WaterTrigger_ISR, CHANGE); // establish trigger
    if (outputOnSerial) Serial.println((String)"\nSet Gpio" + WATERSENSOR_READ + " to first WaterTrigger_ISR routine");
  }
}
void detachWaterInterrupt() {   // disconnectt Waterinterrupt to prevent inteference while doing serial communication
  detachInterrupt(WATERSENSOR_READ); // trigger at every change
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << WATERSENSOR_READ);  // Prevent calls as per expressif intruction
}


/*
  ISR  water pulse primary (select via W,w) will detach during excessive vibration
*/
void WaterTrigger_ISR()
{
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << WATERSENSOR_READ);  // 26mar21 Ptro done at start as per advice espressif
  digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // invert BLUE ked
  waterTriggerCnt++ ;             // increase our call counter
  long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
  waterTriggerTime  = time;       // set time of this read

  waterTriggerState = digitalRead(WATERSENSOR_READ); // read unstable watersensor pin
  if (loopbackRx2Tx2) digitalWrite(SERIAL_TX2, waterTriggerState); // here test trigger to find if it is attached
}

/*
  ISR water pulse, alternate (select via W,w) will detach during excessive vibration
*/
void WaterTrigger1_ISR()
{
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << WATERSENSOR_READ);  // 26mar21 Ptro done at start as per advice espressif
  waterTriggerCnt++ ;             // increase call counter
  long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
  waterTriggerTime  = time;       // set time of this read

  if ( (waterTriggerCnt) > 200 ) {
    // Input went or is invibration, disconnect ourselves a couuple a milliseconds
    // detachInterrupt(WATERSENSOR_READ); // trigger at every change
    detachWaterInterrupt();
    waterTriggerCnt = 1;          // indicate ISR has been withdrawn
    // waterTriggerTime  = waterTriggerTime + 10000; // forward our interrupt 10mS (disabled: now done in mainloop)
  }
  waterTriggerState = digitalRead(WATERSENSOR_READ); // read unstable watersensor pin
  if (loopbackRx2Tx2) digitalWrite(SERIAL_TX2, waterTriggerState); // here test trigger to find if it is attached

}

/* 20mar21 ISR read water sensor on default pin grpio4 and respect debouncetime 40mSec */
/*
  void WaterTrigger2()    // routine is to excessive and does not operate correctly
  {
  long time = micros();                // current counter µSec ; Debounce is wait timer to achieve stability
  long debounce_time = time - waterTriggerTime;  // µSec - previous_time in micro seconds
  if (debounce_time > waterReadDebounce * 1000) { // debounce_time (mSec
      waterTriggerTime = time;        //
      waterReadState   = digitalRead(WATERSENSOR_READ); // read watersensor pin
      if (!waterReadState) waterReadCounter++;        // count falldowns
  }
  }
*/

/*
  Convert device id to HEX String
*/
String GetAddressToString(DeviceAddress deviceAddress) {
  String str = "";
  for (uint8_t i = 0; i < 8; i++) {
    if ( deviceAddress[i] < 16 ) str += String(0, HEX);
    str += String(deviceAddress[i], HEX);
  }
  return str;
}

/*
   Need a local yield that that calls yield() and mqttClient.loop()
   The system yield() routine does not call mqttClient.loop()
*/
void mqtt_local_yield()   // added V21 as regular yeield does not call Pubsubclient
{
    // Allow / Do (likely) superfluous control to esp8266 routines
  yield();        // give control to wifi management
  // delay(50);      // delay 50 mSec
  ESP.wdtFeed();  // feed the hungry timer
  if (client.connected()) client.loop();  // feed the mqtt client
}



// useless data to be kept for reference
/*
 * // lwip lower memory no features
        Executable segment sizes:
        RODATA : 4500  ) / 81920 - constants             (global, static) in RAM/HEAP 
        IRAM   : 29036   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
        IROM   : 289728          - code in flash         (default or ICACHE_FLASH_ATTR) 
        DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP 
        BSS    : 28184 )         - zeroed variables      (global, static) in RAM/HEAP 
        Sketch uses 324608 bytes (31%) of program storage space. Maximum is 1044464 bytes.
        Global variables use 34028 bytes (41%) of dynamic memory, leaving 47892 bytes for local variables. Maximum is 81920 bytes.
  // lwip lower memory no features
        Executable segment sizes:
        BSS    : 28248 )         - zeroed variables      (global, static) in RAM/HEAP 
        IRAM   : 29036   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
        DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP 
        RODATA : 4508  ) / 81920 - constants             (global, static) in RAM/HEAP 
        IROM   : 299792          - code in flash         (default or ICACHE_FLASH_ATTR) 
        Sketch uses 334680 bytes (32%) of program storage space. Maximum is 1044464 bytes.
        Global variables use 34100 bytes (41%) of dynamic memory, leaving 47820 bytes for local variables. Maximum is 81920 bytes.        
  // removed httpclient
        Executable segment sizes:
        IROM   : 299792          - code in flash         (default or ICACHE_FLASH_ATTR) 
        BSS    : 28248 )         - zeroed variables      (global, static) in RAM/HEAP 
        RODATA : 4508  ) / 81920 - constants             (global, static) in RAM/HEAP 
        DATA   : 1344  )         - initialized variables (global, static) in RAM/HEAP 
        IRAM   : 29036   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
        Sketch uses 334680 bytes (32%) of program storage space. Maximum is 1044464 bytes.
        Global variables use 34100 bytes (41%) of dynamic memory, leaving 47820 bytes for local variables. Maximum is 81920 bytes.           
        
 */
