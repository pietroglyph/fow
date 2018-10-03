/*
    Copyright (c) 2017-2018 Declan Freeman-Gleason. All rights reserved.

    This file is part of Ferries Over Winslow.

    Ferries Over Winslow is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, eitherE version 3 of the License, or
    (at your option) any later version.

    Ferries Over Winslow is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this Ferries Over Winslow.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef StepperClockOutputManager_h
#define StepperClockOutputManager_h

#include <functional>
#include "FerryHelper.h"
#include "OutputManagerInterface.h"
#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>

class StepperClockOutputManager : public OutputManagerInterface {
  public:
    StepperClockOutputManager();

    void update(std::function<DataManager::FerryData (int)> dataSupplier);
    void calibrate();
  private:
    const int departingDockLightPin = 14;
    const int arrivingDockLightPin = 12;
    const int lightIntensity = 255;

    const int stepperMaxTicks = 812;
    const double stepperMaxSpeed = 100.0;
    const double stepperMaxAccel = 100.0;
    const unsigned long recalibrationOverdriveTime = 1000; // In milleseconds
    const uint8_t steppingMode = DOUBLE;

    void updateLightMode(FerryHelper::Modes mode);
    void updateOutput(DataManager::FerryData data, AccelStepper* stepper, Adafruit_StepperMotor* rawStepper, FerryHelper* lights, int* departingDockLightVal, int* arrivingDockLightVal, unsigned long* recalibrationTime);

    FerryHelper* primaryLights;
    FerryHelper* secondaryLights;
    OutputManagerInterface::States state = OutputManagerInterface::States::UNCALIBRATED;
    Adafruit_MotorShield motorShield;
    Adafruit_StepperMotor* primaryAdafruitStepper = NULL;
    Adafruit_StepperMotor* secondaryAdafruitStepper = NULL;
    AccelStepper* primaryStepper = NULL;
    AccelStepper* secondaryStepper = NULL;
    unsigned long primaryRecalibratedTime;
    unsigned long secondaryRecalibratedTime;
};

#endif
