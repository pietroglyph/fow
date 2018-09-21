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

#include "StepperClockOutputManager.h"

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

StepperClockOutputManager::StepperClockOutputManager() {
  using namespace Wrapper;

  motorShield = Adafruit_MotorShield();
  motorShield.begin();

  primaryAdafruitStepper = motorShield.getStepper(stepperMaxTicks * 2, 1);
  secondaryAdafruitStepper = motorShield.getStepper(stepperMaxTicks * 2, 2);

  /*
     Using onestep instead of step here is very important, because step blocks,
     while onestep doesn't. step will sometimes block for a very long time,
     because of other seemingly random timing minutae, causes a soft watchdog
     timer reset.
  */
  std::function<void(void)> l_forwardPrimary = [&] {
    primaryAdafruitStepper->onestep(FORWARD, steppingMode);
  };
  std::function<void(void)> l_backwardPrimary = [&] {
    primaryAdafruitStepper->onestep(BACKWARD, steppingMode);
  };

  std::function<void(void)> l_forwardSecondary = [&] {
    secondaryAdafruitStepper->onestep(FORWARD, steppingMode);
  };
  std::function<void(void)> l_backwardSecondary = [&] {
    secondaryAdafruitStepper->onestep(BACKWARD, steppingMode);
  };

  setMotors(l_forwardPrimary, l_backwardPrimary, l_forwardSecondary, l_backwardSecondary);

  primaryStepper = new AccelStepper(forwardPrimaryWrapper, backwardPrimaryWrapper);
  secondaryStepper = new AccelStepper(forwardSecondaryWrapper, backwardSecondaryWrapper);

  primaryStepper->setMaxSpeed(stepperMaxSpeed);
  primaryStepper->setAcceleration(stepperMaxAccel);

  secondaryStepper->setMaxSpeed(stepperMaxSpeed);
  secondaryStepper->setAcceleration(stepperMaxAccel);

  // These magic numbers are the pins for the lights
  primaryLights = new LightHelper(14, 13, 15);
  secondaryLights = new LightHelper(12, 0, 16);

  primaryLights->setupPins();
  secondaryLights->setupPins();

  updateLightMode(LightHelper::Modes::DISCONNECTED);

  primaryLights->setDirection(LightHelper::Directions::PORT);
  secondaryLights->setDirection(LightHelper::Directions::STARBOARD);
}

void StepperClockOutputManager::calibrate() {
  primaryStepper->run();
  secondaryStepper->run();

  updateLightMode(LightHelper::Modes::DISCONNECTED);

  if (state == OutputManagerInterface::States::UNCALIBRATED) {
    primaryStepper->moveTo(stepperMaxTicks);
    secondaryStepper->moveTo(stepperMaxTicks);
    state = OutputManagerInterface::States::CALIBRATING;
  } else if (state == OutputManagerInterface::States::CALIBRATING) {
    if (!primaryStepper->isRunning() && !secondaryStepper->isRunning()) {
      Serial.println("Calibration finished.");
      primaryStepper->setCurrentPosition(0);
      secondaryStepper->setCurrentPosition(0);
      state = OutputManagerInterface::States::RUNNING;
      // This fixes a bug where we get 0 as an actual ferry position, and don't set the light mode as a result
      updateLightMode(LightHelper::Modes::RUNNING);
    }
  }
}

void StepperClockOutputManager::update(std::function<double (int)> dataSupplier) {
  if (state != OutputManagerInterface::States::RUNNING) {
    calibrate();
    return;
  }

  double primaryProgress = dataSupplier(0); // We know which index is which because these are always ordered the same by the server
  double secondaryProgress = dataSupplier(1);
  updateOutput(primaryProgress, primaryStepper, primaryAdafruitStepper, primaryLights, &primaryRecalibratedTime);
  updateOutput(secondaryProgress, secondaryStepper, secondaryAdafruitStepper, secondaryLights, &secondaryRecalibratedTime);
}

void StepperClockOutputManager::updateOutput(double progress, AccelStepper* stepper, Adafruit_StepperMotor* rawStepper, LightHelper* lights, unsigned long* recalibrationTime) {
  stepper->run();

  long progressTicks = -1 * (long)(progress * stepperMaxTicks);
  if (stepper->targetPosition() != progressTicks) {
    if (progress == 0 || progress == 1) lights->setMode(LightHelper::Modes::DOCKED);
    else lights->setMode(LightHelper::Modes::RUNNING);
    stepper->moveTo(progressTicks);
  }
  // We totally bypass AccelStepper here because it's much simpler to use the underlying stepper to recalibrate when we dock
  if (millis() - *recalibrationTime <= recalibrationOverdriveTime && (progress == 0 || progress == 1) && !stepper->isRunning())
    rawStepper->onestep(progress == 0 ? FORWARD : BACKWARD, steppingMode);
  else if (stepper->isRunning() && (progress == 0 || progress == 1)) *recalibrationTime = millis();

  lights->update();
}

void StepperClockOutputManager::updateLightMode(LightHelper::Modes mode) {
  primaryLights->setMode(mode);
  secondaryLights->setMode(mode);
  primaryLights->update();
  secondaryLights->update();
}
