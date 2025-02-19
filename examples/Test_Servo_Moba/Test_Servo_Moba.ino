//*****************************************************************************************************
//
// File:      Test_Servo_Moba.ino
// Author:    Aiko Pras
// History:   2025/02/11 
//
// Tested on AVR128DA48 (Curiosity Nano)
// 
// Skeleton sketch of how to use the Servo Moba (Modellbahn) classes.
// This sketch uses the TCA0, as well as the TCA1 class, thus supporting 6 servos in total.
// It is also possible to use only one of these classes, thus limiting the number of servos to 3.
//
// The required amount of program storage is roughly 2,5 KByte. Per servo, 92 Bytes of RAM are needed;
// 56 of these are needed for the buffer that contains the coordinates for the curve.
//
// For details regarding the implementation of the Servo_Moba classes, see ../extras/ServoMoba
//
// Make sure you connect the servo's to supported pins: 
// See: extras/ProcessorsAndPins.md
//
//******************************************************************************************************
#include <Arduino.h>
#include <EEPROM.h>
#include <Servo_TCA0_MoBa.h>
#include <Servo_TCA1_MoBa.h>


ServoMoba  servo0;           // Instantiate the three servo's on TCA0
ServoMoba  servo1;
ServoMoba  servo2;
ServoMoba1  servo3;          // Instantiate another servo on TCA1
ServoMoba1  servo4;          // Instantiate another servo on TCA1
ServoMoba1  servo5;          // Instantiate another servo on TCA1


void makeEepromCurve() {
  EEPROM.update(100,   0);
  EEPROM.update(101,  40);
  EEPROM.update(102,   2);
  EEPROM.update(103, 100);
  EEPROM.update(104,   4);
  EEPROM.update(105, 200);
  EEPROM.update(106,   0);
  EEPROM.update(107,   0);
}


void setup() {
  Serial1.begin(115200);
  delay(1000);
  Serial1.println("");
  Serial1.println("Start");
  
  // ==========================================================
  // The pinMode commands below are for testing purposes.
  pinMode(PIN_PE0, OUTPUT);
  pinMode(PIN_PE1, OUTPUT);
  pinMode(PIN_PE2, OUTPUT);

  pinMode(PIN_PD4, OUTPUT);
  pinMode(PIN_PD5, OUTPUT);
  pinMode(PIN_PD6, OUTPUT);
  pinMode(PIN_PD7, OUTPUT);

  // ==========================================================
  // We may initialise the pulse width before we do the attach.
  servo0.constantOutput(0);
  // servo1.writeMicroseconds(1500);
  // servo2.writeMicroseconds(2000);

  // =========================================================================
  // After the servo has moved, should the servo still receive pulses. If not, 
  // should the signal on the servo's pulse line become high or low?
  // If the servo should not receive pulses while idle, how many pulses must 
  // still be generated before and after the servo movement?
  // The first parameter may take the values continuous, high or low.
  servo0.initPulse(ServoMoba::low, 1, 1);
  // servo1.initPulse(ServoMoba::continuous, 1, 1);
  // servo2.initPulse(ServoMoba::low, 1, 1);

  // ==========================================================================
  // If present, initialise the hardware that controls the servo's 5V power pin
  // initPower has five parameter:
  // - should power be switch off while idle?
  // - what pin is used to switch the servo power 
  // - is a HIGH or LOW signal needed to enable the 5V?
  // - number of 20ms ticks the power switches on before pulse start
  // - number of 20ms ticks the power remains on after pulse end
  servo0.initPower(true, PIN_PE2, HIGH, 1, 1);
  // servo1.initPower(false, PIN_PB1, HIGH, 0, 0);
  // servo2.initPower(true, PIN_PB2, LOW, 0, 0);

  // ===============================================
  // Attach the (upto) 3 servo's that are using TCA0
  servo0.attach(PIN_PE0);
  // servo1.attach(PIN_PE1);
  // servo2.attach(PIN_PE2);

  // ======================================================
  // set (in us) the treshold values for the servo movement
  servo0.setTreshold1(1000);
  servo0.setTreshold2(2000);

  // ======================================================
  // Load a curve from PROGMEM or EEPROM
  // makeEepromCurve();
  // servo0.initCurveFromEEPROM(0, 0, 100);
  servo0.initCurveFromPROGMEM(0, 2);
  // servo0.printCurve();  
  // servo0.moveServoAlongCurve(1);
}


// Parameters for the main loop
long lastMillis;
uint8_t dir;

void loop() { 
  // digitalWriteFast(CPU_Pin, 1);
  // digitalWriteFast(CPU_Pin, 0);
  if ((millis() - lastMillis) > 2500) {
    lastMillis = millis();
    Serial1.println("");
    servo0.moveServoAlongCurve(dir);
    if (dir > 0) dir = 0; 
      else dir = 1;
  }
  servo0.checkServo();
  // servo1.checkServo();
  // servo2.checkServo();
}
