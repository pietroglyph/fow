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

#ifndef DataManager_h
#define DataManager_h

#include <vector>
#include <float.h> // for finding the maximum capacity of doubles
#include "ConnectionManager.h"

class DataManager {
  public:
    DataManager(ConnectionManager* conn);

    const unsigned long refreshRate = 5000; // in milliseconds

    void update();
    double getProgress(int i);
  private:
    typedef struct {
      unsigned long startTimeOffset = 0;
      double start = 0;
      double end = 0;
    } Progress;

    unsigned long lastUpdated = 0;
    std::vector<Progress> progresses;
    ConnectionManager* connection;
    unsigned long endDurationAhead = 5000; // The end progress is 5 seconds ahead of the first

    std::vector<String> split(const String &text, char sep);

    void(* resetFunc) (void) = 0;
};

#endif
