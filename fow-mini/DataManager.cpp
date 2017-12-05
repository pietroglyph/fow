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

#include "DataManager.h"

DataManager::DataManager(ConnectionManager* conn) : connection(conn) {
  Serial.begin(115200);
  delay(10);
}

void DataManager::update() {
  if (millis() - lastUpdated < refreshRate) {
    return;
  }

  std::vector<String> response;
  response = split(connection->get(), ',');
  lastUpdated = millis();
  if (response.size() != 3) {
    Serial.printf("Invalid size of %i when splitting response.\n", response.size());
    return;
  }

  char *end;
  startProgress = atof(response.at(0).c_str());
  endProgress = atof(response.at(1).c_str());
  // progressStartTimeOffset is how long ago (in milliseconds) the first number was valid
  progressStartTimeOffset = millis() - strtoul(response.at(2).c_str(), &end, 10);
}

double DataManager::getProgress() {
  long double divisor = millis() - (progressStartTimeOffset + millis() - lastUpdated) + endDurationAhead;
  if (millis() > LDBL_MAX) {
    // We will overflow interpFactor if this is true (unsigned long can get twice as large as long double, even though they're the same precision)
    // We're forced to reset if this happens, but it's a very edge case so something this drastic is acceptable
    Serial.println("millis would overflow a double, resetting.");
    resetFunc();
    return 0.0;
  }
  long double interpFactor = double(millis() / divisor);
  return (((endProgress - startProgress) * interpFactor) + startProgress);
}

std::vector<String> DataManager::split(const String &text, char sep) {
  std::vector<String> tokens;
  size_t start = 0, end = 0;
  while ((end = text.indexOf(sep, start)) != -1) {
    if (end != start) {
      tokens.push_back(text.substring(start, end));
    }
    start = end + 1;
  }
  if (end != start) {
    tokens.push_back(text.substring(start));
  }
  return tokens;
}
