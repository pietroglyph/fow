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
#include <tuple>

class LightHelper {
  public:
    LightHelper(int arrivingPin, int departingPin, int redIntensity, int greenIntensity);

    enum class Modes {
      RUNNING,
      DOCKED,
      DISCONNECTED,
      SELF_TEST,
      OFF
    };
    enum class Directions {
      DEPARTING, ARRIVING
    };

    void update();
    void setMode(Modes mode);
    void setDirection(Directions direction);
    void setupPins();
    static double getPulsingIntensity(int maxIntensity) {
      double scaledMaxIntensity = static_cast<double>(maxIntensity) / 2.0;
      double percentIntensity = std::sin(static_cast<double>(millis()) / (pulseDuration / PI));

      return round(percentIntensity * scaledMaxIntensity + scaledMaxIntensity);
    };
  private:
    Modes mode;
    Directions direction;

    static constexpr double pulseDuration = 800; // In milleseconds

    const int redIntensity;
    const int greenIntensity;
    const uint8_t arrivingPin;
    const uint8_t departingPin;
};

#endif
