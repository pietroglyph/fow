/*
    Copyright (c) 2017-2019 Declan Freeman-Gleason.

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
#include "Wire.h"

LightHelper::LightHelper(int departingPin, int arrivingPin, int redIntensity, int greenIntensity) : departingPin(departingPin), arrivingPin(arrivingPin), redIntensity(redIntensity), greenIntensity(greenIntensity) {}

void LightHelper::update() {
  switch (mode) {
    case Modes::OFF :
      analogWrite(arrivingPin, LOW);
      analogWrite(departingPin, LOW);
      break;
    case Modes::SELF_TEST :
    case Modes::RUNNING :
      // Assumes that ARRIVING == red light
      if (this->direction == Directions::ARRIVING) {
        analogWrite(arrivingPin, redIntensity);
        analogWrite(departingPin, LOW);
      } else {
        analogWrite(arrivingPin, LOW);
        analogWrite(departingPin, greenIntensity);
      }
      break;
    case Modes::DOCKED :
      analogWrite(arrivingPin, LOW);
      analogWrite(departingPin, LOW);
      break;
    case Modes::DISCONNECTED :
      // The idea here is to switch between the two lights as fast as we can to give the appearance of them both being on at once
      this->direction = this->direction == Directions::ARRIVING ? Directions::DEPARTING : Directions::ARRIVING;

      double maxIntensity = greenIntensity;
      if (this->direction == Directions::ARRIVING) {
        maxIntensity = redIntensity;
      }

      double pulsingIntensity = getPulsingIntensity(maxIntensity);
      if (this->direction == Directions::ARRIVING) {
         analogWrite(arrivingPin, pulsingIntensity);
         analogWrite(departingPin, LOW);
      } else {
         analogWrite(arrivingPin, LOW);
         analogWrite(departingPin, pulsingIntensity);
      }
      break;
  }
}

void LightHelper::setupPins() {
  pinMode(arrivingPin, OUTPUT);
  pinMode(departingPin, OUTPUT);
}

void LightHelper::setMode(Modes mode) {
  this->mode = mode;
}

void LightHelper::setDirection(Directions direction) {
  this->direction = direction;
}
