#include <Arduino.h>
#include <PID_v1.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

//LCD pin to Arduino
#define pin_RS 8
#define pin_EN 9 
#define pin_d4 4 
#define pin_d5 5 
#define pin_d6 6 
#define pin_d7 7 
#define pin_BL 10 
LiquidCrystal lcd(pin_RS,  pin_EN,  pin_d4,  pin_d5,  pin_d6,  pin_d7);

#define failMax 7 // Fail time length in seconds
#define pumpMax 200 //Max 255

#define pumpPin 11
#define sensorPin A1
#define keyPin A0
#define pressureOut 3

unsigned long nxtime, failtime;
unsigned long sumSensor, sensorVal;
float pressureValue;
int sensorRead;
int brightness;

float psiLimit = 43.0;

int viewMode = 0;

int readKey, keyStay, keyCnt;

byte fullBlk[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

byte emptyBlk[] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111
};

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=2, Ki=5, Kd=1;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void rendBar()
{
  for(int i = 0; i < 16; i++)
  {
    lcd.setCursor(i,1);
    if((brightness/16)+1 > i)
      lcd.write((int)0);
    else
      lcd.write((int)1);
  }
}

void viewmode(int mode)
{
  switch(mode) {
  case 0:   // Main view
  viewMode = 0;
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print("Raw");
  lcd.setCursor(13,0);
  lcd.print("PSI");
  lcd.setCursor(8,1);
  lcd.print("*");
  lcd.setCursor(9,1);
  lcd.print(psiLimit);
  lcd.setCursor(13,1);
  lcd.print("PSI");
  break;
  case 1:   // Brightness
  lcd.clear();
  viewMode = 1;
  lcd.setCursor(3,0);
  lcd.print("Brightness");
  rendBar();
  break;
  case 2:   // Default PSI
  lcd.clear();
  viewMode = 2;
  lcd.setCursor(3,0);
  lcd.print("Target PSI");
  lcd.setCursor(6,1);
  lcd.print(psiLimit);
  break;
  }
}

void keymode(int key)
{
  switch(key) {
  case 0:   // Menu
  if(viewMode==0)
    viewmode(1);
  else if(viewMode==1)
  {
    EEPROM.update(1,brightness);
    viewmode(2);
  }
  else if(viewMode==2)
  {
    float storedPSI;
    EEPROM.get(2, storedPSI);
    if(psiLimit != storedPSI)
    {
      Setpoint = psiLimit;
      EEPROM.put(2, psiLimit);
    }
    viewmode(0);
  }
  
  break;
  case 1:   // UP
  if(viewMode==2)
  {
    if(psiLimit+0.5 <= 60)
    {
      psiLimit = psiLimit + 0.5;
      lcd.setCursor(6,1);
      lcd.print(psiLimit);
    }
  }
  break;
  case 2:   // DOWN
  if(viewMode==2)
  {
    Serial.println("DOWN");
    if(psiLimit+0.5 > 0)
    {
      psiLimit = psiLimit - 0.5;
      lcd.setCursor(6,1);
      lcd.print(psiLimit);
    }
  }
  break;
  case 3:   // LEFT
  if(viewMode==1)
  {
    if(brightness > 16)
      brightness = brightness - 16;
     rendBar();
     analogWrite(pin_BL, brightness);
  }
  break;
  case 4:   // RIGHT
  if(viewMode==1)
  {
    if(brightness < 255)
      brightness = brightness + 16;
     rendBar();
     analogWrite(pin_BL, brightness);
  }
  break;
  }
}

void keyDelay(int key) {
  if(keyStay==key)
      keyCnt++;
  else
   keyCnt = 0;
  keyStay = key;
  if(keyCnt >300)
  {
   keyCnt = 0;
   keymode(key);
  }
}

/*EEPROM state
 * 0 = status (1=booted)
 * 1 = brightness
 * 2 = PSI store
 */

void setup(void) {
  Serial.begin(115200);    
  pinMode(pumpPin,OUTPUT);
  pinMode(pressureOut,OUTPUT);
  pinMode(pin_BL,OUTPUT);
  //analogReference(INTERNAL);

  if(analogRead(keyPin) < 60) // Press right while boot enters override mode, pump output 120 all the time
  {
    analogWrite(pumpPin, 130);
    Serial.println("Overrided mode");
    analogWrite(pin_BL, 255);
    lcd.setCursor(0,0);
    lcd.print("Overrided mode");
    while(1);
  }

  if(EEPROM.read(0)!=1){  //Initialize first time data
    EEPROM.write(0,1);
    EEPROM.write(1,255);
    EEPROM.put(2,43.0);
  }

  brightness = EEPROM.read(1);
  EEPROM.get(2, psiLimit);

  Setpoint = psiLimit;
  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0,pumpMax);

  for(int i = 0; i < 41; i++) // Fill in first 31 reading
  {
    sensorRead = analogRead(sensorPin);
    if(i >= 10) // Dump first 10 readings
      sumSensor =+ sumSensor;
  }

  //brightness = 255;
  analogWrite(pin_BL, brightness);
  

  lcd.begin(16, 2);

  lcd.createChar(0, fullBlk);
  lcd.createChar(1, emptyBlk);

  viewmode(0);
}

char sVal[4];

void showPSI(void) {
  if(millis()>nxtime && viewMode==0)
  { 
    itoa(sensorVal, sVal, 10);
    if(sensorVal/1000 == 1)
    {
      lcd.setCursor(0,0);
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print("0");
      lcd.setCursor(1,0);
      if(sensorVal/100 == 0)
      {
        lcd.print("0");
        lcd.setCursor(2,0);
      }
    }
    lcd.print(sVal);

    lcd.setCursor(9,0);
    lcd.print(pressureValue, 1);

    lcd.setCursor(0,1);
    lcd.print(Output,2);
    
    nxtime = millis()+1000;
  }
}

int failCnt = 0;

void loop(void) {
  sensorRead = analogRead(sensorPin);
  //analogWrite(pressureOut, map(sensorRead,0,1023,0,255));
  sumSensor = sumSensor + sensorRead - sensorVal;
  sensorVal = sumSensor / 32;

  if(sensorVal>=100)
    pressureValue = ((sensorVal-100)*100)/800.0; 
    //pressureValue = map(sensorVal, 100, 800.0, 0, 100);
  else
    pressureValue = 0;
  //if(pressureValue > psiLimit)
  //  pressureValue = psiLimit;
  if(pressureValue > 100)
    pressureValue = 99.9;

  analogWrite(pressureOut, map(pressureValue,0,100,0,255));
  Input = pressureValue;
  myPID.Compute();
  analogWrite(pumpPin, Output);
  //analogWrite(pumpPin, 130);

  if(millis()>failtime)
  {
    if(Output==pumpMax)
    {
      failCnt++;
      //if(failCnt>1000)
      Serial.println(failCnt);
      if(failCnt >= failMax)
      {
        analogWrite(pumpPin, 0);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Check pressure!");
        exit(0);
      }
      failtime = millis()+1000;
    }
    else
      failCnt = 0;
  }

  showPSI();

  readKey = analogRead (keyPin);
  lcd.setCursor(10,1);
  if (readKey < 60) {
    keyDelay(4);
  }
  else if (readKey < 200) {
    keyDelay(1);
  }
  else if (readKey < 400){
    keyDelay(2);
  }
  else if (readKey < 600){
    keyDelay(3);
  }
  else if (readKey < 800){
    keyDelay(0);
  }
}
