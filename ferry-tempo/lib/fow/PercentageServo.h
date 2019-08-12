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

#ifndef PercentageServo_h
#define PercentageServo_h

#include <Servo.h>
#include <math.h>

class PercentageServo : public Servo {
  public:
    PercentageServo(int minimumPosition, int maximumPosition, boolean isInverted);
    void write(int value) = delete;
    void write(double percentage);
    int getPin();
    uint8_t attach(int pin);
  private:
    int pin;
    double minimumPosition, maximumPosition, range;
    boolean isInverted;
};

#endif
