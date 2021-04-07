W.i.P Home Automation via ESP8266 WiFi chip
===========================================

This project brings control using the ESP8266 NodeMCU 12E board to the Home.
The project aims control throught MQTT as a central control module for
- Thermostat state and Heating control
- measure/reading serial 115K2 serial connected P1 electricity status
- Measuring Pulsed Watermeter
- getting Lux/Light state
- measuring Hotwater usage

### Installing / using Arduino IDE
- TBD

- Install the current upstream Arduino IDE at the 1.8 level or later. The current version is at the [Arduino website](http://www.arduino.cc/en/main/software).
- Start Arduino and open Preferences window.
- Enter ```http://arduino.esp8266.com/stable/package_esp8266com_index.json``` into *Additional Board Manager URLs* field. You can add multiple URLs, separating them with commas.
- Open Boards Manager from Tools > Board menu and install *esp8266* platform  level 2.4.1 (and don't forget to select your ESP8266 nodemcu-12E board from Tools > Board menu after installation).


### Installing / using Platform IO
- TBD

#### Latest release 
Boards manager link: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`

Documentation: TBD

### Using git version

- Install Arduino 1.8.2 from the [Arduino website](http://www.arduino.cc/en/main/software).
- Go to Arduino directory
- Clone this repository into a source directoryh
```bash
git clone https://github.com/esp8266/Arduino.git esp8266
```
### Using PlatformIO

[PlatformIO](http://platformio.org?utm_source=github&utm_medium=arduino-esp8266) is an open source ecosystem for IoT
development with cross platform build system, library manager and full support
for Espressif (ESP8266) development. It works on the popular host OS: macOS, Windows,
Linux 32/64, Linux ARM (like Raspberry Pi, BeagleBone, CubieBoard).

- [What is PlatformIO?](http://docs.platformio.org/en/latest/what-is-platformio.html?utm_source=github&utm_medium=arduino-esp8266)
- [PlatformIO IDE](http://platformio.org/platformio-ide?utm_source=github&utm_medium=arduino-esp8266)
- [PlatformIO Core](http://docs.platformio.org/en/latest/core.html?utm_source=github&utm_medium=arduino-esp8266) (command line tool)
- [Advanced usage](http://docs.platformio.org/en/latest/platforms/espressif8266.html?utm_source=github&utm_medium=arduino-esp8266) -
  custom settings, uploading to SPIFFS, Over-the-Air (OTA), staging version
- [Integration with Cloud and Standalone IDEs](http://docs.platformio.org/en/latest/ide.html?utm_source=github&utm_medium=arduino-esp8266) -
  Cloud9, Codeanywhere, Eclipse Che (Codenvy), Atom, CLion, Eclipse, Emacs, NetBeans, Qt Creator, Sublime Text, VIM, Visual Studio, and VSCode
- [Project Examples](http://docs.platformio.org/en/latest/platforms/espressif8266.html?utm_source=github&utm_medium=arduino-esp8266#examples)

### Building with make

TBD 
[makeEspArduino](https://github.com/plerup/makeEspArduino) is a generic makefile for any ESP8266 Arduino project.
Using make instead of the Arduino IDE makes it easier to do automated and production builds.

### Documentation
TBD 

Documentation for latest development version: t.b.d.

### Issues and support ###

[P1 Community Mail] - via GIT

If you encounter an issue which you think is a bug in Source or the modified libraries, you are welcome to submit it here on Github: https://github.com/ptrooms/P1meter/issues.

Please provide as much context as possible:

- ESP8266/Arduino core version which you are using (you can check it in Boards Manager)
- your sketch code; please wrap it into a code block, see [Github markdown manual](https://help.github.com/articles/basic-writing-and-formatting-syntax/#quoting-code)
- when encountering an issue which happens at run time, attach serial output. Wrap it into a code block, just like the code.
- for issues which happen at compile time, enable verbose compiler output in the IDE preferences, and attach that output (also inside a code block)
- ESP8266 development board model
- IDE settings (board choich, flash size)

### Contributing

For minor fixes of code and documentation, please go ahead and submit a pull request.

Check out the list of issues which are easy to fix — [easy issues for V.19](https://github.com/ptrooms/P1meter/issues). Working on them is a great way to move the project forward.

Larger changes (rewriting parts of existing code from scratch, adding new functions to the core, adding new libraries) should generally be discussed by opening an issue first.

Feature branches with lots of small commits (especially titled "oops", "fix typo", "forgot to add file", etc.) should be squashed before opening a pull request. At the same time, please refrain from putting multiple unrelated changes into a single pull request.

### License and credits ###

P1meter is developed and maintained by Peter Ooms. 
The IDE and all libraries are licensed under GPL.
ESP8266 core includes an xtensa gcc toolchain, which is also under GPL.

Esptool written by Christian Klippel is licensed under GPLv2, currently maintained by Ivan Grokhotkov: https://github.com/igrr/esptool-ck.
Espressif SDK included in this build is under Espressif MIT License.
ESP8266 core files are licensed under LGPL.

[SPI Flash File System (SPIFFS)](https://github.com/pellepl/spiffs) written by Peter Andersson is used in this project. It is distributed under MIT license.
[umm_malloc](https://github.com/rhempel/umm_malloc) memory management library written by Ralph Hempel is used in this project. It is distributed under MIT license.

[axTLS](http://axtls.sourceforge.net/) library written by Cameron Rich, built from https://github.com/igrr/axtls-8266, is used in this project. It is distributed under [BSD license](https://github.com/igrr/axtls-8266/blob/master/LICENSE).
