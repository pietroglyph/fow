/*
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

#ifdef IS_SERVO_CLOCK

#include "ServoClockOutputManager.h"

ServoClockOutputManager::ServoClockOutputManager() {
  // These magic numbers are the servo GPIO pins
  primaryServo.attach(13);
  secondaryServo.attach(0);
  primaryServo.write(0.5);
  secondaryServo.write(0.5);

  // These magic numbers are the pins for the lights (departing, arriving, light intensity)
  primaryLights = new FerryHelper(5, 14, lightIntensity);
  secondaryLights = new FerryHelper(4, 12, lightIntensity);
  primaryLights->setupPins();
  secondaryLights->setupPins();

  pinMode(departingDockLightPin, OUTPUT);
  pinMode(arrivingDockLightPin, OUTPUT);
  analogWrite(departingDockLightPin, 0);
  analogWrite(arrivingDockLightPin, 0);

  primaryLights->setDirection(FerryHelper::Directions::ARRIVING);
  secondaryLights->setDirection(FerryHelper::Directions::DEPARTING);

  updateLightMode(FerryHelper::Modes::DISCONNECTED);
}

void ServoClockOutputManager::calibrate() {
  updateLightMode(FerryHelper::Modes::DISCONNECTED);

  state = OutputManagerInterface::States::RUNNING; // We can't track servo progress, so we just go straight to RUNNING
}

void ServoClockOutputManager::update(std::function<DataManager::FerryData (int)> dataSupplier) {
  if (state != OutputManagerInterface::States::RUNNING) {
    calibrate();
    return;
  }

  int departingDockLightVal = 0;
  int arrivingDockLightVal = 0;

  // We know which index is which because these are always ordered the same by the server
  updateOutput(dataSupplier(0), &primaryServo, primaryLights, &departingDockLightVal, &arrivingDockLightVal);
  updateOutput(dataSupplier(1), &secondaryServo, secondaryLights, &departingDockLightVal, &arrivingDockLightVal);

  analogWrite(departingDockLightPin, departingDockLightVal * dockLightIntensity);
  analogWrite(arrivingDockLightPin, arrivingDockLightVal * dockLightIntensity);
}

void ServoClockOutputManager::updateOutput(DataManager::FerryData data, PercentageServo* servo, FerryHelper* lights, int* departingDockLightVal, int* arrivingDockLightVal) {
  servo->write(1 - data.progress);

  if (data.progress == 0 || data.progress == 1) {
    lights->setMode(FerryHelper::Modes::DOCKED);
    if (data.progress == 0) *arrivingDockLightVal = 1;
    else if (data.progress == 1) *departingDockLightVal = 1;
  }
  else lights->setMode(FerryHelper::Modes::RUNNING);

  lights->setDirection(data.direction);
  lights->update();
}

void ServoClockOutputManager::updateLightMode(FerryHelper::Modes mode) {
  primaryLights->setMode(mode);
  secondaryLights->setMode(mode);
  primaryLights->update();
  secondaryLights->update();
}

#endif