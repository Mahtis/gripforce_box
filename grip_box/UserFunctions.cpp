#include "UserTypes.h"
// User data functions.  Modify these functions for your data items.

// Start time for data
static uint32_t startMillis;

// Acquire a data record.
void acquireData(data_t* data) {
  data->time = millis();
  data->adc[0] = analogRead(6);
  data->adc[1] = analogRead(7);
}

// Print a data record.
void printData(Print* pr, data_t* data) {
  if (startMillis == 0) {
    startMillis = data->time;
  }
  pr->print(data->time - startMillis);
  for (int i = 0; i < ADC_DIM; i++) {
    pr->write(',');
    pr->print(data->adc[i]);
  }
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {
  startMillis = 0;
  pr->print(F("time"));
  for (int i = 0; i < ADC_DIM; i++) {
    pr->print(F(",adc"));
    pr->print(i);
  }
  pr->println();
}

// Sensor setup
void userSetup() {
}
