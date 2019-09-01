/*`
    Copyright (c) 2017, 2018 Declan Freeman-Gleason.

    This file is part of Ferries Over Winslow.

    Ferries Over Winslow is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Ferries Over Winslow is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this Ferries Over Winslow.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SettingsManager_h
#define SettingsManager_h

#include <Arduino.h>
#include <EEPROM.h>

class SettingsManager {
  public:
    enum class Setting {
      SSID,
      PASSWORD
    };

    SettingsManager();

    String getSetting(Setting index);
    void setSetting(Setting index, String value);
    bool isInSetupMode();
    void exitSetupMode(); // There is no way to enter setup mode because that only happens on a full reset, which is handled internally
    void updateFullResetTimer();

    const unsigned int maximumSettingLength = 128; // In bytes
  private:
    // maximum amount of time allowed between reset button presses for a full reset to occur, in milleseconds
    const unsigned long fullResetButtonPressDelay = 3000;
    const int pressesForFullReset = 3; // Number of presses to start a full reset
    const int eepromSize = 512; // In bytes
    const int flagByteAddress = 0;

    bool hasTurnedOnResetBit = false;
    bool hasTurnedOffResetBit = false;
    byte originalFlagByte;

    int getAddressForSetting(Setting setting) const {
      // We add 1 because the first byte of the EEPROM holds the reset timer flags and the setup mode flag
      return static_cast<int>(setting) * maximumSettingLength + 1;
    };
};

#endif
