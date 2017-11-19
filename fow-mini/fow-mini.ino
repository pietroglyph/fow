#include "ConnectionManager.h"

ConnectionManager conn("fow-mini");

void setup() {
}

void loop() {
  conn.update();
}
