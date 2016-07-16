#include <LiquidCrystal.h>

/*
The circuit:

* LCD RS pin to digital pin 12
* LCD Enable pin to digital pin 11
* LCD D4 pin to digital pin 9
* LCD D5 pin to digital pin 6
* LCD D6 pin to digital pin 5
* LCD D7 pin to digital pin 3
* LCD R/W pin to ground
* 10K variable resistor:
* ends to +5V and ground
* wiper to LCD VO pin (pin 5)
*/

// Initialize the library with the interface pins
LiquidCrystal lcd(12, 11, 9, 6, 5, 3);

byte highLevelChar[8] = {
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

byte lowLevelChar[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b10101,
  0b01110,
  0b00100
};

byte separatorChar[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

float lowVoltThreshold = 0;
float highVoltThreshold = 0;
float tolerance = 0;
float highAlarmVolts = 0;
float lowAlarmVolts = 0;
float voltage = 0;
float stepV = 0.1;

int lowVoltLed = 2;
int highVoltLed = 4;
int highRelay = 4;
int lowRelay = 2;
int alarmLed = 7;
int modeButton = 8;
int upButton = 10;
int downButton = 13;
int modeButtonStage = 0;
int upButtonStage = 0;
int downButtonStage = 0;
int modeButtonCounter = 0;
int alarmIsOpened = 0;

enum MODE { 
  NORMAL,
  SETHIGHLEVEL,
  SETLOWLEVEL
  };
MODE mode;

// the setup routine runs once when you press reset:
void setup() {
  mode = SETHIGHLEVEL;
  
  // create new custom characters
  lcd.createChar(0, highLevelChar);
  lcd.createChar(1, lowLevelChar);
  lcd.createChar(2, separatorChar);
  
  lcd.begin(16, 2);
  lcdPrint("Loading...", 0, 0);
  
  lowVoltThreshold = 2.9;  
  highVoltThreshold = 4.5;
  tolerance = 0.3;

  highAlarmVolts = highVoltThreshold + tolerance;
  lowAlarmVolts = lowVoltThreshold - tolerance;

  pinMode(lowVoltLed, OUTPUT);
  pinMode(highVoltLed, OUTPUT);
  pinMode(highRelay, OUTPUT);
  pinMode(lowRelay, OUTPUT);
  pinMode(alarmLed, OUTPUT);
  pinMode(modeButton, INPUT);
  pinMode(upButton, INPUT);
  pinMode(downButton, INPUT);

  // initialize serial communication at 9600 bits per seconds  
  Serial.begin(9600);
  delay(3000);
  lcdClear();
}

void loop() {
  modeButtonStage = digitalRead(modeButton);
  if(modeButtonStage){
    modeButtonCounter++;
    modeButtonStage = 0;
  }

  if(modeButtonCounter >= 3 || modeButtonCounter == 0){
    modeButtonCounter = 0;
    modeTask(modeButtonCounter);
    normalModeProccess();
  }else if(modeButtonCounter == 1){
    modeTask(modeButtonCounter);
    float newHighLevelVoltage = setHighLevelThreshold();
    updateHighLevelScreen(newHighLevelVoltage);
  }else if(modeButtonCounter == 2){
    modeTask(modeButtonCounter);
    float newLowLevelVoltage = setLowLevelThreshold();
    updateLowLevelScreen(newLowLevelVoltage);
  }

  if(lowVoltThreshold == 0 || highVoltThreshold == 0 || lowVoltThreshold >= highVoltThreshold){
    return;
  }
  delay(300);
}

void normalModeProccess(){
  lcdClear();
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5.0 / 1023.0);  
  displayCurrentVoltage(voltage);

  if(voltage <= lowVoltThreshold && voltage > lowAlarmVolts){
    setProcess("DOWN");
    alarmIsOpened = 0;
  }

  if(voltage >= highVoltThreshold && voltage < highAlarmVolts){
    setProcess("UP");
    alarmIsOpened = 0;
  }

  if(voltage > highAlarmVolts || voltage < lowAlarmVolts){
    alarmProcess();
    displayAlarm();
    alarmIsOpened = 1;
  }

  if(!alarmIsOpened){
    displayHighLowLevels(highVoltThreshold, lowVoltThreshold);
  }
}

void displayAlarm(){
  lcdPrint("     ", 5, 1);
  delay(500);
  lcdPrint("Alarm", 5, 1);
}

void modeTask(int mode){
  if(mode == 0){
    setModeToNormal();
  }else if(mode == 1){
    setModeToHihgLevel();
  }else if(mode == 2){
    setModeToLowLevel();
  }
}

float setHighLevelThreshold(){
  upButtonStage = digitalRead(upButton);
  downButtonStage = digitalRead(downButton);

  if(upButtonStage && !downButtonStage && highVoltThreshold <= 5){
    highVoltThreshold += stepV;
  }else if(!upButtonStage && downButtonStage && highVoltThreshold >= 0){
    highVoltThreshold -= stepV;
  }
  
  return highVoltThreshold;
}

float setLowLevelThreshold(){
  upButtonStage = digitalRead(upButton);
  downButtonStage = digitalRead(downButton);

  if(upButtonStage && !downButtonStage && lowVoltThreshold <= 5){
    lowVoltThreshold += stepV;
  }else if(!upButtonStage && downButtonStage && lowVoltThreshold >= 0){
    lowVoltThreshold -= stepV;
  }
  
  return lowVoltThreshold;
}

void updateHighLevelScreen(float volt){
  highAlarmVolts = highVoltThreshold + tolerance;
  lowAlarmVolts = lowVoltThreshold - tolerance;
  lcdClear();
  lcdPrint("Set HL", 0, 0);
  lcdPrint(String(volt), 12, 0);
  lcdPrint("V", 15, 0);
}

void updateLowLevelScreen(float volt){
  highAlarmVolts = highVoltThreshold + tolerance;
  lowAlarmVolts = lowVoltThreshold - tolerance;
  lcdClear();
  lcdPrint("Set LL", 0, 0);
  lcdPrint(String(volt), 12, 0);
  lcdPrint("V", 15, 0);
}

void setModeToNormal(){
  mode = NORMAL;
//  Serial.println("mode" + String(mode));
}

void setModeToHihgLevel(){
  mode = SETHIGHLEVEL;
//  Serial.println("mode" + String(mode));
}

void setModeToLowLevel(){
  mode = SETLOWLEVEL;
//  Serial.println("mode" + String(mode));
}

MODE getMode(){
  return mode;
}

void displayCurrentVoltage(float currentVoltage){
  lcdPrint(String(currentVoltage) + "V", 11, 0);
}

void displayHighLowLevels(float highLevel, float lowLevel){
  lcdPrint(String(highLevel), 5, 1);
  lcdWrite(0, 9, 1);
  lcdWrite(2, 10, 1);
  lcdPrint(String(lowLevel), 11, 1);
  lcdWrite(1, 15, 1);
}

void lcdWrite(int specialChar, int x, int y){
  lcd.setCursor(x, y);
  lcd.write((uint8_t)specialChar);
}

void lcdPrint(String message, int x, int y){
  lcd.setCursor(x, y);
  lcd.print(message);
}

void lcdClear(){
  lcd.clear();
}

void alarmProcess(){
  setLed(alarmLed, HIGH);
  setLed(lowVoltLed, HIGH);
  setRelay(lowRelay, HIGH);
  setLed(highVoltThreshold, LOW);
  setRelay(lowRelay, LOW);
}

void setProcess(String process){
  if(process.equals("DOWN")){
    setRelay(highRelay, LOW);
    setRelay(lowRelay, HIGH);
    setLed(lowVoltLed, HIGH);
    setLed(highVoltThreshold, LOW);
    setLed(alarmLed, LOW);
  }else if(process.equals("UP")){
    setRelay(lowRelay, LOW);
    setRelay(highRelay, HIGH);
    setLed(highVoltThreshold, HIGH);
    setLed(lowVoltLed, LOW);
    setLed(alarmLed, LOW);
  }
}

void setRelay(int relayId, int statusRelay){
  digitalWrite(relayId, statusRelay);
}

void setLed(int ledId, int statusLed){
  digitalWrite(ledId, statusLed);
}

