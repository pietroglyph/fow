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

#include "DataManager.h"

DataManager::DataManager() {
}

void DataManager::update(String rawDataString) {
  std::vector<String> compositeResponse;
  compositeResponse = split(rawDataString, ':');
  lastUpdated = millis();

  endDurationAhead = std::atol(compositeResponse.back().c_str());
  if (endDurationAhead == -1) {
    // -1 means that the data on the server is stale
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

    Progress progress{};

    progress.start = std::atof(response.at(0).c_str());
    progress.end = std::atof(response.at(1).c_str());

    // startTimeOffset is how long ago (in milliseconds) the first number was valid
    progress.startTimeOffset = std::strtoul(response.at(2).c_str(), nullptr, 0);

    const String directionString = response.at(3);
    if (directionString == "DEPARTING") progress.direction = LightHelper::Directions::DEPARTING;
    else if (directionString == "ARRIVING") progress.direction = LightHelper::Directions::ARRIVING;
    else {
      Serial.println(String("Unknown direction string of ") + directionString);
      progress.direction = LightHelper::Directions::DEPARTING;
    }

    progresses.push_back(progress);
  }
}

bool DataManager::shouldUpdate() {
  return millis() - lastUpdated > refreshRate;
}

DataManager::FerryData DataManager::getProgress(unsigned int i) {
  FerryData result{};

  if (i >= progresses.size()) {
    Serial.println(String("Attempt to get progress of nonexistant ferry index ") + i);
    result.isValid = false;
    return result; // Empty struct, see definition for default values
  }

  result.isNew = (millis() - lastUpdated) < isNewDuration;

  Progress progress = progresses.at(i);
  result.direction = progress.direction;
  if (progress.startTimeOffset == 0) {
    result.progress = progress.start;
    return result; // If the offset is zero, then the ferry is docked
  }

  if (PREVENT_BUZZING) {
    result.progress = progress.start;
    return result;
  }

  // Care must be taken here to:
  // a. Avoid narrowing conversions (hence the use of Uniform Initialization).
  // b. Avoid truncating when casting. This isn't much of a problem because unsigned long (uint32) fits within a double (real64).
  double percentPerMsec{(progress.end - progress.start) / static_cast<double>(endDurationAhead)};
  double msecSinceUpdate{millis() - (lastUpdated - progress.startTimeOffset)};
  result.progress = double{(msecSinceUpdate * percentPerMsec) + progress.start};
  return result;
}