#define TEST_MODE       // set for Arduino to prevent default production compilation
#define DEBUG_ESP_OTA   // v49 wifi restart issues 
/*  documentation 
    https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
    api reference: https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
    sdk: https://github.com/espressif/ESP8266_NONOS_SDK/tree/release/v2.2.x
                  https://espressif.com/sites/default/files/documentation/2a-esp8266-sdk_getting_started_guide_en.pdf
*/

/* tbd 
  cleanout no longer needed code
  error: 06jul25 13u00 
      ESP8266-ResetReason: Hardware Watchdog
      Found Temp ghost device at 5 but could not detect address.
      ESP8266-ResetReason: Software Watchdog
      Crash after line: --> ESP8266-chip-size: 4194304 , ESP8#'+/37;?BFJNRVZ^bfjnrvz
      (ESP8266-sdk-version: 2.2.1(cfd48f3) )
  tbd: power protection
        -- check excessive power usage
  tbd: smart thermostat
      controls heat based on own thermo couple
          -- learn feedback time
  tbd: feed check, 
      veriy we have commands on a daily basis
  tbd: activate antifreeze protection 
      for 1 hour when any of the temperature below average threshold 10-12°C during > 15 minutes
      check if heat is produced by monitoring warmtelink
  tbd: alarm flashing blueleds ??
      design code to alternate flash
  tbd: read/write countersmqtt: Sents (C__) Misses (Cxx) Fails (Z_) Recovers (R)
      we have a miss if the P1 RX premature  ends < 340µSec, usually <= 86.016
      we have a fail if CRC does not match
      we have a Recver if we coull reconstruct CRC
      we have an OK if none of the above and we output things 

*/

/* change history
  V52 13jul25: restart information display
     protection "suspend @getValuesFromP1Record while mqttCnt < 5" unneeded, now commented
    'e0'  = divide error
    'e1'  = infinite loop fault
    'E'   = Enforce read fault in Reading P1 data
    testing serial processing on stable 2.4.1 and less stable 2.7.1
  V52 06jul25: restart information display
    command 'S'/'s' serialStopP1 serialStopRX2 (prohibit serial data)
    RX_yieldcount ignore serial if too many failures
    print & save restart reason information en report this at first mqttCnt into mqtt Errortopic
    format localIp() address to string for display 'D', can also IPAddress just as an array of 4 ints.
    diagnose state display using @-signal loop , &-got rx2 record
    DEBUG_PRINTxx macros
    counters for reads, misses, rx2, masks
    relocated H/h test command_testH1 command_testH2 routines for inline casting etc.etc.
    added and removed comments
  V51 03jul25: skipped 50 trying to centralise p1mqtt which irratically corrupts things
    added RX2 check availabiluy, we must have 1 read overy 7 mqttcnt record, else error:004 with successcount (bGot)
    added verboseLevel (int  verboseLeve) do minimize data, default as it was, we van incease by command 'v'
    made 'L' switch a toggle, 'l' will alsway go to off
    using snprint(... with converting format to const* using formatdata.c_str())
    assisted by deepseek  Serial.println(String("Value: " + analogRead(A0)).c_str());
    hereafter removed/ commented out lots of no longer needed casting/toarry (amongst search key // rm)
    skipped v50/v50a version as w.i.p.
  V49 29jun25: flashed and take into production on MAC: 60:01:94:7b:7c:2a
    wifi reboot loop issues, played around with WiFi.persistent(true); WiFi.persistent(false);
    replaced faulty device that had a blown schottky power diode 
    Add code in to check valid processTemperatures() reading to prevent -127C reports
    log to mqtt /error/... topic for failing sensors (p1-rj11, adc/light, temperature)
  V48 05jun25: improve code style after inpecting
    casting fix reinterpret_cast<unsigned char*>(telegram)
    define unsigned literal using  unsigned char tempLiteral[] = {0x0A};
    Fix some outbound array risks
    add '?'on mqtt command for help
    add 'M/m' mqtt command to print masking and masked P1 record
  V47 14may25: bugfix
    Waterswitch sometimes stops when looptimer to check debounce transitioned to 0 after 70 minutes
    WaterTrigger_ISR renamed to WaterTrigger0_ISR
    Trigger routine made single threading
    solved flaw1: will not process watercount during serial read.
    solved issue: sometimes watercount is not increasing when tapping, TBD: interloop check Gpio5 before and after
  V46 23feb25:
    27feb25 : ignore zero fields that (prefix field !) that should always > 0 (Like total Power/Heat/Flow)
    Either Flow or Heat is reported (occupiy the samen telegram record)
    modification to include Read GJ from warmtelink 0-1:24.2.1(250222224600W)(65.478*GJ)<| to HeatConsumption
    Note: 0-1:24.2.1(230227190235W)(84.108*m3) was moved to HeatFlowConsumption (v39)
  v45 Jan2025: 
    start to compare difference for recovery crc
    improved protection by not using BLUE-LED2 (gpio2) for debug when softwarematic rs232/Tx2Function is used
    debugging rs232 use keytags (mskio /recovered characters  , R_ /recovered record)
    implemented masked recovery using collected state of fixed positions.
      reading reliability improved from approx 88% to 97%
    mqttServer1 --> using coded  mqttServer or fallback to mqttServer2
    tested IP address through dns by switching off setting P1_STATIC_IP
    commented superfluous stability code
    telegram_crcIn & telegram_crcOut in , source line 2800
    add crcIn16() routine in order to have created a full datarecord for crcRecovery
    done some straighlining
    FYI:PLATFORM: Espressif 8266 (1.7.0) > NodeMCU 1.0 (ESP-12E Module)
        PACKAGES: 
        - framework-arduinoespressif8266 @ 1.20401.3 (2.4.1) 
        - tool-esptool @ 1.413.0 (4.13) 
        - tool-esptoolpy @ 1.30000.201119 (3.0.0) 
        - toolchain-xtensa @ 2.40802.200502 (4.8.2)
        Dependency Graph
        |-- ArduinoOTA @ 1.0
        |-- DallasTemperature @ 3.9.1
        |-- ESP8266WiFi @ 1.0
        |-- PubSubClient @ 2.7
        |-- P1SoftwareSerial @ 2.4.1
        RAM:   [=====     ]  53.7% (used 43988 bytes from 81920 bytes)
        Flash: [=         ]   7.2% (used 303636 bytes from 4194304 bytes)
        Compression reduced firmware size by 72% (was 307776 bytes, now 220591 bytes)
  -------------------------------------------- comments to block v51
  V44 enhanced comments alignment
    array size for  rs232 increased to 1024 bytes.
    display at command 'F' the toggle rx2_function (renamed from v38_function) statusfunction
    reconnect(0 renamed to mqtt_reconnect()
    add FindCharInArrayFwd to do forward search (corrected)
    loop check yield1500 changed tigger from 1.030 second to 1.080 seconds
  v43 removed 2 stability bytes, line 2215, readded+1 as this looks to be needed to improve 1/30 (more C then Z)
    Compression reduced firmware size by 72% (was 305008 bytes, now 218719 bytes)
    removal superfluous increased error rate from 1/12 to 1/7
  v42 added WiFi.setSleepMode(WIFI_NONE_SLEEP); // trying to hget wifi stable see https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/generic-class.html
  v41 slightly modify water intewrrupt to reset status to LOW at exceeding trigger count
  v40 improve WarmteLink detection, sometimes skipped....
  v39 LGTM collect data value from RX2/P1 & output to mqtt , 17mar23 stabilised
  v38 rx2_function false LGTM stabilized by inter/procedure bytes
  ------------------------------------------------------------------------------------------------------------------
  require  core 2.4.1 , NodeMCU 1.0 ESP12E  @ 192.168.1.35, V2 lower memory, BuiltIn led (GPIO)16
          -- Arduino BoardsManager --> esp8266 --> ensure it has 2.4.1
    additional https://raw.githubusercontent.com/VSChina/azureiotdevkit_tools/master/package_azureboard_index.json,
      http://arduino.esp8266.com/stable/package_esp8266com_index.json,
      https://raw.githubusercontent.com/stm32duino/BoardManagerFiles/master/STM32/package_stm_index.json
*/

/* Water sensor logic: (doc v51)
  ------------------------------------------------------------------------------------------------------------------------------
    WaterTrigger0/1_ISR() attach/detachWaterInterrupt is activated in loop() during peiodes not reading serial1/2 data.
      With command 'W' we can test/switch between WaterTrigger0/1_ISR (used for development reasons)
      (tbd: With command 'w' we could change/force next pullup behavior, now active/low/high automatically)

    Using a wheel that senses mirrored Infrared: 1 pulse (going in --> going out) = 1 Liter
        We use active GPIO5 Interrupt Service Routine for the cycle below, where a segmented turn = liter
        An UP or Down is only valid if this continues to be the case for at least 50mS (v51, waterReadDebounce=50)
           (debounce measuremnt starts/resets in ISR and is monitored in in loop())
    then: in loop() If -->
      A) GPIO5=activeHigh to low, wheel sensor has as entered blink zone and shorts ActiveHigh to GND
            GPIO5 now goes goes L and we disable the pull resistor waiting for active-low  (mode Input)
      B) GPIO5=atciveLow to high, waited until wheelsensor left by pulling activeLow to VCC
            GPIO5 now goes H and we enable the pull resistor for ActiveHigh (mode Input PullUp)
  ------------------------------------------------------------------------------------------------------------------------------    
*/

/* Log errors v49 to topic /error/x1
  001 = RJ11 to P1 is not connected
  002 = Temperature sensort fault (< REQUIRED_DSB18B20_SENSORS of < 127C)
  003 = ADC lightlevel fault (< REQUIRED_ANALOG_ADC)
*/

// Todo1: safegurard routing if P1 expire often then 10-12 seconds... at 1sec rate routine is not honoured

// W A R N I N G : Note check port and nodemcu port before uploading using Arduino Framework
// see/for git: https://github.com/ptrooms/P1Meter
// Update $ git checkout -b v36 on https://github.com/ptrooms/P1Meter.git to correct watercnt errors 

#define P1_Override_Settings    // set in PlatoformIO ; override wifi/server settings
#define UseP1SoftSerialLIB   //  use the old but faster SoftwareSerial structure based on 2.4.1 and use P1serial verion to listen /header & !finish
#define RX2TX2LOOPBACK false  // OFF , ON:return Rx2Tx2 (loopback test) & TX2 = WaterTriggerread
#define P1_STATIC_IP       // if define we use Fixed-IP, else (undefined) we use dhcp
#define NoTx2Function         // Do no serial out on D4/Gpio2/Blueled2
#define REQUIRED_DSB18B20_SENSORS 6 // v49 miniomal required DS18B20 sensors
#define REQUIRED_ANALOG_ADC 10 // v49 minimal required lightlevel for LDR sensor

/*
  ensure in PlatformIO the set PROD_MODE priotizes the else TEST_MODE
*/
#ifdef PROD_MODE
  #ifdef TEST_MODE
    #warning "This PROD_MODE is set by platformio to override inline TEST_MODE version."
    #undef TEST_MODE
  #endif
#endif

#ifdef TEST_MODE
  #define P1_BAUDRATE  115100       // use this baudrate for the Test p1 serial connection, usb require e bit lower speed
  #define P1_BAUDRATE2 115100       // use this baudrate for the Test p1 serial connection, usb require e bit lower speed
#else
  // note: 115250 < baudrate < 115150 will increase errors and misreads
  #define P1_BAUDRATE  115200       // use this baudrate for the p1 serial connection, 115200 is de sweepspot
  #define P1_BAUDRATE2 115200       // use this baudrate for the p1 serial connection, 115200 is de sweepspot
#endif

#ifdef TEST_MODE
  #warning This is the TEST version, be informed
  #define P1_VERSION_TYPE "t1"      // "t1" for ident nodemcu-xx and other identification to seperate from production
  #define DEF_PROG_VERSION 1152.241 // current version (displayed in mqtt record)
      #define TEST_CALCULATE_TIMINGS    // experiment calculate in setup-() ome instruction sequences for cycle/uSec timing.
      #define TEST_PRINTF_FLOAT       // Test and verify vcorrectness of printing (and support) of prinf("num= %4.f.5 ", floa 
#else
  #warning This is the PRODUCTION version, be warned
  #define P1_VERSION_TYPE "p1"      // "p1" production
  #define DEF_PROG_VERSION 2152.241 //  current version (displayed in mqtt record)
#endif
// #define ARDUINO_<PROCESSOR-DESCRIPTOR>_<BOARDNAME>
// tbd: extern "C" {#include "user_interface.h"}  and: long chipId = system_get_chip_id();

// *
// * * * * * L O G  B O O K
// *
// 28nov24 21u50: v43 PlatformIO 1.7.0 (=core 2.4.1), removed 2 stability bytes
// 09jul23 14u53: v42 added WiFi.setSleepMode(WIFI_NONE_SLEEP); // trying to get wifi more stable
// 17mar23 15u37: v40 improve WL value detection as value is sometimes missed when OK record is on position 1
//    added char TranslateForPrint(char c); // to translate unprintable values <null>00_ nl=10| cr=13 val>127~
// 28feb23 22u00: v39 stabilize bytes and output retrieved P2 code to mqtt
//    Sketch uses 303800 bytes (29%) of program storage space. Maximum is 1044464 bytes.
//    Global variables use 41504 bytes (50%) of dynamic memory, leaving 40416 bytes for local variables. Maximum is 81920 bytes.
// 28feb23 03u29: v38 BLueLed re-assignment via command 'f' cycle: Waterpulse, HotWaterState, Off & P1-CrcFault
//    Sketch uses 303740 bytes (29%) of program storage space. Maximum is 1044464 bytes.
//    Global variables use 41500 bytes (50%) of dynamic memory, leaving 40420 bytes for local variables. Maximum is 81920 bytes
// 28feb23 02u45: v38 Reandomlty added slack bytes (dummyxx) to check if this stabilize RX1/Crc (not sure)
// 28feb23 01u55: v38 Added 3mS during loop to allow for  yield time delay if RX2 is used
// 27feb23 22u45: v38 Implement reading RX2 Warmelink using gpio4 attached to mySerial2/115K2-8N1
// 26feb23 22u19: v37 Disable TX2 serial function, , add loop-limit vor WatertriggerISR and defined BLUE_LED2 for Waterpuls
// 06feb23 13u53: v36 Corrected debounce time *100 to *1000 & restructured mainloop "WaterPulse trigger when debounce " 
//    Sketch uses 302548 bytes (28%) of program storage space. Maximum is 1044464 bytes.
//    Global variables use 41100 bytes (50%) of dynamic memory, leaving 40820 bytes for local variables. Maximum is 81920 bytes.
// 03okt22 23u11: v35 serial2 inverted slightly better
// 30sep22 22u26: v35 p1Baudrate2 115k2 for "warmtelink" 
// 25sep22 22u34: v34 test compile to ehck correctness chksum 0x2d csum 0x2d v614f7c32 Sketch uses 302312 bytes
// 28apr21 21u09: modified P1 adapted serial and put getCycleCountIram - used to calculate serial timing - into localised Iram of SoftwareSerial241-P1
//  Compiled on Arduino: p1-2133.24 getFullVersion:SDK:2.2.1(cfd48f3)/Core:2.4.1/lwIP:2.0.3(STABLE-2_0_3_RELEASE/glue:arduino-2.4.1) 
// 27apr21 23u38: getFullVersion:SDK:2.2.1(cfd48f3)/Core:2.4.1/lwIP:2.0.3(STABLE-2_0_3_RELEASE/glue:arduino-2.4.1)
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


/* 
   Note changing libarieslooks to be useless as only 2.4.1 seem to resist WDT reset and is 99% (Serial) reliable
        but we levae the oportunity to centrally control which specifx libary will be included/
        any library must be made available with(in) Arduino/esp8266 Boardmanager
*/
#define libuse241     // 297428/prod, 297412/test bytes, best version offering reliability and NO spontaneous wdt resets
// #define libuse242  // 301136 bytes, not very reliable, serial produces not many successes
// #define libuse250  // TBI 322320 bytes, not very trustfull, serial is interrupt and causes soft errors, not sure about wdt
// #define libuse252  // unreliable (in the past, not sure how i operates with adapted P1 softserial), UseNewSoftSerialLIB
// #define libuse241  // 297428 bytes, very stable, fast compile (note we use the softserial 2.4.1. adapted to p1), 
// #define libuse274  // unstable, the esp remains for at 10 to 35 minutes active and then goes into wdt, UseNewSoftSerialLIB
// #define libuse240  // very goed library, fast compile to use UseNewSoftSerialLIB

// - - - > following was for test experimenting with library use directived. Can be deleted.
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

/*
  Define debug using Macros https://forum.arduino.cc/t/sketch-espconn-error/1028011
    DEBUG_PRINTLN("ESPConn - Looks not connected.");
    DEBUG_PRINT("Sending: ");
    str = esp.readStringUntil(':'); DEBUG_PRINT(str);
    DEBUG_WRITE_BYTES(data, length); DEBUG_PRINTLN();
*/
#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_WRITE_BYTES(x, y) Serial.write(x, y)
  #define DEBUG_WRITE(x) Serial.write(x)
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_WRITE_BYTES(x, y)
  #define DEBUG_WRITE(x)
#endif


// communication mode settings, is the Startbit RISING edge or (inverted) FALLING edge
#ifdef TEST_MODE
bool bSERIAL_INVERT = false;  // Simulatated P1 meter is USB interface of a PC to GPIO, does not required invert
#else
bool bSERIAL_INVERT = true;  // Direct P1 GPIO connection require inverted serial levels (TRUE) for RS232
#endif

// #define bSERIAL2_INVERT false // GJ meter is as far as we  know normal  serial (FALSE) RS232  < 03okt22
// #define bSERIAL2_INVERT true // GJ meter is as far as we  know normal (FALSE) RS232
#define bSERIAL2_INVERT true // GJ meter is as far as we  know normal  serial (FALSE) RS232 @direct p1 03okt22


// hardware PIN settings, change accordingly , total esp8266 pins 17
#define BLUE_LED         D0  // pin GPIO16 output to BLUE signal indcator
#define WATERSENSOR_READ D1  // pin GPIO5  input  for watermeter input (require 330mS debouncing)
#define SERIAL_RX2       D2  // pin GPIO4  input  for SoftwareSerial RX  GJ
#define DS18B20_SENSOR   D3  // pin GPIO0 Pin where DS18B20 sensors are mounted (High@Boot)

#ifdef NoTx2Function
  #define SERIAL_TX2       -1 // pin GPIO2 Secondary TX GJ  passive High@Boot (parallel 3k3 with 270R-opto)
#else
  #define SERIAL_TX2       D4 // pin GPIO2 Secondary TX GJ  passive High@Boot (parallel 3k3 with 270R-opto)
#endif

#define BLUE_LED2        D4  // pin GPIO2 
#define SERIAL_RX        D5  // pin GPIO14 input  for SoftwareSerial RX P1 Port
#define LIGHT_READ       D6  // pin GPIO12 input  for LDR read (passive lowR = LOW)
#define THERMOSTAT_READ  D7  // pin GPIO13 input  for button read in (via relay)
#define THERMOSTAT_WRITE D8  // pin GPIO15 output for button write out VALVE (via relay)
#define ANALOG_IN        A0  // pin gpio17 ADC    for Lightlevel LDR 
// D9   = 2;
// D10  = 2;

const int  prog_Version = DEF_PROG_VERSION;  // added ptro 2021 version

#ifndef ARDUINO_ESP8266_RELEASE 
  /*
   2.4.1: file /home/pafoxp/.platformio/packages/framework-arduinoespressif8266@1.20401.3/cores/esp8266/core_version.h
     main file:   [/home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/core_esp8266_main.cpp]
        static void loop_wrapper() --> setup() else loop(); run_scheduled_functions(); esp_schedule()
        post_mortem at crash: [/home/pafoxp/.platformio/packages/framework-arduinoespressif8266@1.20401.3/cores/esp8266/core_esp8266_postmortem.c]


  note: file   [/home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/core_version.h] contains 
  */ 
  #include <core_version.h>
#endif  
// const const char *core_version_P1Meter_Release = ARDUINO_ESP8266_RELEASE ; 

// Standard includes for Wifi, OTA, ds18b20 temperature
#include <ESP8266WiFi.h>           // standard, from standard Arduino/esp8266 platform

const char *hostName = "nodemcu"        P1_VERSION_TYPE;    // Note our hostname nodemcup1 nodemcut1
const char *mqttClientName = "nodemcu-" P1_VERSION_TYPE;    // Note our ClientId nodemcu-p1 nodemcu-t1
const char *mqttTopic      = "/energy/" P1_VERSION_TYPE;            // 'P' on/off outputMqttPower
const char *mqttLogTopic   = "/log/"    P1_VERSION_TYPE;            // 'L' on/off outputMqttLog
const char *mqttErrorTopic = "/error/"  P1_VERSION_TYPE;            // The error topic will also log into serverlog
const char *mqttPower      = "/energy/" P1_VERSION_TYPE "power";    // 'p' on/off outputMqttPower2
const int   mqttPort = 1883;
const char *mqttLogTopic2  = "/log/wl"  P1_VERSION_TYPE;            // 'l' on/off outputMqttLog for RX2 data
#ifdef P1_Override_Settings      // include our P1_Override_Settings
  #include "P1OverrideSettings.h"   // which contains our privacy/site related setting 
  // #warning Using override settings
#else
  #warning Using default settings, please use override for settings
  #ifdef TEST_MODE                  // Note: we use the override file at compile
    const char *ssid = "Production ssid";    // "Pafo SSID4"    //  T E  S T   setting
    const char *password = "wifipassword";
    const char *mqttServer = "192.168.1.8";                    // our mqtt server adress
    const char *mqttServer2 = "192.168.1.88";                  // our backup test mqtt server adress (desk top test)
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
    const char *mqttServer  = "192.168.1.254";                        // our mqtt server address
    const char *mqttServer2 = "192.168.1.253";                        // our backup mqtt server address
    #ifdef P1_STATIC_IP
      IPAddress local_IP(192, 168, 1, 35);
      IPAddress gateway(192, 168, 1, 1);
      IPAddress subnet(255, 255, 255, 0);
      IPAddress primaryDNS(192, 168, 1, 8);   //optional
      IPAddress secondaryDNS(192, 168, 1, 1); //optional
    #endif    
  #endif  // TEST_MODE 
#endif  // P1_Override_Settings

/*
  save restart register for later display / diagnose
*/
uint32 save_reason, save_epc1, save_epc2, save_epc3, save_excvaddr, save_depc;       // v52: save restart reason

// administrative timers for loop, check to limit  risks for WDT resets
unsigned long currentMillis  = 0; // millis() mainloop set at start
unsigned long currentMicros  = 0; // micros() mainloop set at start
unsigned long previousMicros = 0; // micros() mainloop set at finish V47
unsigned long startMicros    = 0; // micros() time readTelegram()

// used to research and find position of wdt resets
unsigned long test_WdtTime = 0;   // time the mainloop
unsigned long loopcnt = 0;        // count the loop

// control the informative led within the loop
unsigned long previousBlinkMillis = 0; // used to shortblink the BLUELED, at serialinput this is set to high value
const long    intervalBlinkMillis = 250;

// CRC16/arc redundancy check (data including between / and ! of P1 Telegram )
unsigned int currentCRC = 0;    // add CRC routine to calculate CRC of data
#include "CRC16.h"              // CRC-16/ARC

