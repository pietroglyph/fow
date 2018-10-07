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

#include "FerryHelper.h"

FerryHelper::FerryHelper(int departingPin, int arrivingPin, int lightIntensity) : departingPin(departingPin), arrivingPin(arrivingPin), lightIntensity(lightIntensity) {}

void FerryHelper::update() {
  switch (mode) {
    case Modes::RUNNING :
      if (this->direction == Directions::ARRIVING) {
        analogWrite(arrivingPin, lightIntensity);
        analogWrite(departingPin, 0);
      } else {
        analogWrite(arrivingPin, 0);
        analogWrite(departingPin, lightIntensity);
      }
      break;
    case Modes::DOCKED :
      analogWrite(arrivingPin, 0);
      analogWrite(departingPin, 0);
      break;
    case Modes::DISCONNECTED :
      double scaledLightIntensity = (static_cast<double>(lightIntensity)) / 2.0;
      double pulsingIntensity = sin(static_cast<double>(millis()) / (blinkDuration / PI)) * scaledLightIntensity + scaledLightIntensity;
      if (this->direction == Directions::ARRIVING) analogWrite(arrivingPin, pulsingIntensity);
      else analogWrite(departingPin, pulsingIntensity);
      break;
  }
}

void FerryHelper::setupPins() {
  pinMode(arrivingPin, OUTPUT);
  pinMode(departingPin, OUTPUT);
}

void FerryHelper::setMode(Modes mode) {
  this->mode = mode;
}

void FerryHelper::setDirection(Directions direction) {
  this->direction = direction;
}
