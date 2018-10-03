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

#ifndef OutputManagerInterface_h
#define OutputManagerInterface_h

#include <functional>
#include "DataManager.h"

class OutputManagerInterface {
  public:
    // We never delete objects who implement this class, so we don't need a virtual destructor

    // Passing functions insulates the implementor of this interface from the details of the structure that stores the data
    virtual void update(std::function<DataManager::FerryData (int)> dataSupplier);
    virtual void calibrate();
  protected:
    // Child classes don't need to use these, but they're provided for convinence
    enum class States {
      UNCALIBRATED,
      CALIBRATING,
      RUNNING
    };
};

#endif