// #include <exception>          // added to allow/throw exceptions 2021-04-28 00:34:20 , not used as I do not know yet how to force an exception

// start regular program soruce segment
#include <PubSubClient.h>   // mqtt doc: https://github.com/knolleary/pubsubclient
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
//                                    // Arduino    /home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/libraries/ESP8266WiFi
//                                    // Platformio /home/pafoxp/.platformio/packages/framework-arduinoespressif8266@1.20401.3/libraries/ESP8266WiFi

// #include <ESP8266mDNS.h>        // standard, ESP8266 Multicast DNS req. ESP8266WiFi AND needed for dynamic OTA
// #include <ESP8266HTTPClient.h>   // not needed
// #include <WiFiUdp.h>             // not needed

#include <ArduinoOTA.h>           // standard, will also automgically include mDNS & UDP

// ================================================================ Temperature processing ===============
// #include <OneWire.h>           // <DallasTemperature.h will ask/include this themselves
#include <DallasTemperature.h>    // DS18B20 temporatur Onewire     
//------------------------------------------------------------------------------------------------ DS18B20
#define ONE_WIRE_BUS DS18B20_SENSOR       // GPIO0 Pin D3 to which is attached a temperature sensor 
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
int intervalP1cnt =  360;     // count * : = of maximum 72 minutes (360*12)  will increase at each success until 144minutes
int loopTelegram2cnt = 6;     // Maximum of readtelegram2 on 2nd RX port before we  flsh and doe force regular p1 again.
bool forceCheckData = false;  // force check on data in order to output some essential mqtt (Thermostat & Temprature) functions.
bool firstRun = true;         // do not use forceCheckData (which might be activated during setup-() or first loop-() )

// Tracks the time since last event fired between serial reads
unsigned long previousMillis = 0;      // this checks the functional loop
unsigned long previousP1_Millis = 0;   // this controls the inner P1_serial loop (halfway, abort-rx2)
unsigned long previousLoop_Millis = 0; // this tracks the main loop-() < 1080mSec  (hardcoded)

//for debugging, outputs info to serial port and mqtt function 'f' will cycle
bool blue_led2_Water = false;    // Use blue_led2 to indicate Watertrigger
bool blue_led2_HotWater = false; // Use blue_led2 to indicate Hotstate
bool blue_led2_Crc = true;       // Use blue_led2 to indicate valid Crc

bool allowOtherActivities = true; // allow other activities if not reading serial P1 port
bool p1SerialActive   = false;    // start program with inactive P1 port
bool p1SerialFinish   = false;    // transaction finished (at end of !xxxx )

long p1Baudrate  = P1_BAUDRATE;   //  V31 2021-05-05 22:13:25: set programmatic speed which can be influenced by teh bB command.
long p1Baudrate2 = P1_BAUDRATE2;   //  V35 2022-09-30 22:25:25: set programmatic speed which can be influenced by teh bB command.
long p1TriggerDebounce = 1000;   //  1000 mSeconds between yields while tapping water, which may bounce
long p1TriggerTime    = 0;       // initialise for Timestamp when P1 is active with serial communication

int  telegramError    = false;   // indicate the P1 Telegram contains non-printable data ((e=...)
// -------------------------------------------------------------------------------------------------- DEBUG output

#define VERBOSE_MAX  6    // maximum after we cylced to 0
#define VERBOSE_CORE 6    // Do not process any serial
#define VERBOSE_MQTT 5    // print all below and MQTT
#define VERBOSE_P1   4    // print all below and P1
#define VERBOSE_RX2  3    // print all below and RX2 Warmtelink
#define VERBOSE_GPIO 2    // print all below and GPIO with interrupts
#define VERBOSE_ON   1    // tbd
#define VERBOSE_OFF  0    // tbd
#ifdef TEST_MODE
  bool outputOnSerial  = true;    // "D" debug default output in Testmode
  int  verboseLevel    = VERBOSE_MQTT;
#else
  bool outputOnSerial  = false;   // "D" No debug default output in production
  int  verboseLevel    = VERBOSE_P1;       // Verbose level, all messages
#endif


// bool outputOnSerial  = true;    // "D" debug default output in Testmode
// ---------------------------------------------------------------------------------------------------

bool useWaterTrigger1 = false;   // 'W" Use standard WaterTrigger or (on) WaterTrigger1 ISR routine,
bool useWaterPullUp   = false;   // 'w' Use external (default) or  internal pullup for Wattersensor readpin
bool loopbackRx2Tx2   = RX2TX2LOOPBACK; // 'T' Testloopback RX2 to TX2 (OFF, ON is also WaterState to TX2 port)
bool outputMqttLog    = false;   // "l" false -> true , output to /log/p1
bool outputMqttPower  = true;    // "P" true  -> false , output to /energy/p1
bool outputMqttPower2 = true;    // "p" true  -> false , output to /energy/p1power
bool rx2_function     = true;    // "F" true  -> false , use v38 RX2 processing (27feb23 tested, LGTM)
bool outputMqttLog2   = false;   // "L" false -> true , output RX2 data mqttLogRX2 /log/wl2
bool serialStopP1     = false;   // 'S' stop read/inputting serial1 P1 data  (v52)
bool serialStopRX2    = false;   // 's' stop read/inputting serial2 RX2 data (v52)
long rx2ReadInterval  = 7;       // 's' decreases also de readinterval
bool doReadAnalog     = true;    // 'a' yes/no read analog port for value
bool doForceFaultP1   = false;   // 'E' yes/no force fault in Read telegram

// Vars to store meter readings & statistics
bool mqttP1Published = false;        //flag to check if we have published
long mqttCnt = 0;                    // updated with each publish as of 19nov19
long p1MissingCnt = 0;         // v52 updated when we failed to read any P1
long p1CrcFailCnt = 0;         // v52 updated when we Crc failed
long p1RecoverCnt = 0;         // v52 updated when we successfully recovered P1
long p1FailRxCnt  = 0;         // v52 updated P1 does not seem to be connnected
long p1ReadRxCnt  = 0;         // v52 updated P1 has read an ok Crc

long RX_yieldcount = 3 ;    // v52 if we have  succesive yields to 0 hold reading RJ11 5 times to allow OTA

bool validTelegramCRCFound  = false;     // Set by DdecodeTelegram if Message CRC = valid and send with Mqtt
bool validTelegram2CRCFound = false;    // v46 Set by readTelegram2 to check validity of RX2 record
// P1smartmeter timestate messages
long currentTime   =  210402;        // Meter reading Electrics - datetime  0-0:1.0.0(180611210402S)
char currentTimeS[] = "210401";      // same in String format time, will be overriden by Telegrams
const char dummy5[] = {0x0000};    // prevent overwrite

// v45 below values from full record 
long currentTime2    =  210402;       // Meter reading Electrics - datetime  0-0:1.0.0(180611210402S)
char currentTimeS2[] = "210401";      // same in String format time, will be overriden by Telegrams
const char dummy6[]  = {0x0000};      // prevent overwrite
long powerConsumptionLowTariff2  = 0; // Meter reading Electrics - consumption low tariff in watt hours
long powerConsumptionHighTariff2 = 0; // Meter reading Electrics - consumption high tariff  in watt hours
long powerProductionLowTariff2   = 0; // Meter reading Electrics - return low tariff  in watt hours
long powerProductionHighTariff2  = 0; // Meter reading Electrics - return high tariff  in watt hours
long CurrentPowerConsumption2    = 0; // Meter reading Electrics - Actual consumption in watts
long CurrentPowerProduction2     = 0; // Meter reading Electrics - Actual return in watts

long powerConsumptionLowTariff  = 0; // Meter reading Electrics - consumption low tariff in watt hours
long powerConsumptionHighTariff = 0; // Meter reading Electrics - consumption high tariff  in watt hours
long powerProductionLowTariff  = 0;  // Meter reading Electrics - return low tariff  in watt hours
long powerProductionHighTariff = 0;  // Meter reading Electrics - return high tariff  in watt hours
long CurrentPowerConsumption = 0;    // Meter reading Electrics - Actual consumption in watts
long CurrentPowerProduction  = 0;    // Meter reading Electrics - Actual return in watts
long GasConsumption = 0;             // Meter reading Gas in m3
long HeatFlowConsumption = 0;        // v39 Meter reading P2 in m3 - value from RX2 Warmtelinkl
long HeatConsumption    = 0;            // v46 Meter reading P2 in MJ - tbd value from RX2 Warmtelinkl
long HeatConsumptionOld = 0;            // v46 Meter reading P2 in MJ - tbd value from RX2 Warmtelinkl
long OldPowerConsumptionLowTariff  = 0; // previous reading
long OldPowerConsumptionHighTariff = 0; // previous reading
long OldPowerProductionLowTariff  = 0;  // previous reading
long OldPowerProductionHighTariff = 0;  // previous reading
long OldGasConsumption = 0;

bool thermostatReadState  = LOW;      // assume Input is active THERMOSTAT_READ
bool thermostatWriteState = LOW;      // Set initial command output THERMOSTAT_WRITE
bool lightReadState       = LOW;      // assume Input is active HEATING Light read

void WaterTrigger0_ISR(void) ICACHE_RAM_ATTR;  // store the ISR prod routine in cache
void WaterTrigger1_ISR(void) ICACHE_RAM_ATTR; // store the ISR test routine in cache
long waterTriggerCnt   = 0;   // initialize trigger count  0-ISRdetached , > 0 attached interrupt and counting
long debounce_time     = 0;   // v47 used in loop to check if things are stabilised
long waterDebounceCnt  = 0;   // administrate usage  for report
bool waterReadState    = LOW; // switchsetting
long waterReadDebounce = 50; // 200mS debounce, v47 set to 50
long waterReadCounter  = 0;   // nothing to read yet for regular water counter
long waterReadCounterPrevious  = 0;   // V47 to dsicriminate tapping in display
long waterReadHotCounter = 0; // nothing to read yet for HotWaterCounter
volatile unsigned long waterTriggerTime  = 0;   // initialise for ISR WaterTrigger()
volatile bool waterTriggerState = LOW; // switchsetting during trigger
volatile bool waterISRActive    = false; // V47: If ISR currently active this is High

volatile unsigned long      ISR_time = 0;  // V47 keep track of the timing of recent interrupts
volatile unsigned long last_ISR_time = 0;  // V47 keep track of the timing of recent interrupts
volatile unsigned long ISR_time_cnt  = 0;  // administrate usage for report

// used for analog read ADC port
int nowValueAdc = 0;      // actual read
int pastValueAdc = 0;     // previous read
int filteredValueAdc = 0; // average between previous and and published


//Infrastructure stuff
#define MQTTCOMMANDLENGTH 80          // define our receiving buffer
#define MQTTBUFFERLENGTH 480          // define our sending mqtt buffer
#define MAXLINELENGTH 1024           // full P1 serial message: the 589 byte message takes about 0.440 Seconds
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
const char dummy2a[] = {0x0000};    // prevent overwrite
char telegram[MAXLINELENGTH+32];       // telegram maxsize bytes for P1 meter
char telegramLast[3];               // used to catch P1meter line termination bracket
bool telegramP1header = false;      // used to trigger/signal Header window /KFM5KAIFA-METER
//DebugCRC int  testTelegramPos  = 0;          // where are we
//DebugCRC char testTelegram[MAXLINELENGTH];   // use to copy over processed telegram // ptro 31mar21

unsigned int dataInCRC  = 0;          // CRC routine to calculate CRC of data telegram_crcIn
int publishP1ToMqttCrc = 0;  // 0=failed, 1=OK, 2=recovered
bool validCrcInFound = false;         // Set by Decode when Full (recovered) datarecord has valid Crc
int  telegram_crcIn_rcv = 0;          // number of times DataIn could be recovered
int  telegram_crcIn_cnt = 0;          // number of times CrcIn was called
char telegram_crcIn[MAXLINELENGTH+32];   // active telegram that is checked for crc
int  telegram_crcIn_len = 0;          // length of this record
int  telegram_crcOut_cnt = 0;         // number of times masking was positioned
char telegram_crcOut[MAXLINELENGTH+32];  // processed telegram with changed positions to X
const char dummy2b[] = {0x0000};      // prevent save overwrite
// int  telegram_crcIn_cnt1 = 0;      // number of times CrcIn was called
int  telegram_crcOut_len = 0;         // length of this record
// int  telegram_crcIn_cnt2 = 0;      // number of times CrcIn was called

/* At header: initialise telegram_crcin
  during CRC we accumulate record positions into telegram_crcin
  At trailer:
    if telegram_crcIn_len = telegram_crcOut_len and crcValidCounter > 0
      if CRC=OK
        // Mark telegram_crcOut with X changes comparing same telegram_crcin
        for i=0 to len
          if telegram_crcOut(i) != X and telegram_crcOut(i) != telegram_crcIn(i)
          then telegram_crcOut(i) = X
               crcMaskCounter++      // count masked positions
        loop
        crcValidCounter++            // count valid CRC
      
      else:   // CRC failure on input, try to recover
        // try recover characters position: 
        for i=0 to len
          if telegram_crcOut(i) != X and telegram_crcOut(i) != telegram_crcIn(i)
          then  telegram_crcIn(i) = telegram_crcOut(i)
                crcRecoverCounter++
        loop
        if CRCOK telegram_crcIn
          we have recovered the record, 
          report recovery crcRecoverCounter++
          redo value extraction
        else:
        // recovery failed, nothng much we can do here.
        crcValidCounter--     // reduce valid CRC
    
    else: // different patatoe recordlength, reinitialise things
      if CRC=OK
        Copy telegram_crcin to telegram_crcout
        telegram_crcOut_len = telegram_crcIn_len
        for i=0 to len
          telegram_crcOut(i) = telegram_crcIn(i)
        loop
        crcValidCounter = 1   // start validity all over
        crcMaskCounter  = 0   // assume no masked positions
      
      else    // CRC failure on input and we have changed records
        // recovery failed, nothing to initialise & reduce validness
        crcValidCounter--     // reduce valid CRC

*/


// RX2 buffer on RX/TX Gpio4/2 , use a small buffer 128 as GJ meter read are on request
#define MAXLINELENGTH2 256
char telegram2[MAXLINELENGTH2+32]; // RX2 serial databuffer during outside P1 loop Plus overflow
// char telegramLast2[3];             // overflow used to catch line termination bracket
// char telegramLast2o[19];           // overflow-area to prevent memory leak
bool bGot_Telegram2Record = false;    // RX2 databuffer  between /header & !trailer
long Got_Telegram2Record_prev = 0;    // RX2 number of sucessfull RX2 records before loop
long Got_Telegram2Record_cnt  = 0;    // RX2 number of sucessfull RX2 records total loop
long Got_Telegram2Record_last = 0;    // mqttCnt last Telegram2Record received
const char dummy3a[] = {0x0000};      // prevent overwrite memoryleak
char telegram2Record[MAXLINELENGTH2+32]; // telegram extracted data maxsize for P1 RX2 
const char dummy4a[] = {0x0000};      // prevent overwrite memoryleak


// Callback mqtt value which is used to received data
char mqttServer1[64] ;               // v45 used to hold the currrently active mqttserver
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
SoftwareSerial mySerial( SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH); // (RX, TX. inverted, buffer)
SoftwareSerial mySerial2(SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2); // (RX, TX, noninverted, buffer)
#endif

// Wifi https://docs.arduino.cc/language-reference/en/functions/wifi/client/
/*
  https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/client-class.html
  WiFi library is very similar to the Ethernet library,
*/ 
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

#include <user_interface.h>     // v52: support to display/ger restart reasons
// rst_info *resetInfo;            // v52: pointer (global)

void setup()
{
  asm(".global _printf_float");            // include floating point support
  pinMode(BLUE_LED, OUTPUT);               // Declare Pin mode Builtin LED Blue (nodemcu-E12: GPIO16)

// struct rst_info *rtc_info = system_get_rst_info();
// Serial.printf(("reset reason: %x\n", rtc_info->reason);

//    os_printf("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x,depc=0x%08x\n",
//    rtc_info->epc1, rtc_info->epc2, rtc_info->epc3, rtc_info->excvaddr, rtc_info->depc);

rst_info *resetInfo;     // v52: ptr made global
resetInfo = ESP.getResetInfoPtr();  // v52: get information pointer
 
#ifdef NoTx2Function
  pinMode(BLUE_LED2, OUTPUT);             // v37 Declare Pin mode Builtin LED Blue (nodemcu-E12: GPIO2)
#endif


  // pinMode(THERMOSTAT_READ, INPUT);      // Declare Pin mode INPUT read always low
  pinMode(THERMOSTAT_READ, INPUT_PULLUP);  // Declare Pin mode INPUT with pullup to Read room thermostate
  pinMode(LIGHT_READ, INPUT_PULLUP);       // Declare Pin mode INPUT with pullup to read HotWater valve

  pinMode(WATERSENSOR_READ, INPUT_PULLUP); // Woggle the Input pin to read Waterpulsed infrared
  pinMode(WATERSENSOR_READ, INPUT);        // Declare Pin mode INPUT with no pullup

  pinMode(THERMOSTAT_WRITE, OUTPUT);       // Declare Pin mode OUTPUT to control Heat-Valve

  digitalWrite(BLUE_LED, LOW);             //Turn the LED ON  (active-low)
#ifdef NoTx2Function
  digitalWrite(BLUE_LED2, HIGH);           //Turn the LED OFF  (inactive-high)
#endif

  // Thermostats initialisation
  thermostatReadState   = digitalRead(THERMOSTAT_READ); // read current room state
  thermostatWriteState  = thermostatReadState;          // move this to output 
  digitalWrite(THERMOSTAT_WRITE, thermostatWriteState); // ThermostatWriteout valve (TBD: only when required)

  // Lightread initialisation
  lightReadState   = digitalRead(LIGHT_READ); // read Hotwater valve state

#ifdef NoTx2Function
  if (blue_led2_HotWater) digitalWrite(BLUE_LED2, lightReadState);  // debug readstate1
#endif
  
  // prepare wdt
  ESP.wdtDisable();
  // ESP.wdtEnable(WDTO_8S); // 8 seconds 0/15/30/60/120/250/500MS 1/2/4/8S
  ESP.wdtEnable(33000); // allow three passses missing

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


  // https://reference.arduino.cc/reference/en/libraries/wifi/wifi.config/
  #ifdef P1_STATIC_IP
    // Configures static IP address
    // WiFi.config(IPAddress(192,168,1,125), IPAddress(192,168,1,1), IPAddress(255,255,255,0));    
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.print((String)"STA Failed to configure: ");
    } else {
      Serial.print((String)"STA Using fixed address: ");
    }
    Serial.println(local_IP );
  #endif

  /*
    Added 2021-04-26 22:06:55 to test/check wifi issues
  */
  // WiFi.persistent(false); // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083) since 29jun25 causes bootloop
  WiFi.persistent(true); // Do not overwrites the FLASH if the settings are the same https://github.com/esp8266/Arduino/issues/1054#issuecomment-2662968960
  // WiFi.disconnect(true); // Delete SDK wifi config (after is has connected)
  delay(200);
  // WiFi.mode(WIFI_STA); // Disable AP mode
  // WiFi.setSleepMode(WIFI_MODEM_SLEEP); // Disable sleep (Esp8288/Arduino core and sdk default)
      
  Serial.println("Settingup WifiSTAtion.");  // wait 5 seconds before retry
  
  WiFi.mode(WIFI_STA);            // Client mode
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // 09jul23 try to get Wifi stable disable sleep

  Serial.println((String) "Connecting to " + ssid);  // wait 5 seconds before retry
  WiFi.begin(ssid, password);     // login to AP
  Serial.println((String) "Wifi Password sent");  // v49 added for debug
    
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print((String)"Connection to "+ ssid +", Failing ..");  // wait before retry
    for (int i = 0; i < 9; i++) {                       // while flashing led
      bool blueLedState = digitalRead(BLUE_LED);
      digitalWrite(BLUE_LED, !blueLedState);
      delay(250);
      digitalWrite(BLUE_LED, blueLedState);
      delay(250);
    }
    Serial.print((String) "Connection to "+ ssid +", Failed, restarting in 2 seconds..");  // wait 5 seconds before retry
    delay(2000);
    // WiFi.disconnect(true);   // ToTest 2021-04-26 22:30:51 still not erasing the ssid/pw. Will happily reconnect on next start
    // WiFi.begin("0","0");     // ToTest 2021-04-26 22:30:51 adding this effectively seems to erase the previous stored SSID/PW
    ESP.restart();
    // delay(1000);             // ToTest 2021-04-26 22:30:51 (read https://github.com/espressif/arduino-esp32/issues/400)
  }
  // print your WiFi shield's IP address:
  Serial.print((String)"Connected and booted " + hostName + ", using IP Address:") ;    // v45 fyi
  Serial.println(WiFi.localIP());

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
  Serial.println("ArduinoOTA.setHostname set" );
  
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
  Serial.println("ArduinoOTA.begin() activated." );
  
//    #define P1_VERSION_TYPE "t1"      // "t1" for ident nodemcu-xx and other identification to seperate from production
  // #define DEF_PROG_VERSION 1123.240
  ArduinoOTA.begin();

  Serial.printf("Reset reason code: %x\n", resetInfo->reason);     // v52: print restart reason
  Serial.println((String) "\n\rRestart time " + micros() + " for " + __FILE__ + " cause:" +  resetInfo->reason); // same but nicer
  Serial.printf("\t epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x,depc=0x%08x\r\n\n",
        resetInfo->epc1, resetInfo->epc2, resetInfo->epc3, resetInfo->excvaddr, resetInfo->depc); // v52: print registers

  save_reason   = resetInfo->reason;      // v52: save restart reason
  save_epc1     = resetInfo->epc1;        // v52: save restart register
  save_epc2     = resetInfo->epc2;
  save_epc3     = resetInfo->epc3;
  save_excvaddr = resetInfo->excvaddr;
  save_depc     = resetInfo->depc;

  Serial.println("Firmware version: "+ (String)P1_VERSION_TYPE + "-" + (String)DEF_PROG_VERSION+ "." );
  Serial.println ("ESP getFullVersion:" + ESP.getFullVersion());   
  Serial.println ((String)"\nArduino esp8266 core: "+ ARDUINO_ESP8266_RELEASE);  // from <core.version>
  // DNO:  Serial.println ((String)"LWIP_VERSION_MAJOR: "+ LWIP_VERSION_MAJOR);
  Serial.println ("IP address: " + String(WiFi.localIP().toString().c_str()) );  // v52: convert,  WiFi.localIP() is a reverse value
  Serial.println ("ESP8266-ResetReason: "+  String(ESP.getResetReason()));
  Serial.println ("ESP8266-free-space: "+   String(ESP.getFreeSketchSpace()));
  Serial.println ("ESP8266-sketch-size: "+  String(ESP.getSketchSize()));
  Serial.println ("ESP8266-sketch-md5: "+   String(ESP.getSketchMD5()));
  Serial.println ("ESP8266-chip-size: "+    String(ESP.getFlashChipRealSize()));
  Serial.println ("ESP8266-sdk-version: "+  String(ESP.getSdkVersion()));
  Serial.println ("ESP8266-getChipId: "+    String(ESP.getChipId()));             // sudden crash...
  Serial.println ("ESP8266-FreeHeap: "+     String(ESP.getFreeHeap()));

  WiFi.printDiag(Serial);   // print data  
  strcpy(mqttServer1,mqttServer );         // v45 initialise for reference
  client.setServer(mqttServer, mqttPort);  // v45 set mqtt server

  digitalWrite(BLUE_LED, LOW);   // Turn the LED ON and ready to go for process
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
  // read: https://en.cppreference.com/w/c/io/fprintf
  Serial.printf ("\r\tmqttCnt ValueD = %05d\n", mqttCnt);   // strange lont number 2681....8192.0
  Serial.printf ("\r\tmqttCnt Value4.1f = %4.1f\n", mqttCnt);   // strange lont number 2681....8192.0
  Serial.printf ("\r\t12.345 ValeUu2 = %3u_\n", 12.345);        // strange 3607772529_
  Serial.printf ("\r\t9.0453 Value4.2f = %4.2f\n", 9.0453);     // 9.05
  Serial.printf ("\r\t9.1 Value4.1f = %4.1f\n", 9.1);           //  9.1
  Serial.printf ("\r\t9.12340 Value5.1f = %5.2f\n", 9.12340);   //  9.12
  Serial.printf ("\r\t 9.5 Value4.1f = %6.1f\n", 9.5);          //    9.5
  Serial.printf ("\r\t1234.567 Value6.1f = %6.6f\n\r", 1234.567); // 1234.567000
