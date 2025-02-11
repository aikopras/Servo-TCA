//******************************************************************************************************
//
// file:      Servo_TCA1_MoBa.h
// author:    Aiko Pras
// history:   2025-02-22 V1.0.0 ap initial version
//
// purpose:   Class for MoBa Servo's. Inherits from Servo.
//            To be used in conjunction with the TCA1 timer
//
// There are two, nearly identical classes: ServoMoba and ServoMoba1
// - ServoMoba uses the TCA0 timer
// - ServoMoba1 uses the TCA1 timer
// The user sketch may include one or both header files for these classes, and instantiate objects
// from one or both of these classes.
// 
//******************************************************************************************************
#pragma once
#include "Servo_TCA1.h"
#include "TCA_MobaCurves/curves.h"

class ServoMoba1: public Servo1 {

  public:
    enum idlePulseDefault_t {                      // default servo signal in the idle state 
      low,  
      high,
      continuous
    };

    ServoMoba1();                                   // constructor
    void checkServo();                             // Must be called as often as possible from the main loop 

    void moveServoAlongCurve(uint8_t direction);   // Start moving along the path selected with initCurve
    
    void initCurveFromEEPROM(                      // use a predefined curve from EEPROM
      int adresEeprom,                             // The starting address in EEPRROM of this curve
      uint8_t timeStretch                          // 1..255
    );
    
    void initCurveFromPROGMEM(                     // use a predefined curve from PROGMEM
      uint8_t indexCurve,                          // See curves.cpp for possible curves
      uint8_t timeStretch                          // 1..255
    );
    
    void initPulse(                                // What to do with the servo puls signal in idle state?
      idlePulseDefault_t idleDefault,              // low, high or continuous
      uint8_t pulseBeforeMoving,                   // 0.255. Steps are in 20 ms
      uint8_t pulseAfterMoving                     // 0.255. Steps are in 20 ms
    );
    
    void initPower(                                // What to do with the servo power signal in idle state?
      boolean idlePowerIsOff,                      // should power be switch off while idle?
      uint8_t powerEnablePin,                      // the pin used to switch the servo power on and off
      boolean powerEnableValue,                    // does the power enable hardware require a HIGH or LOW signal?
      uint8_t powerOnBeforeMoving,                 // 0.255. Steps are in 20 ms
      uint8_t powerOffAfterMoving                  // 0.255. Steps are in 20 ms
    );
    
    void setTreshold1(uint16_t value);             // Treshold1 can be higher or lower than Treshold2 
    void setTreshold2(uint16_t value);             // Value in us.
    uint16_t getTreshold1();                       // returns Treshold1 
    uint16_t getTreshold2();                       // returns Treshold2


    void printCurve();                             // May be used for testing. Uses Serial1

  //====================================================================================================
  private:
    typedef struct {                               // A segment is built from a pair of curve points
      uint8_t index;
      int16_t xFrom;
      int16_t xTo;
      int16_t xDelta;
      int16_t yFrom;
      int16_t yTo;
      int16_t yDelta;
    } segment_t;

    segment_t segment;
    
    enum state_t {                                 // the states the servo may be in
      idle,
      start,
      moving,
      finish
    } servoState;

    void servoIdle();                              // Actions to be perfomed while in the idle phase
    void servoStart();                             // Actions to be perfomed while in the start phase
    void servoMoving();                            // Actions to be perfomed while in the moving phase
    void servoFinish();                            // Actions to be perfomed while in the finish phase

    // Moving state: Curves for possible servo movements
    curvePoint_t myCurve[SIZE_SERVO_CURVE];        // myCurve is the destination Array
    void fillSegment(uint8_t index);               // initialises the variable for the next segment
 
    // Moving state: pulsewidth must always stay between these treshold values (in us) 
    int16_t treshold1;                            // Servo may not move beyound this treshold (signed integer!)
    int16_t treshold2;                            // Servo may not move beyound this treshold (signed integer!)

    // Moving state: methods and attributes to control the movement along the curve
    uint16_t positionIn_us(uint16_t xValue);       // Mapping function for the X-axis
    uint16_t valueTo_us(uint8_t yValue);           // Mapping function for the Y-axis
    uint16_t ticks;                                // Curve time. One tick every 20us
    uint8_t timeStretch;                           // 1..255: will be multiplied to ticks (=> X-coordinate)
    uint8_t index;                                 // Which segment of the curve is this?
    uint16_t lastPulseWidth;                       // Current / previous pulse time in us (=> Y-coordinate)
    uint8_t servoDirection;                        // Used in valueTo_us to change both tresholds
    
    // Idle state: what happens at the servo pulse line?
    idlePulseDefault_t idlePulseDefault;           // low, high or continuous
    uint8_t pulseOnBeforeMoving;                   // 0.255. Steps are in 20 ms
    uint8_t pulseOffAfterMoving;                   // 0.255. Steps are in 20 ms
    bool PulseOffNextTick;                         // Flag to stop the pulses, change in 20ms
    void pulseOff();                               // Stop the servo pulses

    // Idle state: what happens at the servo power line?
    uint8_t powerEnablePin;                        // Pin to enable / disable the servo power
    uint8_t powerEnableValue;                      // 0: Enable with LOW, otherwise: HIGH 
    uint8_t powerOnBeforeMoving;                   // 0.255. Steps are in 20 ms
    uint8_t powerOffAfterMoving;                   // 0.255. Steps are in 20 ms
    bool idlePowerIsOff;                           // If true, power will be switch off while idle
    bool PowerOnNextTick;                          // Flag for power switch pin, change in 20ms
    bool PowerOffNextTick;                         // Flag for power switch pin, change in 20ms
    void powerOn();                                // Switch power now
    void powerOff();                               // Switch power now

    // Internal counters for the start and finish states
    uint8_t countServo;
    uint8_t countPulse;
    uint8_t countPower;

   
};
