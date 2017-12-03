#ifndef DataManager_h
#define DataManager_h

#include "ConnectionManager.h"

class DataManager {
  public:
    DataManager(ConnectionManager* conn);

    long refreshRate = 5000; // in milliseconds

    void update();
    double getProgress();
  private:
    unsigned long lastUpdated = 0;
    double startProgress;
    double endProgress;

    void processResponse(String resp);

    ConnectionManager* connection;
};

#endif
