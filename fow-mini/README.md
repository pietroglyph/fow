# Ferries Over Winslow Mini
This is the code that runs on a microcontroller to power smaller-scale displays (clocks, sliding displays, etc.) The client is written in C++ and runs with the Arduino support libraries, but actually runs on an ESP8266.
## Flashing your microcontroller
The easiest way to get the code onto your ESP8266 (the code requires an ESP8266, but it doesn't really matter what's on the rest of the board, the NodeMCU DEVKIT 1.0 and Adafruit Feather Huzzah are both known to work) is to use the Arduino IDE (based on Processing), and use the IDE's included tools to get the required libraries and tooling to compile your code and flash it to the ESP8266.

You can follow the below steps:
1. Download and install the Arduino IDE for your operating system from [here](https://www.arduino.cc/en/Main/Software) (the web editor wont work; if you have a package manager of some sort, you can also install using that).
2. Install the tooling for the ESP8266 by following the "Install with Boards Manager" section of the esp8266/Arduino README [here](https://github.com/esp8266/Arduino#installing-with-boards-manager).
3. Install the `AccelStepper` library following [this](https://www.arduino.cc/en/Guide/Libraries#toc3) guide (using the library manager).
4. In the Arduino IDE, go to `Tools > Board` and then select your board (under the ESP8266 modules section).
5. Optionally you can set your upload speed to 921600 baud under `Tools > Upload Speed` if you don't want to wait forever.
6. You should also change any other settings applicable to your board under `Tools`.
7. Press the `Upload` button (Looks like ->) in the top left hand corner of the Arduino IDE.

