
/**
 * This program logs data to a binary file.  Functions are included
 * to convert the binary file to a csv text file.
 *
 * Samples are logged at regular intervals.  The maximum logging rate
 * depends on the quality of your SD card and the time required to
 * read sensor data.  This example has been tested at 500 Hz with
 * good SD card on an Uno.  4000 HZ is possible on a Due.
 *
 * If your SD card has a long write latency, it may be necessary to use
 * slower sample rates.  Using a Mega Arduino helps overcome latency
 * problems since 12 512 byte buffers will be used.
 *
 * Data is written to the file using a SD multiple block write command.
 */
#include <ShiftedLCD.h>
#include <SPI.h>
#include "SdFat.h"
#include "FreeStack.h"
#include "UserTypes.h"

#ifdef __AVR_ATmega328P__
#include "MinimumSerial.h"
MinimumSerial MinSerial;
#define Serial MinSerial
#endif  // __AVR_ATmega328P__
//==============================================================================
// Start of configuration constants.
//==============================================================================


//LCD is connected on pin A5
const int lcdSelector = A5;
LiquidCrystal lcd(lcdSelector);  // 

// UI control button pins
const int backButton = A1;
const int nextButton = A2;
const int okButton = A3;

// LPT trigger pins
const int triggers[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int nOfTriggers = 8;

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


// Abort run on an overrun.  Data before the overrun will be saved.
#define ABORT_ON_OVERRUN 1
//------------------------------------------------------------------------------
//Interval between data records in microseconds.
const uint32_t LOG_INTERVAL_USEC = 2000;
//------------------------------------------------------------------------------
// Set USE_SHARED_SPI non-zero for use of an SPI sensor.
// May not work for some cards.
#ifndef USE_SHARED_SPI
#define USE_SHARED_SPI 1
#endif  // USE_SHARED_SPI
//------------------------------------------------------------------------------
// Pin definitions.
//
// SD chip select pin.
const uint8_t SD_CS_PIN = sdSelector;
//
// Digital pin to indicate an error, set to -1 if not used.
// The led blinks for fatal errors. The led goes on solid for
// overrun errors and logging continues unless ABORT_ON_OVERRUN
// is non-zero.
#ifdef ERROR_LED_PIN
#undef ERROR_LED_PIN
#endif  // ERROR_LED_PIN
const int8_t ERROR_LED_PIN = -1;
//------------------------------------------------------------------------------
// File definitions.
//
// Maximum file size in blocks.
// The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// This file is flash erased using special SD commands.  The file will be
// truncated if logging is stopped early.
const uint32_t FILE_BLOCK_COUNT = 256000;
//
// log file base name if not defined in UserTypes.h
#ifndef FILE_BASE_NAME
#define FILE_BASE_NAME "data"
#endif  // FILE_BASE_NAME
//------------------------------------------------------------------------------
// Buffer definitions.
//
// The logger will use SdFat's buffer plus BUFFER_BLOCK_COUNT-1 additional
// buffers.
//
#ifndef RAMEND
// Assume ARM. Use total of ten 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 10;
//
#elif RAMEND < 0X8FF
#error Too little SRAM
//
#elif RAMEND < 0X10FF
// Use total of two 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 2;
//
#elif RAMEND < 0X20FF
// Use total of four 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 4;
//
#else  // RAMEND
// Use total of 12 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 12;
#endif  // RAMEND
//==============================================================================
// End of configuration constants.
//==============================================================================
// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME FILE_BASE_NAME "##.bin"

// Size of file base name.
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
const uint8_t FILE_NAME_DIM  = BASE_NAME_SIZE + 7;
char binName[FILE_NAME_DIM] = FILE_BASE_NAME "00.bin";

SdFat sd;

SdBaseFile binFile;

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 4)/sizeof(data_t);

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 4 - DATA_DIM*sizeof(data_t);

