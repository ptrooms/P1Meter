# EspSoftwareSerial

Implementation of the Arduino software serial library for the ESP8266
adapted to specifically process/read P1 telegrams from DSMR electriticy meters.

These meters produce telegrams start start with '/' headers and terminate with
'!' trailer. In addition the serial read process @115200 Baud is fully put in
Iram/cache to minimize the risk of WDT-imeouts and improve serial read reliability.

Same functionality as the corresponding AVR library but several instances can be active at the same time.
Speed up to 115200 baud is supported. The constructor also has an optional input buffer size.

Please note that due to the fact that the ESP always have other activities ongoing, there will be some inexactness in interrupt
timings. This may lead to bit errors when having heavy data traffic in high baud rates.


