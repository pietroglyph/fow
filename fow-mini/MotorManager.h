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

#ifndef MotorManager_h
#define MotorManager_h

#include <functional>
#include "LightManager.h"
#include "DataManager.h"
#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>

class MotorManager {
  public:
    enum class Modes {
      DOUBLE_CLOCK,
      DOUBLE_SLIDE,
      SINGLE_TEST_PRI,
      SINGLE_TEST_SEC
    };

    MotorManager(Modes mode, DataManager* data, LightManager* lights);

    void update();
    void calibrate();
    void setMode(Modes mode);
  private:
    enum class States {
      UNCALIBRATED,
      CALIBRATING,
      RUNNING
    };

    const int k_stepperMaxTicks = 812;
    const double k_stepperMaxSpeed = 100.0;
    const double k_stepperMaxAccel = 100.0;

    FerryLights* departingLights;
    FerryLights* arrivingLights;
    States state = States::UNCALIBRATED;
    Modes mode;
    DataManager* data;
    LightManager* lights;
    Adafruit_MotorShield motorShield;
    Adafruit_StepperMotor *primaryAdafruitStepper = NULL;
    Adafruit_StepperMotor *secondaryAdafruitStepper = NULL;
    AccelStepper *primaryStepper = NULL;
    AccelStepper *secondaryStepper = NULL;
};

#endif
