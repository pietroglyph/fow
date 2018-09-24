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

#include "FerryHelper.h"

FerryHelper::FerryHelper(int portPin, int starboardPin, int lightIntensity) : portPin(portPin), starboardPin(starboardPin), lightIntensity(lightIntensity) {}

void FerryHelper::update() {
  switch (mode) {
    case Modes::RUNNING :
      if (direction == Directions::STARBOARD) {
        analogWrite(starboardPin, lightIntensity * starboardLuminanceScaleFactor);
        analogWrite(portPin, 0);
      } else {
        analogWrite(starboardPin, 0);
        analogWrite(portPin, lightIntensity);
      }
      break;
    case Modes::DOCKED :
      analogWrite(starboardPin, 0);
      analogWrite(portPin, 0);
      break;
    case Modes::DISCONNECTED :
      double scaledLightIntensity = ((double)lightIntensity) / 2.0;
      double pulsingIntensity = sin(((long double)millis()) / (blinkDuration / PI)) * scaledLightIntensity + scaledLightIntensity;
      if (direction == Directions::STARBOARD) analogWrite(starboardPin, pulsingIntensity);
      else analogWrite(portPin, pulsingIntensity);
      break;
  }
}

void FerryHelper::setupPins() {
  pinMode(starboardPin, OUTPUT);
  pinMode(portPin, OUTPUT);
}

void FerryHelper::setMode(Modes mode) {
  this->mode = mode;
}

void FerryHelper::setDirection(Directions direction) {
  this->direction = direction;
}
