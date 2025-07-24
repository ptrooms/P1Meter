#define TEST_MODE           // set for Arduino to prevent default production compilation
// #define DEBUG_ESP_OTA    // v49 wifi restart issues 
//Note: disabled MDNS in  file://home/pafoxp/.platformio/packages/framework-arduinoespressif8266@1.20401.3/libraries/ArduinoOTA/ArduinoOTA.cpp

#define VERSION_NUMBER "58" // number this version


#include <core_version.h>       // v57 ensure we have the Arduino build version here (main.cpp --> )
#ifndef ARDUINO_ESP8266_RELEASE
  #error canceled core_version.h not found
  // should have #define ARDUINO_ESP8266_RELEASE "2_4_1" // ~/.platformio/packages/framework-arduinoespressif8266@1.20401.3/cores/esp8266/core_esp8266_main.cpp 
  // should have #define ARDUINO_ESP8266_RELEASE "2_7_1" // ~/.platformio/packages/framework-arduinoespressif8266@3.20701.0/cores/esp8266/core_version.h
#endif

/* dev 1153 epc1=0x401022c5 PROD_MODE v58c P1Meter.ino version 2158.2_4_1, compiled Jul 23 2025 18:12:52

   dev 1153 epc1=0x401025dd, COP_MODE V58c P1Meter.ino version 6158.2_4_1, compiled Jul 23 2025 18:13:16 wdt1153
    assemble file: file://home/pafoxp/code-P1Meter/.pio/build/p1meter-production_241_copy/firmware.asm
        401025ce:	fe6421               	l32r	a2, 40101f60 <trc_NeedRTS+0x238>
        401025d1:	fe6431               	l32r	a3, 40101f64 <trc_NeedRTS+0x23c>
        401025d4:	81a442               	movi	a4, 0x481
        401025d7:	f99a01               	l32r	a0, 40100c40 <ppTxqUpdateBitmap+0x28>
        401025da:	0000c0               	callx0	a0
        401025dd:	ffff06               	j	401025dd <wDev_ProcessFiq+0x341>
        401025e0:	000000               	ill
        401025e3:	c0                      	.byte 0xc0
*/

/*  documentation   
  feeding data test_mode: while sleep 8; do ./sendp1_cr.sh > /dev/ttyUSB2; sleep 2; ./sendp2.sh > /dev/ttyUSB2; done
  feeding data  cop_mode: while sleep 8; do ./sendp1_crlf.sh > /dev/ttyUSB2; sleep 2; ./sendp2.sh > /dev/ttyUSB2; done

  21jul25: debug serial2.4.1. library debug : 
  1. search SoftwareSerial241.cpp.o in /home/pafoxp/code-P1Meter/.pio/build
    /home/pafoxp/code-P1Meter/.pio/build/p1meter-production_271/lib4c2/SoftwareSerial241-P1/SoftwareSerial241.cpp.o:
     file format elf32-xtensa-le
  2. Note: we use Xobjdump which is locally linked by:
      ln /home/pafoxp/.platformio/packages/toolchain-xtensa/bin/xtensa-lx106-elf-objdump /home/pafoxp/.local/bin/Xobjdump 
  3. Disassembl;e      
      Xobjdump -C -d /home/pafoxp/code-P1Meter/.pio/build/p1meter-production_241_copy/lib4c2/SoftwareSerial241-P1/SoftwareSerial241.cpp.o
      Note add this to conserve out: > /home/pafoxp/code-P1Meter/debug/SoftwareSerial241`date +%Y%M%d-%H%m`.asm


    https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
    api reference: https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
    sdk: https://github.com/espressif/ESP8266_NONOS_SDK/tree/release/v2.2.x
                  https://espressif.com/sites/default/files/documentation/2a-esp8266-sdk_getting_started_guide_en.pdf

    def RX2TX2LOOPBACK false -->  def NoTx2Function  <> loopbackRx2Tx2 will light led

  Locations:
    commands: void ProcessMqttCommand(char* payload, unsigned int myLength)
    Setup ()
    P1 processing: void readTelegramP1()
    P2 processing: void readTelegramWL()

  Summarised loop( ) sequence (approx 5000/sec): 
      set timings & check P1 not active
      else swap start Serial P1 <--> P2
      Check if/do water trigger
      If P2 Do WL process 
      If P1 Do P1 process, when data processGpio() functions, publishP1ToMqtt();  
      set timings, trace line and "loopcnt % 10"
      If no data: do forced processGpio() functions, publishP1ToMqtt();  
      OTA handle

*/

