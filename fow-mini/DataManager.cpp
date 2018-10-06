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

#include "DataManager.h"

DataManager::DataManager() {
}

void DataManager::update(String rawDataString) {
  std::vector<String> compositeResponse;
  compositeResponse = split(rawDataString, ':');
  lastUpdated = millis();

  char* end;

  endDurationAhead = strtoul(compositeResponse.back().c_str(), &end, 10);
  if (endDurationAhead == -1) {
    // -1 means that the data on the server is stale, so we set it up to check back soon
    lastUpdated += refreshRate * 0.8;
    return;
  }
  progresses.clear();

  compositeResponse.pop_back(); // All valid responses end with a ":" so it splits that, and we want to remove it

  if (compositeResponse.size() != 2)
    Serial.printf("Suspect size of %i when splitting composite response.\n", compositeResponse.size());

  for (auto const& responseString : compositeResponse) {
    std::vector<String> response;
    response = split(responseString, ',');
    if (response.size() != 4) {
      Serial.printf("Invalid size of %i when splitting non-composite response.\n", response.size());
      return;
    }

    Progress progress;

    progress.start = atof(response.at(0).c_str());
    progress.end = atof(response.at(1).c_str());

    // startTimeOffset is how long ago (in milliseconds) the first number was valid
    progress.startTimeOffset = strtoul(response.at(2).c_str(), &end, 10);

    const String directionString = response.at(3);
    if (directionString == "DEPARTING") progress.direction = FerryHelper::Directions::DEPARTING;
    else if (directionString == "ARRIVING") progress.direction = FerryHelper::Directions::ARRIVING;
    else {
      Serial.println(String("Unknown direction string of ") + directionString);
      progress.direction = FerryHelper::Directions::DEPARTING;
    }

    progresses.push_back(progress);
  }
}

bool DataManager::shouldUpdate() {
  return !(millis() - lastUpdated < refreshRate);
}

DataManager::FerryData DataManager::getProgress(int i) {
  FerryData result;

  if (i >= progresses.size()) {
    Serial.println(String("Attempt to get progress of nonexistant ferry index ") + i);
    return result; // Empty struct, see definition for default values
  }

  Progress progress = progresses.at(i);
  result.direction = progress.direction;
  if (progress.startTimeOffset == 0) {
    result.progress = progress.start;
    return result; // If the offset is zero, then the ferry is docked
  }
  long double percentPerMsec = (progress.end - progress.start) / endDurationAhead;
  result.progress = ((static_cast<long double>(millis()) - (lastUpdated - progress.startTimeOffset)) * percentPerMsec) + progress.start;

  return result;
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
