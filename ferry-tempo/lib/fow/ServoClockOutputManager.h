/*
    Copyright (c) 2017-2018 Declan Freeman-Gleason. All rights reserved.

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

#ifndef ServoClockOutputManager_h
#define ServoClockOutputManager_h

#include <functional>
#include "LightHelper.h"
#include "OutputManagerInterface.h"
#include <Wire.h>
#include "PercentageServo.h"

class ServoClockOutputManager : public OutputManagerInterface {
  public:
    ServoClockOutputManager(const int servMaxPosition, const int servMinPosition, const int redIntensity, const int greenIntensity, const std::tuple<bool, bool> servosReversed);

    void update(std::function<DataManager::FerryData (int)> dataSupplier);
    void calibrate();
  private:
    const int servoMaxPosition;
    const int servoMinPosition;
    const std::tuple<bool, bool> servosReversed;
    static constexpr int dockLightIntensity = 255;
    static constexpr int departingDockLightPin = 2;
    static constexpr int arrivingDockLightPin = 15;
    static constexpr double calibrationHoldTime = 2000; // Milliseconds
    
    unsigned long calibrationStartTime = 0;

    void updateLightMode(LightHelper::Modes mode);
    void updateOutput(const DataManager::FerryData &data, PercentageServo &servo, LightHelper &lights, int &departingDockLightVal, int &arrivingDockLightVal, const bool servoReversed);

    PercentageServo primaryServo = PercentageServo(servoMinPosition, servoMaxPosition, false);
    PercentageServo secondaryServo = PercentageServo(servoMinPosition, servoMaxPosition, false);
    LightHelper primaryLights;
    LightHelper secondaryLights;
    OutputManagerInterface::States state = OutputManagerInterface::States::UNCALIBRATED;
};

#endif

#endif
