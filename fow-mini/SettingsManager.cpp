/*
    Copyright (c) 2017 Declan Freeman-Gleason. All rights reserved.

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

#include "SettingsManager.h"

SettingsManager::SettingsManager() {
  EEPROM.begin(eepromSize);
}

void SettingsManager::updateFullResetTimer() {
  if (millis() < fullResetButtonPressDelay && !hasTurnedOnResetBit) {
    hasTurnedOnResetBit = true;

    Serial.println("Checking reset bits...");
    
    byte b = EEPROM.read(flagByteAddress);
    originalFlagByte = b;
    b = (b>>1) | b; // Add a 1 after the last bit that is 1, or the second most significant bit, whichever comes first
    if (b == 0x00) b = 0x40; // Special case if b initially equals 0x00

    // Check how many ones there are after the most significant bit; we reset if there are as many ones as there are presses for a full reset
    if ((b | 0x80) >= static_cast<byte>(0xFF<<(7-pressesForFullReset))) {
      Serial.println("Full reset triggered. Clearing flag bit and restarting...");
      EEPROM.write(flagByteAddress, 0x00);
      EEPROM.commit();
      ESP.restart();
      hasTurnedOffResetBit = true;
      return;
    }
    
    EEPROM.write(flagByteAddress, b);
    EEPROM.commit();
  } else if (millis() > fullResetButtonPressDelay && !hasTurnedOffResetBit) {
    hasTurnedOffResetBit = true;
    
    EEPROM.write(flagByteAddress, originalFlagByte);
    EEPROM.commit();
  }
}

String SettingsManager::getSetting(Setting setting) {
  String resultBuf;
  int startAddress = getAddressForSetting(setting);
  for (int i = 0; i < maximumSettingLength; i++) {
    byte b = EEPROM.read(startAddress + i);
    if (b == '\0') break;
    else resultBuf += b;
  }
  return resultBuf; // The String class adds the NUL character at the end, so we don't have to worry about it
}

void SettingsManager::setSetting(Setting setting, String value) {
  int startAddress = getAddressForSetting(setting);
  int lastIndex;
  for (int i = 0; i < value.length() + 1 && i < maximumSettingLength; i++) { // We add 1 to value.length() to get the NUL character at the end
    EEPROM.write(startAddress + i, value.charAt(i));
  }
  EEPROM.commit();
}

bool SettingsManager::isInSetupMode() {
  if ((EEPROM.read(flagByteAddress) & 0x80) == 0x80) return false; // Mask off everything but the first bit and check if it's 1
  else return true;
}

void SettingsManager::exitSetupMode() {
  byte flagByte = EEPROM.read(flagByteAddress);
  EEPROM.write(flagByteAddress, flagByte | 0x80); // Set the first bit to 1
  EEPROM.commit();
}

int SettingsManager::getAddressForSetting(Setting setting) {
  return static_cast<int>(setting) * maximumSettingLength + 1; // We add 1 because the first byte of the EEPROM holds the reset timer flags and the setup mode flag
}