#endif

#ifdef TEST_CALCULATE_TIMINGS

  unsigned long start3e = ESP.getCycleCount();  // initialize
  unsigned long start3f = ESP.getCycleCount();  // initialize
  unsigned long start3  = ESP.getCycleCount(); 
  start3  = ESP.getCycleCount()+1; 
  for (int i = 0; i < 100; i++) {
    asm(
      "NOP;"
    );
  }
  start3  = ESP.getCycleCount()+1; 
  for (int i = 0; i < 1; i++) {
    asm(
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
    );
  }
  start3e = ESP.getCycleCount()+1;  // delta between these two = 348 cycles
  for (int i = 0; i < 1; i++) {
    asm(
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
      "NOP;"
    );
  }
  start3f = ESP.getCycleCount()+1;  // delta between these two = 348 cycles
  // 10.000xnop1 vs nop2 Start3=2109430182, start3e=2109480889(nop1=50707), start3f=2109541239(nop2=60350) .
  // 10nops vs 20nops Start3=2782556574, start3e=2782556936(nop1=362), start3f=2782557306(nop2=370) . Only 8 cycles difference (compiler optimized?)
  Serial.println((String) "\n Start3=" + start3 + ", start3e=" + start3e + "(nop10=" + (start3e - start3) + ")" + ", start3f=" + start3f + "(nop20=" + (start3f - start3e) + ") .");

  // serial test timings
  // 8bitSStop TestRead4Rx start4=3666068548, end4=3666075579,  diff4=7031, wait4=7367, bittime4=694  = 87,94 µS = 10   bits
  // 8bitstart TestRead4Rx start4=3405964740, end4=3405970810,  diff4=6070, wait4=6673, bittime4=694  = 72,84 µS =  8.5 bits
  // 4bitSStop TestRead4Rx start4= 668982987, end4= 668987238,  diff4=4251, wait4=4591, bittime4=694
  // 1bitStart TestRead4Rx start4=4090473783, end4=4090475689,  diff4=1906, wait4=2509, bittime4=694
  // 1bitWait  TestRead4Rx start4=1430700974, end4=1430701833,  diff4=859,  wait4=1121, bittime4=694
  // 8bitOnly  TestRead4Rx start4=1597461092, end4=1597462357,  diff4=1265, wait4=427,  bittime4=694
  // 4bitOnly  TestRead4Rx start4=2339998248, end4=2339999243,  diff4=995,  wait4=427,  bittime4=694
  // 2BitOnly  TestRead4Rx start4=3978366322, end4=3978367168,  diff4=846,  wait4=427,  bittime4=694
  // 1bitOnly  TestRead4Rx start4=2601377338, end4=2601377762,  diff4=424,  wait4=427,  bittime4=694  (time to read 1 bit = 424 cycles)
  // noloop:   TestRead4Rx start4= 547644023, end4= 547644025,  diff4=2,    wait4=427,  bittime4=694
  // waitonly: TestRead4Rx start4= 625878073, end4= 625878512,  diff4=439,  wait4=1121, bittime4=694  (427+694=1121)

  // Setup our test skeleton
    #define WAITtest4  { while (ESP.getCycleCount()-start4 < wait4) if (!m_highSpeed4) optimistic_yield(1); wait4 += m_bitTime4; }
    Serial.println((String)"m-bitime 115K2=" + (ESP.getCpuFreqMHz() * 1000000 / 115200)); // = 694 cycles for 115K2@80Mhz
    int m_rxPin4 = SERIAL_RX;     // gpio14
    unsigned long speed4 = 115200;
    unsigned long m_bitTime4 = ESP.getCpuFreqMHz()*1000000/speed4;
    int m_buffSize4 = 66; // test
    bool m_overflow4 = false;
    bool m_P1active4 = false;
    bool m_highSpeed4 = true;
    // simulate the serial routine for 1byte   
    unsigned long wait4 = m_bitTime4 + m_bitTime4/3 - 498;		// offset 501=425 offset 498=427cycles 115k2@80MHz
    
    // below a 1:1 copy of the serial routine of plerup, this to check/simulate the time for. 
    unsigned long start4 = ESP.getCycleCount();
    uint8_t rec = 0;
    // WAITtest4;    // wait/skip startbit 427 cycles
    for (int i = 0; i < 8; i++) {  // this routine takes 424 cycles without any wait and for-test-consumes 12cyles
      WAITtest4; // while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
      rec >>= 1;
      if (digitalRead(m_rxPin4)) rec |= 0x80;
    }
    WAITtest4; // stopbit while (getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; 
    if (m_rxPin4) rec = ~rec;   // simulate the invert
    unsigned int m_inPos4 = 10;
    unsigned int m_outPos4 = 5;
    int next4 = (m_inPos4 +1) % m_buffSize4;  // (10+1) % 66 = 11
    if (next4 != m_outPos4) {
        if (rec == '/') m_P1active4 = true ;   // 26mar21 Ptro P1 messageing has started by header
        if (rec == '!') m_P1active4 = false; // 26mar21 Ptro P1 messageing has ended due valid trailer
        // m_buffer[m_inPos4] = rec;
        m_inPos4 = next4;
    } else {
        m_P1active4 = false;                   // 26mar21 Ptro P1 messageing has ended due overflow
        m_overflow4 = true;
    }
    
    unsigned long start4e = ESP.getCycleCount();
    Serial.println((String)" TestRead4Rx start4=" + start4 + ", end4="+ start4e 
            + ", diff4=" + (start4e-start4) + ", wait4=" + wait4 
            + ", bittime4=" + m_bitTime4 );   // v51: start4=1171498770, end4=1171504757, diff4=5987, wait4=6673, bittime4=694


    // calculate getCycleCnt
    wait4 = m_bitTime4 + m_bitTime4/3 - 498;
    start4 = ESP.getCycleCount(); 
    start4e = ESP.getCycleCount();  // delta between these two = 348 cycles
    // TestIramWait0 start4=1709979665, end4=1709979667, diff4=2, wait4=427, bittime4=694
    Serial.println((String)" TestIramWait0 start4=" + start4 + ", end4="+ start4e + ", diff4=" + (start4e-start4) + ", wait4=" + wait4 + ", bittime4=" + m_bitTime4 );
    start4 = ESP.getCycleCount();
    // while loop here takes 91 cycles
    { while (getCycleCountIramLocal()-start4 < wait4) if (!m_highSpeed4) optimistic_yield(1); wait4 += m_bitTime4; }
    start4e = ESP.getCycleCount();
    // TestIramWait1 start4=594504443, end4=594504882,   diff4=439, wait4=1121, bittime4=694
    Serial.println((String)" TestIramWait1 start4=" + start4 + ", end4="+ start4e + ", diff4=" + (start4e-start4) + ", wait4=" + wait4 + ", bittime4=" + m_bitTime4 );

    // Test inline assembled copy of GetCycleCount
    for (int i = 0; i < 3; i++) {
      wait4 = m_bitTime4 + m_bitTime4/3 - 498;
      start4 = getCycleCountIramLocal();
      // while loop here takes 91 cycles
      // { while (getCycleCountIramLocal()-start4 < wait4) if (!m_highSpeed4) optimistic_yield(1); wait4 += m_bitTime4; }
      start4e = getCycleCountIramLocal();
      // TestIramWait3 start4=1713581182, end4=1713581184, diff4=2, wait4=427, bittime4=69
      Serial.println((String)" TestIramWait3 start4=" + start4 + ", end4="+ start4e + ", diff4=" + (start4e-start4) + ", wait4=" + wait4 + ", bittime4=" + m_bitTime4 );
      start4 = getCycleCountIramLocal();
      { while (getCycleCountIramLocal()-start4 < wait4) if (!m_highSpeed4) optimistic_yield(1); wait4 += m_bitTime4; }
      start4e = getCycleCountIramLocal();
      // TestIramWait4 start4=1714171078, end4=1714171510, diff4=432, wait4=1121, bittime4=694
      Serial.println((String)" TestIramWait4 start4=" + start4 + ", end4="+ start4e + ", diff4=" + (start4e-start4) + ", wait4=" + wait4 + ", bittime4=" + m_bitTime4 );
      

      // getCycleCountIramLocal();   // 9 cycles
      // TestIramWait5 start4=541067741, end4=541068175, diff4=434, wait4=1121, bittime4=694
      // { while (getCycleCountIramLocal()-start4 < wait4) ; wait4 += m_bitTime4; }

      start4 = getCycleCountIramLocal();
      wait4 = start4 + (m_bitTime4 + m_bitTime4/3 - 498);
      { while (getCycleCountIramLocal() < wait4) ; wait4 += m_bitTime4; }
      start4e = getCycleCountIramLocal();
      // TestIramWait7 start4=3440482983, end4=3440483418, diff4=435, wait4=3440484104, bittime4=694
      Serial.println((String)" TestIramWait7 start4=" + start4 + ", end4="+ start4e + ", diff4=" + (start4e-start4) + ", wait4=" + wait4 + ", bittime4=" + m_bitTime4 );
     
    } // for loop
  
    wait4 = m_bitTime4 + m_bitTime4/3 - 498;
    start4 = ESP.getCycleCount();
    long test4 = 0;
    for (long i = 0; i < 46; i++) {  // predef10=61cycles, predef100=601cycles, i100=947 , i75=798 , i50=648, i47=630, i46=624
        // long test4 = ESP.getCycleCount(); // predef10=61cycles, predef100=601cycles, i100=947 , i75=798 , i50=648, i47=630, i46=624
        // long test4 = ESP.getCycleCount()+ESP.getCycleCount(); // add/sub@i46=715 , 
        // long test4 = ESP.getCycleCount()+ESP.getCycleCount()+ESP.getCycleCount(); // triple=808
        // long test4 = (ESP.getCycleCount()+ESP.getCycleCount())-(ESP.getCycleCount()+ESP.getCycleCount()); // quadro46 = 1245    
        test4 = (ESP.getCycleCount()+ESP.getCycleCount())-(ESP.getCycleCount()+ESP.getCycleCount()); // inlinequadro46 = 1245 
    }
    start4e = ESP.getCycleCount();
    // for (int i = 0; i < 1000; i++) = 348
    // Serial.println((String)" TestIramWait8 start4=" + start4 + ", end4="+ start4e + ", diff4=" + (start4e-start4) + ", test=" + test4 );
    // TestIramWait9 gc4-1=906115867, gc4-2=906120354, gc4-3=906124820, gc4-4=906128621
    Serial.println((String)" TestIramWait8 start4=" + start4 + ", end4="+ start4e + ", diff4=" + (start4e-start4) + ", test4=" + test4 );

    // diffs: 3801-4487-4466
    Serial.println((String)" TestIramWait9 gc4-1=" + ESP.getCycleCount() + ", gc4-2="+ ESP.getCycleCount() + ", gc4-3="+ ESP.getCycleCount() + ", gc4-4="+ ESP.getCycleCount());

    // insert a small loop to stabilize-routine
    for (int i = 0; i < 100; i++) {
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
  cli();   // hold interrupts ( or noInterrupts() )
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
  sei();   // resume interrupts   (or interrupts() )     // v51 correction
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

#endif  // TEST_CALCULATE_TIMINGS

  // intervalP1cnt =  10;        // initialise timeout count
  // intervalP1  = 30000;        // 30.000 mSec
  previousMillis    = millis();           // initialise previous interval
  previousP1_Millis = previousMillis;  // initialise previous interval
  // attachInterrupt(WATERSENSOR_READ, WaterTrigger_ISR, CHANGE); // trigger at every change
  // attachWaterInterrupt();

  waterTriggerTime = 0;  // ensure and assum no trigger yet
  test_WdtTime = 0;  // set first loop timer
  loopcnt = 0;              // set loopcount to 0
  Serial.print("\r\nfinish Setup()."); // exit loop to check if we have entered the the buulding
//  WiFi.printDiag(Serial);   // print data
} // setup


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
  //  WiFi.printDiag(Serial) 
  // Serial.printDiag(Wifi);
  // WiFi.printDiag(Serial);
*/
void loop()
{ 
  if (verboseLevel == 1) Serial.print("\b \b"); // exit loop to check if we have entered the the buulding
  // note this loop() routine is as of date v51 04jul25 approximately called 5769/sec.dry, without P1/RX2
  
  // declare global timers loop(s)
  // millis() and micros() return the number of milliseconds and microseconds elapsed after reset, respectively.
  //  warning: this number will overflow (go back to zero), after approximately 70 minutes.
  currentMillis = millis(); // Get snapshot of runtime
  previousMicros = currentMicros; // get V47 previous loop time
  currentMicros = micros(); // get current cycle time
  if (previousMicros >= currentMicros) {  // V47 cycle started where micros transitioned to 0
    waterTriggerTime = currentMicros;     // let/reset to Fuzzy waterTriggerTime
  }

  // below is to fix/make water pulse working which currently is intermittently spiking
  debounce_time = currentMicros - waterTriggerTime;    // µSec - waterTriggerTime in micro seconds set by last ISR waterTrigger
  if (debounce_time < 0 || waterTriggerTime < 1) {    // v47 over the 70 minutes or Debounce request, reset timer
      waterTriggerTime = currentMicros;
      debounce_time = 0;
  }
  
  // if (test_WdtTime < currentMillis and !outputOnSerial )  {  // print progress 
  if (test_WdtTime < currentMillis ) {
    loopcnt++ ;
    // Serial.print((String)"-" +(loopcnt%10)+" \b");
    Serial.print((String) ((waterReadCounter != waterReadCounterPrevious) ? "_" : "-") +(loopcnt%10)+" \b"); // v47 test water tapping 
    waterReadCounterPrevious = waterReadCounter; 
    test_WdtTime = currentMillis + 1000;  // next interval
  }

  // Following will trackdown loops on execeeding serial reads using timeouts RX_yieldcount
  if (currentMillis > (previousLoop_Millis + 1080) ) { // exceeding 1.08 second  ?, warn user
    // yield();
    if (RX_yieldcount > 0) RX_yieldcount-- ;     // v52 decrease successive yieldcount  with any or valid P1 read
    if (RX_yieldcount < 1)  RX_yieldcount = 8;   // v52 when < 1, pause P1 serial reads ubtuk yield is again < 4
    mqtt_local_yield();  // do a local yield with 50mS delay that also feeds Pubsubclient to prevent wdt
    if (outputOnSerial) {
      Serial.printf("\r\nLoop %6.3f exceeded#%d at prev %6.3f !!yield1080\n", 
          RX_yieldcount ,     // v52: print yield value
          ((float) currentMicros / 1000000), ((float) previousLoop_Millis / 1000));
    } else {
      if (RX_yieldcount > 3)  {
          Serial.print("@");  // signal a yieldloop '@'
      } else {
          Serial.print("#");  // v52: yield protection false reads
      }
    }
  }
  previousLoop_Millis = currentMillis;  // registrate our loop time

  // note do not use wifi.connected test in loop... it is not needed nor reuired per
  // https://stackoverflow.com/questions/49205073/soft-wdt-reset-esp8266-nodemcu
  if (!client.connected()) {     // this handles the mosquitto connection
    mqtt_reconnect();               // mqtt_reconnect Mqtt
    mqttP1Published = false;   // flag to check if we have published procssed data
    delay(1000);               // delay processing to prevent overflow on error messages
    Serial.println("\n #!!# ESP P1 mqtt reconnected."); // print message line
    // WiFi.printDiag(Serial);   // print data ESP8266WiFiClass::printDiag(Print& p)
  }

  mqtt_local_yield();      //   client.loop(); // handle mqtt

  //  ------------------------------------------------------------------------------------------------------------------------- START allowOtherActivities
  if (!allowOtherActivities) {     // are we outside P1 Telegram processing (which require serial-timed resources)
    if (waterTriggerCnt != 0) detachWaterInterrupt();
  } else {                         // else we are allowed to do other acivities
    // we are now outside P1 Telegram processing (which require serial-timed resources) and deactivated interrupts
    if (!p1SerialActive) {      // P1 (was) not yet active, start primary softserial to wait for P1

      if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + " serial started at " + currentMillis);
      p1SerialActive = !p1SerialActive ; // indicate we have started
      p1SerialFinish = false; // and let transaction finish
      mySerial2.end();          // Stop- if any - GJ communication
      mySerial2.flush();        // Clear GJ buffer
      telegramError = 0;        // start with no errors
      // Start secondary serial connection if not yet active
    #ifdef UseNewSoftSerialLIB
      // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
      mySerial.begin  (p1Baudrate, SWSERIAL_8N1, SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH, 0); // Note: Prod use require invert
      // mySerial2.begin (  1200,SWSERIAL_8N1,SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2,0);
    #else
      // mySerial.begin(P1_BAUDRATE);    // P1 meter port 115200 baud
      if (!serialStopP1) mySerial.begin(p1Baudrate);    // P1 meter port 115200 baud, v52 stop/start
      // mySerial2.begin(p1Baudrate2);  // GJ meter port   1200 baud     // required during test without P1
    #endif

    } else {    // if (!p1SerialActive)

      if (p1SerialFinish) {     // P1 transaction completed, we can start GJ serial operation at Serial2
        if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + " serial stopped at " + currentMillis);
        // if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + "." ); // v47 superfluous after preceding debug line
        // --> end of p1 read electricity
        // if (!outputOnSerial) Serial.print((String) "\t stopped:" + micros() + " ("+ (micros()-currentMicros) +")" + "\t");
        if (!outputOnSerial) Serial.printf("\t endP1: %11.6f (%6.0f)__\b\b\t", ((float)micros() / 1000000), ((float)micros() - startMicros));
        p1SerialFinish = !p1SerialFinish;   // reverse this
        p1SerialActive = true;  // ensure next loop sertial remains off
        mySerial.end();    // P1 meter port deactivated
        mySerial.flush();  // Clear P1 buffer
        loopTelegram2cnt = 0;  // allow readtelegram2  for a maximum of "6 loops" of actual receives

        // Only activate myserial2  atdesignated times
        // int checkIntervalRx2 = mqttCnt % 7;
        if ( rx2_function && (mqttCnt == 2  || (mqttCnt > 0 && ((mqttCnt % rx2ReadInterval) == 0)) ) ) {  // only use RX2 port at these intervals
          // Start secondary serial connection if not yet active
    #ifdef UseNewSoftSerialLIB
          // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
          // mySerial.begin  (P1_BAUDRATE,SWSERIAL_8N1,SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH,0); // Note: Prod use require invert
          if (!serialStopRX2) mySerial2.begin (p1Baudrate2, SWSERIAL_8N1, SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2, 0);
    #else
          // mySerial.begin(P1_BAUDRATE);    // P1 meter port 115200 baud
          if (!serialStopRX2) mySerial2.begin(p1Baudrate2);    // GJ meter port   1200 baud
    #endif
        }
        
        previousP1_Millis = currentMillis;  // indicate time we have stopped.
        previousMillis    = currentMillis;  // initialise our start counter for the overall proces.
      } else {  // p1SerialFinish

        // Serial are closed and not yet (re)started
        // check timer to set p1SerialActive to false in order to commence P1 data
        if ( (unsigned long)(currentMillis - previousP1_Millis) >= (intervalP1 / 2) ) {  // terminate serial2 and restart serial1 halfway
          p1SerialActive = false;   // activate serial in next round
          p1SerialFinish = false;  // and let it finish
          previousP1_Millis = currentMillis; // Use the snapshot to set track time to prepare next P1 serial event

        } // check  initiate serial read
      } // else p1SerialFinish
    } // else !!p1SerialActive

    ESP.wdtFeed();  // as advised by forums

    // Check for WaterPulse triggerstate interrupt when debounce has passed
    if ( debounce_time > (waterReadDebounce * 1000)) {  // check  (currentMicros - waterTriggerTime) in mS
      if (waterTriggerCnt == 0) {       // Activate interrupt if not yet active
          waterDebounceCnt++;           // administrate usage for report Dbc#
          attachWaterInterrupt();            
          waterTriggerTime = currentMicros;  // reset our trigger counter
          // waterTriggerCnt  = 2;              // Leave at 2 so the ISR routine can increase it to 3
      } else {
        if (waterTriggerTime != 0) {                     //  v47 set by current loop time
            // waterTriggerTime = currentMicros;            // reset ISR counter for next debouncec trigger
            if (waterReadState != waterTriggerState) {   // switchsetting) {  // read chanted switch state
               waterReadState = waterTriggerState ;       // stabilize switch
               if (waterReadState) {                      // accomodate/enforce stability (note: was inverted)
                  pinMode(WATERSENSOR_READ, INPUT_PULLUP); // Watersensor is Low, increase sensivity by pull-up
                  waterReadCounter++;                     // v47 count this as pulse if switch went LOW
                  if (!digitalRead(LIGHT_READ)) waterReadHotCounter++; // v47 count this as hotwater
                  if (outputOnSerial) Serial.println((String)"\nSet Gpio" + WATERSENSOR_READ + "=L INPUT_PULLUP");
                  waterTriggerTime = currentMicros;            // reset ISR counter for next debounced trigger
               } else {
                  pinMode(WATERSENSOR_READ, INPUT);        // Watersensor is High, weaken sensivity without pull-up
                  if (outputOnSerial) Serial.println((String)"\nSet Gpio" + WATERSENSOR_READ + "=H INPUT");
               }
            #ifdef NoTx2Function
               if (!loopbackRx2Tx2 && blue_led2_HotWater) digitalWrite(BLUE_LED2, !(digitalRead(LIGHT_READ)));  // debug readstate0
               // if (!loopbackRx2Tx2 && blue_led2_Water)    digitalWrite(BLUE_LED2, LOW); //Turn the LED OFF  (inactive-high)
            #endif

            }
        }
      }
    }      

    /*
    if ( (waterTriggerCnt == 1) && (debounce_time > waterReadDebounce * 1000)) {    // was 100 but we must accomodate mS
      // we are in hold read interrupt mode , re-establish interrupts
      waterDebounceCnt++;                // administrate usage  for report
      attachWaterInterrupt();
      waterTriggerTime = currentMicros;  // reset our trigger counter
      waterTriggerCnt  = 2;              // Leave at 2 so the ISR routine can increase it to 3
    } else {  // check if we must add a pulse counter
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
    }
    */
    
    // old dlocation of  cechking for incoming mqtt  commands

    // if (loopbackRx2Tx2) Serial.print("Rx2 "); // print message line
    if (loopTelegram2cnt < 6) readTelegram2();   // read RX2 GJ gpio4-input (if available), write gpio2 output
  }   // if-else allow other activities
  //  ------------------------------------------------------------------------------------------------------------------------- END of allowOtherActivities

  // readTelegram2();    // read RX2 GJ gpio4-input (if available), write gpio2 output (now done in closed setting)

  // (p1SerialActive && !p1SerialFinish) , process Telegram data
  
  if (RX_yieldcount < 4) { // assume all if well and we had of have surived any previous yieldcount
    readTelegram();     // read RX1 P1 gpio14 (if serial data available)
  }

  mqtt_local_yield();  // do a local yield with 50mS delay that also feeds Pubsubclient to prevent wdt
  ESP.wdtFeed();  // as advised by forums

  p1TriggerTime = millis();   // indicate our mainloop yield
  // check and prevail deadlock timeout
  // executed at no p1-message intervalP1<12 seconds, decrease reliability intervalP1cnt=360 until <1 which restarts; 
  if (previousMillis > p1TriggerTime) {  // V47 ensure time transition to 0 will not colide as failure.
    previousMillis = p1TriggerTime;
  } 

  if   ((unsigned long)(currentMillis - previousMillis) >= intervalP1) {  // is our deadlock check interval passed  12secs ?
    //  if (millis() > 600000) {  // autonomous restart every 5 minutes
    intervalP1cnt--;            // decrease reliability
    if ( intervalP1cnt < 1) {   // if we multiple or frequent misses, restart esp266 to survive
      if (outputMqttLog) publishMqtt(mqttLogTopic, (String) "ESP P1 rj11 not connected" );  // report we have survived this interval, v51 String'ed
      Serial.print("\n #!!# ESP P1 rj11 not connected, cnt="); // print message line
      Serial.println( intervalP1cnt );
      mqttP1Published = false;
    } else {    // reliabilitty intervalP1cnt=360>0
      p1FailRxCnt++;  // v52 count failed RJ11 connections
      // report to error mqtt, // V20 candidate for a callable error routine
      // tbd: consider to NOT send values
      // char mqttOutput[128]; // v51 not used as we do 
      String mqttMsg = "{";  // start of Json
      mqttMsg.concat("\"error\":001 ,\"msg\":\"P1 rj11 serial not reading\""); 
      // String mqttMsg = "Error, "; // build mqtt frame 
      // mqttMsg.concat("serial not connected ");  
      // mqttMsg,concat("interval="); // build mqtt frame 
      // char mqttCntS[8] = ""; ltoa(mqttCnt,mqttCntS,10); mqttMsg.concat(mqttCntS));
      mqttMsg.concat((String)", \"mqttCnt\":"+(mqttCnt+1));    // +1 to reflect the actual mqtt message
      mqttMsg.concat("}");  // end of json
      // mqttMsg.toCharArray(mqttOutput, 128);

      publishMqtt(mqttErrorTopic, mqttMsg); // report to /error/P1 topic
      if (outputMqttLog && !serialStopP1)
          publishMqtt(mqttLogTopic, "ESP P1 rj11 Active interval checking" );  // report we have survived this interval
      
      // Alway print this serious timeout failure
      // if (outputOnSerial) {
      if (!serialStopP1)
         Serial.printf("\n\r# !!# ESP P1 rj11 Active interval at %11.6f, checking %6d, timecP:%d, timec2:%d .\n\r",   
                          ((float)  micros() / 1000000), 
                          intervalP1cnt,
                          previousMillis,
                          currentMillis);
    /*    
      // ((float)ESP.getCycleCount()/80000000), (mqttCnt + 1), ((float) startMicros / 1000000));
      Serial.print("\n\r# !!# ESP P1 rj11 Active interval checking ");   // v45: add carriage-return
      Serial.print(intervalP1cnt );
      Serial.print(", timeP:");
      Serial.print(previousMillis);
      Serial.print(", time2:");
      Serial.print(currentMillis); // print message line
      Serial.println("."); // print message line
    */  

      // output on secondary  TX if not already used for loopback
    #ifndef NoTx2Function
      if (!loopbackRx2Tx2) mySerial2.println((String)"t=" + currentMillis + " ." );  // testoutput , disabled as of v37
    #endif
      // }

      forceCheckData = true;  // enforce  mode active
      validTelegramCRCFound  = false;   // Enforce we (might) have a CRC error
      processGpio();            // Do regular functions of the system
      publishP1ToMqtt();        // PUBLISH this mqtt
      forceCheckData = false; // enforce  mode inactive

      mqttP1Published = true;   // assume publish to prevent conditional restart if things are running here

      /*  // v52 disable as we do not want to wait on disconnections !!!!
      if (RX_yieldcount > 0) RX_yieldcount-- ;     // v52 decrease successive yieldcount  with any or valid P1 read
      if (RX_yieldcount < 1) RX_yieldcount = 8;   // v52 when < 1, pause P1 serial reads ubtuk yield is again < 4
      */
    } // else millis exceeded intervalP1 hile enforce mqtt for tempsensors

    // If no MQTT poublished in this timout interval, whatever the reason, execute a restart to reset
    if (!mqttP1Published) {  // in case no publish energy yet (client not connected) , try restart
      Serial.println("ESP timeout.mqttP1Published.restart"); // print message line
      publishMqtt(mqttLogTopic, (String) "ESP timeout.restart" );
      ESP.restart();     // if no/yet data published force restart
    }

    // if (outputMqttLog) client.publish(mqttLogTopic, "ESP P1 Active interval checking rj11" );  // report we have survived this interval
    previousMillis = currentMillis; // Use the snapshot to set track time until next loop event
    mqttP1Published = false;        // assume we have not yet published processed data
  }

  if (mqttReceivedCommand[0] != '\x00') {
      ProcessMqttCommand(mqttReceivedCommand,2); // process incoming Mqtt command (if any)
      memset(mqttReceivedCommand,0,sizeof(mqttReceivedCommand));
  }

  ArduinoOTA.handle();             // check if we must service on the Air update
  if (verboseLevel == 1) Serial.print(">"); // exit loop to check if we have left the building
}