/* tbd 
  change record/idś to definitions like "/KFM5KAIFA-METER"
  change \r\n to \r\n to 
  sometimes after OTA restart, noping  & Attempt MQTT connection to nodemcu-d1 ...failed to 192.168.1.8
      -- strange as wifi proces during setup( ) before was OK
  change verboselevel == 1 to ==2 that will print myLenbgths of P1 records print
  documentation for mqtt commands
  cleanout no longer needed code
    error: 06jul25 13u00 (using faulty esp, port0 > 3.6volts) 
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
  - v58c refactor len to myLen, debugging options
    - testing for (int i = 14; i < 11; i--) {asm NOP}
    - adding 4 delay()s to check if this stabilizes the COP_MODE version
  - v58b refactor myserial1/2 names, enhanced bittime, stop and baudrate control
    - we added serial read mode 0/1/2: 0-physical port, 1-P1 record, 2=WL-record
  - v58a  --> moved to production
    - likely cause that serial processing stays tool long in ISR, with wifi debug , we see dev:1153
          https://github.com/espressif/ESP8266_NONOS_SDK/issues/90
  v58 based on corrected master
    - we extended platformio to generate Disassemble after compile, this to interrogate result
      which could als be doen by executing the Extensa "objdump" that reads the ELF file to disassemble output.
      This is done via Python post_build.py (for Arduino 2.4.1) and post_build_271.py (2.7.x)
      Activated is adding & activating the  .ini options per project environment
          ;extra_scripts = post_build.py
          ;targets = disasm
    - during testing we exprience frequent WDT crashes as related to Wifi, why is a mystery
      Only the Live production  seems more stable. Perhaps this interelates with actual telegram data.
    - inroduced COP_MODE to differentiate TEST/PROD_MODE only by IP_Address and Mqtt-prefix:x1 & versionid: 61 
      
      internally TEST_MODE & PROD_MODE are different code lines to check and test
        During TESTing we use real usb serial while PRODduction uses negative polaritye
          DUP_MODE replicates and revert the active serial polarity and character termination
          COP_MODE does not change any serial behavior and only differentiate on IP address and Mqtt prefix
      
      We now have (summarised) the following 6 code paths with platformio (Arduino framework 2.4.1):
        PROD_MODE   : prefix p1 and versionid 21  - [env:p1meter-production_241]      LGTM 
          DUP_MODE  : prefix d1 and versionid 41  - [env:p1meter-dup-production_241]  erratic ??
          COP_MODE  : prefix x1 and versionid 61  - [env:p1meter-production_241_copy] limited
        TEST_MODE   : prefix t1 and versionid 11  - [env:t1meter-test-OTA_241] 
          DUP_MODE  : prefix e1 and versionid 31  - n.i.u.
          COP_MODE  : prefix x1 and versionid 71  - n.i.u.
    in addition we have (optional) flag:
        DUP_MODE_NOINVERT which reverses serial polarity but keeps the end-data character

    Note: Platformio is also used to differentiate Arduino on platform or framework:
    All are using the SDK framework to generate/ompile code via the "toolchain-xtensa"
      - Arduino 2.4.1 has it platform  espressif8266@1.7.0 as package framework-arduinoespressif8266@1.20401.3
      - Arduino 2.7.1 has it platform  espressif8266@2.5.3 as package framework-arduinoespressif8266@3.20701.0
        Note we have also others (2.4.0, 2.6.2, 2.7.4, 2.7.8, 3.1.2) but these are merely to check support issues 
      
    - copy PROD contents to testdevice, the thing collaapses as test data is temrinated with CR (\r = 0x0d)
    - created 3 data options
      --> sendp1_nl.sh = termnated solely by NewLine  (\n = 0x0A) (works on PROD_MODE)
      --- sendp1_crlf.sh = termnated solely by Carriage Return & NewLine  (\r = 0x0D & \n = 0x0A)
      --- sendp1_cr.sh = termnated solely by Carriage Return (\r = 0x0D)
      Test op PROD_MOPDE 
  v57 troubles  as master
    - One way or the other the DUP_MODE of PROD_MODE behaves differently and crashes after approx 10-80 cycles
      Possible while the routine collapses on unexpexted serialised input terminated by CR (0x0D)
    - furthermore, we've had quite some problems to gfet the version numbers right.
      Now on master, we created v58 as new syncrpoint
  v57 improve state line diagnostics, updated Read.me, ensure P1/RX swapping
      - 2.7.1 not stable on duptest goes after 20-100 mqtt  into wdt_reset @ 401031f1
woes in Wifi/Lamx layer
        read also https://www.esp8266.com/viewtopic.php?f=9&t=3979&start=20
         "wdev.c 1166" is printed on the UART before the crash. 
            401031e0:	0adc                	bnez.n	a10, 401031f4 <wDev_ProcessFiq+0x28c>
            401031e2:	fe8e21               	l32r	a2, 40102c1c <rcReachRetryLimit+0x19c>
            401031e5:	fe8e31               	l32r	a3, 40102c20 <rcReachRetryLimit+0x1a0>
            401031e8:	8ba442               	movi	a4, 0x48b
            401031eb:	fa2801               	l32r	a0, 40101a8c <ppTxqUpdateBitmap+0x28>
            401031ee:	0000c0               	callx0	a0
            401031f1:	ffff06               	j	401031f1 <wDev_ProcessFiq+0x289>
      - tested using Arduino 2.7.1
      - PlatformIO enhancements
      - introducing P1 records via bash script command
        $ while sleep 8; do ./sendp1.sh > /dev/ttyUSB2; sleep 2; ./sendp2.sh > /dev/ttyUSB2; done
        -  P1: /home/pafoxp/code-P1Meter/sendp1.sh
        -  P2: /home/pafoxp/code-P1Meter/sendp2.sh
  master - 18jul25 - github stable
  v56c - as merged from/to v52
      - stabilized
  v56c  investigage  erratic based on v56
      - cosmetic: nam chanted to  readTelegramP1 readTelegramWL to ease usage
      - RX_yieldcount = 8  --> RX_yieldcount = 3 when Yield1080 is hit uin a row to ease reads
  v56 - reworked to stable
      - rpelicated to test/production
      - For DUP_MODE (running code on test device with different ID: d1 iso p1 we use normall serial
        However, before V56  - for serial - this was slishtly different for the Test-duplicate device.
        that was using regular PC/USB serial, hence reverse the invert for normal polarity.
        When we use flag DUP_MODE_NOINVERT this - using regular serial USB - behavior is activated.
      - 271 activated to check on things
      - 241 OK 
  v55f - restored back to v55b
  v55e - erratic
  v55d - works good, based on v55c but added some serial.debugs, a bit worse than v55b
    - production:  epc1=0x401031f1, epc2=0x00000000, epc3=0x00000000, excvaddr=0x00000000,depc=0x00000000
    - remove to ...../debug/SoftwareSerial
    - commented
    - reduplicated from v55b
    - time 1 cycled updated from 497 to 500 in libs/SoftwareSerial241-P1/SoftwareSerial241.cpp
  v55c - woes completely on serial
  v55b = perfectly tuned
  v55a - debug to checkout code differences for: teststable
    - introducing DUP_MODE to identically replicate TESYT/PROD_MODE with
         reference DUP_MODE with TEST_MODE ip 185 = ip 185 with prefix e1 and version 3xxxx
         reference DUP_MODE with PROD_MODE ip 35  = ip 185 with prefix d1 and version 4xxxx
    - 2.7.1  started to use on production, LGTM
    - 2.4.1. PROD_MODE require about 10-17 (unused) "delay()"" to arrange that code goes stable.
            if not the use of an printf() statement (in section line 2585) causes a ridiculous instability
  v55 comments new line
    - this version looks to be very unstable
  v54 from master to restart porting 2.4.1. where we left off
    - 2.4.1 CRC succesfull, see NL article [https://gathering.tweakers.net/forum/list_message/82920194#82920194]
    - 'T' command (diagnose data):
          loopbackRx2Mode  0-9 , no data: swap/switch
          loopbackRx2Mode == 1 --> switch off debug (outputOnSerial), activate RX2 read evey second (rx2ReadInterval = 1)
          If set > 0: we produce diagnostic data when RX2 is read  {_   _}
            myLength/telegram2 record
          If set > 5: when CRC is valid, we output the hexbytes and reset back to 0
          If set = 3: we print crc and record
          If set = 2: we print crc and first 20 bytes of header in hex
          If set = 1: we print echo while reading RX2 to debug console {_  }..len=3:'abc'.. _}  
    - Note: on PROD_MODE we process serialdata inverted, using TEST_MODE this is standard (1=rising).
  v53 unstable branch when we have (inactive) conditional prints which crashed, detached
  V53 new version on V52 renamed to master
    - using 2.4.1 i stable, 2.7.1 is less reliable
      perhaps timing is different. TBI
    
    - initialised , note: v52 version (2.5GB) saved into /media/pafoxp/movies1/save_platformio/code-P1Meter
      Force up to dat branch to be master: 
        read: [https://www.geeksforgeeks.org/git/how-to-replace-master-branch-with-another-branch-in-git/]
        git remote -v
        git checkout v52        // ensure on branch
        git pull origin v52     // sync
        git branch -D master    // delete master
        git branch -m master    // rename current to master
        git push origin master --force  // force sync
        git fetch origin        // get back
        git checkout master     // checkout master
        git log                 // check log status
  V52 13jul25 02u35 renamed to master
  V52 13jul25: restart information display
     protection "suspend @getValues2FromP1Record while mqttCnt < 5" unneeded, now commented
    'e0'  = divide error
    'e1'  = infinite loop fault
    'E'   = Enforce read fault in Reading P1 data
    testing serial processing on stable 2.4.1 and less stable 2.7.1
  V52 06jul25: restart information display
    command 'S'/'s' serial1Stop serial2Stop (prohibit serial data)
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
    WaterTrigger0/1_ISR() attach/detachWaterInterrupt is activated in loop( ) during peiodes not reading serial1/2 data.
      With command 'W' we can test/switch between WaterTrigger0/1_ISR (used for development reasons)
      (tbd: With command 'w' we could change/force next pullup behavior, now active/low/high automatically)

    Using a wheel that senses mirrored Infrared: 1 pulse (going in --> going out) = 1 Liter
        We use active GPIO5 Interrupt Service Routine for the cycle below, where a segmented turn = liter
        An UP or Down is only valid if this continues to be the case for at least 50mS (v51, waterReadDebounce=50)
           (debounce measuremnt starts/resets in ISR and is monitored in in loop( ))
    then: in loop( ) If -->
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
  // note 115100 with physical ionverted device is not reliable.
  #define P1_BAUDRATE  115200       // use this baudrate for the Test p1 serial connection, usb require e bit lower speed
  #define P1_BAUDRATE2 115200       // use this baudrate for the Test p1 serial connection, usb require e bit lower speed
#else
  // note: 115250 < baudrate < 115150 will increase errors and misreads
  #define P1_BAUDRATE  115200       // use this baudrate for the p1 serial connection, 115200 is de sweepspot
  #define P1_BAUDRATE2 115200       // use this baudrate for the p1 serial connection, 115200 is de sweepspot
#endif


// ARDUINO_ESP8266_RELEASE
#ifdef TEST_MODE
  #warning This is the TEST version, be informed
  #ifdef DUP_MODE   // test is same as test with different i/o & id settings
    #define P1_VERSION_TYPE "e1"      // "t1" for ident nodemcu-xx and other identification to seperate from production
    #define DEF_PROG_VERSION "31" VERSION_NUMBER "." ARDUINO_ESP8266_RELEASE // current version (displayed in mqtt record)
  #elif defined(COP_MODE)
    #define P1_VERSION_TYPE "y1"      // "y1" test
    #define DEF_PROG_VERSION "71" VERSION_NUMBER "." ARDUINO_ESP8266_RELEASE // current version (displayed in mqtt record)    
  #else
    #define P1_VERSION_TYPE "t1"      // "t1" for ident nodemcu-xx and other identification to seperate from production
    #define DEF_PROG_VERSION "11" VERSION_NUMBER "." ARDUINO_ESP8266_RELEASE // current version (displayed in mqtt record)
  #endif
  #define TEST_CALCULATE_TIMINGS    // experiment calculate in setup-() ome instruction sequences for cycle/uSec timing.
  #define TEST_PRINTF_FLOAT       // Test and verify vcorrectness of printing (and support) of prinf("num= %4.f.5 ", floa 
#else
  #warning "This is the PRODUCTION version be warned !!!!!."
  #ifdef DUP_MODE   // prod is same as prod with different i/o & id settings
    #define P1_VERSION_TYPE "d1"      // "p1" production
    #define DEF_PROG_VERSION "41" VERSION_NUMBER "." ARDUINO_ESP8266_RELEASE // current version (displayed in mqtt record)    
  #elif defined(COP_MODE)
    #define P1_VERSION_TYPE "x1"      // "x1" production
    #define DEF_PROG_VERSION "61" VERSION_NUMBER "." ARDUINO_ESP8266_RELEASE // current version (displayed in mqtt record)    
  #else
    #define P1_VERSION_TYPE "p1"      // "p1" production
    #define DEF_PROG_VERSION "21" VERSION_NUMBER "." ARDUINO_ESP8266_RELEASE // current version (displayed in mqtt record)
  #endif
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
// 30sep22 22u26: v35 serial2Baudrate 115k2 for "warmtelink" 
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
// - 13apr21 18u38 V21 improved WDT as we call "mqtt client".loop( ) during speific yields,
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
    DEBUG_WRITE_BYTES(data, myLength); DEBUG_PRINTLN();
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
  #ifdef DUP_MODE_NOINVERT    // v57 reverse for TEST_MODE, normally uninverted
    bool bSERIAL_INVERT  = true;  // Direct P1 GPIO connection require inverted serial levels (TRUE) for RS232
    bool bSERIAL2_INVERT = true;  // GJ meter is inverted (output RX2 does pulldown Gpio) v53
  #else
    bool bSERIAL_INVERT  = false; // Simulated P1 meter is USB interface of a PC to GPIO, does not required invert
    bool bSERIAL2_INVERT = false; // Simulated GJ meter is USB interface , does not require invert v53
  #endif

#else
  // note: DUP_MODE will invert to allow regular USB/serial, else we'll use the default (inverted polarity of serial-P1)
  #ifdef DUP_MODE_NOINVERT    // v56 reverse for PROD_MODE normally inverted
    bool bSERIAL_INVERT  = false; // v53a Simulated P1 meter is USB interface of a PC to GPIO, does not required invert
    bool bSERIAL2_INVERT = false; // v53a Simulated GJ meter is USB interface , does not require invert v53
  #else
    bool bSERIAL_INVERT  = true;  // Direct P1 GPIO connection require inverted serial levels (TRUE) for RS232
    bool bSERIAL2_INVERT = true;  // GJ meter is inverted (output RX2 does pulldown Gpio) v53
  #endif
#endif

// #define bSERIAL2_INVERT false // GJ meter is as far as we  know normal  serial (FALSE) RS232  < 03okt22
// #define bSERIAL2_INVERT true // GJ meter is as far as we  know normal (FALSE) RS232
// #define bSERIAL2_INVERT true // GJ meter is as far as we  know normal  serial (FALSE) RS232 @direct p1 03okt22


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

const char  *prog_Version = DEF_PROG_VERSION;  // added ptro 2021 version , v57 changed from int to char

#ifndef ARDUINO_ESP8266_RELEASE 
  /*
   2.4.1: file /home/pafoxp/.platformio/packages/framework-arduinoespressif8266@1.20401.3/cores/esp8266/core_version.h
     main file:   [/home/pafoxp/.arduino15/packages/esp8266/hardware/esp8266/2.4.1/cores/esp8266/core_esp8266_main.cpp]
        static void loop_wrapper() --> setup( ) else loop( ); run_scheduled_functions(); esp_schedule()
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
      IPAddress local_IP(192, 168, 1, 185);
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
unsigned long startMicrosP1  = 0; // micros() when P1 went active (reading header '/')

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
#ifndef MAXTELEGRAM2CNT       // define maximum number of RX2 reads during readtelegram2
  #define MAXTELEGRAM2CNT 2
#endif
int loopTelegram2cnt = MAXTELEGRAM2CNT;  // Maximum of readtelegram2 on 2nd RX port before we  flsh and doe force regular p1 again.
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

bool bSerial1State = false; // v57 indicate state
bool bSerial2State = false; // v57 indicate state


long serial1Baudrate  = P1_BAUDRATE;   //  V31 2021-05-05 22:13:25: set programmatic speed which can be influenced by teh bB command.
long serial2Baudrate = P1_BAUDRATE2;   //  V35 2022-09-30 22:25:25: set programmatic speed which can be influenced by teh bB command.
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
int  loopbackRx2Mode  = 0;       // '0' Testloopback RX2 to TX2 (OFF, 1 test-check, 5=crc
bool outputMqttLog    = false;   // "l" false -> true , output to /log/p1
bool outputMqttPower  = true;    // "P" true  -> false , output to /energy/p1
bool outputMqttPower2 = true;    // "p" true  -> false , output to /energy/p1power
bool rx2_function     = true;    // "F" true  -> false , use v38 RX2 processing (27feb23 tested, LGTM)
bool outputMqttLog2   = false;   // "L" false -> true , output RX2 data mqttLogRX2 /log/wl2
bool serial1Stop     = false;   // 'S' stop read/inputting serial1 P1 data  (v52)
bool serial2Stop    = false;   // 's' stop read/inputting serial2 RX2 data (v52)
long rx2ReadInterval  = 7;       // 's' decreases also de readinterval
bool doReadAnalog     = true;    // 'a' yes/no read analog port for value
bool doForceFaultP1   = false;   // 'E' yes/no force fault in Read telegram
int serial1PortMode   = 0;       // v58b instruct SS241 to read 0=GPIO, 1 - use internal for test
int serial2PortMode   = 0;       // v58b instruct SS241 to read 0=GPIO, 2 - use internal for test
char serial1terminator = '\x0a';  // v58c termination telegrams serial1 (test/dup = \x0d)
char serial2terminator = '\x00';  // v58c termination telegrams serial1 (test/dup = \x0d)

// Vars to store meter readings & statistics
bool mqttP1Published = false;        //flag to check if we have published
long mqttCnt = 0;                    // updated with each publish as of 19nov19
long p1MissingCnt = 0;         // v52 updated when we failed to read any P1
long p1CrcFailCnt = 0;         // v52 updated when we Crc failed
long p1RecoverCnt = 0;         // v52 updated when we successfully recovered P1
long p1FailRxCnt  = 0;         // v52 updated P1 does not seem to be connnected
long p1ReadRxCnt  = 0;         // v52 updated P1 has read an ok Crc
bool bP1Active_signalled = false; // v57 indicated we have/had a P1Active() while doing serial read

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
// char dummy2[17];       // add some spare bytes, v55b remove
// const char dummy2a[] = {0x0000};    // prevent overwrite, v55b remove
char telegram[MAXLINELENGTH+32];       // telegram maxsize bytes for P1 meter
char telegramLast[3];               // used to catch P1meter line termination bracket
bool telegramP1header = false;      // used to trigger/signal Header window /KFM5KAIFA-METER
//DebugCRC int  testTelegramPos  = 0;          // where are we
//DebugCRC char testTelegram[MAXLINELENGTH];   // use to copy over processed telegram // ptro 31mar21

unsigned int dataInCRC  = 0;          // CRC routine to calculate CRC of data telegram_crcIn
int publishP1ToMqttCrc = 0;           // 0=failed, 1=OK, 2=recovered
bool validCrcInFound = false;         // Set by Decode when Full (recovered) datarecord has valid Crc
int  telegram_crcIn_rcv = 0;          // number of times DataIn could be recovered
int  telegram_crcIn_cnt = 0;          // number of times CrcIn was called
char telegram_crcIn[MAXLINELENGTH+32];   // active telegram that is checked for crc
int  telegram_crcIn_len = 0;          // myLength of this record
int  telegram_crcOut_cnt = 0;         // number of times masking was positioned
char telegram_crcOut[MAXLINELENGTH+32];  // processed telegram with changed positions to X
// const char dummy2b[] = {0x0000};      // prevent save overwrite, v55b remove
// int  telegram_crcIn_cnt1 = 0;      // number of times CrcIn was called
int  telegram_crcOut_len = 0;         // myLength of this record
// int  telegram_crcIn_cnt2 = 0;      // number of times CrcIn was called

/* At header: initialise telegram_crcin
  during CRC we accumulate record positions into telegram_crcin
  At trailer:
    if telegram_crcIn_len = telegram_crcOut_len and crcValidCounter > 0
      if CRC=OK
        // Mark telegram_crcOut with X changes comparing same telegram_crcin
        for i=0 to myLen
          if telegram_crcOut(i) != X and telegram_crcOut(i) != telegram_crcIn(i)
          then telegram_crcOut(i) = X
               crcMaskCounter++      // count masked positions
        loop
        crcValidCounter++            // count valid CRC
      
      else:   // CRC failure on input, try to recover
        // try recover characters position: 
        for i=0 to myLen
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
        for i=0 to myLen
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
char dummyh4[MAXLINELENGTH2+32];        // v57c-3 after this somewhat stable

char telegram2[MAXLINELENGTH2+32];        // RX2 serial databuffer during outside P1 loop Plus overflow
char telegram2_org[MAXLINELENGTH2+32];    // RX2 serial databuffer during outside P1 loop Plus overflow v54
char telegram2Record[MAXLINELENGTH2+32];  // telegram extracted data maxsize for P1 RX2  v54 moved below
// char telegramLast2[3];             // overflow used to catch line termination bracket
// char telegramLast2o[19];           // overflow-area to prevent memory leak
bool bGot_Telegram2Record = false;    // RX2 databuffer  between /header & !trailer
long Got_Telegram2Record_prev = 0;    // RX2 number of sucessfull RX2 records before loop
long Got_Telegram2Record_cnt  = 0;    // RX2 number of sucessfull RX2 records total loop
long Got_Telegram2Record_last = 0;    // mqttCnt last Telegram2Record received
// const char dummy3a[] = {0x0000};      // prevent overwrite memoryleak, v55b remove
// const char dummy4a[] = {0x0000};      // prevent overwrite memoryleak, v55b remove


// Callback mqtt value which is used to received data
char mqttServer1[64] ;               // v45 used to hold the currrently active mqttserver
const char* submqtt_topic = "nodemcu-" P1_VERSION_TYPE "/switch/port1";  // to control Port D8, heating relay
int new_ThermostatState = 2;       // 0=Off, 1=On, 2=Follow, 3=NotUsed-Skip
void callbackMqtt(char* topic, byte* payload, unsigned int myLength);   // pubsubscribe
char mqttReceivedCommand[MQTTCOMMANDLENGTH] = "";      // same in String format time, will be overriden by Telegrams

// Note Software serial uses "attachinterrupt" FALLING on pin to calculate BAUD timing
// class: SoftwareSerial(int receivePin, int transmitPin, bool inverse_logic = false, unsigned int buffSize = 64);
#ifdef UseNewSoftSerialLIB
  //  2.5.2+ (untstable): swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
  SoftwareSerial mySerial1;      // declare our classes for serial1 (P1 115200 8N1 inverted baud
  SoftwareSerial mySerial2;     // declare our classes for serial2 (GJ 1200 8N1 baud)
#else
  /// @param baud the TX/RX bitrate
  /// @param config sets databits, parity, and stop bit count
  /// @param rxPin -1 or default: either no RX pin, or keeps the rxPin set in the ctor
  /// @param txPin -1 or default: either no TX pin (onewire), or keeps the txPin set in the ctor
  /// @param invert true: uses invert line level logic
  /// @param bufCapacity the capacity for the received bytes buffer
  /// @param isrBufCapacity 0: derived from bufCapacity (used for/with asynchronous)
  // 274 rubbish // SoftwareSerial mySerial1(SERIAL_RX, -1, true, MAXLINELENGTH); // (RX, TX. inverted, buffer)
  SoftwareSerial mySerial1(SERIAL_RX, -1          , bSERIAL_INVERT  , MAXLINELENGTH); // (RX, TX. inverted, buffer)
  SoftwareSerial mySerial2(SERIAL_RX2, SERIAL_TX2 , bSERIAL2_INVERT , MAXLINELENGTH2); // (RX, TX, noninverted, buffer)
  SoftwareSerial mySerial3(SERIAL_RX, -1          , (int) 3, 100);   // (RX, TX. bittiming)
  // SoftwareSerial mySerial4(SERIAL_RX2, SERIAL_TX2 , (int) 3, 100);   // (RX, TX, bittiming)
  // next causes a problem
  // SoftwareSerial mySerial3(SERIAL_RX, -1          , (int) 10, MAXLINELENGTH); // (RX, TX. inverted, buffer)
  // SoftwareSerial mySerial4(SERIAL_RX2, SERIAL_TX2 , (int) 10, MAXLINELENGTH2); // (RX, TX, noninverted, buffer)

  
  // SoftwareSerial mySerial2(SERIAL_RX2, SERIAL_TX2, (int) 10, MAXLINELENGTH2); // (RX, TX, noninverted, buffer)
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


void command_testH6(){    // v57c-2
  //                   // we remoived some unused protection arrays, improved ISR-time,
    String msg3 = "{"; // build mqtt frame 
    char msgpub3[64];     // allocate a message buffer
    // char output3[64];     // use snprintf to format data
    msg3.concat("\"currentTime\":\"%d\"");                  // %s is string whc will crash
    msg3.concat(",\"CurrentPowerConsumption\":%lu }");    // P1
    msg3.toCharArray(msgpub3, 64);   
    msg3.toCharArray(msgpub3, sizeof(msgpub3)); // v51 convert/move/vast to char[]
    publishMqtt(mqttErrorTopic, msg3);       // v51: {"currentTime":"%s","CurrentPowerConsumption":%lu
    // publishMqtt(mqttErrorTopic, msgpub3);    // v51: {"currentTime":"%s","CurrentPowerConsumption":%lu

}

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
  
  ESP.wdtEnable(WDTO_2S); // 8 seconds 0/15/30/60/120/250/500MS 1/2/4/8S , v55b activated
  // ESP.wdtEnable(33000); // allow three passses missing, v55b disabled

  Serial.begin(115200);
  Serial.println("Booting debug");              // message to serial log

  #ifdef DISABLE_MDNS_OTA_PTRO_INACTIVE           // check if we have disabled MDNS in ArduinoOTA.cpp
    Serial.println("MDNS is disabled. ");         // to snsure MDNS will not cause wdt bu excessive data
  #elif(defined DISABLE_MDNS_OTA_PTRO)         // Do we want by option to disable MDNS ?
    Serial.println("Please check DISABLE_MDNS_OTA_PTRO option in ArduinoOTA.cpp ");              // message to serial log
  #endif

  // v58c check if ArduinoOTA.cpp is answering this......
  #ifdef DISABLE_MDNS_OTA_PTRO_INACTIVE
    #warning "DISABLE_MDNS_OTA_PTRO_INACTIVE ArduinoOTA.cpp"
  #endif
  #ifdef DISABLE_MDNS_OTA_PTRO_ACTIVE
    #warning "DISABLE_MDNS_OTA_PTRO_ACTIVE ArduinoOTA.cpp"
  #endif
  #if(defined DISABLE_MDNS_OTA_PTRO)
    #ifndef DISABLE_MDNS_OTA_PTRO_INACTIVE
      #warning "DISABLE_MDNS_OTA_PTRO requested but not replied back by ArduinoOTA.cpp."
    #else 
      #warning "DISABLE_MDNS_OTA_PTRO requested and honoured by ArduinoOTA.cpp."
    #endif
  #endif
    
  Serial.setDebugOutput(true); 
  // dev1153

  /*  /home/pafoxp/.platformio/packages/framework-arduinoespressif8266@1.20401.3/libraries/ESP8266WiFi/src/

      wdt reset
      load 0x4010f000, myLen 1384, room 16
      tail 8
      chksum 0x2d
      csum 0x2d
      v614f7c32
      ~ld
   (Booting debug)
   [!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)]
      STUB: dhcp_stop   (>-- ?? ESP8266WiFiSTA.cpp)
   [WiFi.mode(WIFI_STA);            // Client mode]      
   [WiFi.persistent(true)]
   (STA Using fixed address: 192.168.1.185)
      scandone      (<-- ?? ESP8266WiFiScan.cpp)
      state: 0 -> 2 (b0)
      state: 2 -> 0 (2)
      reconnect     (<-- ESP8266WiFiSTA.cpp)
      scandone
      state: 0 -> 2 (b0)
      state: 2 -> 3 (0)
      state: 3 -> 5 (10)
      add 0
      aid 11 .. 3   (<--  ESP8266WiFiType.h ,  WiFiEventSoftAPModeStation/Disconnected)
      cnt
      connected with Pafo SSID4, channel 6
      ip:192.168.1.185,mask:255.255.255.0,gw:192.168.1.1
      ip:192.168.1.181
   [WiFi.setSleepMode(WIFI_MODEM_SLEEP)]
      sleep disable
   (Connecting to Pafo SSID4)
   (Wifi Password sent)
      scandone      (<-- ESP8266WiFiScan.cpp)
   (Connected and booted nodemcux1, using IP Address:192.168.1.185
   (ArduinoOTA.setHostname set)
   (ArduinoOTA.begin() activated.)
   (Reset reason code: 1)
    ....
    pm open,type:0 0    (?? open authentication and/or open network)
    ....
    dev 1153    (?? .../tools/sdk/lib/libpp.a)
    --> wdtreset

    MDNS:
      Reading answers RX: REQ, ID:0, Q:0, A:1, NS:0, ADD:0
      Not expecting any answers right now, returning


  22jul25 17u30 we saw wifi tery to reconnect
      bcn_timout,ap_probe_send_start
   
  22jul25 17u35
        bcn_timout,ap_probe_send_start
    -8-9

    ap_probe_send over, rest wifi status to disassoc
    state: 5 -> 0 (1)
    rm 0
    pm close 7
      Attempt MQTT connection to nodemcu-x1 ...failed to 192.168.1.8^scandone

    state: 0 -> 2 (b0)
    st
    ate: 2 -> 3 (0)
    state: 3 -> 5 (10)
    add 0
    aid 6
    cnt
      connected with Pafo SSID4, channel 1

      ip:192.168.1.185,mask:255.255.255.0,gw:192.168.1.1
      ip:192.168.1.185,mask:255.255.255.0,gw:192.168.1.1

      ^... try again in 5 seconds...
    , connect-rc=-1

    Attempt MQTT connection to nodemcu-x1 ...(re)connected to 192.168.1.8, connect-rc=0
    ; subscribed to >nodemcu-x1/switch/port1< .

   At OTA termination, we migt see:
    state: 5 -> 0 (0)
    rm 0
    pm close 7
    del if0
    usl
  */

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
    v55: test WiFi.persistent(false); to check if this affects stability of code changes
  */
  WiFi.persistent(true); // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083) since 29jun25 causes bootloop
  // WiFi.persistent(true); // Do not overwrites the FLASH if the settings are the same https://github.com/esp8266/Arduino/issues/1054#issuecomment-2662968960
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
  WiFi.setSleepMode(WIFI_NONE_SLEEP); // 22jul25 perhaps redo after WiFi.waitForConnectResult()
  // print your WiFi shield's IP address:
  Serial.print((String)"Connected and booted " + hostName + ", using IP Address:") ;    // v45 fyi
  Serial.println(WiFi.localIP());

  /*
    #ifdef UseNewSoftSerialLIB
      // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
      mySerial1.begin  (115200,SWSERIAL_8N1,SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH,0); // Note: Prod use require invert
      mySerial2.begin (  1200,SWSERIAL_8N1,SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2,0);
      // newserial: mySerial1.begin(115200, SWSERIAL_8N1, SERIAL_RX, -1, true , MAXLINELENGTH,0);  // define softserial  GPIO14
      // Serial.swap();       // use the alternative serial pins
      // Serial.begin(115200);   // use use UART0 (TX = GPIO1, RX = GPIO3) hardware
    #else
      mySerial1.begin(115200);    // P1 meter port 115200 baud
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
  ;                       // https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html
  ArduinoOTA.begin();     // ArduinoOTA.begin(false) = disable mdns

  Serial.printf("Reset reason code: %x\n", resetInfo->reason);     // v52: print restart reason
  Serial.println((String) "\r\nRestart time " + micros() + " for " + __FILE__ + " cause:" +  resetInfo->reason); // same but nicer
  Serial.printf("\t epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x,depc=0x%08x\r\n\n",
        resetInfo->epc1, resetInfo->epc2, resetInfo->epc3, resetInfo->excvaddr, resetInfo->depc); // v52: print registers

  save_reason   = resetInfo->reason;      // v52: save restart reason
  save_epc1     = resetInfo->epc1;        // v52: save restart register
  save_epc2     = resetInfo->epc2;
  save_epc3     = resetInfo->epc3;
  save_excvaddr = resetInfo->excvaddr;
  save_depc     = resetInfo->depc;

  Serial.println("Firmware version: "+ (String)P1_VERSION_TYPE + "-" 
                  + "(" + __DATE__ + " " + __TIME__ + ")." );
  Serial.println   ("ESP getFullVersion:" + ESP.getFullVersion());
  Serial.println ((String)"Arduino esp8266 core: "+ ARDUINO_ESP8266_RELEASE);  // from <core.version>
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
    Serial.printf ("\r\t1234.567 Value6.1f = %6.6f\r\n", 1234.567); // 1234.567000
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
  // note this loop( ) routine is as of date v51 04jul25 approximately called 5769/sec.dry, without P1/RX2
  
  // declare global timers loop(s)
  // millis() and micros() return the number of milliseconds and microseconds elapsed after reset, respectively.
  //  warning: this number will overflow (go back to zero), after approximately 70 minutes.
  currentMillis = millis(); // Get snapshot of runtime
  previousMicros = currentMicros; // get V47 previous loop time
  currentMicros = micros(); // get current cycle time

    // v58c: check if this stabilizes
    for (int i = 14; i < 11; i--) {
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

  /*
    time how long the P1 record took
  */
  if (mySerial1.P1active()  && startMicrosP1 == 0UL) {
    startMicrosP1 = micros();
  } else if (!mySerial1.P1active() && startMicrosP1 > 0UL)  {
    if (outputOnSerial) Serial.print ((String) "\tP1time=" + (micros() - startMicrosP1) + " " );
    startMicrosP1 = 0;
  }    

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
    /* 
        here we display the number of line loops -0-1-2-3-4-5-6-7-8-9    
    */
    // Serial.print((String)"-" +(loopcnt%10)+" \b");
    
    // Serial.print((String) (
    //   (waterReadCounter != waterReadCounterPrevious) ? "_" : "-") +(loopcnt % 10)+" \b"); // v47 test water tapping 
    Serial.print((String) (                             // display diagnostic second loopcounter
          (waterReadCounter != waterReadCounterPrevious) ?   // v47 check test water tapping active
              (bSerial1State ? 
                  (bSerial2State ? "*"  : "\'")         // water &  and P1:  P2 -->  On* Off"
                 :(bSerial2State ? "\"" : "_") )       // water &   no P1:  P2 -->  On' Off_
              :   
              (bSerial1State ? 
                  (bSerial2State ? ":" : "-")          // no water and P1:  P2 -->  On: Off-
                 :(bSerial2State ? ";" : ".") )        // no water  no P1:  P2 -->  On; Off.
          )
          +   (loopcnt % 10)+" \b");

    test_WdtTime = currentMillis - test_WdtTime;          
    if (verboseLevel == 1 && test_WdtTime != 1UL )  {      // v58 print long looptimes to diagnose excessive delays
        Serial.printf(" looptime=%lu ",  test_WdtTime);
    }
    bP1Active_signalled = false; // v57 reset his inteval of signalled situation
    waterReadCounterPrevious = waterReadCounter; 
    test_WdtTime = currentMillis + 1000;  // next interval

  }


  // Following will trackdown loops on execeeding serial reads using timeouts RX_yieldcount
  if (currentMillis > (previousLoop_Millis + 1080) ) { // exceeding 1.08 second  ?, warn user
    // yield();
    if (RX_yieldcount > 0) RX_yieldcount-- ;     // v52 decrease successive yieldcount  with any or valid P1 read
    if (RX_yieldcount < 1)  RX_yieldcount = 3;   // v56c 8 --> 3 when < 1, pause P1 serial reads ubtuk yield is again < 4
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

  mqtt_local_yield();      //   client.loop( ); // handle mqtt

  //  --------------------------------------------------------------------------------------------- START allowOtherActivities
  if (!allowOtherActivities) {     // are we outside P1 Telegram processing (require serial-timeing)
    if (waterTriggerCnt != 0) detachWaterInterrupt();
    // Serial.print ((String) waterTriggerCnt );    // v56c checking why this is alway activated
                                                    // likely while decode telgram was not executed 
                                                    // that reset

  } else {                         // else we are allowed to do other acivities
    // if (loopbackRx2Mode == 3) Serial.print((String) loopbackRx2Mode 
    //         + (p1SerialFinish  ? "f" : "g")
    //         + (p1SerialActive  ? "a" : "b")
    //          ); // v57 print incoming 3fa/3ga/3fa/3gb <-- after serial send this remains 3ga

    // we are now outside P1 Telegram processing (which require serial-timed resources) and deactivated interrupts
    if (!p1SerialActive) {      // P1 (was) not yet active, start primary softserial to wait for P1
      /* 
        =========================================
        Try to read connected P1
        =========================================
      */         
      if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + " serial started at " + currentMillis);
      p1SerialActive = !p1SerialActive ; // indicate we have started
      p1SerialFinish = false; // and let transaction finish
      mySerial2.end();          // Stop- if any - GJ communication
      mySerial2.flush();        // Clear GJ buffer
      bSerial2State = false; // v57 indicate state
      if (loopbackRx2Mode > 0) Serial.print((String) "_}" ); // v54 print incoming
      
      telegramError = 0;        // start with no errors
      // Start secondary serial connection if not yet active
      if (!serial1Stop) {
        #ifdef UseNewSoftSerialLIB
          // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
          mySerial1.begin(serial1Baudrate, SWSERIAL_8N1, SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH, 0); // Note: Prod use require invert
          // mySerial2.begin (  1200,SWSERIAL_8N1,SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2,0);
        #else
          // mySerial1.begin(P1_BAUDRATE);    // P1 meter port 115200 baud
          mySerial1.end();          // v58b: not sure  but to acertain,  finish any active
          mySerial1.flush();        // v58b: not sure  but to acertain,  Clear GJ buffer
          mySerial1.begin(serial1Baudrate, serial1PortMode);    // v58a, V58b Use simulated data P1
          // mySerial1.begin(serial1Baudrate);  // < v58a ss241 P1 meter port   115k2    // required during test without P1
        #endif
          bSerial1State = true; // v57 indicate state
      }        

    } else {    // if (!p1SerialActive)
      if (p1SerialFinish) {     // P1 transaction completed, we can start GJ serial operation at Serial2

        if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + " serial stopped at " + currentMillis);

        // if (outputOnSerial) Serial.println((String) P1_VERSION_TYPE + "." ); // v47 superfluous after preceding debug line
        // --> end of p1 read electricity
        // if (!outputOnSerial) Serial.print((String) "\t stopped:" + micros() + " ("+ (micros()-currentMicros) +")" + "\t");

        if (!outputOnSerial) {
          unsigned long diffMicros  = micros() - startMicros; // micros() time readTelegram()
          if (startMicros == 0) diffMicros = 0; // v54 startMicros = 0 if no P1 read executed
          Serial.printf("\t endP1: %11.6f (%9.6f)__\b\b\t", 
              ( (float) micros()    / 1000000UL ), 
              ( (float) diffMicros / 1000000UL ) ); 
        }                

        p1SerialFinish = !p1SerialFinish;   // reverse this
        p1SerialActive = true;  // ensure next loop sertial remains off
        mySerial1.end();    // P1 meter port deactivated
        mySerial1.flush();  // Clear P1 buffer
        bSerial1State = false; // v57 indicate state

        loopTelegram2cnt = 0;  // allow readtelegram2  for a maximum of "6 loops" of actual receives

        // Only activate myserial2  atdesignated times
        // int checkIntervalRx2 = mqttCnt % 7;
        if ( rx2_function && 
                (mqttCnt == 2  || 
                (mqttCnt > 0 && ((mqttCnt % rx2ReadInterval) == 0)) ) ) {  // only use RX2 port at these intervals
          // Start secondary serial connection if not yet active
          if (!serial2Stop) {
              #ifdef UseNewSoftSerialLIB
                // 2.7.4: swSer.begin(BAUD_RATE, SWSERIAL_8N1, D5, D6, false, 95, 11);
                // mySerial1.begin  (P1_BAUDRATE,SWSERIAL_8N1,SERIAL_RX, -1, bSERIAL_INVERT, MAXLINELENGTH,0); // Note: Prod use require invert
                // mySerial2.begin(serial2Baudrate, SWSERIAL_8N1, SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2, 0);
                 mySerial2.begin(serial2Baudrate, SWSERIAL_8N1, SERIAL_RX2, SERIAL_TX2, bSERIAL2_INVERT, MAXLINELENGTH2);
              #else
                // mySerial2.begin(serial2Baudrate);    // P1 meter port 115200 baud
                mySerial2.end();          // v58b: not sure  but to acertain,  finish any active
                mySerial2.flush();        // v58b: not sure  but to acertain,  Clear GJ buffer
                mySerial2.begin(serial2Baudrate,serial2PortMode);    // v58a, v58b ss241 =0 or simulate 1=P1; 2=RX/WL data.
                // #else 
                //   mySerial2.begin(serial2Baudrate,0);    // Using physical 0-port
                // #endif
              #endif
              bSerial2State = true; // v57 indicate state
              if (loopbackRx2Mode > 0) Serial.print((String) "{_" ); // v54 print incoming
              // Serial.print((String) "{_" ); // v54 print incoming
          }
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
    if (loopTelegram2cnt < MAXTELEGRAM2CNT) readTelegramWL();   // read RX2 GJ gpio4-input (if available), write gpio2 output
  }   // if-else allow other activities
  //  ---------------------------------------------------------------------------------------------- END of allowOtherActivities

  // readTelegram2();    // read RX2 GJ gpio4-input (if available), write gpio2 output (now done in closed setting)

  // (p1SerialActive && !p1SerialFinish) , process Telegram data
  
  if (RX_yieldcount < 4) { // assume all if well and we had of have surived any previous yieldcount
      readTelegramP1();     // read RX1 P1 gpio14 (if serial data available)
  } else {
      Serial.print ((String) " Yc="+ RX_yieldcount + " " );    // v56c
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

      /*
        ====================================
          we have 10 seconds of no P1 reads 
        ====================================
      */
      p1FailRxCnt++;  // v52 count failed RJ11 connections
      p1SerialFinish = true;  // v53: enforce to accomodate faults normal set at  processing P1 record
      p1SerialActive = true; // v53: enforce to accomodate faults normal set at  processing P1 record

      // mySerial1.end();         // v53: enforce finish , done at line 1775
      // mySerial1.flush();       // v53: enforce flush the buffer (if any) line 1775

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
            
      // Alway print this serious timeout failure
      // if (outputOnSerial) {
      if (!serial1Stop && loopbackRx2Mode == 0) {
         if (outputMqttLog) publishMqtt(mqttLogTopic, "ESP P1 rj11 Active interval checking" );  // report we have survived this interval

         Serial.printf("\r\n# !!# ESP P1 rj11 Active interval at %11.6f, checking %6d, timecP:%d, timec2:%d .\r\n",   
                          ((float)  micros() / 1000000), 
                          intervalP1cnt,
                          previousMillis,
                          currentMillis);

      } else {
         Serial.print("|\r\n:") ;        // v54: if no RJ11 message print some indication
         p1SerialFinish = true;
         p1SerialActive = true;          // v57
         allowOtherActivities = true;     // v57
      }                          
    /*    
      // ((float)ESP.getCycleCount()/80000000), (mqttCnt + 1), ((float) startMicros / 1000000));
      Serial.print("\r\n# !!# ESP P1 rj11 Active interval checking ");   // v45: add carriage-return
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
void readTelegramP1() {

  unsigned long p1Debounce_time = millis() - p1TriggerTime;		// mSec - p1TriggerTime  set by this() and updated while reading P1 data
  // if (p1SerialActive && p1Debounce_time > p1TriggerDebounce ) { // debounce_time (mSec
  if (p1Debounce_time > p1TriggerDebounce ) { // debounce_time (mSec
    // yield();                // allow time to others
    mqtt_local_yield();
    if (outputOnSerial) {   // indicate on console
      Serial.print((String) " !!>" + millis() + " yield due " + p1TriggerTime + "< exceeded !!" ); // myLength processed previous line
    }
    p1TriggerTime = millis();
  }

  #ifdef UseP1SoftSerialLIB              // Note this serial version will P1Active while reading between / and !
    /*
      Using our 241 adapted library, we wait between / and ! 
    */
    if ( mySerial1.P1active())  {    // does not seem to activate
        if (!bP1Active_signalled) Serial.print((String) "\\" ); // myLength processed previous line
        bP1Active_signalled = true;
        return ;  // quick return if serial is receiving, function of SoftwareSerial for P1
    }
  #endif  
  if (!mySerial1.available() || serial1Stop ) return ;  // quick return if no data, v52

  if (outputOnSerial)    {
    if ( !telegramP1header) Serial.print((String) P1_VERSION_TYPE + " DataRx started at " + millis() + " s=s123..\b\b\b\b\b") ; // print message line
    // if (  telegramP1header) Serial.print("\r\nxP= "); // print message line if message is broken in buffer
  }
  startMicros = micros();  // Exact time we started
  // if (!outputOnSerial) Serial.print((String) "\rDataCnt "+ (mqttCnt+1) +" started at " + micros());
  if (!outputOnSerial) Serial.printf("\r\n ReadT%d: %12.9f D%d%s%sC%s%s: %5u start: %11.6f \b\b ", 
        RX_yieldcount,    // v52: check countlevel
        ((float)ESP.getCycleCount()/80000000),
          (int) new_ThermostatState, (thermostatReadState ? "d" : "T"),  (thermostatWriteState ? "A" : "i"),
          (digitalRead(WATERSENSOR_READ) ? "h" : "l"), (digitalRead(LIGHT_READ) ? "c" : "w"),
        (mqttCnt + 1), ((float) startMicros / 1000000));

  // Cycle: 04.781228065

  if (outputMqttLog && client.connected()) publishMqtt(mqttLogTopic, mqttClientName );

  int myLenTelegram = 0;
  memset(telegram, 0, sizeof(telegram));   // initialitelegram buffer-array to 0
  memset(telegramLast, 0, sizeof(telegramLast));     // initialise 3 byte array to 0

  //DebugCRC memset(testTelegram, 0, sizeof(testTelegram));   // initialise 640byte telegram array to 0 to test/check single array

  // Serial.println("Debug: startbuf..");

  while (mySerial1.available())     {      // If serial data in buffer
    // Serial.print((String) "xa\b"); no need to display serial available this goes ok
    
    /*
        if (outputOnSerial) {         // do we want/need to print the Telegram
          Serial.print("\nDataP1>");
        }
    */
    // https://www.arduino.cc/reference/en/language/functions/communication/stream/streamreadbytesuntil/
    // note Serial.setTimeout() sets the maximum and defaults to 1000 milliseconds.
    // The function returns the characters up to the last character before the supplied terminator.

    /* v58c removed and relaced by 
      #if(defined DUP_MODE || defined TEST_MODE)  
        // serial1terminator = '\x0d';
        int myLen = mySerial1.readBytesUntil(serial1terminator, telegram, MAXLINELENGTH - 2); // read a max of  MAXLINELENGTH-2 per line, termination is not supplied
      #else
        // serial1terminator = '\x0a';
        int myLen = mySerial1.readBytesUntil(serial1terminator, telegram, MAXLINELENGTH - 2); // read a max of  MAXLINELENGTH-2 per line, termination is not supplied
      #endif
    */
    int myLen = mySerial1.readBytesUntil(serial1terminator, telegram, MAXLINELENGTH - 2); // v58c read a max of  MAXLINELENGTH-2 per line, termination is not supplied

      yield();                    // v58 do background processing required for wifi etc.
      ESP.wdtFeed();              // v58 feed the hungry timer
    
      if (loopbackRx2Mode == 6) {   // diagnose header    // v57
          Serial.print((String)"\r\n\t P1header1-32=(" );
          for (int i = 0; i < 33 && i < myLen; i++) {
              Serial.printf("%02x ", telegram[i]); 
          }
          Serial.println((String)")" );
      }

    // Serial.print((String) "yb\b"); no need to display RX progess this goes ok
    

    if (myLen > 1) {      // correct dual Carrigage returns, Windows formatted testdata on Linux
      // decrease myLen as the dual 0x0d 0x0d is from streamed testdata
      if (telegram[myLen-2] == '\x0d' && telegram[myLen-1] == '\x0d' ) myLen-- ;  // double carriage return ?
    }
    // debug this
    // if (myLen > 1)  Serial.print((String)" La="+ (int)telegram[myLen-2]);
    // if (myLen > 0)  Serial.print((String)" Lb="+ (int)telegram[myLen-1]);
    // if (myLen == 0) Serial.print((String)" Lc="+ (int)telegram [myLen]);
    
    if (verboseLevel == 0) Serial.print((String) "[Lp1=" + (myLen-1) + ']' );   // v52/test replaced \n into \r

    if (outputOnSerial && verboseLevel >= VERBOSE_P1) {     // do we want/need to print the Telegram for Debug
      Serial.print((String)"\rlT=" + (myLen-1) + " \t[");   // v33 replaced \n into \r
      for (int cnt = 0; cnt < myLen; cnt++) {
          
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
             Serial.printf("readbytesuntil=%u :", myLen);
             for (int cnt = 0; cnt < myLen; cnt++) {
                 Serial.printf("%02x",telegram[cnt-1]);
             }
             Serial.println(". ");
    */
    // myLen == 0 ? myLenTelegram = -1 : myLenTelegram += myLen;   // if myLen = 0 indicate an error report

    //debugCRC if ( myLen == 0 && outputOnSerial) Serial.print(" <0> "); // debug print processing serial data
    //debugCRC if (outputOnSerial) Serial.printf("{%02X %02X %02X}",telegram[myLen-2],telegram[myLen-1],telegram [myLen]);

    myLenTelegram = myLenTelegram + (myLen - 1); // accumulate actual number of characters (excluding cr/lf)
    if ( myLen == 0) myLenTelegram = -1; // if myLen = 0 indicate for report


    //DebugCRC int myLenSave = myLen;                     // the the original value
    /* // debugCRC
       if (outputOnSerial) Serial.print(" <"); // debug print processing serial data
       if (outputOnSerial) Serial.print(myLen) ; // debug print processing serial data
       if (outputOnSerial) Serial.print(">"); // debug print processing serial data
    */

    if (myLen > 0 )  {  // myLen=0 indicates serial error, a myLength of 1 means the zerobyte (0x00-0d-0a) string.

      /* //debugCRC
               // Extract last character into a printable string to  //debugCRC
               telegramLast[0] = telegram[myLen - 2];  // 3byte-telegram readible lastbyte  bracket-2 null-1 cr-0
               telegramLast[1] = '\n';               // 3byte telegram lastbyte termination string
               telegramLast[2] = 0;                  // 3vyte-telegram lastbyte terminaion variable

               // Make Telegram printable , Note this may could overwrite the running buffer
               telegram[myLen + 1] = '\n';    // string set lastcharacter to \n
               telegram[myLen + 2] = 0;       // variable next character before null

               // sprintf("%02X", theNumber); // take alot of memory
               if (outputOnSerial) Serial.print((String)"\r\n1:"+String(telegramLast[0],HEX)+"="+telegramLast[0]+"  >"); // Hex debug print line last character
      */

      if (decodeTelegram(myLen + 1)) {        // process data did we have a 'true' end of message condition including terminating CR

        /*  //debugCRC
            // print/display readed myLength
            if (outputOnSerial) {           // this line is alway printed.
                Serial.print(" [");         // debug print transaction myLength
                Serial.print(myLenTelegram);  // myLength processed previous line
                Serial.print("] #OK# ");    // display we have a successful end of data
                // Serial.println("\r\n");
            }

        */
        yield();                    // do background processing required for wifi etc.
        ESP.wdtFeed();              // feed the hungry timer
        
        // print validity status of processed for debug reasons.
        allowOtherActivities = true;      // v56c resume finished processing of this P1 record.
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
        if (outputOnSerial)
            Serial.print((String) "\r\n" + P1_VERSION_TYPE + " serial1 time "
                + " "  +  mySerial1.peek(M_TIME_START)
                + " "  +  mySerial1.peek(M_TIME_RX_START)
                + ". "
                + " "  +  mySerial1.peek(M_TIME_RX_END) 
                + " +" + (mySerial1.peek(M_TIME_BEGIN_START) - mySerial1.peek(M_TIME_RX_END))
                + "= " +  mySerial1.peek(M_TIME_BEGIN_START) 
                + " +" + (mySerial1.peek(M_TIME_BEGIN_END)   - mySerial1.peek(M_TIME_BEGIN_START)) 
                + "="  +  mySerial1.peek(M_TIME_BEGIN_END) 
                + " +" + (mySerial1.peek(M_TIME_AVAIL_START) - mySerial1.peek(M_TIME_BEGIN_END))
                + "="  +  mySerial1.peek(M_TIME_AVAIL_START)
                + " +" + (mySerial1.peek(M_TIME_AVAIL_END)   - mySerial1.peek(M_TIME_AVAIL_START))
                + "="  +  mySerial1.peek(M_TIME_AVAIL_END)
                ) ;  

        mySerial1.end();      // flush the buffer (if any) v38
        mySerial1.flush();      // flush the buffer (if any)
        if (outputOnSerial)
            Serial.println((String) ", "
                + " "  +  mySerial1.peek(M_TIME_START)
                + ". " +  mySerial1.peek(M_TIME_START)
                + "+"  + (mySerial1.peek(M_TIME_RX_END)      - mySerial1.peek(M_TIME_START))
                + "="  +  mySerial1.peek(M_TIME_RX_END)
                + ". "
                );
    
        bSerial1State = false; // v57 indicate state

      }
      /* //debugCRC
           else {                              // data not yet finished or in transit
               if (outputOnSerial) {           // this line is alway printed.
                   Serial.print(" [");         // debug print transaction myLength
                   Serial.print((String) myLenTelegram + "," + myLenSave);  // myLength processed previous line
                   Serial.print("] ");         // single of with println: double lines spacing
                   // Serial.println("\r\n");
               }
         } // else
      */

    } else { // if myLen > 0
        // resume oher activities if we have not processed any data myLen < 0
        // this activates when we feed the input pins with programmatic data (0A0D) seperation.
        Serial.print((String) " P1-len=" + myLen + " -#r=" + (bool) allowOtherActivities + "-" ); // v56c
        allowOtherActivities = true;  // v56c
    }
  } // while serial available
  // Serial.println("startend..");
} // void readTelegram



/* 
  process the secondary serial input port, to be used  in future for 1200Baud GJ meter
*/
void readTelegramWL() {
  // if (loopbackRx2Tx2) Serial.print("Rx2 "); // print message line
  
  #ifdef UseP1SoftSerialLIB
    /*
      Using our 241 adapted library, we wait between / and ! 
    */
    if (mySerial1.P1active()) {    // return if P1 is active
      if (!bP1Active_signalled) Serial.print((String) "\\" ); // myLength processed previous line
          bP1Active_signalled = true;
          return ;  // quick return if serial is receiving, function of SoftwareSerial for P1
    }
  #endif  

  if (serial2Stop) return;    // v52 return if we have stopped this serial
  if (!bSerial2State) return;  // v58 useless to continue without active serial


  bGot_Telegram2Record  = false;      // v38 check RX2 seen
  
  int myLenTelegram2 = 0;
  /*
    if (mySerial2.P1active())   {               // P2 blocked
      // if (loopbackRx2Tx2) Serial.print("RxA "); // print message line
      if (outputOnSerial)    {
        Serial.print("\r\n Rx2 blocked, mySerial2.P1active"); // print message line
      }
    }
  */

  // 30mar21: no data available .....
  if (mySerial2.available())   {
    if (outputOnSerial && verboseLevel >= VERBOSE_RX2)    {
      Serial.print((String) "\r\n Rx2N:" + loopbackRx2Mode + "="); // print message line
    }
    memset(telegram2,     0,       sizeof(telegram2));       // initialise telegram array to 0
    memset(telegram2_org, 0,       sizeof(telegram2_org));   // initialise telegram array to 0)
    // memset(telegramLast2, 0,   sizeof(telegramLast2));    // initialise array to 0
    // memset(telegram2Record, 0, sizeof(telegram2Record));  // initialise telegram array to 0

    // initialise positions
    validTelegram2CRCFound = false; // v54 initialise no valid CRC
    int telegram2_Start = -1;
    int telegram2_End   = -1;
    int telegram2_Pos   = 0;

    while (mySerial2.available() && loopTelegram2cnt < MAXTELEGRAM2CNT && !bGot_Telegram2Record )     {    // number of periodic reads  && !bGot_Telegram2Record
       // type casting https://www.geeksforgeeks.org/cpp/cpp-program-for-int-to-char-conversion/
      // if (loopbackRx2Mode > 0) Serial.print((String)  + char(mySerial2.peek()) ); // v54 print incoming
      // int myLen = mySerial2.readBytesUntil('\n', telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
      // int myLen = mySerial2.readBytesUntil('!', telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
      

      /* v58c test dup/test readBytesUntil removed and relaced by 
        #if(defined DUP_MODE || defined TEST_MODE)
          // serial2terminator = '\x0d';
          myLen = mySerial2.readBytesUntil(serial2terminator, telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
        #else 
          // serial2terminator = '\x00';
          myLen = mySerial2.readBytesUntil(serial2terminator, telegram2, MAXLINELENGTH2 - 2); // read a max of  64bytes-2 per line, termination is not supplied
        #endif  
      */
      int myLen = mySerial2.readBytesUntil(serial2terminator, telegram2, MAXLINELENGTH2 - 2); // v58c, read a max of  64bytes-2 per line, termination is not supplied
      yield();                    // v58 do background processing required for wifi etc.
      ESP.wdtFeed();              // v58 feed the hungry timer
       
      if (loopbackRx2Mode == 6) {   // diagnose header    // v57
          Serial.print((String)"\r\n\t P2header1-32=(" );
          for (int i = 0; i < 33 && i < myLen; i++) {
              Serial.printf("%02x ", telegram2[i]); 
          }
          Serial.println((String)")" );
      }

      // String telegram2_str(telegram2); // does work but still prints beyond 0x00
      if (loopbackRx2Mode > 0 && myLen > 0) Serial.print((String) 
                  + " ..len=" + myLen 
                  + ":\'" + (loopbackRx2Mode == 3 ? (String) "\r\n" + telegram2 + "\r\n": (String) "$")
                  + "\'.. " ); // v54 print incoming
            
      // myLen == 0 ? myLenTelegram = -1 : myLenTelegram += myLen;   // if myLen = 0 indicate for report
      myLenTelegram2 = myLenTelegram2 + myLen;
      if ( myLen == 0) myLenTelegram2 = -1; // if myLen = 0 indicate for report

      if (myLen > 0)  {   // do we have bytes in buffer
        // remember  last character
        // telegramLast2[0] = telegram2[myLen - 2];  // 3byte-telegram readible lastbyte  bracket-2 null-1 cr-0
        // telegramLast2[1] = '\n';                // 3byte telegram lastbyte termination string
        // telegramLast2[2] = 0;                   // 3byte-telegram lastbyte terminaion variable
        if ( myLen > (MAXLINELENGTH2 - 2) ) {            // we have clearly an overflow
           myLen =  (MAXLINELENGTH2 - 2) ;
        }
        telegram2[myLen + 1] = '\n';    // string set lastcharacter to \n
        telegram2[myLen + 2] = 0;       // variable next character before null
        /*
         * Check ascii output
         */

        if (!bGot_Telegram2Record) {   // reset numbers if not yet a record from WarmteLink 
          telegram2_Start = -1;
          telegram2_End   = -1;
          telegram2_Pos   = 0;
        }
        
        for (int i = 0; (i <= myLen && !bGot_Telegram2Record); i++) {  // forward

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
                  telegram2_org[telegram2_Pos]   = 0;   // initialise first position v54
                  telegram2_End = -1;    // reset to find exclamation-!

                }
                if (telegram2[i] == '!')  {
                    if (telegram2_End   < 0 && (i < (myLen - 6)) ) telegram2_End = i + 6;  // anticiptae after checksum position !1234.6
                    if (telegram2_Start < 0 ) telegram2_End = -1;  // reset if not yet started
                } 
                
                if (telegram2_Start >= 0  && (telegram2_End < 0 || i < telegram2_End) ) {
                  telegram2Record[telegram2_Pos] = TranslateForPrint(telegram2[i]); // move value as translated output
                  telegram2_org[telegram2_Pos]   = telegram2[i];  // move v54
                  telegram2_Pos++;
                  telegram2Record[telegram2_Pos] = 0;     // initialize next position
                  telegram2_org[telegram2_Pos]   = 0;     // initialize next position v54
                } // if (telegram2_Start > 0  && (telegram2_End == 0 || i < telegram2_End) 
              } // if ( telegram2_Pos < MAXLINELENGTH2 && (telegram2_Start == 0 || telegram2_Pos > 0 || i < telegram2_End ) )
              if (telegram2_Pos > 4) {      // check for myLength start2finish before Checksum
                if (telegram2Record[0] == '/' && telegram2Record[telegram2_Pos - 5] == '!')  {
                  bGot_Telegram2Record = true;
                  Got_Telegram2Record_cnt++;      // v51 count for this receive

                  if (outputOnSerial)
                  
                      Serial.print((String) "\r\n" + P1_VERSION_TYPE + " serial2 time "
                          + " " + mySerial2.peek(M_TIME_START)
                          + " " + mySerial2.peek(M_TIME_RX_START)
                          + ". "
                          + " "  +  mySerial2.peek(M_TIME_RX_END) 
                          + " +" + (mySerial2.peek(M_TIME_BEGIN_START)  - mySerial2.peek(M_TIME_RX_END))
                          + "= " +  mySerial2.peek(M_TIME_BEGIN_START) 
                          + " +" + (mySerial2.peek(M_TIME_BEGIN_END)    - mySerial2.peek(M_TIME_BEGIN_START)) 
                          + "="  +  mySerial2.peek(M_TIME_BEGIN_END) 
                          + " +" + (mySerial2.peek(M_TIME_AVAIL_START)  - mySerial2.peek(M_TIME_BEGIN_END))
                          + "="  +  mySerial2.peek(M_TIME_AVAIL_START)
                          + " +" + (mySerial2.peek(M_TIME_AVAIL_END)    - mySerial2.peek(M_TIME_AVAIL_START))
                          + "="  +  mySerial2.peek(M_TIME_AVAIL_END)

                          ) ;  

                  mySerial2.end();          // v38 Stop- if any - GJ communication
                  mySerial2.flush();        // v38 Clear GJ buffer
                  if (outputOnSerial)
                      Serial.println((String) ", "
                          + " "  +  mySerial2.peek(M_TIME_START)
                          + ". " +  mySerial2.peek(M_TIME_START)
                          + "+"  + (mySerial2.peek(M_TIME_RX_END) - mySerial2.peek(M_TIME_START))
                          + "="  +  mySerial2.peek(M_TIME_RX_END)

                          + ". "
                          );


                  bSerial2State = false; // v57 indicate state
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
            // Serial.println((String) "\r\n\t.s2>[myLen=" + myLen + " Pos=" + telegram2_Pos + "]" + telegram2) ;
            /*                     
            for (int i = 200; i <= myLen; i++) {  // forward
                Serial.print(" ") ;         // debug print processing serial data
                Serial.print((int8) telegram2[i] ) ;         // debug print processing serial data
                Serial.print((String) + "=" + telegram2[i] ) ;         // debug print processing serial data
            }
            */
        }
        if (outputOnSerial && verboseLevel >= VERBOSE_RX2) {
          Serial.println((String) "\n.s2>[myLen=" + myLen + " Pos=" + telegram2_Pos + "]" + telegram2) ;
        }

        // if (rx2_function) { // do we want to execute v38 rx2_function (superfluous , as routine her eis v38)
        /*
          Process console
        */
        if (bGot_Telegram2Record) {   // v38 print record serial2 if we have catched a record
                                      // v58
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
                unsigned int currentCRC2 = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram2_org)+startChar,(endChar-startChar)+1); // include \header+!trailer '!' into CRC, v48-casting
                validTelegram2CRCFound = (strtol(messageCRC2, NULL, 16) == currentCRC2);   // check if we have a 16base-match https://en.cppreference.com/w/c/string/byte/strtol

              

                if ( validTelegram2CRCFound && loopbackRx2Mode == 5 ) {   // v55 debug/print one validated hex characters
                  Serial.println("");
                  Serial.printf(" crt=%s, crc=%x \r\n", messageCRC2, currentCRC2);    // v54 insert textual CRC and calculated CCRC

                  for (int i = startChar ; i  < ((endChar-startChar)+1+4); i++ ) {
                    
                    // diagnose instability of code.....
                    // Serial.print(telegram2_org[i]);             // 001 only, is stable
                    // -------------------------------------------------------------------------------------
                    
                    // Serial.print(telegram2_org[i]);             // 002 try to unlock, just somewhat more stable

                    /*
                    Serial.print(telegram2_org[i]);             // 003 try to unlock, instable
                    Serial.print(telegram2_org[i]);
                    */
                    

                    // ---------------------------------------------------------------------------- keep below
                    // Serial.printf(" %02x", telegram2_org[i]);      // 000-005 causing unstablity
                    // ---------------------------------------------------------------------------- keep above

                    // Serial.print(telegram2_org[i]);             // 004 after try to unlock, instable

                    /*
                      asm(                    // 005 try to insert unsued code, no effect
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
                    */     
                  // // teststable2-only
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   // ---------------------------- 009 line 2585 added printf (unstable) 
                  //   delay(0);     // 008 adding 009 test remove, add 013 more stable (rx2-30%)
                  //   delay(0);     // 008 adding 009 test remove, add 012 stable (rx2-10%)

                  //   delay(0);     // 008 adding 009 test remove, add 011 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 011 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // add13 + add 014 stable (rx2-70%)

                  // // teststable3&2
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   delay(0);     // 008 adding here without printf above , stable
                  //   // ---------------------------- 009 line 2585 added printf (unstable) 
                  //   delay(0);     // 008 adding 009 test remove, add 013 more stable (rx2-30%)
                  //   delay(0);     // 008 adding 009 test remove, add 012 stable (rx2-10%)

                  //   delay(0);     // 008 adding 009 test remove, add 011 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 011 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                  //   delay(0);     // add13 + add 014 stable (rx2-70%)


                    //  // teststable0
                    //  // move020 to void command_testH3() to check if location influnces things
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding here wihout printf above , stable
                    //   delay(0);     // 008 adding 009 test remove, add 013 more stable (rx2-30%)
                    //   delay(0);     // 008 adding 009 test remove, add 012 stable (rx2-10%)
                    //   delay(0);     // 008 adding 009 test remove, add 011 unstable
                    //   delay(0);     // 008 adding 009 test remove, add 011 unstable
                    //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                    //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                    //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                    //   delay(0);     // 008 adding 009 test remove, add 010 unstable
                    //   delay(0);     // add13 + add 014 stable (rx2-70%)
                    //   delay(0);     // add14 + add015 stable (rx2=40%)
                    //   delay(0);     // add15 + add016 less stable (rx2=20%)
                    //   delay(0);     // add16 + add017 stable (rx2=50%)
                    //   delay(0);     // add17 + add018 stable  (rx2=10%)
                    //   delay(0);     // add18 + add019 stable  (rx2=75%)
                    
                   /*
                      if (false) {
                        // delay(0);     // 008 adding here without printf above , unstable
                        // delay(0);     // 008 adding here without printf above , unstable
                        // delay(0);     // 008 adding here without printf above , unstable
                        // delay(0);     // 008 adding here without printf above , unstable
                        // delay(0);     // 008 adding here without printf above , unstable 30% (comment = 22c2 problem)
                        delay(0);     // 008 adding here without printf above , unstable 20%
                        delay(0);     // 008 adding here without printf above , untstale 25%
                        delay(0);     // 008 adding here wihout printf above , stable
                        delay(0);     // 008 adding 009 test remove, add 013 more stable (rx2-30%)
                        delay(0);     // 008 adding 009 test remove, add 012 stable (rx2-10%)
                        delay(0);     // 008 adding 009 test remove, add 011 unstable
                        delay(0);     // 008 adding 009 test remove, add 011 unstable
                        delay(0);     // 008 adding 009 test remove, add 010 unstable
                        delay(0);     // 008 adding 009 test remove, add 010 unstable
                        delay(0);     // 008 adding 009 test remove, add 010 unstable
                        delay(0);     // 008 adding 009 test remove, add 010 unstable
                        delay(0);     // add13 + add 014 stable (rx2-70%
                      }
                    */                      
                    // no zzzzz = instable
                    // z     = instable 20%
                    // zz    = instable
                    // zzz   = less 50% instable
                    // zzzz  = very 60% instable
                    Serial.printf(" %02x", telegram2_org[i]);      // 000-005, 008 causing unstablity
                  }
                  
                  Serial.println((String) "\r\n\t start=" + startChar + ", myLen=" + ((endChar-startChar)+1) );
                  loopbackRx2Mode = 0;   // stop debug
                }

                if (outputOnSerial || loopbackRx2Mode == 3) {   // print serial record count, total receivwed myLength, calculated CRC
                  Serial.printf(" s=%c, e=%c, crt=%s, cr1=%x, ;", telegram2Record[startChar],telegram2Record[endChar], messageCRC2, currentCRC2) ;    // debug print calculated CRC
                  /* below are incorrect
                  currentCRC2 = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram2_org)+startChar,(endChar-startChar));   // ignore header, v48-casting
                  Serial.printf(" cr0=%x,", currentCRC2) ;    // debug print calculated CRC wrong
                  currentCRC2 = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram2_org)+startChar,(endChar-startChar)-1); // ignore header & trailer, v48-casting
                  Serial.printf(" cr-=%x,", currentCRC2) ;    // v53 debug print calculated CRC wrong
                  currentCRC2 = CRC16(0x0000, reinterpret_cast<unsigned char*>(telegram2_org)+startChar,(endChar-startChar)+1); // ignore header & trailer, v48-casting
                  Serial.printf(" Crc=%x,", currentCRC2) ;    // v54 debug print calculated CRC after the end-sign valididated
                  */
                }
                // delay(0);     // 007 adding here wihout printf above , stable. With printf to no avail
                if (loopbackRx2Mode == 2) {   // diagnose header printdata v54
                    Serial.print((String)"\r\n\t" + (validTelegram2CRCFound ? "Valid" : "Invalid" )  
                        + "CRC, Rx2head " // v54 print CRC check
                        + ", startChar=" + (char) telegram2Record[startChar] + (int) startChar
                        + ", endChar="   + (char) telegram2Record[endChar]   + (int) endChar
                        + ", data=\r\n"
                        );
                    for (int i = startChar; i <= endChar+5; i++) {                // v54 header hex bytes
                       Serial.printf("%02x ", telegram2_org[i]); 

                    }
                    Serial.printf(" crt=%s, crc=%x \r\n", messageCRC2, currentCRC2);    // v54 insert textual CRC and calculated CCRC
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
            Serial.print("\r\nWL>");       // v38 debug print processing serial data
            Serial.print(telegram2_End); 
            Serial.print("-");            // print minus
            Serial.print(telegram2_Start); 
            Serial.print("=");            // debug print processing
            Serial.print((telegram2_End - telegram2_Start)); // debug print processing serial data myLength
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
              long tHeatFlowConsumption = getValue(strstr(telegram2Record,"0-1:24.2.1("), 38) ;   // pointer & record myLength = 30-38 to get intger long
              
              Serial.print(" WL-");               //  v46 print value/type unconditionally on new 
              Serial.print(telegram2[valChar+1]);   
              Serial.print(telegram2[valChar+2]);
              if (!validTelegram2CRCFound) {         // print yes/no value
                  Serial.print("x");                // v54 NO t valid CRC
              } else {
                  Serial.print("=");                // v54 Valid CRC
              }
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
        /*
          If this is not activated, we bounce bacl the data to BlueLed pin
        */
        if (loopbackRx2Tx2) mySerial2.println(telegram2); // echo back , disabled as of v37
      #endif
        yield();  // do background processing required for wifi etc.
        ESP.wdtFeed(); // feed the hungry timer

      } // if (myLen > 0)  {   // do we have bytes in buffer
      loopTelegram2cnt++;   /// account this readloop to prevent overrunning on RX2
    } // while data

    if (outputOnSerial) {
      Serial.print((String) " [myLenTelegram2="     + myLenTelegram2 );  // debug print transaction myLength
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
void callbackMqtt(char* topic, byte* payload, unsigned int myLength) {
  previousBlinkMillis = millis() + intervalBlinkMillis ; // end the shortblink (if any)

  // struggling with C++ forbids comparison between pointer and integer
  // char w1 = "" + (char)payload[0];
  // char w1[1];
  // w1[0] = '3';
  // String w1="";
  //  w1 = (char)payload[0];

  String mqttCommand = ""; // initialise as string
  int i = 0;
  for (i = 0; (i < myLength && i < sizeof(mqttReceivedCommand)); i++) { // read mqtt payload
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
    Serial.println((String)(mqttCommand) + ", myLen=" + i );
    // Serial.println((String)mqttReceivedCommand[0] + (int)mqttReceivedCommand[1]); 
    // yield();  // do background processing required for wifi etc.
  }
}


/* 
  Processing queued mqttCommand, received by callbackMqtt 
*/
void ProcessMqttCommand(char* payload, unsigned int myLength) { 
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
    b decrease P1 Baudrate port  Gpio14/D5  by 50 or value 0-9 = 100-1000
    B increase Baudrate P1 port by 50 or value 0-9 = 100-1000
    J change [+-][0-9] m_bitwait of serial
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
  if (  (char)payload[0] != '\x00' && 
      ( (char)payload[1] == '\x00' ||
        (char)payload[2] == '\x00' ||
        (char)payload[3] == '\x00' ||
        (char)payload[4] == '\x00') )  {   // v58b up to 5 commmands
      
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
      if (!outputOnSerial)  Serial.print("Inactive\r\n");
      if (outputOnSerial)   Serial.println("\nActive.");
    } else  if ((char)payload[0] == 'L') {       // v51 log lines to topic mqttLogTopic
      outputMqttLog   = !outputMqttLog ;         // swap
      Serial.print(" MqttLogging ");
      if (!outputMqttLog)  Serial.print("OFF\r\n ");
      if (outputMqttLog)   Serial.println("\nON ");
    } else  if ((char)payload[0] == 'l') {       // v51 make RX record toggle to topic mqttLogTopic2
      outputMqttLog2   = !outputMqttLog2 ;       // swap
      Serial.print(" MqttLogging2 ");
      if (!outputMqttLog2)  Serial.print("OFF\r\n ");
      if (outputMqttLog2)   Serial.println("\nON ");
    } else  if ((char)payload[0] == 'v') {
      if ( (char)payload[1] >= '0' && (char)payload[1] <= '9') {
          verboseLevel =  (int)payload[1] - 48;   // set number myself
          if (verboseLevel == 8) Serial.setDebugOutput(true);    // v58
          if (verboseLevel == 9) Serial.setDebugOutput(false);   // v58
          if (verboseLevel > VERBOSE_MAX) {
            outputOnSerial = true;
            verboseLevel = VERBOSE_MQTT;  // print all below and MQTT
          }
      } else { 
            verboseLevel++ ;
      }
      if (outputOnSerial) Serial.print((String)" Verbose=" +  verboseLevel + " ");
      if (verboseLevel >= VERBOSE_MAX) verboseLevel = VERBOSE_OFF;
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

    } else  if ((char)payload[0] == 'B') {    // v58b: changed to control Baudrate of port 1/2 with +/- 0-9
      if (((char)payload[1] >= '1' && (char)payload[1] <= '3') &&
          ((char)payload[2] == '+' || (char)payload[2] == '-') && 
          ((char)payload[3] >= '1' && (char)payload[3]) ) {
          int temp = (((int)payload[3] - 48) * 25);   // set number 0-9  add or minus
          if ( (char)payload[3] == '6')  temp =   100;   // increase by 100 if 9
          if ( (char)payload[3] == '7')  temp =   500;   // increase by 100 if 9
          if ( (char)payload[3] == '8')  temp =  1000;   // increase by 100 if 9
          if ( (char)payload[3] == '9')  temp =  5000;   // increase by 100 if 9
          if ( (char)payload[2] == '-')  temp = temp * -1;
          if ( (char)payload[1] == '1' || (char)payload[1] == '3') serial1Baudrate = serial1Baudrate + temp; // set number 1-10  
          if ( (char)payload[1] == '2' || (char)payload[1] == '3') serial2Baudrate = serial2Baudrate + temp; // set number 1-10  
          
          Serial.print((String)"\tBaud" + (char)payload[1] + "="   // v58b display result
                 + ( (char)payload[1] == '1' ?  serial1Baudrate : serial2Baudrate )
                 + "\t" );
        }
    } else  if (  ( (char)payload[0] == 'J' || (char)payload[0] == 'j' )  &&              // v58 change m_bitwait
                  ( (char)payload[1] == '+' || (char)payload[1] == '-')   &&
                  ( (char)payload[2] >= '0' && (char)payload[2] <= '9') ) {
        
        int temp = (((int)payload[2] - 48));
        if ( (char)payload[2] == '9')  temp = 10;   // increase by 10 if 9
        // Serial.print((String)"\r\n Changing m_bitwait with " + temp ) ;
        if ((char)payload[1] == '-') temp = temp * -1;
        // Serial.print((String)"\r\n to " + temp ) ;

        if ( (char)payload[0] == 'J' ) 
              temp = mySerial1.m_bitWait + temp;
        else  temp = mySerial2.m_bitWait + temp;

        Serial.print((String)"\tBitait"
              + ( (char)payload[0] == 'J' ? "P1" : "WL" )
              + "=" + temp + "\t" ) ;

        if ( (char)payload[0] == 'J' ) 
              mySerial1.m_bitWait   = temp;
        else  mySerial2.m_bitWait  = temp;

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
       // Clear GJ buffer
    } else  if ((char)payload[0] == 'T') {    // loopbackRx2Mode, 1=invertP1, 2=invertRX2
      if ( (char)payload[1] >= '0' && (char)payload[1] <= '9') {
         loopbackRx2Mode = (((int)payload[1] - 48) * 1);  // set number myself
         // T1 enforces debug diagnose to verify serial processing
         if (loopbackRx2Mode == 1 || loopbackRx2Mode == 3) {  // v57
            outputOnSerial = false;   // disable debug details as we focus on serial input display
            rx2ReadInterval = 1;      // process this for any RX2 loop
            verboseLevel = 4;         // v57
         }
        /* 
            Playing here with myserial is useless
            as this is inner class and does not  reflect upper routines

         if        (loopbackRx2Mode == 1 ) { // redefine myserial2
            SoftwareSerial mySerial2(SERIAL_RX2, SERIAL_TX2, !bSERIAL2_INVERT, MAXLINELENGTH2); // (RX, TX, invertmode, buffer)
            if (!serial2Stop) mySerial2.begin(serial2Baudrate - 10);
            Serial.print((String) " RX2 baudrateB1 =" + mySerial2.baudRate() // v53: print diagse status if resetted myserial2
                    + (loopbackRx2Tx2 == true ? ":ON" : ":OFF")  
                    + ", serial2Baudrate=" + serial2Baudrate  
            );
          } else if (loopbackRx2Mode == 2 ) {
            SoftwareSerial mySerial2(SERIAL_RX2, SERIAL_TX2,  bSERIAL2_INVERT, MAXLINELENGTH2); // (RX, TX, invertmode, buffer)
            if (!serial2Stop) mySerial2.begin(serial2Baudrate + 10);
            Serial.print((String) " RX2 baudrateB2 =" + mySerial2.baudRate() // v53: print diagse status if resetted myserial2
                    + (loopbackRx2Tx2 == true ? ":ON" : ":OFF")  
                    + ", serial2Baudrate=" + serial2Baudrate  
            );
          }
          // print loopTelegram2cnt ???
        */
      } else {
         loopbackRx2Tx2   = !loopbackRx2Tx2 ; // loopback serial port
         Serial.print((String) " P1 baudrateA1=" + mySerial1.baudRate() // v53: print diagse status if resetted myserial2
                             + " WL baudrateA2=" + mySerial2.baudRate() // v53: print diagse status if resetted myserial2
                             + " loopbackRx2Tx2=" + (loopbackRx2Tx2 == true ? ":ON" : ":OFF")
                             + " " );
      }

      // mySerial2.begin( 1200);    // GJ meter port   1200 baud
      // mySerial2.println("..echo.."); // echo back

      if (outputOnSerial) {
         Serial.print((String) "RX2TX2 looptest=" 
              + (loopbackRx2Tx2 == true ? "ON" : "OFF") 
              + " mode:" + loopbackRx2Mode);
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
      Serial.println((String)"\r\n dataIn telegram_crcIn myLen=" + telegram_crcIn_len + " som>>");
      for (int cnt = 0; cnt < telegram_crcIn_len+4; cnt++) {
        if (isprint(telegram_crcIn[cnt])) {             // if printable
            Serial.print(telegram_crcIn[cnt]);
        } else if (telegram_crcIn[cnt] == '\x0d') {     // carriage return
            Serial.print("\r");
        } else if (telegram_crcIn[cnt] == '\x0a') {     // linefeed
            Serial.print("\n");
        } else if (telegram_crcIn[cnt] == '\x00') {     // end of data
            Serial.print("|");
            // break;
        } else  {
            Serial.print("?");                    // unprintable
        }
      }
      Serial.println((String)"<< eom");    // v33 debug lines didnot end in newline
    } else  if ((char)payload[0] == 'M') {       // v48 10jun25 print M-asking array
      Serial.println((String)"\r\n Recovery telegram_crcOut myLen=" + telegram_crcOut_len + " som>>");
      for (int cnt = 0; cnt < telegram_crcOut_len+4; cnt++) {
        if (isprint(telegram_crcOut[cnt])) {             // if printable
            Serial.print(telegram_crcOut[cnt]);
        } else if (telegram_crcOut[cnt] == '\x0d') {     // carriage return
            Serial.print("\r");
        } else if (telegram_crcOut[cnt] == '\x0a') {     // linefeed
            Serial.print("\n");
        } else if (telegram_crcOut[cnt] == '\x00') {     // end of data
            Serial.print("|");
            // break;
        } else  {
            Serial.print("?");                    // unprintable
        }
      }
      Serial.println((String)"<< eom");    // v33 debug lines didnot end in newline
    
    } else  if ( (char)payload[0] == 'S') {  // v52 serial stop activating P1
        if  (    (char)payload[1] == '0') p1SerialActive = !p1SerialActive;   // set number myself
        else if ((char)payload[1] == '1') p1SerialFinish = !p1SerialFinish ;   // set number myself
        else if ((char)payload[1] == 'p') serial1PortMode = 0;    // v58b use physical port data SS241
        else if ((char)payload[1] == 'T') serial1PortMode = 1;    // v58b use internal simulation  SS241
        else if ((char)payload[1] == '\x00') serial1Stop = !serial1Stop;

        if (outputOnSerial) Serial.print((String) 
              " Serial1 P1="  + (!serial1Stop  ? "Active" : "disabled") +
            + " mode="        + serial1PortMode    // v58b
            + " Finish="  + (p1SerialFinish  ? "1" : "0") + " )" 
            + " Active="  + (p1SerialActive  ? "1" : "0") + " )" ) ;

        else Serial.print((String) "\tSerial1=" 
              + (!serial1Stop  ? "E" : "D") 
              + serial1PortMode
              + (p1SerialActive  ? "A" : "a")
              + (p1SerialFinish  ? "F" : "f")              
              + "\t" );
    } else  if ( (char)payload[0] == 's') {  // v52 serial stop activating RX2
        if (     (char)payload[1] >= '1' && (char)payload[1] <= '9') 
             rx2ReadInterval = (int)payload[1] - 48;   // set number myself
        else if ((char)payload[1] == 'p') serial2PortMode = 0;    // v58b use physical port data SS241
        else if ((char)payload[1] == 'T') serial2PortMode = 2;    // v58b use internal simulation  SS241
        else if ((char)payload[1] == '\x00') serial2Stop = !serial2Stop;
        
        if (outputOnSerial) Serial.print((String) 
              " Serial2 WL="" (" + (!serial2Stop  ? "Active" : "disabled") 
            + " mode="           + serial2PortMode    // v58b   
            + " interval="       + rx2ReadInterval ) ;   //  % mqttCnt
        else Serial.print((String) "\tserial2=" 
              + (!serial2Stop  ? "e" : "d") 
              + serial2PortMode + "\t"  );

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
          if ( (char)payload[1] == '1') command_testH1();    // v52: check mqtt empty strings
          if ( (char)payload[1] == '2') command_testH2();    // v52: check mqtt empty strings
          if ( (char)payload[1] == '3') command_testH3();    // v55 
          if ( (char)payload[1] == '4') command_testH4();    // v55a redudant delay()
    } else  if ((char)payload[0] == '?') {       // v48 Print help , v51 varbls https://gcc.gnu.org/onlinedocs/cpp/Standard-Predefined-Macros.html
          Serial.println((String)"\r\n? Help commands"  + __FILE__ 
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
          Serial.println((String)"L log WL to "  + mqttLogTopic2   + "\t" +  (outputMqttLog2  ? "ON" : "OFF") );
          Serial.println((String)"e 1/2 force exception ( heap:"+ ESP.getFreeHeap() +")" );   // v52: display FreeHeap
          Serial.println((String)"E force ReadP1 fault:"          + "\t" + (doForceFaultP1  ? "Yes" : "No"));
          Serial.println((String)"B 12/+-/0-8|9 Baudrate25\t"
                                    + " serial1=" +  serial1Baudrate
                                    + " serial2=" +  serial2Baudrate);
          Serial.println((String)"l Stoplog "+ mqttLogTopic);
          Serial.println((String)"F ON/off test Rx2 function:"    + "\t" + (rx2_function  ? "Yes" : "No")  );
          Serial.println((String)"f Blueled cycle CRC/Water/Hot:" + "\t" + (blue_led2_Crc ? "Y" : "N") 
                                                                         + (blue_led2_Water ? "Y" : "N") 
                                                                         + (blue_led2_HotWater ? "Y" : "N") );
          Serial.println((String)"T RX loopback Blue0, Test1:"    + "\t" + (loopbackRx2Tx2  ? "ON" : "OFF")
                                                                  + ", mode:" + loopbackRx2Mode );
          Serial.println((String)"W on/OFF Watertrigger1:"        + "\t" + (useWaterTrigger1  ? "ON" : "OFF") ) ;
          Serial.println((String)"w on/OFF Water Pullup:"         + "\t" + (useWaterPullUp  ? "ON" : "OFF")   );
          Serial.println((String)"y print water debounce");
          Serial.println((String)"Z zero counters " + 
                + "( faults: " 
                + " Miss=" + p1MissingCnt         // v52 failed to read any P1
                + ", Crc=" + p1CrcFailCnt         // v52 Crc failed
                + ", Rcvr=" + p1RecoverCnt        // v52 recovered P1 
                + ", Rp1=" + p1FailRxCnt          // v52 rj11 not connected
                + ", Yld="+  RX_yieldcount        // V52 Yield count 0-3-8 yes/no process serial data
                + ", lT2="+  loopTelegram2cnt     // V53 print current loopTelegram2cnt
                + " )" );
          Serial.println((String)"I intervalcount 2880="          + "\t" +  intervalP1cnt);
          Serial.println((String)"i decrease interval count:"     + "\t" +  intervalP1cnt);
          Serial.println((String)"P ON/off publish Json:"  + mqttTopic + "\t" +  (outputMqttPower  ? "Yes" : "No") );
          Serial.println((String)"p ON/off publish Power:" + mqttPower + "\t" +  (outputMqttPower2 ? "Yes" : "No") );
          Serial.println((String)"M print Masking array "+ "( MaskX="+ telegram_crcOut_cnt + " )" );   // v52 number of X maskings
          Serial.println((String)"m print Input array ( Processed="+ p1ReadRxCnt + " )" );   // v52 number of Times we validated
          Serial.println((String)"h help testing C=" + __VERSION__ + " on "+ __FILE__ );
          int temp1 = mySerial1.m_bitWait * 1;
            // S P/T|0-1 serial1 on/off finish
            // s P/T|0-9 serial2 interval
          Serial.println((String)"S (p|Test|0-1) ON/off "
                        + "\t" + (!serial1Stop  ? "Yes" : "No") 
                        + "\tserial1P1-"
                        + (bSerial1State  ? "A" : "i")
                        + " , bittime=" + temp1
                        + ", mode=" + serial1PortMode        // v58b display portread or SS241 similated data
                        + ", p1SerialFinish="  + (p1SerialFinish  ? "1" : "0") 
                        + ", p1SerialActive="  + (p1SerialActive  ? "1" : "0") + " )" 
                        );
          
          int temp2 = mySerial2.m_bitWait * 1;
          Serial.println((String)"s (p|Test|0-9) ON/off" 
                        + " \t" + (!serial2Stop  ? "Yes" : "No")          
                        + "\tserial2P2-"
                        + (bSerial2State  ? "A" : "i")
                        + " , bittime=" + temp2
                        + ", mode=" + serial2PortMode       // v58b display portread or SS241 similated data
                        + ", interval:"+ rx2ReadInterval  
                        );
          
          Serial.println((String)"a ON/off/{+-0-9} Analog read:"+ nowValueAdc  +" \t" + (doReadAnalog ? "Yes" : "No") );          
          Serial.println((String)"J/j 12/+-/0-8|9 bitwait1 Jserial1=" + mySerial1.m_bitWait
                                                    + ", jserial2=" + mySerial2.m_bitWait);
          
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

    msg.concat(", \"Version\":\"%s\"");                    // V57 prog_version %u to %s  version "Version":4157.2_7_1"
    msg.concat(" }"); // terminate JSON
    
  // important note: sprinft corrupts and crashes esp8266, use snprinf which CAN handle multiple variables
  //  msg.toCharArray(msgpub, MQTTBUFFERLENGTH);         // 27aug18 changed from 256 to 320 to 360 to MQTTBUFFERLENGTH
  //  sprintf(output, msgpub,           // construct data  http://www.cplusplus.com/reference/cstdio/sprintf/ , formats: http://www.cplusplus.com/reference/cstdio/printf/
    // snprintf(output, sizeof(output), msgpub ,
    char output[MQTTBUFFERLENGTH];             // 20mar21 changed from 320 to 360, 04apr21 to #define 480, moved to here
    memset(output, 0, sizeof(output));  // initialise , v55b tto initialise
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
  if (outputOnSerial) Serial.print("\r\nProcessThermostat:") ;

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
    Serial.print("\r\nAnalog sensor = ");
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
  if (!mqttTopic) {
    Serial.println((String) "!m");
    if (outputOnSerial) Serial.println((String) "\r\n\t mqttTopic is empty.");
    return; // check empty of ! in topic
  }
  if (strstr(mqttTopic,"!") ) {
    Serial.println((String) "!m");
    if (outputOnSerial) Serial.println((String) "\r\n\t mqttTopic=" + mqttTopic + ", prohibits output.");
    return; // check empty of ! in topic
  }    

  
  Serial.print("^");     // signal write mqtt entered
  if (outputOnSerial) {  // debug
    if (verboseLevel >= VERBOSE_MQTT) {
      Serial.println((String) "\r\n[" + mqttTopic + ":" + payLoad + ".");
    } else {
      Serial.print("{");        // print mqtt start operation
    }
  }
  
  if (mqttCnt == 1 && mqttTopic != mqttErrorTopic ) {   // v51: recursive report restart reason
    char output[128];
    snprintf(output, sizeof(output), 
          "{\"info\":000, \"mqttCnt=\":%ld ,\"msg\":\"restart reason 0x%08x"
          ", epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x depc=0x%08x \"}",
          mqttCnt , save_reason, 
          save_epc1, save_epc2, save_epc3, save_excvaddr, save_depc ); // v52: print registers
    if (client.connected()) client.publish(mqttErrorTopic, output);
    // publishMqtt(mqttErrorTopic, output); // report to /error/P1 topic
  }

  if (mqttTopic and payLoad) {    // check for not nullpointer
    char mqttOutput[MAXLINELENGTH];
    payLoad.toCharArray(mqttOutput, MAXLINELENGTH);
    if (client.connected()) {
      // client.publish(mqttTopic, mqttOutput); // report to /error/P1 topic

      // Serial.println((String) "\r\nmqtt>" + __LINE__ + ":" 
      //     + "mqttCnt=" + mqttCnt
      //     + mqttTopic + ", "  + mqttOutput  + "<");  // display error on debug

      client.publish(mqttTopic, mqttOutput);          
      /*  v57: debug called itself loop
        if (mqttCnt > 3) {
        } else {
            client.publish(mqttTopic, mqttOutput);
        }
      */
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
      Serial.print("]\r\n");
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
  if (outputOnSerial and lightReadState)  Serial.print("\r\nLightReadState D6 LOW...");  // debug
  if (outputOnSerial and !lightReadState) Serial.print("\r\nLightReadState D6 HIGH..");  // debug
  return lightReadState;  // return status
}


/* 
  process P1 data start / to ! Let us decode to see what the /KFM5KAIFA-METER meter is reading
  called each time we have a telegram record
*/
bool decodeTelegram(int myLen)    // done at every P1 line read by rs232 that ends in Linefeed \n
{
  // there are 24 telegram lines (electricity() totalling  676bytes + 24 returns = 700 bytes

  //need to check for start
  validTelegramCRCFound = false;  // init telegram record
  validCrcInFound       = false;  // init Crcin record

  int startChar = FindCharInArrayRev(telegram, '/', myLen);  // 0-offset "/KFM5KAIFA-METER" , -1 not found
  int endChar   = FindCharInArrayRev(telegram, '!', myLen);  // 0-offset "!769A" , -1 not found
  bool endOfMessage = false;    // led on during long message transfer

  // if-else-if check if we are on header, trailer of in between those lines
  if (startChar >= 0) {                       // We are at start/first line of Meter reading
    // digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED off
    // digitalWrite(BLUE_LED, LOW);       // Turn the ESPLED on to inform user
    digitalWrite(BLUE_LED, !digitalRead(BLUE_LED)); // invert BLUE ked
    allowOtherActivities = false;      // hold off other serial/interrupt acvities

    // DebugCRC telegramP1header = false;
    // DebugCRC testTelegramPos = 0;     // initialise array

    // initialise CRC
    // currentCRC=CRC16(0x0000,(unsigned char *) telegram+startChar, myLen-startChar);

    //DebugCRC if (outputOnSerial) Serial.printf("{.%02X%02X%02X%02X}",telegram[myLen-startChar-2],telegram[myLen-startChar-1],telegram[myLen-startChar],telegram[myLen-startChar+1]);

    //   xN= {52 0D 00}
    //     {.0D000A00} /KFM5KAIFA-METER [16,17] {00 0D 4B}{29 0D 45}
    //                 0123456789012345
    //                 1234567890123456

    // /KFM5KAIFA-METER.0
    // currentCRC=CRC16(currentCRC,(unsigned char *) "\x0d\x0a\x00\x0d\x0a", 5);    // our intersection messages are terminated with \n which is in fact cr-lf
    //DebugCRC   currentCRC=CRC16(0x0000,(unsigned char *) "/KFM5KAIFA-METER\x0d\x0a\x0d\x0a", 21);    // our intersection messages are terminated with \n which is in fact cr-lf

    if (strncmp(telegram+startChar , "/KFM5KAIFA-METER", strlen("/KFM5KAIFA-METER")) == 0) telegramP1header = true;   // indicate we are in header mode

    currentCRC = Crc16In(0x0000, reinterpret_cast<unsigned char*>(telegram) + startChar, myLen - startChar - 1);  // initialise using header, v48-casting
    if (telegram[startChar+myLen-2] == '\x0d') { // v48-casting add crc16 0x0a
      // currentCRC = Crc16In(currentCRC, (unsigned char *) "\x0a", 1); // add implied NL on header
        unsigned char tempLiteral[] = {0x0A};  // use temporarily literal to "unsigned char* cast of Crc16In, v48-casting
        currentCRC = Crc16In(currentCRC, tempLiteral, 1);
    }

    /*
      if ( myLen = 1 && telegram[myLen-1] == '\x0d') {
        Serial.print("DebugAddCrLf.."); 
        currentCRC = CRC16(currentCRC, (unsigned char *) "\x0a\x0d\x0a", 3); // add stripped header
      }
    */

    /*
         if (outputOnSerial)  {     // Printing /KFM5KAIFA-METER
             // Serial.println("soh>\r\n");
             if (myLen > 4) {
                 for (int cnt = startChar; cnt < myLen - (startChar + 2); cnt++) { // print P1 characters
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
      if (validTelegramCRCFound) {   // temporarily test√erify on Debug switch
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
                getValues2FromP1Record(telegram_crcIn, telegram_crcIn_len);
          } else {                                            // no or myLength changed masking array

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
            telegram_crcOut[0] = 0x00;     // reset mask myLength and counters
            telegram_crcOut_len = 0;
            telegram_crcOut_cnt = 0;
            if (outputOnSerial) Serial.printf(", msk0=%d ",telegram_crcOut_cnt);

        }
        // ----------------------------------------------------------------------------------------------
      } else {    // we have a CRC error on running CRC, try to recover using using created mask
        
        if  (telegram_crcIn_len == telegram_crcOut_len && !doForceFaultP1) {    // if myLength of error is equal , try to unmask differences  ?
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
                for (int cnt = 0; cnt < myLen-2; cnt++) {

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
                //  Serial.println("\r\n"); // disabled at this  interferes with the cast EoT record
           } // else if (endChar >= 0 && telegramP1header)
      */

    } else {                                  // We are reading between header and trailer
      
      // currentCRC = Crc16In(currentCRC, (unsigned char*)telegram, myLen - 1); // calculatate CRC upto/including 0x0D
      //    reinterpret_cast<unsigned char*> to prevent "C-style pointer casting" message
      currentCRC = Crc16In(currentCRC, reinterpret_cast<unsigned char*>(telegram), myLen - 1); // calculatate CRC upto/including 0x0D, v48-casting
      
      // prevent "C-style pointer casting" message
      //  if (telegram[myLen - 2] == '\x0d') currentCRC = Crc16In(currentCRC, (unsigned char *) "\x0a", 1); // add excluded \n
      // dnw:  if (telegram[myLen - 2] == '\x0d') currentCRC = Crc16In(currentCRC, reinterpret_cast<unsigned char*>("\x0a"), 1); // add excluded \n
      // dnw:  if (telegram[myLen - 2] == '\x0d') currentCRC = Crc16In(currentCRC, "\x0a", 1); // add excluded \n
      if (telegram[myLen - 2] == '\x0d') {    // correctly use a literal to addin to Crc16in, v48-casting
        // use temporarily literal to add
        unsigned char tempLiteral[] = {0x0A};
        currentCRC = Crc16In(currentCRC, tempLiteral, 1);
      }

      if (outputOnSerial) {
        // Serial.print("<");
        if (myLen > 2) {  // (was > 2)
          for (int cnt = 0; cnt < myLen - 2; cnt++) {  // read max 64 bytes (was myLen-2)
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
          //DebugCRC if (outputOnSerial) Serial.printf("{%02X}",telegram[myLen-2]);
        }
        /* //DebugCRC
          if (myLen <= 2) {
          Serial.print(charArray [myLen]);
          }
          // Serial.print(">\r\n");
          // Serial.print(">");
        */
      } // if output serial
    } // else if endChar >= 0 && telegramP1header

  } // startChar >= 0

  // if (outputMqttLog && client.connected()) client.publish(mqttLogTopic, telegram );   // debug to mqtt log ?
  
  // if (outputMqttLog) publishMqtt(mqttLogTopic, telegram );   // debug to mqtt log, v51 produces wraparounds
  // if (outputMqttLog) client.publish(mqttLogTopic, telegramLast );

  /*
    if (!(telegram[myLen - 2] == ')') ) { // ignore if not ending on )
      telegram[0] = telegram[myLen - 2];
      telegram[1] = telegram[myLen - 2];
      telegram[2] = telegram[myLen - 2];
      client.publish(mqttLogTopic, telegram );   // log
      return endOfMessage;
    }
  */

  /*
     if (telegram[myLen - 3] == ')' ) client.publish(mqttLogTopic, "tele-3" );   // log
     if (telegram[myLen - 2] == ')' ) client.publish(mqttLogTopic, "tele-2" );   // log valid
     if (telegram[myLen - 1] == ')' ) client.publish(mqttLogTopic, "tele-1" );   // log
     if (telegram [myLen]     == ')' ) client.publish(mqttLogTopic, "tele=0" );   // log
     if (telegram[myLen + 1] == ')' ) client.publish(mqttLogTopic, "tele+1" );   // log
     if (telegram[myLen + 2] == ')' ) client.publish(mqttLogTopic, "tele+2" );   // log
     if (telegram[myLen + 3] == ')' ) client.publish(mqttLogTopic, "tele+3" );   // log

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
      // if (loopback Tx2) endOfMessage = false; // testonly, in this case mqtt are forced in overall loop
    }
    return endOfMessage;
  }

  if ( !telegramP1header) return endOfMessage; // if not in header received mode then return
  // 11jun20 yield();  // do background Wifi processing else things will not work


  // used to catch P1meter line termination bracket
  // if (telegram[myLen - 3] != ')' ) client.publish(mqttErrorTopic, telegram ); // log invalid
  // if (telegram[myLen - 3] != ')' ) client.publish(mqttLogTopic, "tele-2" );   // log invalid

  // tbd v56c: this check before or after while in recovery ????
  /*
    check valid record format 0/1-....)
  */

  if (   telegram[0] != '0' && telegram[0] != '1' && telegram[1] != '-' ) return endOfMessage; // if subrecord not start wih 0- or 1-

  // if ( !(telegram[myLen - 3] == ')' || telegram[myLen - 4] == ')' )) return endOfMessage; // if not terminated by bracket then return
  if ( !(telegram[myLen - 3] == ')')) return endOfMessage; // if not terminated by bracket then return

  /*
    // Serial.println((String)"DebugDecode6:"+ (int)telegram[myLen - 3] );   // with testdata, does not arrive here
    
    // Serial.println((String)"DebugDecode7 (=" + strncmp(telegram, "(", strlen("(")) + " )="+strncmp(telegram, ")", strlen(")"))) ;
    // Serial.println((String)"DebugDecode7 substr1-7(=" + telegram[0]+telegram[1]+telegram[2]+telegram[3]+telegram[4]+"<");
    //  c h e c k   t e s t   P 1   r e c o r d s
    // long val = 0;
    // long val2 = 0;

    // client.publish(mqttLogTopic, telegram );    // show what we have got

    //  0123456789012345678901234567890
    //  0-0:1.0.0(180611014816S) added ptro
    //  0-0:1.0.0(191119182821W)
  */
  
  /*
    below assumes segmented record processing between header and trailer
    Note: at header and/or trailer, the routine was already returned.
    tbd: v56c: consider full record at formwarding fields.....
  */
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
    powerConsumptionLowTariff = getValue(telegram, myLen);
    }

  // 1-0:1.8.2(000560.157*kWh)
  // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.8.2(", strlen("1-0:1.8.2(")) == 0)         // total Watts High
    powerConsumptionHighTariff = getValue(telegram, myLen);

  // 1-0:2.8.1(000348.890*kWh)
  // 1-0:2.8.1 = Elektra opbrengst laag tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.1(", strlen("1-0:2.8.1(")) == 0)         // total Production Low
    powerProductionLowTariff = getValue(telegram, myLen);

  // 1-0:2.8.2(000859.885*kWh)
  // 1-0:2.8.2 = Elektra opbrengst hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.2(", strlen("1-0:2.8.2(")) == 0)         // total Production High
    powerProductionHighTariff = getValue(telegram, myLen);

  // 1-0:1.7.0(00.424*kW) Actueel verbruik
  // 1-0:2.7.0(00.000*kW) Actuele teruglevering
  // 1-0:1.7.x = Electricity consumption actual usage (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.7.0(", strlen("1-0:1.7.0(")) == 0)        // Watts usage
    // client.publish(mqttLogTopic, telegram );
    CurrentPowerConsumption = getValue(telegram, myLen);


  if (strncmp(telegram, "1-0:2.7.0(", strlen("1-0:2.7.0(")) == 0)        // Watts produced
    CurrentPowerProduction = getValue(telegram, myLen);

  // 0-1:24.2.1(150531200000S)(00811.923*m3)
  // 0-1:24.2.1(250222224600W)(65.478*GJ)  -new
  // 0-1:24.2.1 = Gas (DSMR v4.0) on Kaifa MA105 meter
  if (strncmp(telegram, "0-1:24.2.1(", strlen("0-1:24.2.1(")) == 0)      // Gaz
    GasConsumption = getValue(telegram, myLen);

  return endOfMessage;
}

/*
  v45 Move recovered Datavalues back to  processed values
*/
void RecoverTelegram_crcIn() {
  if (outputOnSerial) {   // print debug changed values
    Serial.print("Recovery active.");
    if (currentTime2 != currentTime)  Serial.printf("\r\n\t..RcurrentTime2=%d/%d" , currentTime2 , currentTime );
    if (powerConsumptionLowTariff2  != powerConsumptionLowTariff)  Serial.printf("\r\n\t..RpowerConsumptionLowTariff2=%d/%d" , powerConsumptionLowTariff2 , powerConsumptionLowTariff  );
    if (powerConsumptionHighTariff2 != powerConsumptionHighTariff) Serial.printf("\r\n\t..RpowerConsumptionHighTariff2=%d/%d", powerConsumptionHighTariff2, powerConsumptionHighTariff );
    if (powerProductionLowTariff2   != powerProductionLowTariff)   Serial.printf("\r\n\t..RpowerProductionLowTariff2=%d/%d"  , powerProductionLowTariff2  , powerProductionLowTariff   );
    if (powerProductionHighTariff2  != powerProductionHighTariff)  Serial.printf("\r\n\t..RpowerProductionHighTariff2=%d/%d" , powerProductionHighTariff2 , powerProductionHighTariff  );
    if (CurrentPowerConsumption2    != CurrentPowerConsumption)    Serial.printf("\r\n\t..RCurrentPowerConsumption2=%d/%d"   , CurrentPowerConsumption2   , CurrentPowerConsumption    );
    if (CurrentPowerProduction2     != CurrentPowerProduction)     Serial.printf("\r\n\t..RCurrentPowerProduction2=%d/%d"    , CurrentPowerProduction2    , CurrentPowerProduction     );
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
  v45 Get field values from full CRC validated P1 record 2872
    currentTimeS2,powerConsumptionLowTariff2...CurrentPowerProduction2
    v56c: every field found is also copied back to its segmented (copyback1) equivilent 
*/
void  getValues2FromP1Record(char buf[], int myLen) {  // 716
  // return;
  // if (mqttCnt == 3) Serial.printf(" mqttcount=%d myLen=%d ", mqttCnt, myLen ); // mqttcount=3 myLen=716
  // if (mqttCnt < 5 ) return;   // testmode just to be sure we can do OTA if things go wrong here, v52 disabled

  int f = 0;
  // InitialiseValues();         // Data record, ensure we start fresh with newly values in coming records
                                                          //     01234567890123456789012
  f = FindWordInArrayFwd(buf, "0-0:1.0.0(", myLen, 9);      // =  "0-0:1.0.0(250106212048W)"
  // Serial.printf(" f=%d ", f ); // mqttcount=3 myLen=716
  if (( buf[f+22] == 'S' || buf[f+22] == 'W') && buf[f+23] == ')' )  {  // check for Summer or Wintertime
    char resDate[16];     // maximum prefix of timestamp message
    memset(resDate, 0, sizeof(resDate));       // Pointer to the block of memory to fill.
    if (strncpy(resDate, buf +f+16, 6)) {  // start buffer+10+1 for 6 bytes
      if (isNumber(resDate, 6)) {            // only copy/use new date if this is full numeric (decimal might give trouble)
        currentTime2  = (1 * atof(resDate)); // Convert string to double http://www.cplusplus.com/reference/cstdlib/atof/
        currentTime   = currentTime2;           // v56c, copyback1
        strncpy(currentTimeS2, buf +f+16, 6);  // Ptro 27mar21: added as we like to have readible timestamp in message(s)
        strncpy(currentTimeS , buf +f+16, 6);   // v56c, copyback1
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
      f = FindWordInArrayFwd(buf, "1-0:1.8.1(", myLen, 9);       // total use Low getValues2FromP1Record f=74+35; f=109
      powerConsumptionLowTariff2 = getValue(buf+f, 26);
      powerConsumptionLowTariff  = powerConsumptionLowTariff2;  // v56c copyback1
      
      if (outputOnSerial) { // v56c
        int s = FindCharInArrayRev(buf+f, '(', 26);  // search buffer fro bracket (s=??)
        int l = FindCharInArrayRev(buf+f, '*', 26) - s - 1;  // search buffer fro bracket (l=??)
        //                                           f=109 s=9 s=20 val=0 c1=1 c20=*_C
        Serial.printf("\r\n\t getValues2FromP1Record cpp=%d f=%d s(=%d l*=%d val=%d c1=%c c2=%c \r\n",
                     __LINE__, f, s, l, powerConsumptionLowTariff2, buf[f+s+1], buf[f+s+l]  );
      }        
  }

  // return;
 
  if (f >= 0) {
    f = FindWordInArrayFwd(buf, "1-0:1.8.2(", myLen, 9);       // total use High          (v56c: start at begin of buf)
    powerConsumptionHighTariff2  = getValue(buf+f, 26);
    powerConsumptionHighTariff   = powerConsumptionHighTariff2; // v56c copyback1

    if (outputOnSerial) {   // v56c
      int s = FindCharInArrayRev(buf+f, '(', 26);  // search buffer fro bracket (s=??)
      int l = FindCharInArrayRev(buf+f, '*', 26) - s - 1;  // search buffer fro bracket (l=??)
      //                                           f=109 s=9 s=20 val=0 c1=1 c20=*_C
      Serial.printf("\r\n\t getValues2FromP1Record cpp=%d f=%d s(=%d l*=%d val=%d c1=%c c2=%c \r\n",
                    __LINE__, f, s, l, powerConsumptionHighTariff2, buf[f+s+1], buf[f+s+l]  );
    }        
    
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf, "1-0:2.8.1(", myLen, 9);        // total Production Low   (v56c: start at begin of buf)
    powerProductionLowTariff2  = getValue(buf+f, 26);
    powerProductionLowTariff   = powerProductionLowTariff2;     // v56c copyback1
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf, "1-0:2.8.2(", myLen, 9);        // total Production High  (v56c: start at begin of buf)
    powerProductionHighTariff2 = getValue(buf+f, 26);
    powerProductionHighTariff  = powerProductionHighTariff2;    // v56c copyback1
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf, "1-0:1.7.0(", myLen, 9);        // Watts usage            (v56c: start at begin of buf)
    CurrentPowerConsumption2   = getValue(buf+f, 21);
    CurrentPowerConsumption    = CurrentPowerConsumption2;      // v56c copyback1
  }

  if (f >= 0) {  
    f = FindWordInArrayFwd(buf, "1-0:1.7.0(", myLen, 9);        // Watts produced          (v56c: start at begin of buf)
    CurrentPowerProduction2   = getValue(buf+f, 21);
    CurrentPowerProduction    = CurrentPowerProduction2;        // v56c copyback1
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
  // 0-0:1.0.0(180611014816S)                   = data record myLen=25, string myLength datalen 12
  // 0-1:24.2.1(150531200000S)(00811.923*m3)    = gaz date record which we do not have at our P1
  // 0-1:24.2.1(250222224600W)(65.478*GJ)       = new GJ
  // 1-0:1.8.2(010707.720*kWh)                  = power consumption record myLen=26 datalen 10
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
      if (buffer[22] == 'S') l = 6; // myLength 7-digits does not work vaue too large
      if (buffer[22] == 'W') l = 6; // myLength 7-digits does not work vaue too large
  */

  if (l < 4)  return 0;    // myLength from (-* too short
  if (l > 12) return 0;   // myLength from (-* too long

  char res[16];                 // extract number starting at startbracket+1 for a myLength of l
  memset(res, 0, sizeof(res));

  if (strncpy(res, buffer + s + 1, l))  // start buffer+10+1
  {
    if (isNumber(res, l))         // is this all numeric ?
    {
      return (1000 * atof(res));  // Convert Character String to Float multiplied by 1000 (to remove comma)
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
int FindCharInArrayRev(const char array[], char c, int myLen) {              // Find character >=0 found, -1 failed
  //           123456789012
  // 1234567890123456789012345678901234567890  = 40
  // 0-0:1.0.0(180611014816S)                  = sublen12 myLen=25 pos24=) post10=(=ret10, pos23=S=ret23   , maxlen==23 == return i=10
  // 0-1:24.2.1(150531200000S)(00811.923*m3)
  // 0-1:24.2.1(250222224600W)(65.478*GJ)  - GJ 
  // ...{<0-1:24.2.1(230228155652W)(84.707*m3){<!0F0C   // v39 RX2/P1 record
  for (int i = myLen - 1; i >= 0; i--)   // backward
  // for (int i = myLen - 1; i < 1; i--)   // backward
  {
    if (array[i] == c)
    {
      return i;
    }
  }
  return -1;
}

/*
  find character forwarding for myLen bytes
*/
int FindCharInArrayFwd(const char array[], char c, int myLen) {              // Find character >=0 found, -1 failed
  for (int i = 0; i < myLen+1 ; i++)   // forward
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
     myLen myLength of buf 0-768
     
     currentCRC = CRC16(currentCRC, (unsigned char*)telegram, myLen - 1); // calculatate CRC upto/including 0x0D
*/
unsigned int Crc16In(unsigned int crc, unsigned char *dataIn, int dataInLen) {
  /*

        char telegram_crcIn[MAXLINELENGTH];   // active telegram that is checked for crc
        int  telegram_crcIn_len = 0;          // myLength of this record
        char telegram_crcOut[MAXLINELENGTH];  // processed telegram with changed positions to X
        int  telegram_crcOut_len = 0;  

        for (int i = myLen - 1; i >= 0; i--)   // backward
        // for (int i = myLen - 1; i < 1; i--)   // backward
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
      if (outputOnSerial) {   // print serial record count, total receivwed myLength, calculated CRC
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
    memset(output, 0, sizeof(output));      // init v55b
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
    memset(output, 0, sizeof(output));      // init v55b
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
bool isNumber(const char *res, int myLen)       // False/true if numeriv
{
  for (int i = 0; i < myLen; i++)
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
        if ((ISR_time - last_ISR_time) > 500) {
            last_ISR_time = ISR_time;
            ISR_time_cnt++ ;          // increase our change counter      
        }

        if (outputOnSerial && verboseLevel >= VERBOSE_GPIO ) Serial.print( (String) "i" );    
        interval_delay(1); // V47 wait 1ms --> implemented by flat plain while loop, all other types forbidden in ISR
        if (waterTriggerState != (digitalRead(WATERSENSOR_READ)) ) { // check if we have really a change
            waterTriggerState = !waterTriggerState; // revert to make the same

            waterTriggerCnt++ ;             // increase our call counter
            // long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
            // waterTriggerTime  = time + 1;       // set time of this read and ensure not 0
            waterTriggerTime  = micros() + 1UL;       // set time of this read and ensure not 0, v55b

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
        // long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
        // waterTriggerTime  = time + 1;       // set time of this read and ensure not 0
        waterTriggerTime  = micros() + 1UL;       // set time of this read and ensure not 0, v55b

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
        // long time = micros();           // current counter µSec ; Debounce is wait timer to achieve stability
        // waterTriggerTime  = time + 1;       // set time of this read and ensure not 0
        waterTriggerTime  = micros() + 1UL;       // set time of this read and ensure not 0, v55b

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
    Serial.printf( "2b. Address ptr2 is pointing to: %d\r\r\n\n", ptr2);  // 1073689560
    // ptr = &b; // illegal statement, (assignment of read-only variable ptr)
    *ptr2 = b; // changing the value at the address ptr is pointing to
    Serial.printf( "2c. Value pointed to by ptr2: %c\r\n", *ptr2);  // B
    Serial.printf( "2d. Address ptr2 is pointing to: %d\r\r\n\n", ptr2);  // 1073689560

    const char *const ptr3 = &a;
    Serial.printf( "3a. Value pointed to by ptr3: %c\r\n", *ptr3);  // B
    Serial.printf( "3b. Address ptr3 is pointing to: %d\r\r\n\n", ptr3);  // 1073689560
    ptr = &b; // should be illegal statement and does not do anything, ptr3 unchanged 
    // *ptr = b; // illegal statement, (assignment of read-only location *ptr)
    Serial.printf( "3c. Value pointed to by ptr3: %c\r\n", *ptr3);  // B
    Serial.printf( "3d. Address ptr3 is pointing to: %d\r\r\n\n", ptr3);  // 1073689560

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
    
    // Serial.println((String)"\r\nv51 snprint" + output3 + "n\r");
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

void command_testH3(){    // publish mqtt records in TEST_MODE
  #ifdef TEST_MODE
    publishMqtt(mqttErrorTopic, "TestH3a");      // v51:   CurrentPowerConsumption: %lu
    publishMqtt("", "TestH3b");      // v51:   CurrentPowerConsumption: %lu
    const char *mqttErrorTopic_h3 = "/error!/"  P1_VERSION_TYPE;
    publishMqtt(mqttErrorTopic_h3, "TestH3c");      // v51:   CurrentPowerConsumption: %lu
  #endif        
    /* v55 022 - disabl all code below, after activating WiFi.persistent(false);
                // production unstable, test looks fine (on RX2 noninverted at least)
    
        doesnot change things: code goes unstable if below is removed
  */
  // // make table teststable1 , active with of without 2 or 3 from printf  results in stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     delay(0);     // 008 adding here without printf above , stable
  //                     // ---------------------------- 009 line 2585 added printf (unstable) 
  //                     delay(0);     // 008 adding 009 test remove, add 013 more stable (rx2-30%)
  //                     delay(0);     // 008 adding 009 test remove, add 012 stable (rx2-10%)
  //                     delay(0);     // 008 adding 009 test remove, add 011 unstable
  //                     delay(0);     // 008 adding 009 test remove, add 011 unstable
  //                     delay(0);     // 008 adding 009 test remove, add 010 unstable
  //                     delay(0);     // 008 adding 009 test remove, add 010 unstable
  //                     delay(0);     // 008 adding 009 test remove, add 010 unstable
  //                     delay(0);     // 008 adding 009 test remove, add 010 unstable
  //                     delay(0);     // add13 + add 014 stable (rx2-70%)



                      // delay(0);     // add14 + add015 stable (rx2=40%), remove 021
                      // delay(0);     // add15 + add016 less stable (rx2=20%), remove 021
                      // delay(0);     // add16 + add017 stable (rx2=50%), remove 021
                      // delay(0);     // add17 + add018 stable  (rx2=10%), remove 021
                      // delay(0);     // add18 + add019 stable  (rx2=75%), remove 021
  /* */  
}

void command_testH4(){    // code to maken things stable teststable
  //                   // we remoived some unused protection arrays, improved ISR-time, 
                // v57c-0  added multiple combinations, things remain instable
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability

                    delay(0);     // v58 add to check for stability
                    delay(0);     // v58 add to check for stability
                    delay(0);     // v58 add to check for stability
                    delay(0);     // v58 add to check for stability   // v58c 267 + asm 10

                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability  // v58c 10

                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability   // v58c 3

                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
#ifdef TEST_MODE                    
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
                    delay(0);     // v57 add to check for stability
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

void command_testH5(){    // code to maken things stable teststable v57c-1 no-effect
  //                   // we remoived some unused protection arrays, improved ISR-time,
    String msg3 = "{"; // build mqtt frame 
    char msgpub3[64];     // allocate a message buffer
    // char output3[64];     // use snprintf to format data
    msg3.concat("\"currentTime\":\"%d\"");                  // %s is string whc will crash
    msg3.concat(",\"CurrentPowerConsumption\":%lu }");    // P1
    msg3.toCharArray(msgpub3, 64);   
    msg3.toCharArray(msgpub3, sizeof(msgpub3)); // v51 convert/move/vast to char[]
    publishMqtt(mqttErrorTopic, msg3);       // v51: {"currentTime":"%s","CurrentPowerConsumption":%lu
    publishMqtt(mqttErrorTopic, msgpub3);    // v51: {"currentTime":"%s","CurrentPowerConsumption":%lu

}
