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
 * We use this namespace to get timing right...
 * This is not an idomatic use of namespaces;
 * FIXME?
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
    l_backwardSecondary();
  }
  void backwardSecondaryWrapper() {
    l_backwardSecondary();
  }
}
}

MotorManager::MotorManager(Modes mode, DataManager* data) : mode(mode), data(data) {
  using namespace Wrapper;

  Serial.begin(115200);
  delay(10);

  motorShield = Adafruit_MotorShield();
  motorShield.begin();

  primaryAdafruitStepper = motorShield.getStepper(k_stepperMaxTicks, 1);
  secondaryAdafruitStepper = motorShield.getStepper(k_stepperMaxTicks, 2);

  std::function<void(void)> l_forwardPrimary = [&] {
    if (primaryAdafruitStepper) primaryAdafruitStepper->step(1, FORWARD, SINGLE);
  };
  std::function<void(void)> l_backwardPrimary = [&] {
    if (primaryAdafruitStepper) primaryAdafruitStepper->step(1, BACKWARD, SINGLE);
  };

  std::function<void(void)> l_forwardSecondary = [&] {
    if (secondaryAdafruitStepper) secondaryAdafruitStepper->step(1, FORWARD, SINGLE);
  };
  std::function<void(void)> l_backwardSecondary = [&] {
    if (secondaryAdafruitStepper) secondaryAdafruitStepper->step(1, BACKWARD, SINGLE);
  };

  setMotors(l_forwardPrimary, l_backwardPrimary, l_forwardSecondary, l_backwardSecondary);

  primaryStepper = new AccelStepper(forwardPrimaryWrapper, backwardPrimaryWrapper);
  secondaryStepper = new AccelStepper(forwardSecondaryWrapper, backwardSecondaryWrapper);

  primaryStepper->setMaxSpeed(k_stepperMaxSpeed);
  primaryStepper->setAcceleration(k_stepperMaxAccel);

  secondaryStepper->setMaxSpeed(k_stepperMaxSpeed);
  secondaryStepper->setAcceleration(k_stepperMaxAccel);

  // Zero the steppers by making them run to their end range
  primaryStepper->moveTo(k_stepperMaxTicks);
  secondaryStepper->moveTo(k_stepperMaxTicks);
}

void MotorManager::update() {
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
      Serial.println("DOUBLE_CLOCK motor mode is unimplemented."); // TODO
      break;
    case Modes::DOUBLE_SLIDE :
      long departingProgressTicks = (long)(data->getProgress(0) * k_stepperMaxTicks);
      long arrivingProgressTicks = (long)(data->getProgress(1) * k_stepperMaxTicks);
      if (primaryStepper->targetPosition() != departingProgressTicks)
        primaryStepper->moveTo(departingProgressTicks);
      if (secondaryStepper->targetPosition() != arrivingProgressTicks)
        secondaryStepper->moveTo(arrivingProgressTicks);
      break;
  }

  primaryStepper->run();
  secondaryStepper->run();
}