/* 
  Reconnect lost mqtt connection
*/
void mqtt_reconnect() {                 // mqtt read usage doc https://pubsubclient.knolleary.net/api
  // Loop until we're reconnected
  int mqttConnectDelay    = 2000;     // start reconnect and ease things increasingly to 30000 if this continues to fail
  // strcpy(mqttServer1, mqttServer);    // initialise mqttserver
  
  while (!client.connected())   {
    Serial.print((String)"Attempt MQTT connection to " + mqttClientName + " ..." );
    if (client.connect(mqttClientName))     {                 // nodemcu-P1 (override value)
      Serial.print((String)"(re)connected to " + mqttServer1); // 192.168.1.x
      // Serial.print(client.state());
    } else {
      Serial.print((String)"failed to " + mqttServer1);
      // Serial.print(" try again in 5 seconds...");
      // Wait 2 seconds before retrying
      // delay(2000);
      // delay(mqttConnectDelay);    // delay next try

      delay(1000);                                         // initial delay
      for (int i = 0; i < mqttConnectDelay; i = i + 1000) { // mSecs conenction delay before doing critical function
        for (int b = 0; b < 4; b++) {                    // delay 1 second and blink led fast
          digitalWrite(BLUE_LED, !digitalRead(BLUE_LED));  // flash BLUE LED as warning
          delay(250);
        }
        doCritical();  // do critical process to maintain basic Thermostat & OTA functions
      }
 #ifdef TEST_MODE            
      if (mqttConnectDelay < 11000) {      // v45 check if we are below backup time (2+5+8=15sec)  testmode
 #else
      if (mqttConnectDelay <=27000) {      // v45 check if we are below backup time (2+5+8+11+14+17+20+23+26+29=155sec) production
 #endif      
        mqttConnectDelay = mqttConnectDelay + 3000; // increase retry
        Serial.print((String) "... try again in " + (mqttConnectDelay / 1000) + " seconds...\r\n");
      } else {
        if (strcmp(mqttServer1, mqttServer) == 0 ) {   // current mqttserver is primary
          strcpy(mqttServer1, mqttServer2);    // initialise backup
          client.setServer(mqttServer2, mqttPort);
        } else {
          strcpy(mqttServer1, mqttServer);     // initialise primary
          client.setServer(mqttServer, mqttPort);
        } // else secondary
      }
    }
    Serial.print((String)", connect-rc=" + client.state() + " \r\n");
  }

  if (client.connected()) {     // do only if connected, else this is useless
    client.subscribe(submqtt_topic);
    Serial.print((String)"; subscribed to >" + submqtt_topic) ;
    // Serial.print(submqtt_topic);
  } else {
    Serial.print((String)"; NOT connected to >" + mqttServer) ;
  }
  Serial.println("< .");
} // end setup

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

  unsigned long p1Debounce_time = millis() - p1TriggerTime;		// mSec - p1TriggerTime  set by this() and updated while reading P1 data
  // if (p1SerialActive && p1Debounce_time > p1TriggerDebounce ) { // debounce_time (mSec
  if (p1Debounce_time > p1TriggerDebounce ) { // debounce_time (mSec
    // yield();                // allow time to others
    mqtt_local_yield();
    if (outputOnSerial) {   // indicate on console
      Serial.print((String) " !!>" + millis() + " yield due " + p1TriggerTime + "< exceeded !!" ); // length processed previous line
    }
    p1TriggerTime = millis();
  }

 #ifdef UseP1SoftSerialLIB              // Note this serial version will P1Active while reading between / and !
  if ( mySerial.P1active())  return ;  // quick return if serial is receiving, function of SoftwareSerial for P1
 #endif  
  if (!mySerial.available() || serialStopP1 ) return ;  // quick return if no data, v52

  if (outputOnSerial)    {
    if ( !telegramP1header) Serial.print((String) P1_VERSION_TYPE + " DataRx started at " + millis() + " s=s123..\b\b\b\b\b") ; // print message line
    // if (  telegramP1header) Serial.print("\n\rxP= "); // print message line if message is broken in buffer
  }
  startMicros = micros();  // Exact time we started
  // if (!outputOnSerial) Serial.print((String) "\rDataCnt "+ (mqttCnt+1) +" started at " + micros());
  if (!outputOnSerial) Serial.printf("\r\n Cycl%d: %12.9f D%d%s%sC%s%s: %5u start: %11.6f \b\b ", 
        RX_yieldcount,    // v52: check countlevel
        ((float)ESP.getCycleCount()/80000000),
          (int) new_ThermostatState, (thermostatReadState ? "d" : "T"),  (thermostatWriteState ? "A" : "i"),
          (digitalRead(WATERSENSOR_READ) ? "h" : "l"), (digitalRead(LIGHT_READ) ? "c" : "w"),
        (mqttCnt + 1), ((float) startMicros / 1000000));

  // Cycle: 04.781228065

  if (outputMqttLog && client.connected()) publishMqtt(mqttLogTopic, mqttClientName );

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

    int len = mySerial.readBytesUntil('\n', telegram, MAXLINELENGTH - 2); // read a max of  MAXLINELENGTH-2 per line, termination is not supplied
    // Serial.print((String) "yb\b"); no need to display RX progess this goes ok
    

    if (len > 1) {      // correct dual Carrigage returns, Windows formatted testdata on Linux
      // decrease len as the dual 0x0d 0x0d is from streamed testdata
      if (telegram[len-2] == '\x0d' && telegram[len-1] == '\x0d' ) len-- ;  // double carriage return ?
    }
    // debug this
    // if (len > 1)  Serial.print((String)" La="+ (int)telegram[len-2]);
    // if (len > 0)  Serial.print((String)" Lb="+ (int)telegram[len-1]);
    // if (len == 0) Serial.print((String)" Lc="+ (int)telegram[len]);
    
    if (verboseLevel == 0) Serial.print((String) "[Lp1=" + (len-1) + ']' );   // v52/test replaced \n into \r

    if (outputOnSerial && verboseLevel >= VERBOSE_P1) {     // do we want/need to print the Telegram for Debug
      Serial.print((String)"\rlT=" + (len-1) + " \t[");   // v33 replaced \n into \r
      for (int cnt = 0; cnt < len; cnt++) {
          
        if (isprint(telegram[cnt])) {             // v45 revise to improve print debug 
            Serial.print(telegram[cnt]);
        } else if (telegram[cnt] == '\x0d') {
            Serial.print("_");
        } else  {
            // Serial.print(telegram[cnt - 1]);
            Serial.print("?");
        }
          // Serial.printf("%02x",telegram[cnt-1]);   // hexadecimal formatted
      }
      Serial.println((String)"]");    // v33 debug lines didnot end in newline
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
        
        // print validity status of processed for debug reasons.
        if (validTelegramCRCFound) {
              Serial.print((String) "_C");      // print checked OK
        } else {
          if (validCrcInFound) {
              Serial.print((String) "_R");  // v52 count Recoveries
              p1RecoverCnt++ ; 
          } else {
              Serial.print((String) "Z_");  // v45 print failed or recovered
              p1CrcFailCnt++ ;              // v52 count Crc fails
          } 
        }

    #ifdef NoTx2Function
        if (!loopbackRx2Tx2 && blue_led2_Crc) validTelegramCRCFound ? digitalWrite(BLUE_LED2, HIGH) : digitalWrite(BLUE_LED2, LOW) ; // v38 monitoring
    #endif

        p1TriggerTime = millis();   // indicate we have a yield

        processGpio() ; // do read and process regular data pins and finish  this with mqtt
        
        publishP1ToMqtt();      // PUBLISH this mqtt
        // Note: after this we could do the serial2 process for a period of about 6-8seconds ......
        // this is arranged in the mainloop

        if (intervalP1cnt < 1140) intervalP1cnt++ ; // increase survived read count

        p1SerialFinish = true; // indicate mainloop we can stop P1 serial for a while
        mySerial.end();      // flush the buffer (if any) v38
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

    } // if len > 0
  } // while serial available
  // Serial.println("startend..");
} // void readTelegram



