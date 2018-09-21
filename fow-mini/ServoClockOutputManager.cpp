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

  // These magic numbers are the pins for the lights (dock, starboard, port)
  primaryLights = new LightHelper(2, 5, 16);
  secondaryLights = new LightHelper(14, 0, 4);

  primaryLights->setupPins();
  secondaryLights->setupPins();

  updateLightMode(LightHelper::Modes::DISCONNECTED);

  primaryLights->setDirection(LightHelper::Directions::PORT);
  secondaryLights->setDirection(LightHelper::Directions::STARBOARD);
}

void ServoClockOutputManager::calibrate() {
  primaryServo.write(servoMaxPosition);
  secondaryServo.write(servoMaxPosition);
  state = OutputManagerInterface::States::RUNNING; // We can't track servo progress, so we just go straight to RUNNING
}

void ServoClockOutputManager::update(std::function<double (int)> dataSupplier) {
  if (state != OutputManagerInterface::States::RUNNING) {
    calibrate();
    return;
  }

  double primaryProgress = dataSupplier(0); // We know which index is which because these are always ordered the same by the server
  double secondaryProgress = dataSupplier(1);
  updateOutput(primaryProgress, &primaryServo, primaryLights);
  updateOutput(secondaryProgress, &secondaryServo, secondaryLights);
}

void ServoClockOutputManager::updateOutput(double progress, Servo* servo, LightHelper* lights) {
  servo->write((long)(progress * (double)(servoMaxPosition)));

  if (progress == 0 || progress == 1) lights->setMode(LightHelper::Modes::DOCKED);
  else lights->setMode(LightHelper::Modes::RUNNING);

  lights->update();
}

void ServoClockOutputManager::updateLightMode(LightHelper::Modes mode) {
  primaryLights->setMode(mode);
  secondaryLights->setMode(mode);
  primaryLights->update();
  secondaryLights->update();
}
