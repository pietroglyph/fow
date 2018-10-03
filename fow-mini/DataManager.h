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

#ifndef DataManager_h
#define DataManager_h

#include <vector>
#include <float.h> // for finding the maximum capacity of doubles
#include <Arduino.h>
#include "FerryHelper.h"

class DataManager {
  public:
    DataManager();

    typedef struct {
      double progress = 0;
      FerryHelper::Directions direction;
    } FerryData;

    const unsigned long refreshRate = 5000; // in milliseconds

    void update(String rawDataString);
    bool shouldUpdate();
    FerryData getProgress(int i);
  private:
    typedef struct {
      unsigned long startTimeOffset = 0;
      double start = 0;
      double end = 0;
      FerryHelper::Directions direction;
    } Progress;

    unsigned long lastUpdated = 0;
    std::vector<Progress> progresses;
    unsigned long endDurationAhead = 5000; // The end's progress is 5 seconds ahead of the first. This is the default value, but it is changed based on server output

    std::vector<String> split(const String& text, char sep);
};

#endif
