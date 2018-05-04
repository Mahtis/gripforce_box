#include "UserTypes.h"
// User data functions.  Modify these functions for your data items.

// Start time for data
static uint32_t startMillis;

// Acquire a data record.
void acquireData(data_t* data) {
  data->time = millis();
  data->adc[0] = analogRead(6);
  data->adc[1] = analogRead(7);
  data->trigs[0] = digitalRead(2);
  data->trigs[1] = digitalRead(3);
  data->trigs[2] = digitalRead(4);
  data->trigs[3] = digitalRead(5);
  data->trigs[4] = digitalRead(6);
  data->trigs[5] = digitalRead(7);
  data->trigs[6] = digitalRead(8);
  data->trigs[7] = digitalRead(9);
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
  int triggerValue = 0;
  for (int j = 0; j < 8; j++) {
    // note the importance of trigger being sent in correct order!
    triggerValue += (data->trigs[j] * pow(2,j));
  }
  pr->write(',');
  pr->print(triggerValue);
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {
  startMillis = 0;
  pr->print(F("time,"));
  pr->print(F("grip1,"));
  pr->print(F("grip2,"));
  pr->print(F("trigger"));
  pr->println();
}

// Sensor setup
void userSetup() {
}