/* 
  process the secondary serial input port, to be used  in future for 1200Baud GJ meter
*/
void readTelegram2() {
  // if (loopbackRx2Tx2) Serial.print("Rx2 "); // print message line
 #ifdef UseP1SoftSerialLIB
    if (mySerial.P1active()) return ;   // return if P1 is active
 #endif  


  bGot_Telegram2Record  = false;      // v38 check RX2 seen
  
  if (serialStopRX2) return;    // v52 return if we have stopped this serial

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
    if (outputOnSerial && verboseLevel >= VERBOSE_RX2)    {
      Serial.print("\n\r Rx2N="); // print message line
    }
    memset(telegram2, 0,       sizeof(telegram2));        // initialise telegram array to 0
    // memset(telegramLast2, 0,   sizeof(telegramLast2));    // initialise array to 0
    // memset(telegram2Record, 0, sizeof(telegram2Record));  // initialise telegram array to 0

    // initialise positions
    int telegram2_Start = -1;
    int telegram2_End   = -1;
    int telegram2_Pos   = 0;

    while (mySerial2.available() && loopTelegram2cnt < 6 && !bGot_Telegram2Record )     {    // number of periodic reads  && !bGot_Telegram2Record
       
      // int len = mySerial2.readBytesUntil('\n', telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
      // int len = mySerial2.readBytesUntil('!', telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
      
      int len = mySerial2.readBytesUntil(0, telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
      // len == 0 ? lenTelegram = -1 : lenTelegram += len;   // if len = 0 indicate for report
      lenTelegram2 = lenTelegram2 + len;
      if ( len == 0) lenTelegram2 = -1; // if len = 0 indicate for report

      if (len > 0)  {   // do we have bytes in buffer
        // remember  last character
        // telegramLast2[0] = telegram2[len - 2];  // 3byte-telegram readible lastbyte  bracket-2 null-1 cr-0
        // telegramLast2[1] = '\n';                // 3byte telegram lastbyte termination string
        // telegramLast2[2] = 0;                   // 3byte-telegram lastbyte terminaion variable
        if ( len > (MAXLINELENGTH2 - 2) ) {            // we have clearly an overflow
           len =  (MAXLINELENGTH2 - 2) ;
        }
        telegram2[len + 1] = '\n';    // string set lastcharacter to \n
        telegram2[len + 2] = 0;       // variable next character before null
        /*
         * Check ascii output
         */

        if (!bGot_Telegram2Record) {   // reset numbers if not yet a record from WarmteLink 
          telegram2_Start = -1;
          telegram2_End   = -1;
          telegram2_Pos   = 0;
        }
        
        for (int i = 0; (i <= len && !bGot_Telegram2Record); i++) {  // forward

            // if (rx2_function && !bGot_Telegram2Record ) { // v38 rx2_function
            if (rx2_function && !bGot_Telegram2Record ) { // v38 rx2_function
              //  v38 copy/extract proces a single RX2 record
              /* debug
                if (outputOnSerial && telegram2_Start < 0)   Serial.print("s");
                if (outputOnSerial && telegram2_Start >= 0) Serial.print(telegram2_Pos == 0 ? (String) "-" : (String) telegram2[i] );
                if (outputOnSerial && !(telegram2_End < 0) ) Serial.print("e");
              */ 
              if ( telegram2_Pos < MAXLINELENGTH2 && (telegram2_Start < 0 || telegram2_Pos > 0 || i < telegram2_End ) ) {
                if (telegram2[i] == '/')  {
                  telegram2_Start = i;  // position input slash-/
                  telegram2_Pos = 0;    // reset output buffer
                  telegram2Record[telegram2_Pos] = 0;   // initialise first position
                  telegram2_End = -1;    // reset to find exclamation-!

                }
                if (telegram2[i] == '!')  {
                    if (telegram2_End   < 0 && (i < (len - 6)) ) telegram2_End = i + 6;  // anticiptae after checksum position !1234.6
                    if (telegram2_Start < 0 ) telegram2_End = -1;  // reset if not yet started
                } 
                
                if (telegram2_Start >= 0  && (telegram2_End < 0 || i < telegram2_End) ) {
                  telegram2Record[telegram2_Pos] = TranslateForPrint(telegram2[i]); // move value as translated output
                  telegram2_Pos++;
                  telegram2Record[telegram2_Pos] = 0;     // initialize next position
                } // if (telegram2_Start > 0  && (telegram2_End == 0 || i < telegram2_End) 
              } // if ( telegram2_Pos < MAXLINELENGTH2 && (telegram2_Start == 0 || telegram2_Pos > 0 || i < telegram2_End ) )
              if (telegram2_Pos > 4) {      // check for length start2finish before Checksum
                if (telegram2Record[0] == '/' && telegram2Record[telegram2_Pos - 5] == '!')  {
                  bGot_Telegram2Record = true;
                  Got_Telegram2Record_cnt++;      // v51 count for this receive
                  mySerial2.end();          // v38 Stop- if any - GJ communication
                  mySerial2.flush();        // v38 Clear GJ buffer
                  if (outputOnSerial && verboseLevel >= VERBOSE_RX2) {
                      // debug print positions
                      Serial.print("\nns1=")          ; // debug v38 print processing
                      Serial.print(telegram2_Start)   ; // debug v38 print processing serial /start
                      Serial.print(", ne1=")          ; // debug v38 print processing
                      Serial.print((telegram2_End))   ; // debug v38 print processing serial !End
                      Serial.print(", np1=")          ; // debug v38 print processing
                      Serial.println(telegram2_Pos)   ; // debug v38 print last record positition                      
                  }
                }
              }
            } // if (rx2_function && !bGot_Telegram2Record )
            
            // modify our work buffer for v38 printing (contents may already have been translated)
            telegram2[i] = TranslateForPrint(telegram2[i]);    // move value to  translated output
        } // end for loop scanning serialbuffer

        if (outputMqttLog2 && bGot_Telegram2Record) {       // we have a record, ensure we only process valid ones
            telegram2[telegram2_Pos] = 0x00;    // enforce String-end of this for serial print
            publishMqtt(mqttLogTopic2, (String)
                     "{ rx2-" +  __LINE__   // print RX2 data line, already converted to print
                        + ", \"mqttCnt\":" + mqttCnt
                        + ", \"length\":"  + telegram2_Pos
                        + ", \"rx2=\":\""  + (String) telegram2
                     + "\"}" );
            // Serial.println((String) "\n\r\t.s2>[len=" + len + " Pos=" + telegram2_Pos + "]" + telegram2) ;
            /*                     
            for (int i = 200; i <= len; i++) {  // forward
                Serial.print(" ") ;         // debug print processing serial data
                Serial.print((int8) telegram2[i] ) ;         // debug print processing serial data
                Serial.print((String) + "=" + telegram2[i] ) ;         // debug print processing serial data
            }
            */
        }
        if (outputOnSerial && verboseLevel >= VERBOSE_RX2) {
          Serial.println((String) "\n.s2>[len=" + len + " Pos=" + telegram2_Pos + "]" + telegram2) ;
        }

        // if (rx2_function) { // do we want to execute v38 rx2_function (superfluous , as routine her eis v38)
        /*
          Process console
        */
        if (bGot_Telegram2Record) {   // v38 print record serial2 if we have catched a record
          Serial.print("&&&");     //  v39 print indicator // added 14mar22 to show activeness
          Serial.print("\b\b+r.");     //  v41 print indicator // added 21jun23 to get it stable: Serial.print("\b\b\b+3."); 
          // 21jun23 Global variables use 41516 bytes (50%) of dynamic memory, leaving 40404 bytes for local variables. Maximum is 81920 bytes
                                         //  v39 print indicator // added 14mar22 to get it stable: Serial.print("\b\b\b.");
          // if (outputOnSerial && ((telegram2_Start >= 0 && telegram2_End > telegram2_Start) || bGot_Telegram2Record) ) {   // v38 print record serial2
          /* sample data record
              .3>[220-58=162]/ISk5\2MT382-1000
              
              1-3:0.2.8(50)
              0-0:1.1.0(230227190235W)
              0-1:24.1.0(012)
              0-1:96.1.0(303030304B414D363435343632)
              0-1:24.2.1(230227190235W)(84.108*m3)
              !5EAE
              Note sinde 23feb25 --> 0-1:24.2.1(250222224600W)(65.478*GJ)  -new

              WL>[237-74=163:163,s=0,e=157,v=151]ISk5\2]T382-1000{<{<1-3:0.2.8(50){<0-0:1.1.0(230227211500W){<0-1:24.1.0(012){<0-1:96.1.0(303030304B494C373236323534){<0-1:24.2.1(230227211500W)(84.188*m3){<!C17F{
              WL>245-0=245:244,s=0,e=239,v=233]/WARMTELINK-VI\<|<|1-3:0.2.8(50)<|0-0:1.1.0(250222231918W)<|0-0:96.1.1(3037386633663736366566333065393434643561326461643162646237373865)<|0-1:24.1.0(012)<|0-1:96.1.0(37323632353436343243324433343043)<|0-1:24.2.1(250222231900W)(65.478*GJ)<|!E48B
          */
          

          int startChar = FindCharInArrayRev(telegram2Record, '/',  sizeof(telegram2Record));  // 0-offset "/ISk5\2MT382-1000" , -1 not found
          int endChar   = FindCharInArrayRev(telegram2Record, '!',  sizeof(telegram2Record));  // 0-offset "!769A" , -1 not found
          int valChar   = FindCharInArrayRev(telegram2Record, '*',  sizeof(telegram2Record));  // 0-offset "(84.108*m3)" , -1 not found

          /*
              add CRC check on WL record, required as we have clearly wrong read values.
          */
          if  ( startChar >= 0  && endChar > startChar && valChar < endChar) {    // we have a start to end, ten do CRC check
                char messageCRC2[5];
                strncpy(messageCRC2, telegram2Record+(endChar-startChar)+1, 4);   // copy 4 bytes crc of record and
                messageCRC2[4] = 0;
                unsigned int currentCRC2 = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram2Record)+startChar, (endChar-startChar)+1); // include \header+!trailer '!' into CRC, v48-casting
                validTelegram2CRCFound = (strtol(messageCRC2, NULL, 16) == currentCRC2);   // check if we have a 16base-match https://en.cppreference.com/w/c/string/byte/strtol
                if (outputOnSerial) {   // print serial record count, total receivwed length, calculated CRC
                  Serial.printf(" s=%c, e=%c, crt=%s, cr1=%x, ;", telegram2Record[startChar],telegram2Record[endChar], messageCRC2, currentCRC2) ;    // debug print calculated CRC
                }
                if (outputOnSerial) {   // before and after crc
                  currentCRC2 = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram2Record)+startChar+1,(endChar-startChar));   // ignore header, v48-casting
                  Serial.printf(" cr0=%x,", currentCRC2) ;    // debug print calculated CRC
                  currentCRC2 = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram2Record)+startChar+1,(endChar-startChar)-1); // ignore header & trailer, v48-casting
                  Serial.printf(" cr-1=%x,", currentCRC2) ;    // debug print calculated CRC

                }

          }


          // int valChar   = FindCharInArrayRev(telegram2Record, '*',  sizeof(telegram2Record));  // 0-offset "(84.108*GJ)" , -1 not found

          // .s2>[238]303030304B414D363435343632){<0-1:24.2.1(230228155651W)(84.707*m3){<!8945
          //          {</ISk5\2MT382-1000{<{<1-3:0.2.8(50){<0-0:1.1.0(230228155652W)
          //          {<0-1:24.1.0(012){<0-1:96.1.0(303030304B414D363435343632){<0-1:24.2.1(230228155652W)(84.707*m3){<!0F0C
          // [ telegram2_End-telegram2_Start=telegram2_Pos,startChar,endChar,valChar ]
          //  WL>237-74=163:162,s=0,e=157,v=151  telegram2Record 
          //  /....+....1....+....2....+....3....+....4....+....5....+....6....+....7....+....8....+....9....+....0....+....1....+.
          //  /ISk5\2MT382-1000{<{<1-3:0.2.8(50){<0-0:1.1.0(230228155652W){<0-1:24.1.0(012){<0-1:96.1.0(303030304B414D363435343632)
          //  ...2....+....3....+....4....+....5....+....6..
          //  {<0-1:24.2.1(230228155652W)(84.707*m3){<!0F0C
          //  {<0-1:24.2.1(250222224600W)(65.478*GJ){<!CB0x
          //    9876543210987654321098765432109876543210
          //    ....+....1....+....2....+....3....+..
          // const char * strstr ( const char * str1, const char * str2 );      char * strstr (       char * str1, const char * str2 ); 
          // Locate substring Returns a pointer to the first occurrence of str2 in str1, or a null pointer if str2 is not part of str1.
          if (outputOnSerial && verboseLevel >= VERBOSE_RX2) {
            Serial.print("\n\rWL>");       // v38 debug print processing serial data
            Serial.print(telegram2_End); 
            Serial.print("-");            // print minus
            Serial.print(telegram2_Start); 
            Serial.print("=");            // debug print processing
            Serial.print((telegram2_End - telegram2_Start)); // debug print processing serial data length
            Serial.print(":");            // debug print processing
            Serial.print(telegram2_Pos);  // debug print processing last record positition
            Serial.print(",s=");          // debug print processing
            Serial.print(startChar);      // debug print processing position of / start
            Serial.print(",e=");          // debug print processing
            Serial.print(endChar);        // debug print processing position of !  end
            Serial.print(",v=");          // debug print processing 
            Serial.print(valChar);        // debug print processing position of * value
            Serial.print("]\t");            // debug print processing
            Serial.println(telegram2Record); // debug print processing
            Serial.print("");             // debug print processing serial data
          }

          // search and get value from Heatlink
          // HeatFlowConsumption = 0; // initialise v39
          // HeatConsumption = 0;     // initialise v46
          // if (strncmp(telegram2Record, "0-1:24.2.1(", strlen("0-1:24.2.1(")) == 0 && endChar > 0 && valChar > 0) { // total HeatFlow
          
          // info: strstr() = Locate substring => Returns a pointer to the first occurrence of str2 in str1
          if ( strstr(telegram2Record,"0-1:24.2.1(") && endChar > 0 && valChar > 0) { // if string found, total HeatFlow
              if (outputOnSerial && verboseLevel >= VERBOSE_RX2) {
                Serial.print((String) "\twl-scan:");     //  ident
                Serial.print(telegram2Record+(valChar - 9)); //  print 9 bytes before
              }
              // if (strncmp(telegram2Record, "0-1:24.2.1(", strlen("0-1:24.2.1(")) == 0 && endChar > 0 && valChar > 0) {
              // total HeatFlow
              // HeatFlowConsumption = getValue(strstr(telegram2Record,"0-1:24.2.1("), (endChar - 40) );   // -37:xxx.xxx*m3, -38:xx.xxx GJ
              long tHeatFlowConsumption = getValue(strstr(telegram2Record,"0-1:24.2.1("), 38) ;   // pointer & record length = 30-38 to get intger long
              
              Serial.print("\t WL-");               //  v46 print value/type unconditionally on new 
              Serial.print(telegram2[valChar+1]);
              Serial.print(telegram2[valChar+2]);
              if (validTelegram2CRCFound) {         // print yes/no value
                  Serial.print("=");
              }
              Serial.print("=");                
              Serial.print(tHeatFlowConsumption);   // print value

              if (tHeatFlowConsumption < 1) {       // check for valid value, must be at WL
                Serial.print("-X");                
              } else if ( telegram2[valChar+1] == 'G' ) { // record is expressed in GJ
                HeatFlowConsumption = 0;                  // reset as we have Heat
                HeatConsumptionOld = HeatConsumption;     // save previous read
                HeatConsumption = tHeatFlowConsumption;   // get new read  
                if (HeatConsumptionOld == 0  && HeatConsumption > 0) {  // do not apply compare when 0
                    HeatConsumptionOld = HeatConsumption ;
                }
              } else if ( telegram2[valChar+1] == 'm') {  // record is expressed in m3
                HeatConsumption = 0;                      // v46 reset as we have Flow
                HeatFlowConsumption = tHeatFlowConsumption;  
              }
              Serial.print("\t");                  //  make room
          }

        } // bGot_Telegram2Record
        
 #ifndef NoTx2Function
        if (loopbackRx2Tx2) mySerial2.println(telegram2); // echo back , disabled as of v37
 #endif
        yield();  // do background processing required for wifi etc.
        ESP.wdtFeed(); // feed the hungry timer

      } // if (len > 0)  {   // do we have bytes in buffer
      loopTelegram2cnt++;   /// account this readloop to prevent overrunning on RX2
    } // while data

    if (outputOnSerial) {
      Serial.print((String) " [lenTelegram2="     + lenTelegram2 );  // debug print transaction length
      Serial.print((String) ", loopTelegram2cnt=" + loopTelegram2cnt );
      Serial.print((String) ", Got_Telegram2Record_cnt="+ Got_Telegram2Record_cnt + "].");
      Serial.println("");
    }
  }  // if mySerial2.available 
} // void readTelegram2


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
  processLightRead(LOW);        // process LedLightstatus read pin D6
  processThermostat(digitalRead(THERMOSTAT_READ)) ; // process thermostat switch  D7-in, D8-out

  processTemperatures();      // from DS18B20 tempsensors

  // debug pin status 
  if (outputOnSerial) {

    // cleaner code:
    //  condition ? expression-true : expression-false
    //  printf(i < 0 ? "i is below 0" : i == 0 ? "i equal 0" : "i is over 0");
    
    Serial.print((String) "\nM#@t=" + (mqttCnt+1) + " @" + (millis() / 1000) + ", TimeP1="  + currentTimeS);  // as - including any errors -  extracted from P1 read

    Serial.print((String) ((validTelegramCRCFound) ? " (E#=" : " (e#=") + telegramError + ")" );  // cleaner code; display errors
    // if (validTelegramCRCFound) Serial.print((String) " (E#="          + telegramError + ")"); // data contains errors
    // else               Serial.print((String) " (e#="          + telegramError + ")"); // data contains errors;

    Serial.print((String) "\tGpio5:" + (digitalRead(WATERSENSOR_READ) ? "1" : "0") ); // cleaner code; display sensor state
    // bool tmpd = digitalRead(WATERSENSOR_READ); // read watersensor pin    
    // Serial.print( "\tGpio5:" );
    // if ( tmpd) Serial.print( "1" );
    // if (!tmpd) Serial.print( "0" );
    
    Serial.print( (String)", Water#:"   + waterReadCounter);
    Serial.print( (String)", Hot#:" + waterReadHotCounter);

    Serial.print((String) ", WaterState:" + (waterReadState ? "1" : "0") ); // cleaner code; display state
    // Serial.print( ", WaterState:" );
    // if ( waterReadState) Serial.print( "1" );
    // if (!waterReadState) Serial.print( "0" );
    
    Serial.print( (String) ", Trg#:" + waterTriggerCnt);    // print ISR call counter   
    Serial.print( (String) ", DbC#:" + waterDebounceCnt);   // print Debounce counter
    Serial.print( (String) ", Int#:" + intervalP1cnt);      // print Interval checking counter
    Serial.print( (String) ", Yld#:" + RX_yieldcount);      // v52 current yield value
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
  /* Commands: (single byte)

    ? Print brief helptexst on serial output
    0 heating On  valve relay D8=Gpio15
    1 heating Off
    2 Follow input (thermostat on D7=GPIO13)
    3 No thermostat reading
    R restart esp8266
    D toggle Debug messages on serial output 
    L Log messages to mqtt topi: /log/P1
    l No mqtt logging
    e enforce error div/0 causing a restart
    E same, enforce error div/0 causing a restart
    b decrease P1 Baudrate port  Gpio14/D5  by 50
    B increase Baudrate P1 port by 50
    F toggle (test) Function
    f BlueLed2 Gpio2/d4 led cycle Crc / off / HotWater / Water
    W toggle Watertrigger on Gpio5/D1 or overriding it with Watertrigger1
    w toggle PullUp input resistor Gpio5 (to stabilize optocoupler)
    Z zero counters, like soft restart the MCU
    I set inverval OK counter to 2880
    i decrease interval OK counter 250/100/25/10/2
    T loopback RX2 Gpio4 to TX2 Gpio2 serial port
    P toggle output Power usage to /energy/p1 { json record }
    p toggle output Power2 usage to /energy/p1power CurrentPowerConsumption: 463    
    M Print recovery masking array where X does nog care positions v48
    m Print Input array to be masked v48

    Info - 
          BlueLed is  Gpio16  /d0   - give some visual indication of state
          Water is    Gpio5   /d1   - Optocoupler (with Soft Debounce)
          RX2 GJ  is  Gpio4   /d2   - 115K8N1
          ds18b20 is  Gpio0   /d3   - Onewire
          BLUE_LED2   Gpio2   /d4   - monitor & (also TX2)
          P1          Gpio14  /d5   - inverted 115K8N1
          Hotwater    Gpio12  /d6   - LDR 1/0
          Thermostat  Gpio13  /d7   - Switch
          HeatValve   Gpio15  /d8   - isolated relay
          LightAnalog Gpio17  /A0   - Ldr light 0- Dark1023


  */
  if ((char)payload[0] != '\x00' && 
      ( (char)payload[1] == '\x00' || (char)payload[2] == '\x00' ) )  {   // only single/double command supported for now
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
      if (outputOnSerial)   Serial.println("\nActive.");
    } else  if ((char)payload[0] == 'L') {       // v51 log lines to topic mqttLogTopic
      outputMqttLog   = !outputMqttLog ;         // swap
      Serial.print(" MqttLogging ");
      if (!outputMqttLog)  Serial.print("OFF\n\r ");
      if (outputMqttLog)   Serial.println("\nON ");
    } else  if ((char)payload[0] == 'l') {       // v51 make RX record toggle to topic mqttLogTopic2
      outputMqttLog2   = !outputMqttLog2 ;       // swap
      Serial.print(" MqttLogging2 ");
      if (!outputMqttLog2)  Serial.print("OFF\n\r ");
      if (outputMqttLog2)   Serial.println("\nON ");
    } else  if ((char)payload[0] == 'v') {
      if ( (char)payload[1] >= '0' && (char)payload[1] <= '9') verboseLevel =  (int)payload[1] - 48;   // set number myself
      else verboseLevel++ ;
      if (verboseLevel >= VERBOSE_MAX) verboseLevel = VERBOSE_OFF;
      if (outputOnSerial) Serial.print((String)" Verbose=" +  verboseLevel + " ");
    } else  if ((char)payload[0] == 'e' && (char)payload[1] == '1' ) {
        Serial.print("## forcing divide error");
        while (true) {
          int a = 0;
          int b = 10;
          volatile int c = b / a;
          Serial.printf(" error0 %d ", c); // print used variable 'c'
          // force a never ending loop        
        } 
    } else  if ((char)payload[0] == 'e' && (char)payload[1] == '2' ) {
        Serial.print("## forcing infinite loop");
        while (true) {
          int a = 1;
          int b = 10;
          volatile int c = b / a;
          Serial.printf(" error0 E%d ", c); // print used variable 'c'
          // force a never ending loop        
        } 
    } else  if ((char)payload[0] == 'E') {       // v52 enforce Read fault to check recovering
      doForceFaultP1   = !doForceFaultP1 ;       // swap
      Serial.print("\t doForceFaultP1=");
      if (!doForceFaultP1)  Serial.println("OFF\t");
      if (doForceFaultP1)   Serial.println("ON\t");
    } else  if ((char)payload[0] == 'b') {    // here
        p1Baudrate = p1Baudrate - 50 ;
        Serial.print((String)"Decreasing Baudrate to " + p1Baudrate);
    } else  if ((char)payload[0] == 'B') {    // here
        p1Baudrate = p1Baudrate + 50 ;
        Serial.print((String)"Increasing Baudrate to " + p1Baudrate);
        
  /* DNO , leave it to investigate how to force an exception
      } else  if ((char)payload[0] == 'e') {    // here
            try {
                throwExceptionFunction();
            } catch (const exception& e) {
                Serial.print((String) "Exception activated" + e.what() );
            }
  */
    } else  if ((char)payload[0] == 'F') {
      rx2_function = !rx2_function ; // toggle on/off testing newFunction
      if (outputOnSerial) {
          Serial.print("rx2_function=");
          Serial.print(rx2_function == true ? "ON" : "OFF");
      }

    } else  if ((char)payload[0] == 'f') {  // control Blue_led assignment     //herev38
                if (blue_led2_Water) {
                    blue_led2_Water = !blue_led2_Water;
                    blue_led2_HotWater = !blue_led2_HotWater;          
  #ifdef NoTx2Function                    
                    if (!loopbackRx2Tx2) digitalWrite(BLUE_LED2, HIGH);   // OFF
  #endif                    
                    Serial.print("BlueLed2 = HotWater");         // monitor HotWater to BleuLed2, initial  OFF, v43 add "."
                    // Serial.print(""); // stability test v43 // extra
                    // Serial.print(""); // stability test v43
                    Serial.print("."); // stability test v43

                    // Serial.print("BlueLed2 = HotWater.");         // monitor HotWater to BleuLed2, initial  OFF, v43 add "."
                    // Serial.print(""); // stability test v38    // stability deactive v44
                    // Serial.print("."); // stability test v38   // stability deactive v44
                } else if (blue_led2_HotWater) {
                    blue_led2_HotWater = !blue_led2_HotWater;
                    blue_led2_Crc = !blue_led2_Crc;
                    Serial.print("BlueLed2 = blue_led2_Crc");  // monitor Crc check to BleuLed2 , initial On
                } else if (blue_led2_Crc) {
                    blue_led2_Crc = !blue_led2_Crc;
                    Serial.print("BlueLed2 = Off");            // BlueLed2 Off
                } else {
                   blue_led2_Water = !blue_led2_Water;
  #ifdef NoTx2Function                                       
                   if (!loopbackRx2Tx2) digitalWrite(BLUE_LED2, LOW);   // ON
  #endif
                   Serial.print("BlueLed2 = Water");           // BlueLed2 to Water, initial ON
                }

    } else  if ((char)payload[0] == 'T') {
      loopbackRx2Tx2   = !loopbackRx2Tx2 ; // loopback serial port
      // mySerial2.begin( 1200);    // GJ meter port   1200 baud
      // mySerial2.println("..echo.."); // echo back
      if (outputOnSerial) {
        Serial.print("RX2TX2 looptest=");
        Serial.print(loopbackRx2Tx2 == true ? "ON" : "OFF");
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
      // waterTriggerCnt  = 1;              // indicate we are in detached mode
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
    } else  if ((char)payload[0] == 'y') {
        Serial.println((String) "\r\n debounce_time=" + debounce_time 
              + ", waterTriggerTime=" + waterTriggerTime 
              + ", waterTriggerCnt="  + waterTriggerCnt 
              + ", ISR_time="         + ISR_time
              + ", ISR_time_cnt="     + ISR_time_cnt
              );
    } else  if ((char)payload[0] == 'Z') {
      debounce_time    = 0 ;     // v47 Zero out debounce time
      waterTriggerCnt  = 0 ;     // v47 reset waterTriggerCnt
      waterReadCounter = 0 ;     // Zero the Water counters
      waterReadHotCounter = 0 ;  // Zero the WaterHot counter
      mqttCnt = 0;               // Zero mqttCnt

      p1MissingCnt = 0;      // v52 updated when we failed to read any P1
      p1CrcFailCnt = 0;      // v52 updated when we Crc failed
      p1RecoverCnt = 0;      // v52 updated when we successfully recovered P1
      p1FailRxCnt  = 0;      // v52 updated P1 does not seem to be connnected
      p1ReadRxCnt  = 0;      // v52 updated P1 has read an ok Crc

      waterTriggerTime  = currentMicros;     // initialise debounce ro current looptime
      waterTriggerState = LOW;   // reset debounce
      waterReadState    = LOW;   // read watersensor pin
      if (outputOnSerial) {
        Serial.print("WaterCnt = 0, mqttCnt = 0, reset/Watersensor/timers");
      }
    } else  if ((char)payload[0] == 'I') {
      intervalP1cnt = 2880;              // make P1 Interval not critical
    } else  if ((char)payload[0] == 'i') {              // make P1  stepping down interval
      if (intervalP1cnt > 1000) intervalP1cnt = intervalP1cnt - 250;
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
    } else  if ((char)payload[0] == 'p') {
      outputMqttPower2 = !outputMqttPower2 ;       // Do not publish P1 meter
      if (outputOnSerial) {
        Serial.print("outputMqttPower2 now ");
        if (outputMqttPower2 ) Serial.print("Active.");
        if (!outputMqttPower2 ) Serial.print("Inactive.");
      }
    } else  if ((char)payload[0] == 'm') {       // v48 10jun25 print m-asked Input array
      Serial.println((String)"\r\n dataIn telegram_crcIn len=" + telegram_crcIn_len + " som>>");
      for (int cnt = 0; cnt < telegram_crcIn_len+4; cnt++) {
        if (isprint(telegram_crcIn[cnt])) {             // if printable
            Serial.print(telegram_crcIn[cnt]);
        } else if (telegram_crcIn[cnt] == '\x0d') {     // carriage return
            Serial.print("\r");
        } else if (telegram_crcIn[cnt] == '\x0a') {     // linefeed
            Serial.print("\n");
        } else if (telegram_crcIn[cnt] == '\x00') {     // end of data
            Serial.print("|");
            break;
        } else  {
            Serial.print("?");                    // unprintable
        }
      }
      Serial.println((String)"<< eom");    // v33 debug lines didnot end in newline
    } else  if ((char)payload[0] == 'M') {       // v48 10jun25 print M-asking array
      Serial.println((String)"\r\n Recovery telegram_crcOut len=" + telegram_crcOut_len + " som>>");
      for (int cnt = 0; cnt < telegram_crcOut_len+4; cnt++) {
        if (isprint(telegram_crcOut[cnt])) {             // if printable
            Serial.print(telegram_crcOut[cnt]);
        } else if (telegram_crcOut[cnt] == '\x0d') {     // carriage return
            Serial.print("\r");
        } else if (telegram_crcOut[cnt] == '\x0a') {     // linefeed
            Serial.print("\n");
        } else if (telegram_crcOut[cnt] == '\x00') {     // end of data
            Serial.print("|");
            break;
        } else  {
            Serial.print("?");                    // unprintable
        }
      }
      Serial.println((String)"<< eom");    // v33 debug lines didnot end in newline
    } else  if ((char)payload[0] == 'S') {  // v52 serial stop activating P1
        serialStopP1 = !serialStopP1;
        if (outputOnSerial) Serial.print((String) " Serial P1=" + (!serialStopP1  ? "Active" : "disabled") + " )" ) ;
    } else  if ((char)payload[0] == 's') {  // v52 serial stop activating RX2
        if ( (char)payload[1] >= '1' && (char)payload[1] <= '9') rx2ReadInterval = (int)payload[1] - 48;   // set number myself
        else serialStopRX2 = !serialStopRX2;
        // if (rx2ReadInterval < 1) rx2ReadInterval = 7;   // reset back to 7 (actual 6*12=72secs)
        if (outputOnSerial) Serial.print((String) " Serial RX2=" + rx2ReadInterval  
                            + " (" + (!serialStopRX2  ? "Active" : "disabled") + " )" ) ;
    } else  if ((char)payload[0] == 'a') {  // v52 manipulate analog read value
        // nowValueAdc = 0;
        if ( (char)payload[1] >= '0' && (char)payload[1] <= '9') nowValueAdc = (((int)payload[1] - 48) * 100);   // set number myself
        else if ((char)payload[1] == '-') doReadAnalog = false;
        else if ((char)payload[1] == '+') doReadAnalog = true;
        else doReadAnalog = !doReadAnalog;

        if (outputOnSerial) Serial.print((String) " Analog=" + nowValueAdc  
                            + " (" + (doReadAnalog  ? "Active" : "disabled") + " )" ) ;

    } else  if ((char)payload[0] == 'h') {
          command_testH1();       // v51 check call functions of pointers and data
    } else  if ((char)payload[0] == 'H') {    // testit c-strings and constants
          // command_testH2();    //  v51: Execute test string casting c_str, array, publishmqtt
          command_testH3();       // v52: check mqtt empty strings
    } else  if ((char)payload[0] == '?') {       // v48 Print help , v51 varbls https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html
          Serial.println((String)"\n\r? Help commands"  + __FILE__ 
                                                        + " version " + DEF_PROG_VERSION 
                                                        + ", compiled " __DATE__ + " " + __TIME__ );
          // Serial.println((String)"_ check espconn"  +  espconn.dnsIP );  // espconn was not declared
          Serial.println((String)"0 Heating On"     + (new_ThermostatState == 0 ? " <--" : "" ) );
          Serial.println((String)"1 heating off"    + (new_ThermostatState == 1 ? " <--" : "" ) );
          Serial.println((String)"2 Heat follow ("  + (thermostatWriteState ? "1" : "0" ) + ") Thermostate"  
                                                    + (new_ThermostatState == 2 ? " <--" : "" ) );
          Serial.println((String)"3 Thermostate ("  + (!thermostatReadState ? "1" : "0" ) + ") disable"      
                                                    + (new_ThermostatState == 3 ? " <--" : "" ) );
          Serial.println((String)"R restart (mqttserver=" + mqttServer + ")");
          Serial.println((String)"D debug ( ip=" + String(WiFi.localIP().toString().c_str()) + " )"    + "\t" +  (outputOnSerial ? "Yes" : "No") ); // v51: reverse tupled (35.1.168.192)
          Serial.println((String)"L log WL to " + mqttLogTopic2   + "\t" +  (outputMqttLog2  ? "ON" : "OFF") );
          Serial.println((String)"e 1/2 force exception ( heap:"+ ESP.getFreeHeap() +")" );   // v52: display FreeHeap
          Serial.println((String)"E force ReadP1 fault:"          + "\t" + (doForceFaultP1  ? "Yes" : "No"));
          Serial.println((String)"b Baud decrease gpio14:"        + "\t" +  p1Baudrate);
          Serial.println((String)"B Baud increase gpio14"         + "\t" +  p1Baudrate);
          Serial.println((String)"l Stoplog "+ mqttLogTopic);
          Serial.println((String)"F ON/off test Rx2 function:"    + "\t" + (rx2_function  ? "Yes" : "No")  );
          Serial.println((String)"f Blueled cycle CRC/Water/Hot:" + "\t" + (blue_led2_Crc ? "Y" : "N") 
                                                                         + (blue_led2_Water ? "Y" : "N") 
                                                                         + (blue_led2_HotWater ? "Y" : "N") );
          Serial.println((String)"T rx2 loopback to BlueLed:"     + "\t" + (loopbackRx2Tx2  ? "ON" : "OFF")   );
          Serial.println((String)"W on/OFF Watertrigger1:"        + "\t" + (useWaterTrigger1  ? "ON" : "OFF") ) ;
          Serial.println((String)"w on/OFF Water Pullup:"         + "\t" + (useWaterPullUp  ? "ON" : "OFF")   );
          Serial.println((String)"y print water debounce");
          Serial.println((String)"Z zero counters " + 
                + "( faults: " 
                + " Miss=" + p1MissingCnt         // v52 failed to read any P1
                + ", Crc=" + p1CrcFailCnt         // v52 Crc failed
                + ", Rcvr=" + p1RecoverCnt        // v52 recovered P1 
                + ", Rp1=" + p1FailRxCnt          // v52 rj11 not connected
                + ", Yld="+  RX_yieldcount        // V52 Yeield count 0-3-8 yes/no process serial data
                + " )" );
          Serial.println((String)"I intervalcount 2880="          + "\t" +  intervalP1cnt);
          Serial.println((String)"i decrease interval count:"     + "\t" +  intervalP1cnt);
          Serial.println((String)"P ON/off publish Json:"  + mqttTopic + "\t" +  (outputMqttPower  ? "Yes" : "No") );
          Serial.println((String)"p ON/off publish Power:" + mqttPower + "\t" +  (outputMqttPower2 ? "Yes" : "No") );
          Serial.println((String)"M print Masking array "+ "( MaskX="+ telegram_crcOut_cnt + " )" );   // v52 number of X maskings
          Serial.println((String)"m print Input array ( Processed="+ p1ReadRxCnt + " )" );   // v52 number of Times we validated
          Serial.println((String)"h help testing C=" + __VERSION__ + " on "+ __FILE__ );
          Serial.println((String)"S ON/off serial1 P1 \t" + (!serialStopP1  ? "Yes" : "No") );
          Serial.println((String)"s ON/off/{0-9} serial2 RX2:"+ rx2ReadInterval  +" \t" + (!serialStopRX2  ? "Yes" : "No") );
          Serial.println((String)"a ON/off/{+-0-9} Analog read:"+ nowValueAdc  +" \t" + (doReadAnalog ? "Yes" : "No") );          
          Serial.println((String)"v {0-9} Verboselevel:"                + "\t" +  verboseLevel );
          
          Serial.println((String)"-------log @=yieldloop ^=mqttout &=gotrx2----");
          
          /* // cannot do here as resetInfo is not defined and cannot be not globalised
          Serial.printf("Restart reason: 0x%08x epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x,depc=0x%08x" , 
            resetInfo->reason, resetInfo->epc1, resetInfo->epc2, resetInfo->epc3, resetInfo->excvaddr, resetInfo->depc); // v52: print registers
          */
          Serial.printf("\t restart reason 0x%08x  epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x depc=0x%08x ",
                             save_reason, save_epc1, save_epc2, save_epc3, save_excvaddr, save_depc ); // v52: print registers
          Serial.println();
          Serial.println("Portmap/read: D0/16 D1/05 D2/04 D3/16 D4/02 D5/14 D6/12 D7/13 D8/15 (digital-invert)");          // v51 print portstatus 
          Serial.println((String) "\t" + BLUE_LED          + "=BLUE_LED:"         + !digitalRead(BLUE_LED)         
                                + "\t" + WATERSENSOR_READ  + "=WATERSENSOR_READ:" + !digitalRead(WATERSENSOR_READ) 
                                + "\t" + SERIAL_RX2        + "=SERIAL_RX2:"       + !digitalRead(SERIAL_RX2)       
                                + "\t" + DS18B20_SENSOR    + "=DS18B20_SENSOR:"   + !digitalRead(DS18B20_SENSOR)   
                                + "\t" + BLUE_LED2         + "=BLUE_LED2:"        + !digitalRead(BLUE_LED2)       ); 
          Serial.println((String) "\t" + SERIAL_RX         + "=SERIAL_RX:"        + !digitalRead(SERIAL_RX)        
                                + "\t" + LIGHT_READ        + "=LIGHT_READ:"       + !digitalRead(LIGHT_READ)       
                                + "\t" + THERMOSTAT_READ   + "=THERMOSTAT_READ:"  + !digitalRead(THERMOSTAT_READ)  
                                + "\t" + THERMOSTAT_WRITE  + "=THERMOSTAT_WRITE:" + !digitalRead(THERMOSTAT_WRITE) 
                                + "\t" + ANALOG_IN         + "=ANALOG_IN:"        +   analogRead(ANALOG_IN)       );  
    } else  {   if (outputOnSerial) Serial.print((String)"Invalid command:" + (char)payload[0] + "" ); }

     if (outputOnSerial) Serial.println();   // ensure crlf
  }
}


