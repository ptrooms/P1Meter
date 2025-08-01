![ptro logo](/tools/logo/ptro_930x474_longwhite_ends_bg.svg)
# Changelog
All notable changes to this project will be documented in this file.
Co-authored-by: Peter Ooms <34420738+ptrooms@users.noreply.github.com>

Note : upto V66 we developed on Vxx, master is production. 
Note2: supended deveopment branch
create stable Versions-Vxx which when stable are merged into master.
## [v21.56] - reworked to make it stable again (july2025)
	- platformio extended with local script that disassembles
	- improved coding after inspect
	- v56c pulled and merged to master
	- deleted v56c
	- Tx commands to inspect RX2 data: in mode > 0, =2, =3, =4
## [v21.55] - Unstable / abandoned
	- program behaves erratic in general 
	- serial reading suddenly refuses to read data
	- on git we reworked things vrom v55a to v55g
## [v21.54] - Major release fully stable
	- lot's of changes since, see source code for details. Highlights:
	- working version V52 now made to new master
	- added code to test/check serial offline (desktesting)
	- Tested Platformio eespressif platform 2.5.3. (Arduino 2.7.1) (v55)
	- extended PlatformIO coices for OTA/Production/TestMode using Arduino 2.4.1 and 2.7.1
	- working version V52 now made to new master
	- CRC16 operational on Warmtelink (v52)
	- Added mltiple commands cia mqtt to test/diagnose/control operation, use ? for summary. (v49)
	- centralised MQTT publish to routine (v51)
	- Improved stability to anticipate records failures 
	- P1 telegram recovery by using masking of earliers successes.
	- Migrated to new esp8266 NodeCPU.
	- Resolved/stabilised Water tapping measurements.
	- Improved Wifi stabilty by disabling WifiSleep Persistency"
## [v21.42] - redo production stable PlatformIO
	- v42 added WiFi.setSleepMode(WIFI_NONE_SLEEP); // trying to hget wifi stable see https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/generic-class.html
	- v41 slightly modify water intewrrupt to reset status to LOW at exceeding trigger count
	- v40 improve WarmteLink detection, sometimes skipped....
	- v39 LGTM collect data value from RX2/P1 & output to mqtt , 17mar23 stabilised
	- v38 testv38_Function false LGTM stabilized by inter/procedure bytes
	- v36 Corrected debounce WaterPulse 100 to 1000mS
## [V22.35] - production/stable
	- V22.35 - implement secondary RX2 on GPIO4/D2 INVERTED for direct connection to warmtelink pulldown
    -- impment logging for RX2
## [V22.34] - production/develope
	- V22.34 - develop and use  gpio4 for seocndary RX conneted to Liander warmtelink
    -- solve interrupt overhead and RX2 windowing outside P1 read.
	-- solve and improve logging reporting
## [V21.33] - production
	- V21.33 cosmetic, fix line behavior, Terminate debug output line with crlf
# Note: [V32 8361d6d] Co-authored-by: Peter Ooms <34420738+ptrooms@users.noreply.github.com>
	- as we use email restriction on https://github.com/settings/emails
	-- set mail pafoxp@ubuntuO380:~/code-P1Meter$ git config --global user.email "34420738+ptrooms@users.noreply.github.com"
	-- set author pafoxp@ubuntuO380:~/code-P1Meter$ git commit --amend --reset-author --allow-empty
	-- publish pafoxp@ubuntuO380:~/code-P1Meter$ git push
## [V21.32] - production
	- V21.32Fix/ensure Hotwater sensor in inverted LOW at first mqttCNT to prevent ON state
	-- Done in lightread routine where we simply compare mqttCnt = 0 
	- SynC & Publish all changes as we have done, things works so let it operate
	-- not sure if and to whart state everything was developed.
