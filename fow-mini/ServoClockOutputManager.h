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

#ifndef ServoClockOutputManager_h
#define ServoClockOutputManager_h

#include <functional>
#include "LightHelper.h"
#include "OutputManagerInterface.h"
#include <Wire.h>
#include <Servo.h>

class ServoClockOutputManager : public OutputManagerInterface {
  public:
    ServoClockOutputManager();

    void update(std::function<double (int)> dataSupplier);
    void calibrate();
  private:
    const int servoMaxPosition = 170;

    void updateLightMode(LightHelper::Modes mode);
    void updateOutput(double progress, Servo* stepper, LightHelper* lights);

    Servo primaryServo;
    Servo secondaryServo;
    LightHelper* primaryLights;
    LightHelper* secondaryLights;
    OutputManagerInterface::States state = OutputManagerInterface::States::UNCALIBRATED;
};

#endif
