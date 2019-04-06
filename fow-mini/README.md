# Ferries Over Winslow Mini
This is the code that runs on a microcontroller to power smaller-scale displays (clocks, sliding displays, etc.) The client is written in C++ and runs with the Arduino support libraries, but actually runs on an ESP8266.
## Configuring your Mini
The microcontroller broadcasts a network called `fow-mini`, on which it serves a configuration webpage. Once you connect to that network, you can enter [fow-setup.local](http://fow-setup.local) into your web browser (if that doesn't work, try entering [192.168.4.1](http://192.168.4.1)). On that page you should enter in the network name and password and press apply. The status box will show your connection status. If you can't connect, check the network name and password, and try positioning your microcontroller closer to the access point you're connecting to. If you're having trouble connecting, feel free to [post an issue on Github](https://github.com/pietroglyph/fow/issues/new).
## Flashing your microcontroller
The easiest way to get the code onto your ESP8266 (the code requires an ESP8266, but it doesn't really matter what's on the rest of the board, the NodeMCU DEVKIT 1.0, WeMos D1 Mini and regular, and Adafruit Feather Huzzah are all known to work) is to use [PlatformIO](https://platformio.org/). 

You should follow the below steps to deploy code using PlatformIO:
1. Download and install PlatformIO, either [through VSCode/Atom](https://platformio.org/platformio-ide), or [through your command line interface](https://docs.platformio.org/en/latest/installation.html#installation-methods).
2. Open a terminal in your editor with the working directory in the same folder as this README (just do Ctrl+Shift+\` in VSCode). 
3. Type in `./deploy.sh` (you should add ` -e stepperclock` at the end of that if you want to deploy to the stepper clock). If it says something like "bash: platformio: command not found...", then type in `export PATH=$PATH:~/.platformio/penv/bin`, press enter, and run the script again.

You can follow the below steps if you're using the Arduino IDE (not reccomended, will only deploy for the servo clock unless you modify code):
1. Download and install the Arduino IDE for your operating system from [here](https://www.arduino.cc/en/Main/Software) (the web editor wont work; if you have a package manager of some sort, you can also install using that).
2. Install the tooling for the ESP8266 by following the "Install with Boards Manager" section of the esp8266/Arduino README [here](https://github.com/esp8266/Arduino#installing-with-boards-manager). **The only supported `esp8266` version is 2.5.0! You must select this version when installing from Boards Manager.**
3. Install the `AccelStepper` library following [this](https://www.arduino.cc/en/Guide/Libraries#toc3) guide (using the library manager). The only supported version is 1.57.1. Other versions may or may not work, but they are unsupported and untested for this project.
4. In the Arduino IDE, go to `Tools > Board` and then select your board (under the ESP8266 modules section of the `Tools > Board` pullout).
5. Once you have set your board, set `Tools > lwIP Variant` to "v2 Higher Bandwidth". Other options will likely work, but are unsupported.
6. Optionally you can set your upload speed to 921600 baud under `Tools > Upload Speed` if you don't want to wait forever.
7. Press the `Upload` button (Looks like ->) in the top left hand corner of the Arduino IDE.
