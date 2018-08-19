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

#include "LightHelper.h"

LightHelper::LightHelper(int dockPin, int starboardPin, int portPin) : dockPin(dockPin), starboardPin(starboardPin), portPin(portPin) {}

void LightHelper::update() {
  switch (mode) {
    case Modes::RUNNING :
      analogWrite(dockPin, 0);
      if (direction == Directions::STARBOARD) {
        analogWrite(starboardPin, k_directionLightIntensity);
        analogWrite(portPin, 0);
      } else {
        analogWrite(starboardPin, 0);
        analogWrite(portPin, k_directionLightIntensity);
      }
      break;
    case Modes::DOCKED :
      analogWrite(dockPin, k_dockLightIntensity);
      analogWrite(starboardPin, 0);
      analogWrite(portPin, 0);
      break;
    case Modes::DISCONNECTED :
      double pulsingIntensity = sin(millis() / 150) * 64 + 64;
      analogWrite(dockPin, pulsingIntensity);
      analogWrite(starboardPin, pulsingIntensity);
      analogWrite(portPin, pulsingIntensity);
      break;
  }
}

void LightHelper::setupPins() {
  pinMode(dockPin, OUTPUT);
  pinMode(starboardPin, OUTPUT);
  pinMode(portPin, OUTPUT);
}

void LightHelper::setMode(Modes mode) {
  this->mode = mode;
}

void LightHelper::setDirection(Directions direction) {
  this->direction = direction;
}
