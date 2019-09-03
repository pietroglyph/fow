/*
    Copyright (c) 2017-2019 Declan Freeman-Gleason.

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

#if defined(IS_SERVO_CLOCK) || !defined(IS_STEPPER_CLOCK)

#include "ServoClockOutputManager.h"

ServoClockOutputManager::ServoClockOutputManager(int servMaxPosition, int servMinPosition, int redIntensity, int greenIntensity) :
  servoMaxPosition(servMaxPosition), servoMinPosition(servMinPosition),
  // Below magic numbers are the light pins
  primaryLights(LightHelper(5, 14, redIntensity, greenIntensity)),
  secondaryLights(LightHelper(4, 12, redIntensity, greenIntensity)) {

  // These magic numbers are the servo GPIO pins
  primaryServo.attach(13);
  secondaryServo.attach(0);

  primaryLights.setupPins();
  secondaryLights.setupPins();

  pinMode(departingDockLightPin, OUTPUT);
  pinMode(arrivingDockLightPin, OUTPUT);
  analogWrite(departingDockLightPin, 0);
  analogWrite(arrivingDockLightPin, 0);

  primaryLights.setDirection(LightHelper::Directions::ARRIVING);
  secondaryLights.setDirection(LightHelper::Directions::DEPARTING);

  updateLightMode(LightHelper::Modes::DISCONNECTED);

  calibrationStartTime = millis();
}

void ServoClockOutputManager::calibrate() {
  updateLightMode(LightHelper::Modes::DISCONNECTED);

  int intensity = LightHelper::getPulsingIntensity(dockLightIntensity);
  analogWrite(departingDockLightPin, intensity);
  analogWrite(arrivingDockLightPin, intensity);

  double calibrationSegment = static_cast<double>(millis() - calibrationStartTime) / calibrationHoldTime;
  double pos = 0.5;
  if (calibrationSegment <= 1)
    pos = 0.25;
  else if (calibrationSegment <= 2)
    pos = 0.75;

  primaryServo.write(pos);
  secondaryServo.write(pos);

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
  updateOutput(dataSupplier(0), primaryServo, primaryLights, departingDockLightVal, arrivingDockLightVal);
  updateOutput(dataSupplier(1), secondaryServo, secondaryLights, departingDockLightVal, arrivingDockLightVal);

  analogWrite(departingDockLightPin, departingDockLightVal * dockLightIntensity);
  analogWrite(arrivingDockLightPin, arrivingDockLightVal * dockLightIntensity);
}

void ServoClockOutputManager::updateOutput(const DataManager::FerryData &data, PercentageServo &servo, LightHelper &lights, int &departingDockLightVal, int &arrivingDockLightVal) {
  if (data.progress == 0 || data.progress == 1) {
    lights.setMode(LightHelper::Modes::DOCKED);
    if (data.progress == 0) arrivingDockLightVal = 1;
    else if (data.progress == 1) departingDockLightVal = 1;
    
    digitalWrite(servo.getPin(), LOW);
  }
  else {
    lights.setMode(LightHelper::Modes::RUNNING);
    servo.write(1 - data.progress);
  }

  lights.setDirection(data.direction);
  lights.update();
}

void ServoClockOutputManager::updateLightMode(LightHelper::Modes mode) {
  primaryLights.setMode(mode);
  secondaryLights.setMode(mode);
  primaryLights.update();
  secondaryLights.update();
}

#endif