## [Unreleased] - Development
	- V21.31 changed P1-Rxread to ensure that the wait for clock-sync never exceeds 14bits of waitcycles
	-- Note this is doen by limiting the wait to 10.000 cycles and meant for highspeed connection
	-- at a later stage we can look at adaptation for lower speeds (either integrate or seperation)
	- V21.31 test restoreed to work on production servers, local mqtt & wifi has nu influence on quality
	- V21.31 Do not wait in full for finishing stopbit  at a serial read (hosrten the wait).
	-- this also limits the risk of an infinite loop is ESP.GetCyleCnt does not pace up.
	- V21.31 step by step changes to check why volatile changes corrupts the P1 production meter.
	- V21.25 Core 2.4.1, ESP8266-sdk-version: 2.2.1(cfd48f3) testing stability (wdt reset)
	-- The SoftSerialP1 librbay is changed by putting esp.getcyclecnt readRx as GetCycleCntIram into cache.
	- V21.23_seriallib - testing/adding P1ActiveCnt which correctly tell we have rxRead loop during serial
	-- we can confirm that rxRead is called per byte.
## [V21.24]
	- merged to master
	- tested & verified on test using dummy data
	- restructered decode-routine and disconnected from readecodepart
	- now the readed telegram is executed WITH the offered 0x0D (carriage return)
## [V21.23]
	- production version
	- mqtt Callback routine shortened and incoming command done in mainloop
	- set ESP.wdtEnable(WDTO_8S)in setup() to 8seconds see https://github.com/esp8266/Arduino/issues/555
## [V21.22]
	- 19apr21 21u38 option added to make nodemcu Ip adress static
	- -- override added/extended with production/static Ip information
	- 19apr21 21u38 display library version at start
	- 15apr21 00u02 changed to platformio 1.6.0 (uses arduino 2.4.0) to check if this stabilize
	- 15apr21 00u02 platformio 1.7.0 (uses arduino 2.4.1) wdt reset after 500-800 reads
	- removed delay in local-yields
## [V21.21]
- 14apr21 01u50 only output to mqtt if it is connected  via "if (client.connected())" 
- 13apr21 18u38 V21 added progress line-counter tio research where WDT hits....
	--- (+9sec after last mqtt)
- 13apr21 18u38 V21 improved WDT as we call "mqtt client".loop() during speific yields,
	-- normal yield does support Wifi but NOT the (disconnected) Pubsubclient
	-- Beautified
## [V21.20]
- 11apr21 16u29 V20: added JSON error message to topic /error/.. if P1 serial is not (properly) connected
	-- /error/t1 {"error":001 ,"msg":"serial not connected", "mqttCnt":28}
- 11apr21 15u16 mqtt timeout (override) set from 15 to 60 seconds #define MQTT_KEEPALIVE = 60
	-- sometimes the mqtt server is very busy and we do not want a preliminary reboot
- 11apr21 15u15 Added mqtt server address  in serial debug at startup
## [V21.19]
- 08apr20 05u30 - PtrO Adapted to Arduino-IDE (checked) & PlatformIO  (checked) see ini files.
- 07apr21 19u13 - PtrO Initiial GIT version , Arduino compile succesfull using this Repo
---------------------------------------------------------------------------------------
## Old log within program at development site:
- 04apr21 V21.19/241 Hardware Library 2.4.1 and use Lwip 2.0 lower memory, very stable
- 03apr21 V21.18/274 Lower mempry no features
- 03apr21 V21.18/274 -68 bytes: Refactored incude libraries UseP1SoftSerialLIB will include <SoftwareSerial241>
- 03apr21 V21.18/274 +332 bytes: sprintf causes corruption, use snprinf when using multiple formatted valaues 
		-- removed unused librabries //here ESP8266mDNS.h ESP8266HTTPClient.h WiFiUdp.h OneWire.h
- 03apr21 V21.18/274 +4bytes: Progress message changed to 
		-- M#@t=15 @201, TimeP1="000309" (e#=0)    Gpio5:1, Water#:0, Hot#:0, WaterState:0, Trg#:0, DbC#:0, Int#:344 .
