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

FerryHelper::FerryHelper(int starboardPin, int portPin, int lightIntensity) : portPin(portPin), starboardPin(starboardPin), lightIntensity(lightIntensity) {}

void FerryHelper::update() {
  switch (mode) {
    case Modes::RUNNING :
      if (direction == Directions::STARBOARD) {
        analogWrite(starboardPin, lightIntensity / 10); // Starboard is assumed to be green, and it needs about 10% of red's PWM value to match luminance
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
      double pulsingIntensity = sin(millis() / 150) * 64 + 64;
      analogWrite(starboardPin, pulsingIntensity);
      analogWrite(portPin, pulsingIntensity);
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
