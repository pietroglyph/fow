#include "SettingsManager.h"

SettingsManager::SettingsManager() {
  EEPROM.begin(eepromSize);
}

void SettingsManager::updateFullResetTimer() {

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
