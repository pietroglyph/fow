/*
    Copyright (c) 2017, 2018 Declan Freeman-Gleason.

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

#include "ConnectionManager.h"
#include "DataManager.h"
#include "OutputManagerInterface.h"

#ifdef IS_STEPPER_CLOCK
#include "StepperClockOutputManager.h"
#define OUTPUT_MANAGER_CONSTRUCTOR StepperClockOutputManager
#else
// This is just an else (not an else if) as a fallback to ensure Arduino IDE compatibility
#include "ServoClockOutputManager.h"
#define OUTPUT_MANAGER_CONSTRUCTOR ServoClockOutputManager
#endif

// These objects should never be destroyed
ConnectionManager* conn = nullptr;
DataManager* data = nullptr;
OutputManagerInterface* output = nullptr;

void setup() {
  Serial.begin(115200);
  conn = new ConnectionManager("FERRY TEMPO Setup");
  data = new DataManager();
  output = new OUTPUT_MANAGER_CONSTRUCTOR(HARDWARE_CONSTRUCTOR);
}

void loop() {
  conn->update();
  if (conn->ready()) {
    if (data->shouldUpdate()) data->update(conn->get());
    output->update([](int ferryIndex) {
      return data->getProgress(ferryIndex);
    });
  } else {
    output->calibrate();
  }
}
