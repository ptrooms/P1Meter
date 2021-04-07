# EspSoftwareSerial

Implementation of the Arduino software serial library for the ESP8266 towards P1

It is based upon the fabulous softserial of https://github.com/plerup/espsoftwareserial

The version is based on 3.3.1 where the code is extende with bool P1Active()
which will becom active at reading '/' (header of P1 DSMR metering) and goes off
at '!' (trailer of P1 DSMR telegrams).

Same functionality as the corresponding AVR library but several instances can be active at the same time.
Speed up to 115200 baud is usable. The constructor also has an optional input buffer size.

Please note that due to the fact that the ESP always have other activities ongoing, there will be some inexactness in interrupt
timings. This may lead to bit errors when having heavy data traffic in high baud rates.


