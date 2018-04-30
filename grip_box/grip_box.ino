
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//LCD is connected on pins A4 and A5
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// UI control button pins
const int backButton = A1;
const int nextButton = A2;
const int okButton = A3;

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

int mainScreen = 1;


void setup() {
  lcd.init();
  lcd.backlight();
  screenPrint("BOOTING UP", "");
  pinMode(backButton, INPUT);
  pinMode(nextButton, INPUT);
  pinMode(okButton, INPUT);
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
  switch (mainScreen) {
    case 1:
      screenPrint("READY", "");
      // display title and some info
      break;
    case 2:
      screenPrint("Test grips", "");
      // show live recordings from both sensors
      break;
    case 3:
      screenPrint("Test triggers", "");
      // show live any incoming triggers
      break;
    case 4:
      screenPrint("Change sensitivities", "");
      // separate menu wizard for changing each pot sensitivity
      break;
    case 5:
      screenPrint("Start recording", "");
      // separate menu to start recording, ask filename at least
      break;
    case 6:
      screenPrint("log to csv", "");
      // separate menu to transform a log to csv file, at least file selection
      break;
    default:
      screenPrint("Is something", "wrong?");
      // should never print
      break;  
  }
  if (digitalRead(nextButton) == 1) {
    delay(50);
    if (digitalRead(nextButton)== 1) {
      mainScreen++;
      if (mainScreen > 6) {
        mainScreen = 1;
      }
      // delay to avoid accidental proceeding
      delay(200);
    }
  }
  delay(40);
}

void screenPrint(String msg1, String msg2) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(msg1);
  lcd.setCursor(0,1);
  lcd.print(msg2);
}


