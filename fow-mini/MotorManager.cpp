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

Adafruit_StepperMotor *primaryAdafruitStepper;
Adafruit_StepperMotor *secondaryAdafruitStepper;

void primaryForwardStep() {
  primaryAdafruitStepper->step(1, FORWARD, INTERLEAVE);
}

void primaryBackwardStep() {
  primaryAdafruitStepper->step(1, BACKWARD, INTERLEAVE);
}

void secondaryForwardStep() {
  // XXX: This behavior is undefined if secondary is null (which is easily possible)
  secondaryAdafruitStepper->step(1, FORWARD, INTERLEAVE);
}

void secondaryBackwardStep() {
  // XXX: This behavior is undefined if secondary is null (which is easily possible)
  secondaryAdafruitStepper->step(1, BACKWARD, INTERLEAVE);
}

MotorManager::MotorManager(Modes mode) : mode(mode) {
  Serial.begin(115200);
  delay(10);

  motorShield = Adafruit_MotorShield();
  motorShield.begin();

  switch (mode) {
    case Modes::SINGLE_TEST :
      primaryAdafruitStepper = motorShield.getStepper(300, 1);
      secondaryAdafruitStepper = 0; // null
      break;
    case Modes::DOUBLE_CLOCK :
      Serial.println("DOUBLE_CLOCK motor mode is unimplemented."); // TODO
      break;
    case Modes::DOUBLE_SLIDE :
      Serial.println("DOUBLE_SLIDE motor mode is unimplemented."); // TODO
      break;
  }

  primaryStepper = new AccelStepper(primaryForwardStep, primaryBackwardStep);
  secondaryStepper = new AccelStepper(secondaryForwardStep, secondaryBackwardStep);

  primaryStepper->setMaxSpeed(300.0);
  primaryStepper->setAcceleration(100.0);

  secondaryStepper->setMaxSpeed(300.0);
  secondaryStepper->setAcceleration(100.0);

  // Zero the steppers by making them run to their end range
  primaryStepper->moveTo(1000000); // This appears to be the max for INTERLEAVED steppers
  secondaryStepper->moveTo(1000000);
}

void MotorManager::update() {
  primaryStepper->run();
  secondaryStepper->run();
}