/* 
    / -------------------------------------------------------------------------------
    /                        Publish full as JSONPATH record
    / -------------------------------------------------------------------------------
*/
void publishP1ToMqtt()    // this will go to Mosquitto
{
  // int publishP1ToMqttCrc = 0;           // defining here leads tot memory corruption & stalls ??
  publishP1ToMqttCrc = 0;                  // reset
  if (validTelegramCRCFound) publishP1ToMqttCrc = 1; else if (validCrcInFound) publishP1ToMqttCrc = 2; 
  else p1MissingCnt++;    // v52 count missing Crc (likeley records was not read at all)
  
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
    // char msgpub[MQTTBUFFERLENGTH];             // 20mar21 changed from 320 to 360  04apr21 to #define 480
    char output[MQTTBUFFERLENGTH];             // 20mar21 changed from 320 to 360, 04apr21 to #define 480

    String msg = "{"; // build mqtt frame 
    // msg.concat("\"currentTime\": %lu");                // P1 19nov19 17u12 remove superflous comma
    msg.concat("\"currentTime\":\"%s\"");                  // %s is string
    msg.concat(",\"CurrentPowerConsumption\":%lu");    // P1
    msg.concat(",\"ThermoState\":%u");                 // Johnson
    msg.concat(",\"AnalogRead\":%u");                  // adc
    msg.concat(",\"LedLight1\":%u");                   // Hot water witch on

    if ( powerConsumptionLowTariff > 0  && powerConsumptionHighTariff > 0 ) {
        msg.concat(",\"powerConsumptionLowTariff\":%lu");  // P1   always > 0
        msg.concat(",\"powerConsumptionHighTariff\":%lu"); // P1   always > 0
    } else {
        msg.concat(",\"!powerConsumptionLowTariff\":%lu");  // v46 P1 false or missing read, ignore field
        msg.concat(",\"!powerConsumptionHighTariff\":%lu"); // v46 P1 false or missing read, ignore field
    }
    
    msg.concat(",\"P1crc\":%u");                       // Validity CRC 0 or 1

    char temperatureString[7];
    // sequence does matter
    // TBD: consider logic to ignore if we have a missing sensorcount
    msg.concat(",\"T0\":"); dtostrf(tempDev[5], 2, 1, temperatureString); msg.concat(temperatureString);  // T0
    msg.concat(",\"T1\":"); dtostrf(tempDev[0], 2, 1, temperatureString); msg.concat(temperatureString);  // T1
    msg.concat(",\"T2\":"); dtostrf(tempDev[1], 2, 1, temperatureString); msg.concat(temperatureString);  // T2
    msg.concat(",\"T3\":"); dtostrf(tempDev[2], 2, 1, temperatureString); msg.concat(temperatureString);  // T3
    msg.concat(",\"T4\":"); dtostrf(tempDev[3], 2, 1, temperatureString); msg.concat(temperatureString);  // T4
    msg.concat(",\"T5\":"); dtostrf(tempDev[4], 2, 1, temperatureString); msg.concat(temperatureString);  // T5
    // concatenate remaining number of temperature sensors ,Ti:22
    for (int i = 6; i <= numberOfDsb18b20Devices; i++) {    // v46 start count as of T6
      msg.concat(",\"T"); 
      msg.concat(String(i)); 
      msg.concat("\":"); 
      dtostrf(tempDev[i], 2, 1, temperatureString); 
      msg.concat(temperatureString); 
    } 

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

    // v46: arrange & check that invlaid values are not published to normal Json field
    if ( HeatFlowConsumption > 0 ) {
        msg.concat(", \"HeatM3\":%u");    // valid count flow, should always > 0
    } else {
        msg.concat(", \"!HeatM3\":%u");    // invalid count
    }
    if ( HeatConsumption > 0 && (HeatConsumption - HeatConsumptionOld) <= 4 ) { // v46 Meter reading P2 in MJ - tbd value from RX2 Warmtelinkl  = 0;            // v46 Meter reading P2 in MJ - tbd value from RX2 Warmtelinkl) ) {
        msg.concat(", \"HeatMJ\":%u");    // v46 as of 23feb25 show HeatConsumption in MJ
    } else {
        msg.concat(", \"!HeatMJ\":%u");   // invalid count
    }
    // HeatConsumptionOld = HeatConsumption;  // check for next cycle 

    if (useWaterTrigger1) msg.concat(", \"Trigger1\":1");  // show triggernumber
    if (useWaterPullUp)   msg.concat(", \"PullUp\":1");    // show pullupmode



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
    
  // important note: sprinft corrupts and crashes esp8266, use snprinf which CAN handle multiple variables
  //  msg.toCharArray(msgpub, MQTTBUFFERLENGTH);         // 27aug18 changed from 256 to 320 to 360 to MQTTBUFFERLENGTH
  //  sprintf(output, msgpub,           // construct data  http://www.cplusplus.com/reference/cstdio/sprintf/ , formats: http://www.cplusplus.com/reference/cstdio/printf/
    // snprintf(output, sizeof(output), msgpub ,
    snprintf(output, sizeof(output), msg.c_str(),
            // currentTime,                // metertime difference 52 seconds can also use millis()
            currentTimeS,               // meter time in string format from timedate record
            // millis(),
            CurrentPowerConsumption,
            !thermostatReadState,       // input setting Switch press 1=ON low , Not active High 0=OFF
            filteredValueAdc,           // read analog value
            !lightReadState,            // Hotwater we want 1=ONlight , Not active 0=OFFlight
            powerConsumptionLowTariff,
            powerConsumptionHighTariff,

            // myCrcFound,                         // v45 either original validTelegramCRCFound or recovered validCrcInFound
            // validTelegramCRCFound,              // P1crc=0 or 1 for valid CRC
            publishP1ToMqttCrc,                    // v45 unsigned original validTelegramCRCFound or recovered validCrcInFound

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
            HeatFlowConsumption,        // v39 28feb23 show HeatFlowConsumption RX2
            HeatConsumption,            // v46 23feb25 show HeatConsumption MJ counter RX2
            
            prog_Version );             // (fixed) Version from program , see top

    if (client.connected()) {
      if (outputMqttPower) publishMqtt(mqttTopic, output);   // are we publishing data ? (on *mqttTopic = "/energy/p1")
      mqttP1Published = true;             // yes we have publised energy data
    } else {
      Serial.println((String)"v");        // indicate no connection at datarecord
    }


    if (Got_Telegram2Record_cnt > Got_Telegram2Record_prev) {
        Got_Telegram2Record_prev = Got_Telegram2Record_cnt;   // count for this record
        Got_Telegram2Record_last = mqttCnt;                 // administrate receive
    } else {
        if (((mqttCnt - (Got_Telegram2Record_last)+1) % 7) == 1) {  // signal error at every 7th fault, +1 to prevent initial fault
                                            // normally , every 7th interval we have a RX2 record., 
          if (outputOnSerial) {
             Serial.println((String)"#!!# ESP RX2 timeout Warmtelink=" + intervalP1cnt );
          }
          String mqttMsg = "{\"error\":004 ,\"msg\":\"RX2fail";  // start of Json error message
          mqttMsg.concat((String) " " +  + "\", \"RX2Cnt\":"+ (Got_Telegram2Record_cnt) );      // finish JSON error message
          mqttMsg.concat((String) " " +  + "\", \"mqttCnt\":"+ (mqttCnt) +"}");      // finish JSON error message
          publishMqtt(mqttErrorTopic, mqttMsg); // report to /error/P1 topic
        }
    }

    // digitalWrite(BLUE_LED, HIGH);   //Turn the ESPLED off
    digitalWrite(BLUE_LED, thermostatReadState);   //Turn the ESPLED according to input thermostate
  }
}

/* 
    if output = input then false changed
    bool thermostatChanged = processThermostat(LOW) || thermostatReadState = processThermostat(HIGH) || processThermostat(thermostatWriteState)
    true state was changed , false state was already as instructed

*/
bool processThermostat(bool myOperation)    // my operation is currently readed thermostate during call
{
  thermostatReadState = digitalRead(THERMOSTAT_READ); // read
  if (outputOnSerial) Serial.print("\n\rProcessThermostat:") ;

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
      Serial.print(" ThermostateIn D7 high(OFF), ") ; // Display input Thermostate
    } else                              {
      Serial.print(" ThermostateIn D7 low(ON), ") ;  // LOW means set, HIGH means unset
    }

    if         (new_ThermostatState == 0) {
      Serial.print(" ThermoCommand=OFF "    ) ;        // Display processsing mode
    } else  if (new_ThermostatState == 1) {
      Serial.print(" ThermoCommand=ON "     ) ;
    } else  if (new_ThermostatState == 2) {
      Serial.print(" ThermoCommand=FOLLOW " ) ;
    } else                              {
      Serial.print(" ThermoCommand=LEAVE "  ) ;
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
    if (outputOnSerial) Serial.print(" kept ") ;
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
  if (doReadAnalog) nowValueAdc = analogRead(ANALOG_IN);      // A0 pin, v52 yes/no reading
  // nowValueAdc = 123;
  filteredValueAdc = (pastValueAdc + nowValueAdc) / 2;  // smooth values by avarageing
  if (outputOnSerial and thermostatReadState) {  // debug
    Serial.print("\n\rAnalog sensor = ");
    Serial.print(nowValueAdc);
    Serial.print(" filtered Value = ");
    Serial.print(filteredValueAdc);
  }

  if (nowValueAdc <= REQUIRED_ANALOG_ADC) {     // v49 check if we a Light sensor value reading
    String mqttMsg = "{\"error\":003 ,\"msg\":\"ADC Lightsensor value ";  // start of Json error message
    mqttMsg.concat((String) " " + nowValueAdc + "\", \"mqttCnt\":"+ (mqttCnt+1) +"}");      // finish JSON error message
    
    // char mqttOutput[128];
    // mqttMsg.toCharArray(mqttOutput, 128);
    // if (client.connected()) {
      // publishMqtt(mqttErrorTopic, mqttOutput); // report to /error/P1 topic
      publishMqtt(mqttErrorTopic, mqttMsg); // report to /error/P1 topic
    // }
  }
  return filteredValueAdc;
}


/* 
    Send data to Mqtt topic v51
*/
void publishMqtt(const char* mqttTopic, String payLoad) { // v50 centralised mqtt routine
  // note: to check for occurance on "const char* " , use strstr that searches 
  if (!mqttTopic || strstr(mqttTopic,"!") ) {
    if (mqttTopic) Serial.println((String) "\n\r\t mqttTopic is empty.");
    if (strstr(mqttTopic,"!") && outputOnSerial) Serial.println((String) "\n\r\t" + mqttTopic + "prohibits output.");
    return; // check empty of ! in topic
  }    
  
  Serial.print("^");     // signal write mqtt entered
  if (outputOnSerial) {  // debug
    if (verboseLevel >= VERBOSE_MQTT) {
      Serial.println((String) "\n\r[" + mqttTopic + ":" + payLoad + ".");
    } else {
      Serial.print("{");        // print mqtt start operation
    }
  }

  if (mqttCnt == 1 && mqttTopic != mqttErrorTopic ) {   // v51: recursive report restart reason
    char output[128];
    snprintf(output, sizeof(output), 
          "{\"info\":000, \"mqttCnt=\":%d ,\"msg\":\"restart reason 0x%08x"
          ", epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x depc=0x%08x \"}",
          mqttCnt , save_reason, 
          save_epc1, save_epc2, save_epc3, save_excvaddr, save_depc ); // v52: print registers
    publishMqtt(mqttErrorTopic, output); // report to /error/P1 topic
  }

  if (mqttTopic and payLoad) {    // check for not nullpointer
    char mqttOutput[MAXLINELENGTH];
    payLoad.toCharArray(mqttOutput, MAXLINELENGTH);
    if (client.connected()) {
      client.publish(mqttTopic, mqttOutput); // report to /error/P1 topic
    }   
  } else {
    if (outputOnSerial) {  // debug
      Serial.println("_Error in publishMqtt, nullreference fault_");  // display error on debug
    } else {
      Serial.print("{E}");    // display error sign
    }
  }
  if (outputOnSerial) {  // debug
    if (verboseLevel > VERBOSE_MQTT) {
      Serial.print("]\n\r");
    } else {
      Serial.print("}");      // print mqtt finish oeration
    }
  }

}

// process Ledlightstatus indicating if Hotwater is tapped, return state
bool processLightRead(bool myOperation)
{
  lightReadState   = digitalRead(LIGHT_READ); // read D6
  #ifdef NoTx2Function                      
    if (!loopbackRx2Tx2 && blue_led2_HotWater) digitalWrite(BLUE_LED2, lightReadState); // debug readstate0
  #endif  
  
  if (mqttCnt == 0) lightReadState = HIGH;    // ensure inverted OFF at first publish
  // Process this one-to-one directly to output
  if (outputOnSerial and lightReadState)  Serial.print("\n\rLightReadState D6 LOW...");  // debug
  if (outputOnSerial and !lightReadState) Serial.print("\n\rLightReadState D6 HIGH..");  // debug
  return lightReadState;  // return status
}


