#include "ConnectionManager.h"

ConnectionManager* conn = 0; // null reference on the heap

void setup() {
  conn = new ConnectionManager("fow-mini");
}

void loop() {
  conn->update();
}
