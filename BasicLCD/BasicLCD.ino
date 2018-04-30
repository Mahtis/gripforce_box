//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int counter = 0;

void setup()
{
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Hello world");
}


void loop()
{
  counter++;
  if (counter > 5000 && counter < 10000) {
    lcd.noBacklight();
  }
  if (counter > 10000) {
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("back again");
  }
  delay(1);
}
