/*
    Copyright (c) 2017, 2018 Declan Freeman-Gleason. All rights reserved.

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

#include "ServoClockOutputManager.h"

ServoClockOutputManager::ServoClockOutputManager() {
  // These magic numbers are the servo GPIO pins
  primaryServo.attach(13);
  secondaryServo.attach(12);
  primaryServo.write(servoMaxPosition/2);
  secondaryServo.write(servoMaxPosition/2);

  // These magic numbers are the pins for the lights (port, starboard, light intensity)
  primaryLights = new FerryHelper(5, 16, lightIntensity);
  secondaryLights = new FerryHelper(0, 4, lightIntensity);
  primaryLights->setupPins();
  secondaryLights->setupPins();
  
  pinMode(departingDockLightPin, OUTPUT);
  pinMode(arrivingDockLightPin, OUTPUT);
  analogWrite(departingDockLightPin, 0);
  analogWrite(arrivingDockLightPin, 0);

  primaryLights->setDirection(FerryHelper::Directions::STARBOARD);
  secondaryLights->setDirection(FerryHelper::Directions::PORT);

  updateLightMode(FerryHelper::Modes::DISCONNECTED);
}

void ServoClockOutputManager::calibrate() {
  primaryLights->update();
  secondaryLights->update();

  state = OutputManagerInterface::States::RUNNING; // We can't track servo progress, so we just go straight to RUNNING
}

void ServoClockOutputManager::update(std::function<double (int)> dataSupplier) {
  if (state != OutputManagerInterface::States::RUNNING) {
    calibrate();
    return;
  }

  int departingDockLightVal = 0;
  int arrivingDockLightVal = 0;

  // We know which index is which because these are always ordered the same by the server
  updateOutput(dataSupplier(0), &primaryServo, primaryLights, &departingDockLightVal, &arrivingDockLightVal);
  updateOutput(dataSupplier(1), &secondaryServo, secondaryLights, &departingDockLightVal, &arrivingDockLightVal);

  analogWrite(departingDockLightPin, departingDockLightVal * lightIntensity);
  analogWrite(arrivingDockLightPin, arrivingDockLightVal * lightIntensity);
}

void ServoClockOutputManager::updateOutput(double progress, Servo* servo, FerryHelper* lights, int* departingDockLightVal, int* arrivingDockLightVal) {
  servo->write(servoMaxPosition - (long)(progress * (double)(servoMaxPosition)));

  if (progress == 0 || progress == 1) {
    lights->setMode(FerryHelper::Modes::DOCKED);
    if (progress == 0) *arrivingDockLightVal = 1;
    else if (progress == 1) *departingDockLightVal = 1;
  }
  else lights->setMode(FerryHelper::Modes::RUNNING);

  lights->update();
}

void ServoClockOutputManager::updateLightMode(FerryHelper::Modes mode) {
  primaryLights->setMode(mode);
  secondaryLights->setMode(mode);
  primaryLights->update();
  secondaryLights->update();
}
