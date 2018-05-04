#ifndef UserTypes_h
#define UserTypes_h
#include "Arduino.h"
// User data types.  Modify for your data items.
#define FILE_BASE_NAME "grip_data"
const uint8_t ADC_DIM = 2;
struct data_t {
  uint32_t time;
  uint16_t adc[ADC_DIM];
  uint8_t trigs[8];
};
void acquireData(data_t* data);
void printData(Print* pr, data_t* data);
void printHeader(Print* pr);
void userSetup();
#endif  // UserTypes_h