struct block_t {
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];
  uint8_t fill[FILL_DIM];
};
//==============================================================================
// Error messages stored in flash.
#define error(msg) {sd.errorPrint(&Serial, F(msg));fatalBlink();}
//------------------------------------------------------------------------------
//
void fatalBlink() {
  while (true) {
    SysCall::yield();
    if (ERROR_LED_PIN >= 0) {
      digitalWrite(ERROR_LED_PIN, HIGH);
      delay(200);
      digitalWrite(ERROR_LED_PIN, LOW);
      delay(200);
    }
  }
}
//-----------------------------------------------------------------------------
// Convert binary file to csv file.
void binaryToCsv() {
  uint8_t lastPct = 0;
  block_t block;
  uint32_t t0 = millis();
  uint32_t syncCluster = 0;
  SdFile csvFile;
  char csvName[FILE_NAME_DIM];

  if (!binFile.isOpen()) {
    //screenPrint("No current", "binary file");
    //delay(1000);
    return;
  }
  
  // Create a new csvFile.
  strcpy(csvName, binName);
  strcpy(&csvName[BASE_NAME_SIZE + 3], "csv");

  if (!csvFile.open(csvName, O_WRITE | O_CREAT | O_TRUNC)) {
    //screenPrint("open csvFile", "failed");
    //delay(1000);
  }
  binFile.rewind();
  //screenPrint("Writing: " + String(csvName), "Button to abort");
  printHeader(&csvFile);
  uint32_t tPct = millis();
  while (!stopButtons() && binFile.read(&block, 512) == 512) {
    uint16_t i;
    if (block.count == 0 || block.count > DATA_DIM) {
      break;
    }
    if (block.overrun) {
      csvFile.print(F("OVERRUN,"));
      csvFile.println(block.overrun);
    }
    for (i = 0; i < block.count; i++) {
      printData(&csvFile, &block.data[i]);
    }
    if (csvFile.curCluster() != syncCluster) {
      csvFile.sync();
      syncCluster = csvFile.curCluster();
    }
    if ((millis() - tPct) > 1000) {
      uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
      if (pct != lastPct) {
        tPct = millis();
        lastPct = pct;
      }
    }
    if (stopButtons()) {
      break;
    }
  }
  csvFile.close();
  //screenPrint("Done!", String(0.001*(millis() - t0)) + " Seconds");
}
//-----------------------------------------------------------------------------
void createBinFile() {
  // max number of blocks to erase per erase call
  const uint32_t ERASE_SIZE = 262144L;
  uint32_t bgnBlock, endBlock;
  
  // Delete old tmp file.
  if (sd.exists(TMP_FILE_NAME)) {
    if (!sd.remove(TMP_FILE_NAME)) {
      //screenPrint("Can't remove tmp file", "");
      //delay(2000);
    }
  }
  // Create new file.
  binFile.close();
  if (!binFile.createContiguous(TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
    //screenPrint("createContiguous failed", "");
    //delay(2000);
  }
  screenPrint("created Contiguous", "");
  // Get the address of the file on the SD.
  if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
    //screenPrint("contiguousRange failed", "");
    //delay(2000);
  }
  // Flash erase all data in the file.
  uint32_t bgnErase = bgnBlock;
  uint32_t endErase;
  while (bgnErase < endBlock) {
    endErase = bgnErase + ERASE_SIZE;
    if (endErase > endBlock) {
      endErase = endBlock;
    }
    if (!sd.card()->erase(bgnErase, endErase)) {
      //screenPrint("erase failed", "");
      //delay(2000);
    }
    bgnErase = endErase + 1;
  }
}
//------------------------------------------------------------------------------
// log data
void logData() {
  createBinFile();
  //screenPrint("CREATED", "FILE");
  recordToFile();
  //screenPrint("RECORDED", "FILE");
  renameBinFile();
  //screenPrint("RENAMED", "FILE");
}

//------------------------------------------------------------------------------