-      - debug output: mqttcnt/secs  P1-readed hhmmss  CrcE    state    count     Ledl    sensor        #number Debounce Interval
- 03apr21 V21.17/274 mqtt buffer increated from 360 to 480
		-- if serial data contain a space, it is also counted as invalid character (field e#=xxx)
		-- buffer too small, leading to JSON failures
		-- when dynamically adding "Force, Ttrigger and PullUp fields causing "}" deletion overflow
- 03apr21 V21.17/274 changed BLUE-LED state will invert during watertrigger (will reset at end by thermostate)
		-- Normally this wil show  & relfect the  (stable) Thermostate ON/OFF state
		-- during mqtt & active serial P1 read/processiong it will blink for that  (short) duration
		-- During Watertapping, the BLUE_LED will be pulsed by the attached interrupts (hence twice per liter).
- 03apr21 V21.17/274 streamlined source and added loop-() check timer 1 second interval to limit WDT reset risks
- 02apr21 V21.16/274 testOnly chekc ISR P1serial timings  Header/Trailer (hardly a cycle and near to a µSec)
- 02apr21 V21.15/274 Using My changed 240 to display serial progress
- 02apr21 V21.14/274 Only insert a date from P1Meter data in JSON mqtt if this fully nummeric else use previous (whioch could be "000000"
- 02apr21 V21.14/274 Added runtime in Seconds at Timestamp to ease PD
- 01apr21 V21.14/274 Making CRC work with esp8266 is a change of 1 top 15 as as softserial often goes in missing characters
		-- extended mqtt record with Json P1crc state 0=NotOk, 1=Valid CRC
		-- mqtt log record corrected "powerConsumptionLowTariff" exchanged with "powerConsumptionHighTariff"
		-- hardly no alternative that accepting that almost every tranaction has an error, as long we can read the values in number fields, this is OK.
		-- cleaned comments and debugging to check why CRC16 constantly fails.
- 30mar21 v21.13/274 added yield time control (p1TriggerTime) to prevent we get a wdt reset while waiting for serial input
- 30mar21 v11.13/274 added CRC , using/following [https://github.com/jantenhove/P1-Meter-ESP8266]
- 30mar21 v11.13/274 started with fixing RX2 port on GPIO2 output and GPIO4 Input, rememeber we have two class instances
		-- added yield() after reading P1 available serial data....
- 30mar21 v21.12/274 If mqtt is not connected, continue to support the primary/core thermostatic function
		-- the routines are implemented in doCritical(): processGpio() & handleOTA()
		-- the mqtt client.connect routine is added with extra code to increased wait loops stating at 2 tot 30 seconds this to prevent that unnecessary load.
		-- Note: extensive printing can significantly impact total performance.
		-- remember: the esp8266 is a single task machine that operates in a loop
- 30mar21 v21.12/274 source code added to distinquick Test from Production (TEST_MODE absent)
- 28mar21 v21.11/274 added ESP.wdtFeed() after yield() to feed the WDT timer.
- 27mar21 V21.11/274 If (inverted) ledlight1 is active (waterheating on), accumulate the watercnt also as "HotCnt"
		-- added loop timers: overall 120mSec,
		-- approx 2Sec is for 10Sec/P1 intervals minus margins 1-2 seconds,
		-- which leaves about 6 seconds for other items.
		-- added check not to process P1serial if we are already finished
		-- in case serial reads a zerolength byte, innore this
		-- improved/validation of timestamp. Without P1Meter we use millis() to calculate timestamp
		-- During forced mode (to retain) temprature functions, Mqtt display "Force:cnt"
- 26mar21 V21.10 Changed library /home/pafoxp/Arduino/libraries/SoftwareSerial-2.4.0/SoftwareSerial240.cpp
		-- void ICACHE_RAM_ATTR SoftwareSerial::rxRead() {
		-- GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);  - 26mar21 Ptro done at start as per advice espressif
- 26mar21 V21.10 clearing interrupt for our WATERSENSOR_READ at start of routine
- 26mar21 V21.09 Corrected Debug print inactive/active state at command "D"
- 26mar21 V21.09 Stable version, improved reliability by fluctuating input pull-up to assist state.
- 23mar21 V21.08 hardware set to use external pullup, internal pullup  WaterSensor disabled
		-- in LoopBack testmode, the WaterSensor readstate is forewarded to GPIO2 (TX2)
		-- Introduced "W" to reWire ISR watersensor interrupt WaterTrigger1 (<>Watertrigger)
		-- Introduced "w" to switch ON (<>OFF) internal pullup for watersensor pin
- 23mar21 V21.08 revise debounce WaterTrigger via ISR, add "Z" to zeroise counters
- 21mar21 V21.07 Arrange and use old SoftwareSerial code  (of hw 2.4.0) to accomodate rs232
-   allocated dedicated softeware serial librbary version 3.3.1. as (was) used in hardwaresetup 2.4.0
		-- newer librbaries are to slow and not useful at baudrate > 57600
		-- for this: allocate SoftwareSerial-2.4.0 with name SoftwareSerial240.h/.cpp
		-- code require the use for "#define UseP1SoftSerialLIB" to disinquish from new layout
		-- new serial layout as of version 2.6.0 uses a setup that's more compatible with hardware serial library.
		-- P1 serial databuffer increased 600 bytes
- 20mar21 V21.06  prepare watertap meter T1MeterT.5.ino
		-- structured D0-D8 Arduino port schema
		-- added myserial2 (1200 baud for Kamstrup byte for byte interface)
- 29jun20 V13.05 renamed to T1MeterT.4.ino
- T1MeterT.4, external p1 renamed to t1
- 29jun20 V13.05 copied from /home/pafoxp/code-P1meter/P1Meter1.4 but for test only
- ----------------------------------------------------------------------------------------
- 29jun20 V13.05 testing OTA which failed at laptop
- 10jun20 V13.04 P1 timeout processing, allow do logic wihout P1 connected, i/I-nterval
- 19nov19 V13.03 tiem string with leading zeroes is sometimes/somewhere not acceptable as json-number, reverted to valu number
- 19nov19 V13.03 reverted back to 2.4.0 (2.5.2 instable rs232), add timestring is now substringed from timedate, eases conversion
- 19nov19 V13.03 l/L activates off/on Mqtt logging of kaifka meter
- 28okt18 V13.03 2.5.2 LwIP V2: solve double ,, in json output , add mqtt count
- 28okt18 V13.02 2.4.0 LwIP V2: S/W-inter time in "0-0:1.0.0(181028223640W)" message
- 13sep18 V13.01 2.4.0 LwIP V2: Default P1 follow mode
- 04sep18 V1.3 Compile with esp8266 expressif 2.3.0
- 27aug18 V1.3 MQTT pubsize expanded from 256 to 320 (as we added a field)
- 27aug18 V1.3 Added temperature device 6 (livingroom Whites=GND, Green=Signal, Orange=3.3V) 28aa93bb13130138 as T0 (??)
- 27aug18 V1.3 Thermistor replaces by LDR in Livingroom (Blue/white wire)
- 01aug18 V1.3 Added 4 devices DS18B20 JSON published as T2, T3, T4, T5
- 24jul18 V1.3 DS18B20 temperature reading which is JSON published as T1
- 14jul18 V1.2 MQTT subscription for commands mosquitto_pub -h 192.168.1.8 -p 1883 -t "nodemcu-p1/switch/port1" -m "L"
		-- Reset, noLogmqtt, doDebugserialout, doP1mqtt, 1-heatD8/2-off/3-leave/2-followD7
- 25jun2018 00u00 - added Digital read D6 {LedLight1} for hotwater LedLight sensor
- 24jun2018 00u00 - added analog read A0 (Ajson: {analogRead} for temperature sensor
