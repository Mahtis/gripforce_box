
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//LCD is connected on pins A4 and A5
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// UI control button pins
const int backBtn = A1;
const int nextBtn = A2;
const int okBtn = A3;

// LPT trigger pins
const int trig1 = 2;
const int trig2 = 3;
const int trig3 = 4;
const int trig4 = 5;
const int trig5 = 6;
const int trig6 = 7;
const int trig7 = 8;
const int trig8 = 9;

// SPI selectors for SD card and digital pot
const int sdSelector = 10;
const int potSelector = A0;

// addressess for the two digital pots
const byte pot1Addr = B00010001;
const byte pot2Addr = B00010010;

// Grip sensors connected on analog pins
const int grip1 = A6;
const int grip2 = A7;

// additionally SPI communication uses pins 11, 12 and 13

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("BOOTING UP");
  pinMode(backBtn, INPUT);
  pinMode(nextBtn, INPUT);
  pinMode(okBtn, INPUT);
  pinMode(trig1, INPUT);
  pinMode(trig2, INPUT);
  pinMode(trig3, INPUT);
  pinMode(trig4, INPUT);
  pinMode(trig5, INPUT);
  pinMode(trig6, INPUT);
  pinMode(trig7, INPUT);
  pinMode(trig8, INPUT);
  pinMode(sdSelector, OUTPUT);
  pinMode(potSelector, OUTPUT);
  pinMode(grip1, INPUT);
  pinMode(grip2, INPUT);
}

void loop() {
  
  String msg = ((String)"Button 1: " + digitalRead(button1) + ", Button 2: " + digitalRead(button2) + ", Button 3: " + digitalRead(button3));
  Serial.println(msg);
}
