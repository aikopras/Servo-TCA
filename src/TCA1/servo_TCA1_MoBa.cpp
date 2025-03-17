//******************************************************************************************************
//
// file:      servo_TCA1_MoBa.cpp
// author:    Aiko Pras
// history:   2025-02-22 V1.0.0 ap initial version
//
// purpose:   Class for MoBa Servo's. Inherits from Servo.
//            To be used in conjunction with the TCA1 timer
//
//
//******************************************************************************************************
#include <Arduino.h>
#include <EEPROM.h>
#include "../Servo_TCA1_MoBa.h"
#include "../TCA_MobaCurves/curves.h"

#define IDLE_ON    digitalWriteFast(PIN_PD4,1); digitalWriteFast(PIN_PD5,0); digitalWriteFast(PIN_PD6,0); digitalWriteFast(PIN_PD7,0);
#define START_ON   digitalWriteFast(PIN_PD4,0); digitalWriteFast(PIN_PD5,1); digitalWriteFast(PIN_PD6,0); digitalWriteFast(PIN_PD7,0);
#define MOVING_ON  digitalWriteFast(PIN_PD4,0); digitalWriteFast(PIN_PD5,0); digitalWriteFast(PIN_PD6,1); digitalWriteFast(PIN_PD7,0);
#define FINISH_ON  digitalWriteFast(PIN_PD4,0); digitalWriteFast(PIN_PD5,0); digitalWriteFast(PIN_PD6,0); digitalWriteFast(PIN_PD7,1);

//******************************************************************************************************
// moveServoAlongCurve() moves the servo along the curve that has been stored in the myCurve array.
// It has one parameter: direction.
// 
// Before the first servo movement, or of we want to change to a different curve, we have to fill the 
// myCurve array, and set the timeStretch factor. The myCurve array can be filled from EEPROM,
// or from PROGMEM / RAM.
//
// To fill from EEPROM, we have to call initCurveFromEEPROM(). That call has two parameters:
// - the EEPROM address of the first curve point
// - the timeStretch factor.
// 
// To fill from PROGMEM / RAM, we have to call initCurveFromPROGMEM(). That call has two parameters:
// - the index to identify the curve. The predefined curves can be found in curves.cpp
// - the timeStretch factor.
//******************************************************************************************************
void ServoMoba1::moveServoAlongCurve(uint8_t direction) {
  servoDirection = direction;
  if (powerOnBeforeMoving >= pulseOnBeforeMoving) {
    countServo = powerOnBeforeMoving;
    countPulse = powerOnBeforeMoving - pulseOnBeforeMoving;
    countPower = 0;
  }
  else {
    countServo = pulseOnBeforeMoving;
    countPulse = 0;
    countPower = pulseOnBeforeMoving - powerOnBeforeMoving;
  };
  ticks = 0;
  index = 0;
  servoState = start;
  servoStart();
};


void ServoMoba1::initCurveFromEEPROM(uint8_t curveNumber, uint8_t stretch, int adresEeprom) {
  if (stretch > 0) timeStretch = stretch;
    else timeStretch = 1;
  uint8_t i = 0;                                 // index into the myCurve array
  bool ready = false;
  do {
    myCurve[i].time = EEPROM.read(adresEeprom);
    myCurve[i].position = EEPROM.read(adresEeprom + 1);
    adresEeprom = adresEeprom + 2;
    if ((i > 0) && (myCurve[i].time == 0)) ready = true;
    if (i >= SIZE_SERVO_CURVE) ready = true; // protection against erronuous CV values 
    i++;
  } while (!ready);                          // i is now 2 above the last curve element
  firstCurvePosition = valueTo_us(myCurve[0].position);
  lastCurvePosition = valueTo_us(myCurve[i-2].position); 
  previousCurve = curveNumber;
}