/* 
  process P1 data start / to ! Let us decode to see what the /KFM5KAIFA-METER meter is reading
  called each time we have a telegram record
*/
bool decodeTelegram(int len)    // done at every P1 line read by rs232 that ends in Linefeed \n
{
  // there are 24 telegram lines (electricity() totalling  676bytes + 24 returns = 700 bytes

  //need to check for start
  validTelegramCRCFound = false;  // init telegram record
  validCrcInFound       = false;  // init Crcin record

  int startChar = FindCharInArrayRev(telegram, '/', len);  // 0-offset "/KFM5KAIFA-METER" , -1 not found
  int endChar   = FindCharInArrayRev(telegram, '!', len);  // 0-offset "!769A" , -1 not found
  bool endOfMessage = false;    // led on during long message transfer

  // if-else-if check if we are on header, trailer of in between those lines
  if (startChar >= 0) {        // We are at start/first line of Meter reading
    // digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off
    // digitalWrite(BLUE_LED, LOW);       // Turn the ESPLED on to inform user
    digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // invert BLUE ked
    allowOtherActivities = false;      // hold off other serial/interrupt acvities

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

    if (strncmp(telegram, "/KFM5KAIFA-METER", strlen("/KFM5KAIFA-METER")) == 0) telegramP1header = true;   // indicate we are in header mode

    currentCRC = Crc16In(0x0000, reinterpret_cast<unsigned char*>(telegram) + startChar, len - startChar - 1);  // initialise using header, v48-casting
    if (telegram[startChar+len-2] == '\x0d') { // v48-casting
      // currentCRC = Crc16In(currentCRC, (unsigned char *) "\x0a", 1); // add implied NL on header
        unsigned char tempLiteral[] = {0x0A};  // use temporarily literal to "unsigned char* cast of Crc16In, v48-casting
        currentCRC = Crc16In(currentCRC, tempLiteral, 1);
    }

    /*
    if ( len = 1 && telegram[len-1] == '\x0d') {
      Serial.print("DebugAddCrLf.."); 
      currentCRC = CRC16(currentCRC, (unsigned char *) "\x0a\x0d\x0a", 3); // add stripped header
    }
    */

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

  } else {  // startChar >= 0

    if (endChar >= 0 && telegramP1header) {   // Are we reading the trailer of the Meter with header passed

      currentCRC = Crc16In(currentCRC, reinterpret_cast<unsigned char*>(telegram) + endChar, 1); // include trailer '!' into CRC, v48-casting
      char messageCRC[5];
      strncpy(messageCRC, telegram + endChar + 1, 4);   // copy 4 bytes crc and
      messageCRC[4] = 0;                                // make it an end of string

      validTelegramCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);   // check if we have a 16base-match https://en.cppreference.com/w/c/string/byte/strtol
      if (doForceFaultP1) validTelegramCRCFound = false;    // v52 enforce P1 error
      
      if (outputOnSerial) Serial.printf(", msLt#%d ", telegram_crcOut_len);
      // incoperate CRC reciovery function
      if (validTelegramCRCFound) {   // temporari test√erify on Debug switch
        p1ReadRxCnt++ ; // Count times we have had a succesfull CRC read
        RX_yieldcount = 3; // assume all if well and we had of have surived any yieldcount
        if (currentCRC == dataInCRC) { // on match build masking array
          // ----------------------------------------------------------------------------------------------
          validCrcInFound = validTelegramCRCFound;            // v45 telegram has CRC OK , so CrcIn is OK
          if  (telegram_crcIn_len == telegram_crcOut_len) {    // do we have a masking array ?

                for (int i=0; i < telegram_crcOut_len; i++) {    // check more masks
                    if (telegram_crcOut[i] != 'X' && telegram_crcOut[i] != telegram_crcIn[i] ) {
                          telegram_crcOut_cnt++;                // increase mask count
                          if (outputOnSerial) Serial.printf(", ms#%d=%c%c", i, telegram_crcIn[i], telegram_crcOut[i]);
                          telegram_crcOut[i] = 'X' ;            // mask this position
                    }
                }
                if (outputOnSerial) Serial.printf(", msk#=%d ",telegram_crcOut_cnt);       
                getValuesFromP1Record(telegram_crcIn, telegram_crcIn_len);
          } else {                                            // no or length changed masking array

                if (outputOnSerial) Serial.printf(", msLi#%d:%d ",telegram_crcIn_len, telegram_crcOut_len);
                for (int i=0; i < telegram_crcIn_len; i++) {
                    telegram_crcOut[i] = telegram_crcIn[i];     // (re)create masking array
                }
                telegram_crcOut_len = telegram_crcIn_len;
                telegram_crcOut[telegram_crcOut_len] = 0x00;    // ensure we have a termination string
                if (outputOnSerial) Serial.printf(", msLo#%d:%d ",telegram_crcIn_len, telegram_crcOut_len);

          }
        } else {  // should not happen, perhaps a logic error when running CRC is not same on total CrcIn rtecord.

            Serial.printf(", crE:%x!=%x",currentCRC,messageCRC);
            telegram_crcOut[0] = 0x00;     // reset mask length and counters
            telegram_crcOut_len = 0;
            telegram_crcOut_cnt = 0;
            if (outputOnSerial) Serial.printf(", msk0=%d ",telegram_crcOut_cnt);

        }
        // ----------------------------------------------------------------------------------------------
      } else {    // we have a CRC error on running CRC, try to recover using using created mask
        
        if  (telegram_crcIn_len == telegram_crcOut_len && !doForceFaultP1) {    // if length of error is equal , try to unmask differences  ?
            for (int i=0; i < telegram_crcIn_len; i++) {
                if (telegram_crcOut[i] != 'X' && telegram_crcIn[i] != telegram_crcOut[i] ) {   // Unmask the error ??
                  if (outputOnSerial) {
                    Serial.printf(", mskio=%d:", i);
                    if (isprint(telegram_crcIn[i]))  Serial.print(telegram_crcIn[i]); else Serial.print("?");
                    Serial.print("/");
                    if (isprint(telegram_crcOut[i])) Serial.print(telegram_crcOut[i]); else Serial.print("?");
                  }
                  telegram_crcIn[i] = telegram_crcOut[i];
                }                 
            }
            dataInCRC = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram_crcIn), telegram_crcIn_len ); // Get new native Crcin, v48-casting
            validCrcInFound = (strtol(messageCRC, NULL, 16) == dataInCRC);    // check if we have a 16base-match
            if (validCrcInFound) {
              telegram_crcIn_rcv++ ;                       // count we can/could recover this corrupted telegram read
              if (outputOnSerial) Serial.printf(" crI%d(%d)=%x<=m>%x ,", telegram_crcIn_rcv, telegram_crcIn_len, dataInCRC,strtol(messageCRC, NULL, 16));  // print recovered count and value
              RecoverTelegram_crcIn();
            }

        }
      }

      if (outputOnSerial) Serial.printf(" crJ(%d)\t!%s (arc=%x)", telegram_crcIn_len, messageCRC, currentCRC); // /t produces an error
      if (outputOnSerial) Serial.printf(", msLx#%d ", telegram_crcOut_len);
      
      /* // DebugCRC not activated as most of the time we miss characters which make CRC Invalid
         if (outputOnSerial)  {
             if( validTelegramCRCFound) Serial.print((String)  "\nVALID CRC FOUND ! start=" + startChar + " end="+ endChar );
             if(!validTelegramCRCFound) Serial.print((String)"\nINVALID CRC FOUND ! start=" + startChar + " end="+ endChar );
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
      
      // currentCRC = Crc16In(currentCRC, (unsigned char*)telegram, len - 1); // calculatate CRC upto/including 0x0D
      //    reinterpret_cast<unsigned char*> to prevent "C-style pointer casting" message
      currentCRC = Crc16In(currentCRC, reinterpret_cast<unsigned char*>(telegram), len - 1); // calculatate CRC upto/including 0x0D, v48-casting
      
      // prevent "C-style pointer casting" message
      //  if (telegram[len - 2] == '\x0d') currentCRC = Crc16In(currentCRC, (unsigned char *) "\x0a", 1); // add excluded \n
      // dnw:  if (telegram[len - 2] == '\x0d') currentCRC = Crc16In(currentCRC, reinterpret_cast<unsigned char*>("\x0a"), 1); // add excluded \n
      // dnw:  if (telegram[len - 2] == '\x0d') currentCRC = Crc16In(currentCRC, "\x0a", 1); // add excluded \n
      if (telegram[len - 2] == '\x0d') {    // correctly use a literal to addin to Crc16in, v48-casting
        // use temporarily literal to add
        unsigned char tempLiteral[] = {0x0A};
        currentCRC = Crc16In(currentCRC, tempLiteral, 1);
      }

      if (outputOnSerial) {
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

  // if (outputMqttLog && client.connected()) client.publish(mqttLogTopic, telegram );   // debug to mqtt log ?
  
  // if (outputMqttLog) publishMqtt(mqttLogTopic, telegram );   // debug to mqtt log, v51 produces wraparounds
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

  if (   telegram[0] != '0' && telegram[0] != '1' && telegram[1] != '-' ) return endOfMessage; // if subrecord not start wih 0- or 1-

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
  // 0-1:24.2.1(250222224600W)(65.478*GJ)  -new
  // 0-1:24.2.1 = Gas (DSMR v4.0) on Kaifa MA105 meter
  if (strncmp(telegram, "0-1:24.2.1(", strlen("0-1:24.2.1(")) == 0)      // Gaz
    GasConsumption = getValue(telegram, len);

  return endOfMessage;
}

/*
  v45 Move recovered Datavalues back to  processed values
*/
void RecoverTelegram_crcIn() {
  if (outputOnSerial) {   // print debug changed values
    Serial.print("Recovery active.");
    if (currentTime2 != currentTime)  Serial.printf("\n\r\t..RcurrentTime2=%d/%d" , currentTime2 , currentTime );
    if (powerConsumptionLowTariff2  != powerConsumptionLowTariff)  Serial.printf("\n\r\t..RpowerConsumptionLowTariff2=%d/%d" , powerConsumptionLowTariff2 , powerConsumptionLowTariff  );
    if (powerConsumptionHighTariff2 != powerConsumptionHighTariff) Serial.printf("\n\r\t..RpowerConsumptionHighTariff2=%d/%d", powerConsumptionHighTariff2, powerConsumptionHighTariff );
    if (powerProductionLowTariff2   != powerProductionLowTariff)   Serial.printf("\n\r\t..RpowerProductionLowTariff2=%d/%d"  , powerProductionLowTariff2  , powerProductionLowTariff   );
    if (powerProductionHighTariff2  != powerProductionHighTariff)  Serial.printf("\n\r\t..RpowerProductionHighTariff2=%d/%d" , powerProductionHighTariff2 , powerProductionHighTariff  );
    if (CurrentPowerConsumption2    != CurrentPowerConsumption)    Serial.printf("\n\r\t..RCurrentPowerConsumption2=%d/%d"   , CurrentPowerConsumption2   , CurrentPowerConsumption    );
    if (CurrentPowerProduction2     != CurrentPowerProduction)     Serial.printf("\n\r\t..RCurrentPowerProduction2=%d/%d"    , CurrentPowerProduction2    , CurrentPowerProduction     );
  }
  // moving unconditional  fields from recovery record (todo: candidate to do this always)
  currentTime  = currentTime2;
  strncpy(currentTimeS2, currentTimeS, 6);
  powerConsumptionLowTariff2  = powerConsumptionLowTariff;
  powerConsumptionHighTariff2 = powerConsumptionHighTariff;
  powerProductionLowTariff2   = powerProductionLowTariff;
  powerProductionHighTariff2  = powerProductionHighTariff;
  CurrentPowerConsumption2    = CurrentPowerConsumption;
  CurrentPowerProduction2     = CurrentPowerProduction;
  Serial.printf(" currentTimeS2=%s ", currentTimeS2 );
}

// ---------------------------------------
/*
  v45 Get field values from full P1  record 2872

*/
void  getValuesFromP1Record(char buf[], int len) {  // 716
  // return;
  // if (mqttCnt == 3) Serial.printf(" mqttcount=%d len=%d ", mqttCnt, len ); // mqttcount=3 len=716
  // if (mqttCnt < 5 ) return;   // testmode just to be sure we can do OTA if things go wrong here, v52 disabled

  int f = 0;
  // InitialiseValues();         // Data record, ensure we start fresh with newly values in coming records
  f = FindWordInArrayFwd(buf, "0-0:1.0.0(", len, 9);
  // Serial.printf(" f=%d ", f ); // mqttcount=3 len=716
  if (buf[f+22] == 'S' || buf[f+22] == 'W') {  // check for Summer or Wintertime
    char resDate[16];     // maximum prefix of timestamp message
    memset(resDate, 0, sizeof(resDate));       // Pointer to the block of memory to fill.
    if (strncpy(resDate, buf +f+16, 6)) {  // start buffer+10+1 for 6 bytes
      if (isNumber(resDate, 6)) {            // only copy/use new date if this is full numeric (decimal might give trouble)
        currentTime2  = (1 * atof(resDate)); // Convert string to double http://www.cplusplus.com/reference/cstdlib/atof/
        strncpy(currentTimeS2, buf +f+16, 6);  // Ptro 27mar21: added as we like to have readible timestamp in message(s)
      }
    }
  }
  if (outputOnSerial) Serial.printf(" f=%d currentTimeS2=%s ", f, currentTimeS2 );
  // return;  
  /* Records
             012345678901234567890123456789012
    lT=16 -4[/KFM5KAIFA-METER_]t 2145994s=s123
    lT=0    [_]
    lT=13   [1-3:0.2.8(42)_]
                  3         4         5 
       35    56789012345678901234567890
             01234567890123456789012   
    lT=24   [0-0:1.0.0(250106212048W)_]
    lT=46   [0-0:96.1.1(4530303237303030303034313732353135)_]

       74    4567890123456789
      109    901234567890123456789012345
             012345678901234567890123456  109+26=135       
    lT=25   [1-0:1.8.1(019061.182*kWh)_]
    lT=25   [1-0:1.8.2(018224.564*kWh)_]
    lT=25   [1-0:2.8.1(000000.000*kWh)_]
    lT=25   [1-0:2.8.2(000000.000*kWh)_]
    lT=17   [0-0:96.14.0(0002)_]
    lT=20   [1-0:1.7.0(00.604*kW)_]
    lT=20   [1-0:2.7.0(00.000*kW)_]
    lT=18   [0-0:96.7.21(00003)_]
    lT=17   [0-0:96.7.9(00006)_]
    lT=259  [1-0:99.97.0(8)(0-0:96.7.19)(230418204602S)(0000003840*s)(220902151336S)(0000000245*s)(220512140519S)(0000008254*s)(210207073103W)(0000001404*s)(181103114840W)(0000008223*s)(180911211118S)(0000]
    lT=18   [1-0:32.32.0(00001)_]
    lT=18   [1-0:32.36.0(00001)_]
    lT=13   [0-0:96.13.1()_]
    lT=13   [0-0:96.13.0()_]
    lT=17   [1-0:31.7.0(003*A)_]
    lT=21   [1-0:21.7.0(00.604*kW)_]
    lT=21   [1-0:22.7.0(00.000*kW)_]
    lT=5    [!B734_]
  */

  // 1-0:1.8.1(000992.992*kWh)  
  /*
    walk all fields
  */
                               
  if (f >= 0) { 
                //                          (          *       s=9 s=20 val=0
                //                 0123456789012345678901234567
                // 108             9012345678901234567890123456
                //                           019061.182*kWh)_
                //                           1234567890 
                //                         s+1    
      f = FindWordInArrayFwd(buf, "1-0:1.8.1(", len, 9);       // total use Low getValuesFromP1Record f=74+35; f=109
      // int s = FindCharInArrayRev(buf+f, '(', 26);  // search buffer fro bracket (s=??)
      // int l = FindCharInArrayRev(buf+f, '*', 26) - s - 1;  // search buffer fro bracket (l=??)
      //                                           f=109 s=9 s=20 val=0 c1=1 c20=*_C
      powerConsumptionLowTariff2 = getValue(buf+f, 26);
      // Serial.printf(" getValuesFromP1Record f=%d s=%d l=%d val=%d c1=%c c2=%c", f, s, l, powerConsumptionLowTariff2, buf[f+s+1], buf[f+s+l]  );
  }

  // return;
 
  if (f >= 0) {
    f = FindWordInArrayFwd(buf+f, "1-0:1.8.2(", len-f, 9);       // total use High
    powerConsumptionHighTariff2  = getValue(buf+f, 26);
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf+f, "1-0:2.8.1(", len-f, 9);        // total Production Low
    powerProductionLowTariff2  = getValue(buf+f, 26);
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf+f, "1-0:2.8.2(", len-f, 9);        // total Production High    
    powerProductionHighTariff2 = getValue(buf+f, 26);
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf+f, "1-0:1.7.0(", len-f, 9);        // Watts usage    
    CurrentPowerConsumption2   = getValue(buf+f, 21);
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf+f, "1-0:1.7.0(", len-f, 9);        // Watts produced    
    CurrentPowerProduction2   = getValue(buf+f, 21);
  }

}
// ---------------------------------------


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
  // 0-1:24.2.1(250222224600W)(65.478*GJ)       = new GJ
  // 1-0:1.8.2(010707.720*kWh)                  = power consumption record len=26 datalen 10
  // /ISk5\2MT382-1000...{<0-1:24.2.1(230228155652W)(84.707*m3){<!0F0C   // v39 RX2/P1 record 0-162bytes
  //  .........1.........2.........3.........4.........5

  //  buf+109   901234567890123456789012345
  //            012345678901234567890123456  109+26=135       
  //   lT=25   [1-0:1.8.1(019061.182*kWh)_]
  //                     (          *
  //                   s=9          20


  int s = FindCharInArrayRev(buffer, '(', maxlen - 2);  // search buffer fro bracket (s=25)
  if (s < 8) return 0;    // no valid string
  if (s > 32) s = 32;     // no bracket in range, try at end backward search

                //                          (          *       s=9 l=20 val=0 -->  l-s=11-1=10
                //                           0123456780                                 s+1=10
                //                 0123456789012345678901234567
                // 108             9012345678901234567890123456
                //                           019061.182*kWh)_

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

  char res[16];                 // extract number starting at startbracket+1 for a length of l
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
 * translate to printable
 */
char TranslateForPrint(char c)
{
  if (c == 10) c = '|' ;                 // translate NL to |
  if (c == 13) c = '<' ;                 // translate CR to <
  if (c == 00) c = '_' ;                 // translate NULL to _
  if (c < 32 || c > 127 ) c = '~' ;    // translate unprintable
  return c;
}  

/* 
  generic subroutine to find a character in reverse , return position as offset to 0
*/
int FindCharInArrayRev(const char array[], char c, int len) {              // Find character >=0 found, -1 failed
  //           123456789012
  // 1234567890123456789012345678901234567890  = 40
  // 0-0:1.0.0(180611014816S)                  = sublen12 len=25 pos24=) post10=(=ret10, pos23=S=ret23   , maxlen==23 == return i=10
  // 0-1:24.2.1(150531200000S)(00811.923*m3)
  // 0-1:24.2.1(250222224600W)(65.478*GJ)  - GJ 
  // ...{<0-1:24.2.1(230228155652W)(84.707*m3){<!0F0C   // v39 RX2/P1 record
  for (int i = len - 1; i >= 0; i--)   // backward
  // for (int i = len - 1; i < 1; i--)   // backward
  {
    if (array[i] == c)
    {
      return i;
    }
  }
  return -1;
}

/*
  find character forwarding for len bytes
*/
int FindCharInArrayFwd(const char array[], char c, int len) {              // Find character >=0 found, -1 failed
  for (int i = 0; i < len+1 ; i++)   // forward
  {
    if (array[i] == c)
    {
      return i;
    }
  }
  return -1;
}

/*
  find word/wlen forwarding in array/len
*/
int FindWordInArrayFwd(const char array[], const char word[], int alen, int wlen) {               // Find character >=0 found, -1 failed
  // Serial.printf(" alen=%d wlen=%d ", alen, wlen ); // alen=716 wlen=10
  // return 0;
  for (int i = 0; i < (alen-wlen)+1 ; i++) {  // search for start of searchstring
    if (array[i] != word[0]) continue;            // next array
    for (int x = 0; x <= wlen ; x++) {        // check here for search string
      if (array[i+x] != word[x]) break;           // next array
      if (x == wlen) return i;                     // all found
    }
  }
  return -1;
}


/*
  process CRC16 while accumulating the input to an array
    input:
     crc unsigned integer (2 bytes 0x0000-0xFFFF)
     buf pointer=address of unsigned char to reference characters as int 0-255 values
     len length of buf 0-768
     
     currentCRC = CRC16(currentCRC, (unsigned char*)telegram, len - 1); // calculatate CRC upto/including 0x0D
*/
unsigned int Crc16In(unsigned int crc, unsigned char *dataIn, int dataInLen) {
  /*

        char telegram_crcIn[MAXLINELENGTH];   // active telegram that is checked for crc
        int  telegram_crcIn_len = 0;          // length of this record
        char telegram_crcOut[MAXLINELENGTH];  // processed telegram with changed positions to X
        int  telegram_crcOut_len = 0;  

        for (int i = len - 1; i >= 0; i--)   // backward
        // for (int i = len - 1; i < 1; i--)   // backward
        {
          if (array[i] == c)
          {
            return i;
          }
        }
        return -1;


  */
  // check if we need to (re)initialise) inputarray at crc 0x0000
  if ( crc == 0 ) {
    dataInCRC = 0;          // reset
    telegram_crcIn_cnt = 0; // Initialise 
    telegram_crcIn_len = 0; // Initialise
    telegram_crcIn[telegram_crcIn_len] = 0x00;
    if (outputOnSerial) Serial.println((String)" ch(" + dataInLen + ")" + telegram_crcIn_cnt + ":" + telegram_crcIn_len ) ;
  }
   
  // loop and collect the passed string into inputarray
  for (int i=0; i < dataInLen && telegram_crcIn_len < MAXLINELENGTH ; i++) {  // v48 prevent loop overflowing telegram_crcIn 
    telegram_crcIn[telegram_crcIn_len] = dataIn[i];
    telegram_crcIn_len++;
    telegram_crcIn[telegram_crcIn_len] = 0x00;
    // if (telegram_crcIn_len >= MAXLINELENGTH ) break;
  }

  // if (outputOnSerial) Serial.println((String)" c=" + telegram_crcIn_len + ".");
  // Crc16In(unsigned int crc, unsigned char *dataIn, int dataInLen)
  /*   if (outputOnSerial)  Serial.println((String)" ct(" + dataInLen + ")" + telegram_crcIn_cnt + ":" + telegram_crcIn_len ) ;
    if ( outputOnSerial &&
         telegram_crcIn_len == 761 && 
         telegram_crcIn_cnt == 44) {
           Serial.print(">");
           Serial.print(telegram_crcIn[telegram_crcIn_len - 2]);
           Serial.print(telegram_crcIn[telegram_crcIn_len - 1]);
           Serial.print(telegram_crcIn[telegram_crcIn_len]);                      
           Serial.print("<");
      }

  */ 
  if (  telegram_crcIn_len > 1 && 
        telegram_crcIn_cnt > 1 &&
        telegram_crcIn[telegram_crcIn_len - 1] == '!' ) {  // (single quoted) are we on end of record ??
      dataInCRC = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram_crcIn), telegram_crcIn_len);  // get CRC of whole copied record, v48-casting
      if (outputOnSerial) {   // print serial record count, total receivwed length, calculated CRC
        // Serial.println((String)" ct=" + telegram_crcIn_cnt + ":" + telegram_crcIn_len ) ;
        Serial.printf("\tcr0=%x,crc=%d,crl=%d\t", dataInCRC,  telegram_crcIn_cnt, telegram_crcIn_len) ; 
      }
  }
  telegram_crcIn_cnt++;
  
  return CRC16(crc, reinterpret_cast<unsigned char*>(dataIn), dataInLen);  // initialise using header, v48-casting
}

