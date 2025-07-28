#!/bin/bash
# Mulitple single P1 validated telegram crc=078E to check on speed

# During testing this can be send with $ while sleep 9; do ./sendp1_crlf.sh > /dev/ttyUSB2; done
# withou sleep: scoped at  58mS , processed at esp8266 at 113523 micros , same as sendp1_crlf.sh
# with 0.010 delay for each line: 245mS, processed at esp8266 P1time=623428 microS
#
sleep 0.010; echo  -e -n '/KFM5KAIFA-METER\r\n'
sleep 0.010; echo  -e -n '\r\n'
sleep 0.010; echo  -e -n "1-1:1.1.1( _\"#$%&'()*+,-._0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\`abcdefghijklmnopqrstuvwxyz{|}~)\r\n"
sleep 0.010; echo  -e -n '1-0:1.8.1(012345.111*kWh)\r\n'
sleep 0.010; echo  -e -n '1-0:1.8.2(012345.222*kWh)\r\n'
sleep 0.010; echo  -e -n '1-0:1.7.0(00.560*kW)\r\n'
sleep 0.010; echo  -e -n '1-0:21.7.0(00.560*kW)\r\n'
sleep 0.010; echo  -e -n "1-2:2.2.2( _\"#$%&'()*+,-._0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\`abcdefghijklmnopqrstuvwxyz{|}~)\r\n"
sleep 0.010; echo  -e -n '!78C7 Ascii table'
 
