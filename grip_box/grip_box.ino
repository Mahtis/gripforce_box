
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

//LCD is connected on pins A4 and A5
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// UI control button pins
const int backButton = A1;
const int nextButton = A2;
const int okButton = A3;

// LPT trigger pins
const int triggers[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int nOfTriggers = 8;
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

int mainScreen = 0;
String mainScreenTexts[6] = {"READY", "Test grips", "Test triggers", "Change sensitivities", "Start recording", "log to csv"};
int pot1Value = 100;
int pot2Value = 100;


void setup() {
  lcd.init();
  lcd.backlight();
  screenPrint("BOOTING UP", "");
  pinMode(backButton, INPUT);
  pinMode(nextButton, INPUT);
  pinMode(okButton, INPUT);
  for (int i=0; i < nOfTriggers; i++) {
    pinMode(triggers[i], INPUT);
  }
  /*
  pinMode(trig1, INPUT);
  pinMode(trig2, INPUT);
  pinMode(trig3, INPUT);
  pinMode(trig4, INPUT);
  pinMode(trig5, INPUT);
  pinMode(trig6, INPUT);
  pinMode(trig7, INPUT);
  pinMode(trig8, INPUT);
  */
  pinMode(sdSelector, OUTPUT);
  pinMode(potSelector, OUTPUT);
  // set selectors high to de-select them
  pinMode(grip1, INPUT);
  pinMode(grip2, INPUT);
  // start spi connection
}

void loop() {
  screenPrint(mainScreenTexts[mainScreen], "");
  mainScreen = navigateValues(mainScreen, 0, 5, 200);
  if (digitalRead(okButton) == 1) {
    delay(50);
    if (digitalRead(okButton) == 1) {
      switch (mainScreen) {
    case 0:
      screenPrint("READY", "Grip box 2.0");
      // display title and some info
      break;
    case 1:
      testSensors();
      break;
    case 2:
      testTriggers();
      break;
    case 3:
      changeSensitivity();
      break;
    case 4:
      screenPrint("Start recording", "");
      // separate menu to start recording, ask filename at least
      break;
    case 5:
      screenPrint("log to csv", "");
      // separate menu to transform a log to csv file, at least file selection
      break;
    default:
      mainScreen = 0;
      // should never print
      break;  
  }
    }
    delay(200);
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

int navigateValues(int navValue, int minValue, int maxValue, int wait) {
  if (digitalRead(nextButton) == 1) {
    delay(50);
    if (digitalRead(nextButton)== 1) {
      navValue++;
      if (navValue > maxValue) {
        navValue = minValue;
      }
      // delay to avoid accidental proceeding
      delay(wait);
      return navValue;
    }
  }
  if (digitalRead(backButton) == 1) {
    delay(50);
    if (digitalRead(backButton)== 1) {
      navValue--;
      if (navValue < minValue) {
        navValue = maxValue;
      }
      // delay to avoid accidental proceeding
      delay(wait);
      return navValue;
    }
  }
  return navValue;
}


void testSensors() {
  while(!moveOn()) {
    screenPrint("Grip1      Grip2", String(analogRead(grip1)) + "        " + String(analogRead(grip2)));
    delay(100);
  }
}

void testTriggers() {
  while (!moveOn()) {
    int trigger = getTrigger();
    screenPrint("Trigger: " + String(trigger), "");
   }
}

void changeSensitivity() {
  pot1Value = adjustPot(pot1Value, pot1Addr, 1, grip1);
  pot2Value = adjustPot(pot2Value, pot2Addr, 2, grip2);
}

int adjustPot(int potValue, int potAddress, int potNum, int grip) {
  delay(200);
  while (true) {
    screenPrint("Sens" + String(potNum) + ": " + String(potValue), "Grip" + String(potNum) + ": " + String(analogRead(grip)));
    int oldPotVal = potValue;
    potValue = navigateValues(potValue, 0, 255, 100);
    if (potValue != oldPotVal) {
      digitalPotWrite(potAddress, potValue);
    }
    if (moveOn()) {
      return potValue;
    }
  }
}

void digitalPotWrite(byte address, int value) {
  /*
  // take the SS pin low to select the chip:
  digitalWrite(potSelector, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(potSelector, HIGH);
  */
}

int getTrigger() {
  int triggerValue = 0;
  for (int j=0; j < nOfTriggers; j++) {
    // note the importance of trigger being sent in correct order!
    triggerValue += (digitalRead(triggers[j]) * pow(2,j));
  }
  return triggerValue;
}

boolean moveOn() {
  if (digitalRead(okButton) == 1) {
    delay(40);
    if (digitalRead(okButton)== 1) {
      return true;
    }
  }
  return false;
}


