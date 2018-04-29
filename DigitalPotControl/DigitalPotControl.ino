/*
  Digital Pot Control

 The circuit:
  * CS - to digital pin 10  (SS pin)
  * SDI - to digital pin 11 (MOSI pin)
  * CLK - to digital pin 13 (SCK pin)

*/


// inslude the SPI library:
#include <SPI.h>


// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = A0;
const int sens1 = A6;
const int sens2 = A7;
const byte pot1Addr = B00010001;
const byte pot2Addr = B00010010;

int counter = 0;
int level = 0;

void setup() {
  // set the slaveSelectPin as an output:
  Serial.begin(9600);
  pinMode(slaveSelectPin, OUTPUT);
  // initialize SPI:
  SPI.begin();
}

void loop() {
  counter++;
  // go through the two channels of the digital pot:
  if (counter % 20 == 0) {
    if (level == 255) {
      level = 0;
    } else {
      level = 255;
    }
    digitalPotWrite(pot1Addr, level);
    digitalPotWrite(pot2Addr, level);
  }
  Serial.print("Pot value: ");
  Serial.println(level);  
  Serial.print("Sensor1: ");
  Serial.println(analogRead(sens1));  
  Serial.print("Sensor2: ");
  Serial.println(analogRead(sens2));
  delay(100);
}

void digitalPotWrite(byte address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin, HIGH);
}
