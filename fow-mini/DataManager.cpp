#include "DataManager.h"

// TODO

DataManager::DataManager(ConnectionManager* conn) : connection(conn) {
}

void DataManager::update() {
  if (millis() - lastUpdated > refreshRate) {
    connection->get();
  }
}

double DataManager::getProgress() {}

void DataManager::processResponse(String resp) {}
