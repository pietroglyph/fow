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

#include "PercentageServo.h"

PercentageServo::PercentageServo(int minimumPosition, int maximumPosition, boolean isInverted) : isInverted(isInverted) {
  this->minimumPosition = static_cast<double>(minimumPosition);
  this->maximumPosition = static_cast<double>(maximumPosition);
  this->range = this->maximumPosition - this->minimumPosition;
}

void PercentageServo::write(double percentage) {
  double output = lround(minimumPosition + (range * percentage));
  if (isInverted) output = maximumPosition - output;
  Servo::write(output);
}