void recordToFile() {
  digitalWrite(A5, HIGH);
  const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 1;
  // Index of last queue location.
  const uint8_t QUEUE_LAST = QUEUE_DIM - 1;
  
  // Allocate extra buffer space.
  block_t block[BUFFER_BLOCK_COUNT - 1];
  
  block_t* currentBlock = 0;

  block_t* emptyStack[BUFFER_BLOCK_COUNT];
  uint8_t emptyTop;
  uint8_t minTop;

  block_t* fullQueue[QUEUE_DIM];
  uint8_t fullHead = 0;
  uint8_t fullTail = 0;  

  // Use SdFat's internal buffer.
  emptyStack[0] = (block_t*)sd.vol()->cacheClear();
  if (emptyStack[0] == 0) {
    //screenPrint("cacheClear failed", "");
    //delay(4000);
  }

  // Put rest of buffers on the empty stack.
  for (int i = 1; i < BUFFER_BLOCK_COUNT; i++) {
    emptyStack[i] = &block[i - 1];
  }
  emptyTop = BUFFER_BLOCK_COUNT;
  minTop = BUFFER_BLOCK_COUNT;
  
  // Start a multiple block write.
  if (!sd.card()->writeStart(binFile.firstBlock())) {
    //screenPrint("writeStart failed", "");
    //delay(4000);
  }
  //screenPrint("RECORDING..", "BUTTON STOPS");
  bool closeFile = false;
  uint32_t bn = 0;  
  uint32_t maxLatency = 0;
  uint32_t overrun = 0;
  uint32_t overrunTotal = 0;
  uint32_t logTime = micros();
  while(1) {
     // Time for next data record.
    logTime += LOG_INTERVAL_USEC;
    if (stopButtons()) {
      //screenPrint("WILL CLOSE", "");
      closeFile = true;
    }  
    if (closeFile) {
      if (currentBlock != 0) {
        // Put buffer in full queue.
        fullQueue[fullHead] = currentBlock;
        fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
        currentBlock = 0;
      }
    } else {
      if (currentBlock == 0 && emptyTop != 0) {
        currentBlock = emptyStack[--emptyTop];
        if (emptyTop < minTop) {
          minTop = emptyTop;
        }
        currentBlock->count = 0;
        currentBlock->overrun = overrun;
        overrun = 0;
      }
      if ((int32_t)(logTime - micros()) < 0) {
        //screenPrint("Rate too fast", "");
        //delay(4000);
      }
      int32_t delta;
      do {
        delta = micros() - logTime;
      } while (delta < 0);
      if (currentBlock == 0) {
        overrun++;
        overrunTotal++;
        if (ERROR_LED_PIN >= 0) {
          digitalWrite(ERROR_LED_PIN, HIGH);
        }        
#if ABORT_ON_OVERRUN
        //screenPrint("OVERRUN ABORT", "");
        //delay(4000);
        break;
 #endif  // ABORT_ON_OVERRUN       
      } else {
#if USE_SHARED_SPI
        sd.card()->spiStop();
#endif  // USE_SHARED_SPI   
        acquireData(&currentBlock->data[currentBlock->count++]);
#if USE_SHARED_SPI
        sd.card()->spiStart();
#endif  // USE_SHARED_SPI      
        if (currentBlock->count == DATA_DIM) {
          fullQueue[fullHead] = currentBlock;
          fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
          currentBlock = 0;
        } 
      }
    }
    if (fullHead == fullTail) {
      // Exit loop if done.
      if (closeFile) {
        break;
      }
    } else if (!sd.card()->isBusy()) {
      // Get address of block to write.
      block_t* pBlock = fullQueue[fullTail];
      fullTail = fullTail < QUEUE_LAST ? fullTail + 1 : 0;
      // Write block to SD.
      uint32_t usec = micros();
      if (!sd.card()->writeData((uint8_t*)pBlock)) {
        //screenPrint("write data failed", "");
        //delay(4000);
      }
      usec = micros() - usec;
      if (usec > maxLatency) {
        maxLatency = usec;
      }
      // Move block to empty queue.
      emptyStack[emptyTop++] = pBlock;
      bn++;
      if (bn == FILE_BLOCK_COUNT) {
        // File full so stop
        break;
      }
    }
  }
  if (!sd.card()->writeStop()) {
    //screenPrint("writeStop failed", "");
    //delay(4000);
  }
  // Truncate file if recording stopped early.
  if (bn != FILE_BLOCK_COUNT) {
    if (!binFile.truncate(512L * bn)) {
      //screenPrint("Can't truncate file", "");
      //delay(4000);
    }
  }
}
//-----------------------------------------------------------------------------
void renameBinFile() {
  //digitalWrite(A5, HIGH);
  while (sd.exists(binName)) {
    if (binName[BASE_NAME_SIZE + 1] != '9') {
      binName[BASE_NAME_SIZE + 1]++;
    } else {
      binName[BASE_NAME_SIZE + 1] = '0';
      if (binName[BASE_NAME_SIZE] == '9') {
        //error("Can't create file name");
      }
      binName[BASE_NAME_SIZE]++;
    }
  }
  if (!binFile.rename(sd.vwd(), binName)) {
    //error("Can't rename file");
    }
}

//----------------------------------------------------------------------------------------------------

void setup() {
  lcd.begin(16, 2);
  screenPrint("BOOTING UP", "");
  delay(500);
  pinMode(backButton, INPUT);
  pinMode(nextButton, INPUT);
  pinMode(okButton, INPUT);
  for (int i=0; i < nOfTriggers; i++) {
    pinMode(triggers[i], INPUT);
  }
  pinMode(lcdSelector, OUTPUT);
  pinMode(sdSelector, OUTPUT);
  pinMode(potSelector, OUTPUT);
  // set selectors high to de-select them
  digitalWrite(sdSelector, HIGH);
  digitalWrite(potSelector, HIGH);
  
  pinMode(grip1, INPUT);
  pinMode(grip2, INPUT);
  // start spi connection

  if (ERROR_LED_PIN >= 0) {
    pinMode(ERROR_LED_PIN, OUTPUT);
  }
  if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(50))) {
    screenPrint("NO DICE", "");
    sd.initErrorPrint(&Serial);
    fatalBlink();
  }
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
      startRecording();
      break;
    case 5:
      screenPrint("Converting to", "CSV");
      delay(3000);
      lcd.noDisplay();
      digitalWrite(A5, HIGH);
      binaryToCsv();
      delay(100);
      lcd.begin(16,2);
      screenPrint("DONE AND DONE", "");
      delay(500);
      break;
    default:
      mainScreen = 0;
      // should never be reached
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
  while (true) {
    screenPrint("Grip1      Grip2", String(analogRead(grip1)) + "        " + String(analogRead(grip2)));
    if (moveOn) return;
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

void startRecording() {
  screenPrint("STARTING RECORD", "SCREEN TURNS OFF");
  delay(4000);
  lcd.noDisplay();
  digitalWrite(A5, HIGH);
  logData();
  lcd.begin(16,2);
  screenPrint("REC DONE", "press Button");
  while(!moveOn()) {}
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

boolean stopButtons() {
  return (digitalRead(backButton) == 1 && digitalRead(nextButton) == 1);
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