void ServoMoba1::initCurveFromPROGMEM(uint8_t curveNumber, uint8_t stretch) {
  if (stretch > 0) timeStretch = stretch;
    else timeStretch = 1;
  uint8_t arrayIndex = curveNumber & 0b00111111;
  if (curveNumber <= NUMBER_OF_LAST_CURVE) {
    const curvePoint_t *src = PredefinedCurves[arrayIndex];  
    uint8_t i = 0;
    bool ready = false;
    do {
      myCurve[i].time = lookupTime;            // is a #define
      myCurve[i].position = lookupPosition;    // is a #define
      if ((i > 0) && (myCurve[i].time == 0)) ready = true;
      src++;
      i++;
    } while (!ready);                          // i is now 2 above the last curve element
    firstCurvePosition = valueTo_us(myCurve[0].position);
    lastCurvePosition = valueTo_us(myCurve[i-2].position); 
    previousCurve = curveNumber;
  }
}


//******************************************************************************************************
// Must be called from the main loop as frequent as possible
//******************************************************************************************************
void ServoMoba1::checkServo() {
  // If 20 ms have passed, we may switch on/off power, or do something that is 
  // specific for the state we are in. 
  if (acceptsNewValue()) {                     // A pulse has just been initialised
    if (PowerOnNextTick) powerOn();
    if (PowerOffNextTick) powerOff();  
    if (PulseOffNextTick) pulseOff();
    switch (servoState) {
      case idle: servoIdle();
      break;
      case start: servoStart();
      break;
      case moving: servoMoving();
      break;
      case finish: servoFinish();
      break;
    };
    waitTillNextPulse();                       // check again in 20 ms from now.
  };
}


//******************************************************************************************************
// Internal subroutines, one for each of the four possible states
//******************************************************************************************************
void ServoMoba1::servoIdle() {
//  IDLE_ON;                                                          // TIJDELIJK
};


void ServoMoba1::servoStart() {
//  START_ON;                                                         // TIJDELIJK
  if (countPulse > 0) countPulse--;
    else writeMicroseconds(lastPulseWidth);
  if (countPower > 0) countPower--; 
    else if (idlePowerIsOff) PowerOnNextTick = true;  // switch power on
  if (countServo > 0) countServo--;
    else {
      servoState = moving;
      servoMoving();    
    }
}


void ServoMoba1::servoMoving() {
//  MOVING_ON;
  if ((myCurve[index].time * timeStretch) == (ticks)) {  // new segment
    index++;
    fillSegment(index);
    // Serial1.printf("%u: (%u, %u) => (%u, %u)\n",index, segment.xFrom, segment.yFrom, segment.xTo, segment.yTo);
  }
  lastPulseWidth = positionIn_us(ticks);
  writeMicroseconds(lastPulseWidth);
  // Serial1.printf(" %u: %u us\n",ticks, lastPulseWidth);
  ticks++;
  if ((index > 0) && (myCurve[index].time == 0)) {          // we have had all segments
    countPulse = pulseOffAfterMoving;
    countPower = powerOffAfterMoving;
    servoState = finish;
    servoFinish();
  };
};


void ServoMoba1::servoFinish() {
//  FINISH_ON;
  bool move2idle = ((countPulse == 0) && (countPower == 0));
  if (countPulse > 0) countPulse--;
    else PulseOffNextTick = true;
  if (countPower > 0) countPower--;
    else {
      if (idlePowerIsOff) PowerOffNextTick = true;
    }
  if (move2idle) {
    servoState = idle;
    servoIdle();
  }
};


//******************************************************************************************************
// Support methods for the moving state
//******************************************************************************************************
// Create the X-coordinate. Mapping function for within individual segments:
uint16_t ServoMoba1::positionIn_us(uint16_t xValue) {
  return ((xValue - segment.xFrom) * (long)(segment.yDelta) / segment.xDelta + segment.yFrom);
};

// Create the Y-coordinate. Mapping function can be used for all X-coordinates
// Treshold1 and treshold2 should be signed integers, to allow their difference to be negative
uint16_t ServoMoba1::valueTo_us(uint8_t yValue) {
  if (servoDirection == 0) return (yValue * (long)(treshold2 - treshold1) / 255 + treshold1);
  else return (yValue * (long)(treshold1 - treshold2) / 255 + treshold2);
};

// Fill the segment values for a given index
void ServoMoba1::fillSegment(uint8_t index){
  segment.index = index;
  segment.xFrom = myCurve[index-1].time * timeStretch;
  segment.xTo = myCurve[index].time * timeStretch;
  segment.xDelta = segment.xTo - segment.xFrom;
  segment.yFrom = valueTo_us(myCurve[index-1].position);
  segment.yTo = valueTo_us(myCurve[index].position);
  segment.yDelta = segment.yTo - segment.yFrom;
}