/* 
  Check if we are complete on data retrieval
  then construct and publish output to mqtt LOG topic
*/
bool CheckData()        // 
{

  if (firstRun)  {      // at start initialise and set olddata to new data
    SetOldValues();
    firstRun = false;
    return true;
  }

  if (outputMqttLog) {  // if we LOG status old values not yet set
    // char msgpub[MAXLINELENGTH];
    char output[MAXLINELENGTH];
    String msg = "{ checkdata, ";
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
    
    // msg.toCharArray(msgpub, MAXLINELENGTH);
    // snprintf(output, sizeof(output), msgpub,
    snprintf(output, sizeof(output), msg.c_str(),
            millis(),
            CurrentPowerConsumption,
            powerConsumptionLowTariff, powerConsumptionHighTariff,
            CurrentPowerProduction,
            powerProductionLowTariff, powerProductionHighTariff,
            GasConsumption,
            OldPowerConsumptionLowTariff, OldPowerConsumptionHighTariff,
            OldPowerProductionLowTariff, OldPowerProductionHighTariff,
            OldGasConsumption);
    // if (client.connected()) client.publish(mqttLogTopic, output);
    publishMqtt(mqttLogTopic, output);
    
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

  if (outputMqttPower2)    // output currrent power, flatnumber
  {
    char output[32];     // use snprintf to format data
    String msg = "";      // initialise data
    msg.concat("CurrentPowerConsumption: %lu");       // format data
    // rm char msgpub[32];     // allocate a message buffer    
    // rm msg.toCharArray(msgpub, MAXLINELENGTH);                     // move it to format buffwer
    // rm sprintf(output, msgpub, CurrentPowerConsumption); // insert datavalue  (Note if using multiple values use snprint)
    sprintf(output, msg.c_str(), CurrentPowerConsumption); // insert datavalue  (Note if using multiple values use snprint)
    // rm if (client.connected()) client.publish(mqttPower, output);  
    publishMqtt(mqttPower, output);  // publish outputpower
  }
  // Serial.println("Debug5, Checkdata..true");
  return true;
}

/* 
  Initise consumption variables to zero
*/
void InitialiseValues()  // executed at every telegram new run
{
  CurrentPowerConsumption    = 0;     // v45 added to initialise
  powerConsumptionLowTariff  = 0;
  powerConsumptionHighTariff = 0;
  powerProductionLowTariff   = 0;
  powerProductionHighTariff  = 0;
  GasConsumption             = 0 ;

  CurrentPowerConsumption2    = 0;    // v45
  powerConsumptionLowTariff2  = 0;    // v45
  powerConsumptionHighTariff2 = 0;    // v45
  powerProductionLowTariff2   = 0;    // v45
  powerProductionHighTariff2  = 0;    // v45
  
}

/*
 set OldPower.... values to consumption
*/
void SetOldValues()     // executed at end succesful  telegrams
{
  OldPowerConsumptionLowTariff  = powerConsumptionLowTariff;
  OldPowerConsumptionHighTariff = powerConsumptionHighTariff;
  OldPowerProductionLowTariff   = powerProductionLowTariff;
  OldPowerProductionHighTariff  = powerProductionHighTariff;
  OldGasConsumption             = GasConsumption;
}

// check if data/field is numeric 0-9 & decimal-point
bool isNumber(const char *res, int len)       // False/true if numeriv
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
  Setting the temperature sensor []
  read for howto: https://randomnerdtutorials.com/guide-for-ds18b20-temperature-sensor-with-arduino/
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
  Read and get DS18B20 temperatures to tempDev[]
*/
void processTemperatures() {
  // 29jun25: v49 issue sometimes a sensor is not reading and filled als -127.0°C
  //    Sending temperatures:   23.50   23.81   23.69   -127.00 23.56   25.25 
  //      voltage or poor grounding can cause unreliable readings DS18B20_SENSOR @ D3 = GPIO0
  //      strangely the new esp8266 has about 4 volt on its gpio0
  //      bypass for the issue is when < -126 , we ignore the setting

  bool bTemp_Reading_State = true;    // assume temperatures OK

  String mqttMsg = "{\"error\":002 ,\"msg\":\"";  // start of Json erro message
  if (numberOfDsb18b20Devices < REQUIRED_DSB18B20_SENSORS) {     // check if we have devices
        bTemp_Reading_State = false;
        mqttMsg.concat((String) numberOfDsb18b20Devices + " of " + REQUIRED_DSB18B20_SENSORS + " tempsensors detected "); 
  } 
  // else { 

    for (int i = 0; i < numberOfDsb18b20Devices; i++) {
      float tempC = DS18B20.getTempC( devAddr[i] ); //Measuring temperature in Celsius
      if (tempC > -127.0) { // v49 when valid temperature
        tempDev[i] = tempC; //Save the measured value to the array
      } else {              // else bypass and report if logging is active
          if (bTemp_Reading_State) {
            mqttMsg.concat("Tempsensor failure "); 
            if (outputOnSerial) {   // report failures
              Serial.print((String)" Temperature failure on ");
            }
          }
          if (outputOnSerial) {   // report failures
              Serial.print((String)   " T" + i + "=" + tempC + " " );
          }
          mqttMsg.concat((String) "T" + i + "=" + tempC + " "); 
          bTemp_Reading_State = false;
      }
    }
  // }
  mqttMsg.concat((String)"\", \"mqttCnt\":"+(mqttCnt+1));    // +1 to reflect the actual mqtt message
  mqttMsg.concat("\"}");      // finish JSON error message

  if (!bTemp_Reading_State) { //  report failure on mqtt
      // char mqttOutput[128];
      // mqttMsg.toCharArray(mqttOutput, 128);
      if (client.connected()) {
        // client.publish(mqttErrorTopic, mqttOutput); // report to /error/P1 topic
        publishMqtt(mqttErrorTopic, mqttMsg); // report to /error/P1 topic
      }
  }

  DS18B20.setWaitForConversion(false); // No waiting for measurement
  DS18B20.requestTemperatures();       // Initiate the temperature measurement
  lastTempReadTime = millis();         // Remember the last time measurement

  if (outputOnSerial) {
    char temperatureString[6];
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
  Attach waterinterrupt
  Activate ISR water interrupt and choose between two operative versions W/w: routine 1 or 0
  ISR read water sensor on default pin grpio4 and respect debouncetime approx 40mSec
*/
void attachWaterInterrupt() {   // activate waterinerrupt sensor
  if (useWaterTrigger1) {
    attachInterrupt(WATERSENSOR_READ, WaterTrigger1_ISR, CHANGE); // establish trigger
    if (outputOnSerial) Serial.println((String)"\nSet Gpio" + WATERSENSOR_READ + " to second WaterTrigger1_ISR routine");
  } else {
    attachInterrupt(WATERSENSOR_READ, WaterTrigger0_ISR, CHANGE); // establish trigger
    if (outputOnSerial) Serial.println((String)"\nSet Gpio" + WATERSENSOR_READ + " to first WaterTrigger0_ISR routine");
  }
  waterTriggerCnt = 1;          // indicate ISR has been activated
}

/*
  ISR Detach WATERSENSOR_READ interrupt , setting waterTriggerCnt to 0
*/
void detachWaterInterrupt() {   // disconnectt Waterinterrupt to prevent interference while doing serial communication
  detachInterrupt(WATERSENSOR_READ); // trigger at every change
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << WATERSENSOR_READ);  // Prevent calls as per expressif intruction
  waterTriggerCnt = 0;          // indicate ISR has been withdrawn
  waterISRActive = false;   
}

/*
  V47 Stable live ISR water pulse, alternate (Debug select via W,w command --> 1) 
  will gLow Blueled if sensor triggered
  will detach during more excessive vibration
*/
void WaterTrigger0_ISR()
{
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << WATERSENSOR_READ);  // 26mar21 Ptro done at start as per advice espressif
    if (waterISRActive) {    // set routine already active
        if (outputOnSerial) Serial.print( (String) ".W" );
    } else {
      waterISRActive = true;    // V47 - prevent calling while routine is active

        // implement  secondary debounce routine to check ISR compliance
        ISR_time = millis();
        if (ISR_time - last_ISR_time > 500) {
            last_ISR_time = ISR_time;
            ISR_time_cnt++ ;          // increase our change counter      
        }

        if (outputOnSerial && verboseLevel >= VERBOSE_GPIO ) Serial.print( (String) "i" );    
        interval_delay(1); // V47 wait 20ms --> implemented by flat plain while loop, all other types forbidden in ISR
        if (waterTriggerState != (digitalRead(WATERSENSOR_READ)) ) { // check if we have really a change
            waterTriggerState = !waterTriggerState; // revert to make the same

            waterTriggerCnt++ ;             // increase our call counter
            long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
            waterTriggerTime  = time + 1;       // set time of this read and ensure not 0

            if ( (waterTriggerCnt) > 100 ) {    // v37 ensure we will not loop here, like WaterTrigger1_ISR
              detachWaterInterrupt();
              Serial.print( (String) ", Detach>100WaterISR0="+waterTriggerCnt );    // V47 print ISR call counterwaterTriggerTime
              // waterTriggerCnt = 1;          // indicate ISR has been withdrawn
              // waterTriggerState = LOW;      // v41 v47 force to low to ease things
            }
          #ifdef NoTx2Function
            if (!loopbackRx2Tx2 && blue_led2_Water) digitalWrite(BLUE_LED2, waterTriggerState); // monitor expected to go have/went low 
          #endif
            if (outputOnSerial && verboseLevel >= VERBOSE_GPIO) Serial.print( (String) (waterTriggerState ? "tH " : "tL ") );    // V47 print ISR call counterwaterTriggerTime
          }

      waterISRActive = false;    // alow next interrupt
  }
}


/*
  V47 Stable live debug ISR1 water pulse, alternate (Debug select via W,w command --> 1) 
  will gLow Blueled if sensor triggered
  will detach during more excessive vibration
*/
void WaterTrigger1_ISR()
{
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << WATERSENSOR_READ);  // 26mar21 Ptro done at start as per advice espressif
  Serial.print( (String) ".W" );
  if (!waterISRActive) {    // set routine already active
    waterISRActive = true;

    Serial.print( (String) "p" );    
    if (outputOnSerial) {
        interval_delay(20); // wait 20ms --> implemented by flat plain while loop, all other types forbidden in ISR
    }
    if (waterTriggerState != (digitalRead(WATERSENSOR_READ)) ) { // check if we have really a change
        waterTriggerState = digitalRead(WATERSENSOR_READ); // read possible unstable watersensor pin

        waterTriggerCnt++ ;             // increase our call counter
        long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
        waterTriggerTime  = time + 1;       // set time of this read and ensure not 0

      #ifdef NoTx2Function
        if (!loopbackRx2Tx2 && blue_led2_Water) digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // monitor by invert BLUE ked
      #endif
        if ( (waterTriggerCnt) > 200 ) {    // v37 ensure we will not loop here, like WaterTrigger1_ISR
          detachWaterInterrupt();
          Serial.print( (String) ", Detach>200WaterISR1="+waterTriggerCnt );    // V47 print ISR call counterwaterTriggerTime
          // waterTriggerCnt = 1;          // indicate ISR has been withdrawn
          // waterTriggerState = LOW;      // v41 v47 force to low to ease things
        }
      #ifdef NoTx2Function
        // if (!loopbackRx2Tx2 && blue_led2_Water) digitalWrite(BLUE_LED2, waterTriggerState); // monitor expected to go have/went low 
      #endif
        Serial.print( (String) (waterTriggerState ? "tH " : "tL ") );    // V47 print ISR call counterwaterTriggerTime
      }

    waterISRActive = false;    // alow next interrupt
  }
}

/*
  V47 Unstable ISR2 water pulse primary (select via W,w command --> 0) 
  will gLow Blueled if sensor triggered
  will detach during if excessive due bounces on reflective surface 
*/
void WaterTrigger2_ISR()
{
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << WATERSENSOR_READ);  // 26mar21 Ptro done at start as per advice espressif
  // if (outputOnSerial) Serial.print( (String) ".i" );
  if (!waterISRActive) {    // set routine active
    waterISRActive = true;
    // if (outputOnSerial) Serial.print( (String) "P" );
   
    if (waterTriggerState != (digitalRead(WATERSENSOR_READ)) ) { // check if we have really a change
        waterTriggerState = digitalRead(WATERSENSOR_READ); // read possible unstable watersensor pin

        waterTriggerCnt++ ;             // increase our call counter
        long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
        waterTriggerTime  = time + 1;       // set time of this read and ensure not 0

        if ( (waterTriggerCnt) > 100 ) {    // v37 ensure we will not loop here, like WaterTrigger1_ISR
          detachWaterInterrupt();
          if (outputOnSerial) Serial.print( (String) ", Detach>100WaterISR2"+waterTriggerCnt );    // V47 print ISR call counterwaterTriggerTime
          // waterTriggerCnt = 1;          // indicate ISR has been withdrawn
          // waterTriggerState = LOW;      // v41 v47 force to low to ease things
        }
      #ifdef NoTx2Function
        if (!loopbackRx2Tx2 && blue_led2_Water) digitalWrite(BLUE_LED2, waterTriggerState); // monitor expected to go have/went low 
      #endif
        // if (outputOnSerial) Serial.print( (String) (waterTriggerState ? "tH " : "tL ") );    // V47 print ISR call counterwaterTriggerTime
      }

    waterISRActive = false;    // alow next interrupt
  }
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
  Routine to delay millieseconds by while-loop
*/
void interval_delay(int delayTime)   // Routine to use interval to pause program
{
  if (delayTime > 0 && delayTime < 1000) {        // check limits
    unsigned long temp_currentMillis  = millis();      // get time
    unsigned long temp_previousMillis = temp_currentMillis;
    while ( (temp_currentMillis - temp_previousMillis) < delayTime ) {
      // yield();         // call by ISR causes WDT
      // delay(1);
      // ESP.wdtFeed();   // feed the hungry timer  system_soft_wdt_feed() call by ISR causes WDT
                          // https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/cores/esp8266/Esp.cpp#L102
      temp_currentMillis = millis();   // get new time
    }
  }
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
  if (client.connected()) client.loop();  // feed the mqtt client, required to collect mqtt commands
}

#ifdef TEST_CALCULATE_TIMINGS
uint32_t getCycleCountIramLocal() {   // ICACHE_RAM_ATTR does no influence speed wqhen used in program
    uint32_t ccount;
    __asm__ __volatile__("esync; rsr %0,ccount":"=a" (ccount));
    return ccount;
  }
#endif

/*
  check and test coding of (im)mutable pointers and (im)mutable char/string
*/
void command_testH1() {      // used to checkout / test coding
  #ifdef TEST_MODE      
    publishMqtt(mqttErrorTopic, (String) "command_testH1 file " + __FILE__ +  ", line" + __LINE__ ); 
    // read: https://www.geeksforgeeks.org/c/difference-const-char-p-char-const-p-const-char-const-p/
    char a ='A', b ='B';
    const char *ptr   = &a;

    Serial.printf( "1a value pointed to by ptr : %c\r\n", *ptr);
    //*ptr = b; illegal statement (assignment of read-only location *ptr)
    ptr = &b; // changing the ptr
    Serial.printf( "1b value pointed to by ptr : %c\r\n", *ptr);  // A

    char *const ptr2  = &a;        
    Serial.printf( "2a. Value pointed to by ptr2: %c\r\n", *ptr2); // B
    Serial.printf( "2b. Address ptr2 is pointing to: %d\r\n\r\n", ptr2);  // 1073689560
    // ptr = &b; // illegal statement, (assignment of read-only variable ptr)
    *ptr2 = b; // changing the value at the address ptr is pointing to
    Serial.printf( "2c. Value pointed to by ptr2: %c\r\n", *ptr2);  // B
    Serial.printf( "2d. Address ptr2 is pointing to: %d\r\n\r\n", ptr2);  // 1073689560

    const char *const ptr3 = &a;
    Serial.printf( "3a. Value pointed to by ptr3: %c\r\n", *ptr3);  // B
    Serial.printf( "3b. Address ptr3 is pointing to: %d\r\n\r\n", ptr3);  // 1073689560
    ptr = &b; // should be illegal statement and does not do anything, ptr3 unchanged 
    // *ptr = b; // illegal statement, (assignment of read-only location *ptr)
    Serial.printf( "3c. Value pointed to by ptr3: %c\r\n", *ptr3);  // B
    Serial.printf( "3d. Address ptr3 is pointing to: %d\r\n\r\n", ptr3);  // 1073689560

    char *var0  = "a2960";          // A mutable pointer to mutable character/string  deprecated conversion
        // C and C++ differ in the type of the string literal.
        // In C the type is array of char and in C++ it is constant array of char.
        // Use   foo((char *)"hello")   or  char *x = (char *)"foo bar";
    
    // char *var1  = "b";       //  deprecated conversion
    char *var1  = (char *)"b";  // A mutable pointer to mutable character/string 
    // char *var1  = 'b';       //  invalid conversion from 'char' to 'char*'

    const char * var2 = var0;       // Mutable pointer to an immutable string/character
    char * const var3 = var0;       // Immutable pointer to a mutable string
    const char * const var4 = var0; // Immutable pointer to an immutable string/character
    char  var5  = (int) 99;       // cannot use "c"  
    char  var6  = 99L;            // cannot use "c"  , use format https://en.cppreference.com/w/cpp/language/integer_literal.html
    char  var7  = 99;             // cannot use "c"  
    char  var8  = 99ul;             // cannot use "c"  
    Serial.printf( "4a. Value pointed to by var1: %c\r\n", *var1);  // b  
    Serial.printf( "4b. Value pointed to by var2: %d\r\n", *var2);  // 97   %d will print numeric a = 97 
    var2 = var1;
    Serial.printf( "  4b. mutable ptr var2: %d\r\n", *var2);        // 98   %d will print numeric b = 98 
    Serial.printf( "4c. Value pointed to by var3: %c\r\n", *var3);  // a    %s will crash
    Serial.printf( "4d. Value pointed to by var4: %c\r\n", *var4);  // a
    Serial.printf( "4e. Value pointed to by var5: %c\r\n", var5);   // c
  #endif
}

/*
  Execute test string casting c_str, array, publishmqtt
*/
void command_testH2(){    // Execute test string casting c_str, array, publishmqtt
  #ifdef TEST_MODE      
    // String mqttMsg = "test1234:a" ;  // start of Json error message        
    // const char* mqttMsg = nullptr; // to check for mullptr
    publishMqtt(mqttErrorTopic, (String) "command_testH2 file " + __FILE__ +  ", line" + __LINE__ ); 
    //         + ", c++ version="  + __cplusplus); // v51 with (String) ok marking version=201103 (C++11)
    String arduinoString = "Temperature: %.1f°C";
    char buffer[50];
    float temp = 23.5f;
    // Simple conversion - preferred method
    snprintf(buffer, sizeof(buffer), arduinoString.c_str(), temp);
    publishMqtt(mqttErrorTopic, buffer);      // v51:   CurrentPowerConsumption: %lu

    char output5[128];     // use snprintf to format data
    String msg5 = "{\"currentTime\":\"%d\", \"CurrentPowerConsumption\":%lu }";   // warning %s iso %d will crash
    snprintf(output5, sizeof(output5), msg5.c_str(), millis(), CurrentPowerConsumption);
    publishMqtt(mqttErrorTopic, output5);      // v51:   CurrentPowerConsumption: %lu

  // v51 test this snprint construction , crashes
    char output4[] = "secs:123456 51:51:51";     // use snprintf to format data
    unsigned long allSeconds = currentMillis / 1000;  // take time of mailoop
      int runHours = (allSeconds / 3600) % 24;
      int secsRemaining = allSeconds % 3600;
      int runMinutes = secsRemaining / 60;
      int runSeconds = secsRemaining % 60;
    snprintf(output4, sizeof(output4), "secs:%06d %02d:%02d:%02d", allSeconds, runHours, runMinutes, runSeconds);
    publishMqtt(mqttErrorTopic, output4);      // v51:   CurrentPowerConsumption: %lu

    char msgpub3[128];     // allocate a message buffer
    char output3[128];     // use snprintf to format data
    // std::string msg3 -->  std::string' has no member named 'concat' and so on
    // class String {  --- The string class in /home/pafoxp/.platformio/packages/framework-arduinoespressif8266@1.20401.3/cores/esp8266/WString.h
    //      --> https://cplusplus.com/reference/string/wstring/

    
    String msg3 = "{"; // build mqtt frame 
    msg3.concat("\"currentTime\":\"%d\"");                  // %s is string whc will crash
    msg3.concat(",\"CurrentPowerConsumption\":%lu }");    // P1
    msg3.toCharArray(msgpub3, 64);   
    msg3.toCharArray(msgpub3, sizeof(msgpub3)); // v51 convert/move/vast to char[]
    publishMqtt(mqttErrorTopic, msg3);       // v51: {"currentTime":"%s","CurrentPowerConsumption":%lu
    publishMqtt(mqttErrorTopic, msgpub3);    // v51: {"currentTime":"%s","CurrentPowerConsumption":%lu

    // const char* dst = src.c_str(); // returns const char* to an ASCIIZ (NUL-terminated) representation of the value   
    // const char* dst = src.data();  // returns const char* to the string's internal buffer
    //   const char* output3_data = msg3.data();  // C native returns const char* to the string's internal buffer
    //   const char* output3_str  = msg3.str();  // C native returns const char* to the string's internal buffer
    // snprintf(output3, sizeof(output3), msgpub3, millis(), CurrentPowerConsumption);  // v51 crashes if msgpub3 null
    snprintf(output3, sizeof(output3), "Millis=%d Power=%d", millis(), CurrentPowerConsumption);  // v51 crashes if msgpub3 null
    publishMqtt(mqttErrorTopic, output3);   // v51:   CurrentPowerConsumption: 436

    // snprint: Write formatted output to sized buffer https://cplusplus.com/reference/cstdio/snprintf/
    // snprintf(output3, sizeof(output3), msgpub3,millis(), CurrentPowerConsumption);  // v51 Aduino crashes

    // snprintf(output3, sizeof(output3), msg3.c_str(), millis(), CurrentPowerConsumption);  // v51 Aduino crashes
    // publishMqtt(mqttErrorTopic, output3);   // v51:   CurrentPowerConsumption: 436        
    
    // Serial.println((String)"\n\rv51 snprint" + output3 + "n\r");
    // publishMqtt(mqttErrorTopic, output3);   // v51:   CurrentPowerConsumption: 436

  /*
  // v51 test this snprint construction , crashes
    char msgpub3[64];     // allocate a message buffer
    char output3[64];     // use snprintf to format data
    String msg3 = "{"; // build mqtt frame 
    msg3.concat("\"currentTime\":\"%s\"");                  // %s is string
    msg3.concat(",\"CurrentPowerConsumption\":%lu");    // P1
    msg3.toCharArray(msgpub3, 64);   
    snprintf(output3, sizeof(output3), msgpub3,         // snprint clearly causes crash !!!!
            millis(),
            CurrentPowerConsumption);     
    publishMqtt(mqttErrorTopic, output3);   // v51:   CurrentPowerConsumption: 436
  

  // v51 test thiss construction
    char msgpub2[32];     // allocate a message buffer
    char output2[32];     // use snprintf to format data
    String msg2 = "";      // initialise data
    msg2.concat("CurrentPowerConsumption: %lu");       // format data
    msg2.toCharArray(msgpub2, 32);                     // move it to format buffwer
    sprintf(output2, msgpub2, CurrentPowerConsumption); // insert datavalue  (Note if using multiple values use snprint)
    client.publish(mqttPower, output2);     // v51:   CurrentPowerConsumption: 436
    publishMqtt(mqttErrorTopic, msg2);      // v51:   CurrentPowerConsumption: %lu
    publishMqtt(mqttErrorTopic, output2);   // v51:   CurrentPowerConsumption: 436
  
  // test these approaches
    String mqttMsg999 = "{";  // start of Json
    mqttMsg999.concat("\"error\":999 ,\"msg\":\"Check mqttMsg999\"}");  // v51 ok
    publishMqtt(mqttErrorTopic, mqttMsg999);
    
    publishMqtt(mqttErrorTopic, "constantdata" ); // v51 ok "constantdata", looks as a string
    publishMqtt(mqttErrorTopic, "constantdata" + __LINE__); // v51 wrongly: "SID set:"
    publishMqtt(mqttErrorTopic, "constantdata" + (String)__LINE__); // v51 tbd

    publishMqtt(mqttErrorTopic, (String) "String1_" + __LINE__); // v51 with (String) ok
    publishMqtt(mqttErrorTopic, "string2_" + __LINE__); // v51 wrongly "(null)"
    
    String mqttMsg2 = "String3_" + (String) __LINE__ ; // v51 ok 
    publishMqtt(mqttPower, mqttMsg2); // v51 ok
    String mqttMsg3 = "string4_" + __LINE__ ;
    publishMqtt(mqttPower, mqttMsg3); // v51 wrong --> "ntication Failed"
    publishMqtt(mqttPower, (String) mqttMsg3); // wring --> "ntication Failed"

    char outputData1[32];     // use snprintf to format data
    sprintf(outputData1, "__LINE__=%lu", __LINE__); // Note: if using multiple values use snprint
    client.publish(mqttPower, outputData1);        // v51 ok as outputData1 = String'ed
    
    char outputData3[16]={}; // define array to veriy behavior, initialised to 0x00
    outputData3[0]= 'A';     // this convert A to a number
    outputData3[1]= 'B';     // this convert B to a number
    outputData3[2]= 'C';     // this convert C to a number
    outputData3[3]= 'D';     // this convert D to a number
    if (outputData3[4] == 0x00) outputData3[3] = 'E';   // v51 check behavior of array [4]=0x00 (yes) ABCE
    outputData3[14]= 'Z';    // this convert to a number
    outputData3[15]= 0x00;   // this convert to a number
    client.publish(mqttPower, outputData3);        // v51 works as array is terminated 0x00
    outputData3[2]= 0x00;     // this convert C to a number
    client.publish(mqttPower, outputData3);        // v51 array early terminated (yes) "AB"
  */
    publishMqtt(mqttErrorTopic, (String) "h-test" + __LINE__); // v51 with (String) ok marking
  #endif        
}

void command_testH3(){    // check for null pointers en !mqtttopic
  #ifdef TEST_MODE
    publishMqtt(mqttErrorTopic, "TestH3a");      // v51:   CurrentPowerConsumption: %lu
    publishMqtt("", "TestH3b");      // v51:   CurrentPowerConsumption: %lu
    const char *mqttErrorTopic_h3 = "/error!/"  P1_VERSION_TYPE;
    publishMqtt(mqttErrorTopic_h3, "TestH3c");      // v51:   CurrentPowerConsumption: %lu
  #endif        
}

/* leave this for leaer to investigate why runtime-error is not found ...
void throwExceptionFunction(void) {
    throw runtime_error("exception thrown."); // here2
}
*/


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
