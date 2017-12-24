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

#ifndef LightManager_h
#define LightManager_h

#include <Wire.h>
#include <vector>
#include "Arduino.h" // XXX: I have no idea why this needs to be included

class FerryLights {
  public:
    FerryLights(int dockPin, int starboardPin, int portPin);

    enum class Modes {
      RUNNING,
      DOCKED,
      DISCONNECTED
    };
    enum class Directions {
      PORT, STARBOARD
    };

    void update();
    void setMode(Modes mode);
    void setDirection(Directions direction);
    void setupPins();
  private:
    Modes mode;
    Directions direction;

    const int k_dockLightIntensity = 128;
    const int k_directionLightIntensity = 2;

    int dockPin;
    int starboardPin;
    int portPin;
};

class LightManager {
  public:
    void registerFerry(FerryLights* ferryLights);
    void update();
    void setupPins();
    void setAllModesOnce(FerryLights::Modes mode);
  private:
    std::vector<FerryLights*> ferries;
    FerryLights::Modes lastMode;
};

#endif
