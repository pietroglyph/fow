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

// This is weird, we should probably just put it into its own class
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
    l_backwardPrimary();
  }
  void backwardSecondaryWrapper() {
    l_backwardPrimary();
  }
}
}

MotorManager::MotorManager(Modes mode) : mode(mode) {
  using namespace Wrapper;

  Serial.begin(115200);
  delay(10);

  motorShield = Adafruit_MotorShield();
  motorShield.begin();

  primaryAdafruitStepper = motorShield.getStepper(513, 1);
  secondaryAdafruitStepper = motorShield.getStepper(513, 3);

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

  primaryStepper->setMaxSpeed(300.0);
  primaryStepper->setAcceleration(100.0);

  secondaryStepper->setMaxSpeed(300.0);
  secondaryStepper->setAcceleration(100.0);

  // Zero the steppers by making them run to their end range
  primaryStepper->moveTo(513);
  secondaryStepper->moveTo(513);
}

void MotorManager::update() {
  switch (mode) {
    case Modes::SINGLE_TEST :
      if (primaryStepper->distanceToGo() == 0)
        primaryStepper->moveTo(-primaryStepper->currentPosition());
      break;
    case Modes::DOUBLE_CLOCK :
      Serial.println("DOUBLE_CLOCK motor mode is unimplemented."); // TODO
      break;
    case Modes::DOUBLE_SLIDE :
      Serial.println("DOUBLE_SLIDE motor mode is unimplemented."); // TODO
      break;
  }

  primaryStepper->run();
  secondaryStepper->run();
}