//******************************************************************************************************
// Initialisation
//******************************************************************************************************
ServoMoba1::ServoMoba1() {
  servoState = idle;
  timeStretch = 1;
  treshold1 = 1400;
  treshold2 = 1600;
  // Pulse specific attributes
  idlePulseDefault = continuous;
  pulseOnBeforeMoving = 0;
  pulseOffAfterMoving = 0;
  // Power specific attributes
  powerOnBeforeMoving = 0;
  powerOffAfterMoving = 0;
  idlePowerIsOff = false;
  PowerOnNextTick = false;
  PowerOffNextTick = false;
  // counters
  countServo = 0;
  countPulse = 0;
  countPower = 0;
}

//******************************************************************************************************
void ServoMoba1::initPulse(
    idlePulseDefault_t idleDefault,    // low, high or continuous
    uint8_t pulseBeforeMoving,         // 0.255. Steps are in 20 ms
    uint8_t pulseAfterMoving)          // 0.255. Steps are in 20 ms
  {  
  idlePulseDefault = idleDefault;
  if ((idleDefault == high) && (pulseBeforeMoving < 255)) pulseBeforeMoving++;
  pulseOnBeforeMoving = pulseBeforeMoving;
  pulseOffAfterMoving = pulseAfterMoving;
}

//******************************************************************************************************
void ServoMoba1::initPower(
  boolean idleDefault,               // should power be switch off while idle?
  uint8_t pin,                       // the pin used to switch the servo power
  boolean enableValue,               // power enable is HIGH or LOW?
  uint8_t stepsBeforeMoving,         // 0.255. Steps are in 20 ms
  uint8_t stepsAfterMoving) {        // 0.255. Steps are in 20 ms
  idlePowerIsOff = idleDefault;
  powerEnablePin = pin;
  powerEnableValue = enableValue;
  if (idlePowerIsOff) {
    if (stepsAfterMoving < 255) stepsAfterMoving++;
    powerOnBeforeMoving = stepsBeforeMoving;
    powerOffAfterMoving = stepsAfterMoving;
  }
  else {
    powerOnBeforeMoving = 0;
    powerOffAfterMoving = 0;
  }
  // If this method is called, we have hardware to switch the servo 5V power on/off
  pinMode(powerEnablePin, OUTPUT);
  // Set the 5V power to the desired value
  if (idlePowerIsOff) digitalWrite(powerEnablePin, !powerEnableValue);
    else digitalWrite(powerEnablePin, powerEnableValue);
}

void ServoMoba1::powerOn() {
  digitalWrite(powerEnablePin, powerEnableValue);
  PowerOnNextTick = false;
}

void ServoMoba1::powerOff() {
  digitalWrite(powerEnablePin, !powerEnableValue);
  PowerOffNextTick = false;
}

void ServoMoba1::pulseOff() {
  if (idlePulseDefault == low) constantOutput(0);
  if (idlePulseDefault == high) constantOutput(1);
  PulseOffNextTick = false;
}

//******************************************************************************************************
void ServoMoba1::setTreshold1(uint16_t value) {
  treshold1 = (int16_t)value;
}

void ServoMoba1::setTreshold2(uint16_t value) {
  treshold2 = (int16_t)value;
}

uint16_t ServoMoba1::getTreshold1() {
  return (uint16_t)treshold1;
}

uint16_t ServoMoba1::getTreshold2() {
  return (uint16_t)treshold2;
}

uint16_t ServoMoba1::getFirstCurvePosition() {
  return firstCurvePosition;
}

uint16_t ServoMoba1::getLastCurvePosition() {
  return lastCurvePosition;
}


//======================================================================================================
// Code for debugging and testing
//======================================================================================================
void ServoMoba1::printCurve() {
  uint8_t i = 0;
  bool ready = false;
  do {
    Serial1.print("Time: ");
    Serial1.print(myCurve[i].time);
    Serial1.print(" - Position: ");
    Serial1.println(myCurve[i].position);
    if ((i > 0) && (myCurve[i].time == 0)) ready = true;
    i++;
  } while (!ready);
}
