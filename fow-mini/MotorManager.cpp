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

#include "MotorManager.h"

/*
   This is not an idomatic use of namespaces;
   FIXME?
*/
namespace Wrapper {
std::function<void(void)> l_forwardPrimary;
std::function<void(void)> l_backwardPrimary;
std::function<void(void)> l_forwardSecondary;
std::function<void(void)> l_backwardSecondary;

void setMotors(std::function<void(void)> fp, std::function<void(void)> bp, std::function<void(void)> fs, std::function<void(void)> bs) {
  l_forwardPrimary = fp;
  l_backwardPrimary = bp;
  l_forwardSecondary = fs;
  l_backwardSecondary = bs;
}

extern "C" {
  void forwardPrimaryWrapper() {
    l_forwardPrimary();
  }
  void backwardPrimaryWrapper() {
    l_backwardPrimary();
  }
  void forwardSecondaryWrapper() {
    l_forwardSecondary();
  }
  void backwardSecondaryWrapper() {
    l_backwardSecondary();
  }
}
}

MotorManager::MotorManager(Modes mode, DataManager* dataManager, LightManager* lightManager) : mode(mode), data(dataManager), lights(lightManager) {
  using namespace Wrapper;

  Serial.begin(115200);

  motorShield = Adafruit_MotorShield();
  motorShield.begin();

  primaryAdafruitStepper = motorShield.getStepper(k_stepperMaxTicks * 2, 1);
  secondaryAdafruitStepper = motorShield.getStepper(k_stepperMaxTicks * 2, 2);

  std::function<void(void)> l_forwardPrimary = [&] {
    primaryAdafruitStepper->step(1, FORWARD, DOUBLE);
  };
  std::function<void(void)> l_backwardPrimary = [&] {
    primaryAdafruitStepper->step(1, BACKWARD, DOUBLE);
  };

  std::function<void(void)> l_forwardSecondary = [&] {
    secondaryAdafruitStepper->step(1, FORWARD, DOUBLE);
  };
  std::function<void(void)> l_backwardSecondary = [&] {
    secondaryAdafruitStepper->step(1, BACKWARD, DOUBLE);
  };

  setMotors(l_forwardPrimary, l_backwardPrimary, l_forwardSecondary, l_backwardSecondary);

  primaryStepper = new AccelStepper(forwardPrimaryWrapper, backwardPrimaryWrapper);
  secondaryStepper = new AccelStepper(forwardSecondaryWrapper, backwardSecondaryWrapper);

  primaryStepper->setMaxSpeed(k_stepperMaxSpeed);
  primaryStepper->setAcceleration(k_stepperMaxAccel);

  secondaryStepper->setMaxSpeed(k_stepperMaxSpeed);
  secondaryStepper->setAcceleration(k_stepperMaxAccel);

  // These magic numbers are the pins for the lights
  departingLights = new FerryLights(14, 13, 15);
  arrivingLights = new FerryLights(12, 0, 16);

  lights->registerFerry(departingLights);
  lights->registerFerry(arrivingLights);

  lights->setupPins();

  departingLights->setMode(FerryLights::Modes::DISCONNECTED);
  arrivingLights->setMode(FerryLights::Modes::DISCONNECTED);

  departingLights->setDirection(FerryLights::Directions::PORT);
  arrivingLights->setDirection(FerryLights::Directions::STARBOARD);
}

void MotorManager::calibrate() {
  primaryStepper->run();
  secondaryStepper->run();

  if (state == States::UNCALIBRATED) {
    primaryStepper->moveTo(k_stepperMaxTicks);
    secondaryStepper->moveTo(k_stepperMaxTicks);
    state = States::CALIBRATING;
    return;
  } else if (state == States::CALIBRATING) {
    if (primaryStepper->distanceToGo() == 0 &&
        secondaryStepper->distanceToGo() == 0) {
      Serial.println("Calibration finished.");
      primaryStepper->setCurrentPosition(0);
      secondaryStepper->setCurrentPosition(0);
      state = States::RUNNING;
    }
  }
}

void MotorManager::update() {
  primaryStepper->run();
  secondaryStepper->run();

  switch (mode) {
    case Modes::SINGLE_TEST_PRI :
      if (primaryStepper->distanceToGo() == 0)
        primaryStepper->moveTo(-primaryStepper->currentPosition());
      break;
    case Modes::SINGLE_TEST_SEC :
      if (secondaryStepper->distanceToGo() == 0)
        secondaryStepper->moveTo(-secondaryStepper->currentPosition());
      break;
    case Modes::DOUBLE_CLOCK :
      {
        double departingProgress = data->getProgress(0); // We know which index is which because these are always ordered the same by the server
        double arrivingProgress = data->getProgress(1);
        long departingProgressTicks = -1 * (long)(departingProgress * k_stepperMaxTicks);
        long arrivingProgressTicks = -1 * (long)(arrivingProgress * k_stepperMaxTicks);
        if (primaryStepper->targetPosition() != departingProgressTicks) {
 
          primaryStepper->moveTo(departingProgressTicks);
          if (departingProgress == 0 || departingProgress == 1)
            departingLights->setMode(FerryLights::Modes::DOCKED);
          else
            departingLights->setMode(FerryLights::Modes::RUNNING);
        }
        if (secondaryStepper->targetPosition() != arrivingProgressTicks) {
          secondaryStepper->moveTo(arrivingProgressTicks);
          if (arrivingProgress == 0 || arrivingProgress == 1)
            arrivingLights->setMode(FerryLights::Modes::DOCKED);
          else
            arrivingLights->setMode(FerryLights::Modes::RUNNING);
        }
        break;
      }
    case Modes::DOUBLE_SLIDE :
      Serial.println("DOUBLE_SLIDE motor mode is unimplemented.");
      break;
  }
}
