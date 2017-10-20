#include <Arduino.h>

int main( void ){

  //Initialise Arduino functionality.
  init();

  //Attach USB for applicable processors.
  #ifdef USBCON
    USBDevice.attach();
  #endif

  while( true ){

    //Process the serial libaries event queue.
    if( serialEventRun ) serialEventRun();
  }
  //Execution should never reach this point.
  return 0x00;
}
