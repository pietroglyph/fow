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

#ifndef LightHelper_h
#define LightHelper_h

#include "Arduino.h"
#include <vector>
#include <cmath>

class LightHelper {
  public:
    LightHelper(int arrivingPin, int departingPin, int redIntensity, int greenIntensity);

    enum class Modes {
      RUNNING,
      DOCKED,
      DISCONNECTED
    };
    enum class Directions {
      DEPARTING, ARRIVING
    };

    void update();
    void setMode(Modes mode);
    void setDirection(Directions direction);
    void setupPins();
    static int getPulsingIntensity(int maxIntensity) {
      double scaledLightIntensity = (static_cast<double>(maxIntensity)) / 2.0;
      return round(std::sin(static_cast<double>(millis()) / (blinkDuration / PI)) * scaledLightIntensity + scaledLightIntensity);
    };
  private:
    Modes mode;
    Directions direction;

    static const double blinkDuration = 800; // In milleseconds

    const int redIntensity;
    const int greenIntensity;
    const int arrivingPin;
    const int departingPin;
};

#endif
