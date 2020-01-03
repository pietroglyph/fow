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

ServoClockOutputManager::ServoClockOutputManager(const int servMaxPosition, const int servMinPosition, const int redIntensity, const int greenIntensity, const std::tuple<bool, bool> servosReversed) :
  servoMaxPosition(servMaxPosition), servoMinPosition(servMinPosition),
  // Below magic numbers are the light pins
  primaryLights(LightHelper(5, 14, redIntensity, greenIntensity)),
  secondaryLights(LightHelper(4, 12, redIntensity, greenIntensity)),
  servosReversed(servosReversed) {

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
  std::tuple<double, double> intensities{0, 0};
  std::tuple<double, double> positions{0.5, 0.5};

  double calibrationSegment = static_cast<double>(millis() - calibrationStartTime) / calibrationHoldTime;
  if (calibrationSegment <= 1) {
    std::get<0>(positions) = std::get<0>(servosReversed) ? 0 : 1;
    std::get<1>(intensities) = dockLightIntensity;
    primaryLights.setDirection(LightHelper::Directions::ARRIVING);
  } else if (calibrationSegment <= 2) {
    std::get<0>(positions) = std::get<0>(servosReversed) ? 1 : 0;
    std::get<0>(intensities) = dockLightIntensity;
    primaryLights.setDirection(LightHelper::Directions::DEPARTING);
  } else if (calibrationSegment <= 3) {
    // Let everything come back home
    updateLightMode(LightHelper::Modes::OFF);
  } else if (calibrationSegment <= 4) {
    std::get<1>(positions) = std::get<1>(servosReversed) ? 0 : 1;
    std::get<1>(intensities) = dockLightIntensity;
    secondaryLights.setDirection(LightHelper::Directions::ARRIVING);
  } else if (calibrationSegment <= 5) {
    std::get<1>(positions) = std::get<1>(servosReversed) ? 1 : 0;
    std::get<0>(intensities) = dockLightIntensity;
    secondaryLights.setDirection(LightHelper::Directions::DEPARTING);
  } else {
    double intensity = LightHelper::getPulsingIntensity(dockLightIntensity);
    std::get<1>(intensities) = intensity;
    std::get<0>(intensities) = intensity;
    updateLightMode(LightHelper::Modes::DISCONNECTED);
  }

  if (calibrationSegment <= 2) {
    primaryLights.setMode(LightHelper::Modes::SELF_TEST);
    secondaryLights.setMode(LightHelper::Modes::OFF);
    primaryLights.update();
    secondaryLights.update();
  } else if (calibrationSegment > 3 && calibrationSegment <= 5) {
    primaryLights.setMode(LightHelper::Modes::OFF);
    secondaryLights.setMode(LightHelper::Modes::SELF_TEST);
    primaryLights.update();
    secondaryLights.update();
  }

  primaryServo.write(std::get<0>(positions));
  secondaryServo.write(std::get<1>(positions));

  analogWrite(departingDockLightPin, std::get<0>(intensities));
  analogWrite(arrivingDockLightPin, std::get<1>(intensities));

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
  updateOutput(dataSupplier(0), primaryServo, primaryLights, departingDockLightVal, arrivingDockLightVal, std::get<0>(servosReversed));
  updateOutput(dataSupplier(1), secondaryServo, secondaryLights, departingDockLightVal, arrivingDockLightVal, std::get<1>(servosReversed));

  analogWrite(departingDockLightPin, departingDockLightVal * dockLightIntensity);
  analogWrite(arrivingDockLightPin, arrivingDockLightVal * dockLightIntensity);
}

void ServoClockOutputManager::updateOutput(const DataManager::FerryData &data, PercentageServo &servo, LightHelper &lights, int &departingDockLightVal, int &arrivingDockLightVal, const bool servoReversed) {  
  auto getOutputPercentage = [&servoReversed, &data] {
    return servoReversed ? data.progress : 1 - data.progress;
  };

  if (!data.isValid) {
    lights.setMode(LightHelper::Modes::OFF);
    digitalWrite(servo.getPin(), LOW);
  } else if (data.progress == 0 || data.progress == 1) {
    lights.setMode(LightHelper::Modes::DOCKED);
    if (data.progress == 0) arrivingDockLightVal = 1;
    else if (data.progress == 1) departingDockLightVal = 1;
    
    if (data.isNew) {
      servo.write(getOutputPercentage());
    } else {
      digitalWrite(servo.getPin(), LOW);
    }
  } else {
    lights.setMode(LightHelper::Modes::RUNNING);
    if (!PREVENT_BUZZING || data.isNew) {
      servo.write(getOutputPercentage());
    } else {
      digitalWrite(servo.getPin(), LOW);
    }
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