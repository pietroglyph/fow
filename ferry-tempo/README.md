# FERRY TEMPO
This is the code that runs on a microcontroller to power smaller-scale displays (clocks, sliding displays, etc.) The client is written in C++ and runs with the Arduino support libraries, but actually runs on an ESP8266.


## Configuring your FERRY TEMPO
The microcontroller broadcasts a network called `FERRY TEMPO Setup DEVICE SERIAL NUMBER` (where the device serial number looks something like "0BCDEF"), on which it serves a configuration webpage. Once you connect to that network, you can enter [setup.ferries-over-winslow.org](http://setup.ferries-over-winslow.org) into your web browser (if that doesn't work, try entering [192.168.4.1](http://192.168.4.1)). The page with walk you through the setup process. If the FERRY TEMPO can't connect to the provided network, check the network name and password, and try positioning your microcontroller closer to the access point you're connecting to. If you're still having trouble connecting, feel free to [post an issue on Github](https://github.com/pietroglyph/fow/issues/new).
## Flashing your microcontroller
The easiest way to get the code onto your ESP8266 (the code requires an ESP8266, but it doesn't really matter what's on the rest of the board, the NodeMCU DEVKIT 1.0, WeMos D1 Mini and regular, and Adafruit Feather Huzzah are all known to work) is to use [PlatformIO](https://platformio.org/). 

### Through PlatformIO
You should follow the below steps to deploy code using PlatformIO:
1. Download and install PlatformIO, either [through VSCode](https://platformio.org/platformio-ide), or [through your command line interface](https://docs.platformio.org/en/latest/installation.html#super-quick-mac-linux). Installing through Atom is not supported.
2. **IMPORTANT** If you're installing through VSCode, this will not work until a window pops up at the bottom of VSCode that says "Installing PlatformIO..." and _then_ says "Finished installing PlatformIO". You may have to restart VSCode to get this popup.
3. Open a terminal in your editor with the working directory in the same folder as this README (just do Ctrl+Shift+\` in VSCode). 
4. (If on a POSIX compliant system like macOS or Linux) Type in `./deploy.sh` (you should add ` -e stepper` at the end of that if you want to deploy to the kit stepper clock with dials). If it says something like "bash: platformio: command not found...", then type in `export PATH=$PATH:~/.platformio/penv/bin`, press enter, and run the script again.
4. (If on Windows) Type in `.\deploy.bat` (you should add ` -e dial-stepperclock` at the end of that if you want to deploy to the kit stepper clock with dials). If you have installed PlatformIO Core (the command line part) to a non-standard location then this command may fail. Deploying from Windows will bake in an "unknown" version number, so your device will probably contact the OTA server and update itself pretty quickly.

### Through the Arduino IDE
You can follow the below steps if you're using the Arduino IDE (not reccomended; this will only deploy for the servo clock unless you modify the code):
1. Download and install the Arduino IDE for your operating system from [here](https://www.arduino.cc/en/Main/Software) (the web editor wont work; if you have a package manager of some sort, you can also install using that).
2. Install the tooling for the ESP8266 by following the "Install with Boards Manager" section of the esp8266/Arduino README [here](https://github.com/esp8266/Arduino#installing-with-boards-manager). **The only supported `esp8266` version is 2.5.0! You must select this version when installing from Boards Manager.**
3. Install the `AccelStepper` library following [this](https://www.arduino.cc/en/Guide/Libraries#toc3) guide (using the library manager). The only supported version is 1.57.1. Other versions may or may not work, but they are unsupported and untested for this project.
4. In the Arduino IDE, go to `Tools > Board` and then select your board (under the ESP8266 modules section of the `Tools > Board` pullout).
5. Once you have set your board, set `Tools > lwIP Variant` to "v2 Higher Bandwidth". Other options will likely work, but are unsupported.
6. Optionally you can set your upload speed to 921600 baud under `Tools > Upload Speed` if you don't want to wait forever.
7. Press the `Upload` button (Looks like ->) in the top left hand corner of the Arduino IDE.


## Building OTA Update Files
The FERRY TEMPO can recieve over-the-air updates. Each device periodically checks for new updates in its hardware revision and update channel. If you want to deploy a new update you should run `platformio run -t upload -e HARDWARE NAME`, and `platformio run -t uploadfs -e HARDWARE NAME`. Don't plug in any hardware. Once these have run (it's fine if they fail because no hardware is connected), you should copy `.pio/build/HARDWARE NAME/spiffs.bin` and `.pio/build/HARDWARE NAME/firmware.bin` to another folder. Then, rename these files according to the following table:

| Original Filename | New Filename                                        |
|-------------------|-----------------------------------------------------|
| spiffs.bin        | VERSION@UPDATE_CHANNEL:HARDWARE_REVISION:spiffs.bin |
| firmware.bin      | VERSION@UPDATE_CHANNEL:HARDWARE_REVISION:flash.bin  |

`VERSION` should be the the inner string generated by running `print-version.py` (note that this script is currently GNU/Linux or macOS only), something like `v1.2.0-master` (if you haven't tagged your current commit, or you have uncommited changes, you should commit and make a release, otherwise this string will look like `68dd93cc4d5322fd55b76c084c8f2b8a447f41f5-master-dirty`).

`UPDATE_CHANNEL` should probably be `production`, unless you have a reason to use another update channel (note that you will have to modify a constant in `ConnectionManager.h` if you want a different update channel).

`HARDWARE_REVISION` should *not* be the same as the "hardware name" you used before. For example, if you ran `platformio -t upload -e kitty-v2`, then `kitty-v2` was your hardware name, but your hardware revision string is `dial-servoclock-v2`. You can find what the hardware revision for your hardware name is by looking in the platformio.ini file (for `kitty-v2` you would find `[env:kity-v2]`, and then find the next line starting with `-DHARDWARE_REVISION`).

Once you have renamed these files then you should put them in fow-server's "updates directory" (specified by the server's `--updates-dir` flag), and delete any past update files for the given hardware revision.


## Troubleshooting
### Device LED info
| Meaning                                                                                           | Left Dock Light | Right Dock Light | Ferry 1 Directional Lights | Ferry 2 Directional Lights |
|---------------------------------------------------------------------------------------------------|-----------------|------------------|----------------------------|----------------------------|
| Not connected to any network, no credentials are saved, currently in setup mode                   | Off             | Off              | Pulsing red                | Pulsing green              |
| Not connected to any network, but credentials are saved and the device is attempting to reconnect | Off             | Off              | Blinking green             | Blinking green             |
| Starting up and attempting to connect to a saved network                                          | Off             | Off              | Off                        | Off                        |
| Applying over-the-air update (**DO NOT TURN OFF YOUR DEVICE**)                                    | Off             | Blinking         | Off                        | Off                        |
### My device doesn't broadcast a setup WiFi network
If you cannot connect to the setup WiFi network, first ensure that you aren't already connected by pressing the reset button 3 times in quick succession (this will clear any saved credentials; refer to the device LED info table for details on non-destructive ways to check if your device has any credentials saved).

If you still cannot connect to the setup network (the name should look something like "FERRY TEMPO Setup OBCDEF"), try finding the network from a different device (e.g. if your phone doesn't show the network, try connecting with a laptop).

If you still cannot find the network, please [create a new issue](https://github.com/pietroglyph/fow/issues/new) with a description of your problem. (Note: if you are more technically inclined then you might try looking at the serial output before you open an issue).
### The setup page doesn't open
If the setup page doesn't open automatically on your device, and you've tried conecting to both [setup.ferrytempo.com](http://setup.ferrytempo.com) and [192.168.4.1](http://192.168.4.1), then you should make sure that you're connected to setup WiFi network and within range. Also make sure that no other devices are connected to the setup WiFi network (more than a few connected devices can overload the microcontroller).

If none of the above work then you should try connecting to the setup WiFi network from a different device and a different web browser. If that doesn't work then you should [create a new issue](https://github.com/pietroglyph/fow/issues/new) with a description of your problem.
### I can't connect my device to my home WiFi network
If your device can't see your home WiFi network, or it can see the network but can't connect to it then you should try moving your device closer to your WiFi access point or router (the WeMos D1 Mini microcontroller can have anywhere from 30 to ~125 feet of range depending on the power of your router and the obstacles surrounding it).

If none of the above options work then you can try connecting without a timeout:
 1. Connect to the setup WiFi network.
 2. Navigate to `http://192.168.1.4/connect?ssid=MY SSID&password=MY PASSWORD&notimeout=true` in your browser, replacing MY SSID and MY PASSWORD with your actual SSID (network name) and password, respectively. This request will *never* time out, so if your device really can't connect then you will need to reboot your FERRY TEMPO device. (Note that this will save your WiFi credentials in your browser history, so you may want to clear your browser history.)
 3. Periodically try opening `http://192.168.1.4/status` in your browser. If it doesn't load, then you are still trying to connect.
 4. If the status page says "Connected successfully", then open `http://192.168.1.4/exitsetup` in your browser to save your credentials and exit setup mode.

If you can't connect with the above procedure, then try connecting to a different WiFi network.

If you don't have a different WiFi network that you can use, then [create a new issue](https://github.com/pietroglyph/fow/issues/new) with a description of your problem.
### Other Issues/Reading Serial Output
If you open a GitHub issue, the first thing you will probably be asked to do is capture the device's serial output. If you are a more technically inclined user you might want to try this *before* posting an issue, as the output may provide some helpful troubleshooting tips (regardless of the type of issue). If that sounds like your situation, or if you're just curious about what your FERRY TEMPO is doing, then this is the section for you!

After you have connected your FERRY TEMPO to your computer over USB, try one of the following:
 * If you have the Arduino IDE, open the serial monitor (looks like a magnifying glass at the top right hand corner of the screen), and set the baud rate to `115200`.
 * If you have PlatformIO installed, run `platformio device monitor -b 115200` in the terminal (you may need to run `export PATH=$PATH:~/.platformio/penv/bin` to make this command work).

If you have none of the above installed, the easiest path is probably installing the Arduino IDE and using that.

If the above doesn't work, try a different USB cable (some cables are power only).
