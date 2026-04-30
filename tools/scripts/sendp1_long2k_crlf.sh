#!/bin/bash
# Mulitple single P1 validated telegram crc=078E to check on speed

# During testing this can be send with $ while sleep 9; do ./sendp1_crlf.sh > /dev/ttyUSB2; done
# withou sleep: scoped at  58mS , processed at esp8266 at 113523 micros , same as sendp1_crlf.sh
# with 0.010 delay for each line: 245mS, processed at esp8266 P1time=623428 microS
#
sleep 0.010; echo  -e -n '/KFM5KAIFA-METER\r\n'
sleep 0.010; echo  -e -n '\r\n'
sleep 0.010; echo  -e -n '1-3:0.2.8(42)\r\n'
sleep 0.010; echo  -e -n '0-0:1.0.0(210420113523S)\r\n'
sleep 0.010; echo  -e -n '0-0:96.1.1(1234567890123456789012345678901234)\r\n'
sleep 0.010; echo  -e -n '1-0:1.8.1(012345.111*kWh)\r\n'
sleep 0.010; echo  -e -n '1-0:1.8.2(012345.222*kWh)\r\n'
sleep 0.010; echo  -e -n '1-0:2.8.1(000000.000*kWh)\r\n'
sleep 0.010; echo  -e -n '1-0:2.8.2(000000.000*kWh)\r\n'
sleep 0.010; echo  -e -n '0-0:96.14.0(0002)\r\n'
sleep 0.010; echo  -e -n '1-0:1.7.0(00.560*kW)\r\n'
sleep 0.010; echo  -e -n '1-0:2.7.0(00.000*kW)\r\n'
sleep 0.010; echo  -e -n '0-0:96.7.21(00003)\r\n'
sleep 0.010; echo  -e -n '0-0:96.7.9(00003)\r\n'
sleep 0.010; echo  -e -n '1-0:99.97.0(5)(0-0:96.7.19)(210407073103W)(0000001111*s)(210407073103W)(0000002222*s)(210407073103W)(0000003333*s)(0000004444*s)(210407073103W)(0000005555*s)(0000006666*s)(210407073103W)(0000007777*s)(0000008888*s)(210407073103W)(0000009999*s)(210407073103W)(0000001404*s)(181103114840W)(0000008223*s)(180911211118S)(0000003690*s)(160606105039S)(0000003280*s)(000101000001W)(2147483647*s)\r\n'
sleep 0.010; echo  -e -n '1-0:32.32.0(00000)\r\n'
sleep 0.010; echo  -e -n '1-0:32.36.0(00000)\r\n'
sleep 0.010; echo  -e -n '0-0:96.13.1()\r\n'
sleep 0.010; echo  -e -n '0-0:96.13.0()\r\n'
sleep 0.010; echo  -e -n '1-0:31.7.0(002*A)\r\n'
sleep 0.010; echo  -e -n '1-0:21.7.0(00.560*kW)\r\n'
sleep 0.010; echo  -e -n '1-0:22.7.0(00.000*kW)\r\n'
sleep 0.010; echo  -e -n '!1680 sendp1_long_crlf.sh 2Kbyte. \r\n'
 
