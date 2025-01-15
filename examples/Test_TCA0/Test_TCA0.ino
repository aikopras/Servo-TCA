//*****************************************************************************************************
//
// File:      Test_TCA0.ino
// Author:    Aiko Pras
// History:   2025/01/15 
//
// Tested on AVR128DA48, AVR64EA48, AVR64DD32, ATtiny1607, ATtiny3217, ATtiny1627, 4809
// 
// Test to use three servo's that are connected to TCA0. 
//
// Make sure you connect the servo's to supported pins: 
// See: extras/ProcessorsAndPins.md
//
//******************************************************************************************************
#include <Arduino.h>
#include <servo_TCA0.h>      // For objects of the servo class (TCA0)

#define CPU_Pin PIN_PA7      // To give the main loop something to do. 

Servo  servo0;               // Instantiate the three servo's on TCA0
Servo  servo1;
Servo  servo2;


void setup() {
  // We may initialise the pulse width before we do the attach.
  servo0.writeMicroseconds(1000);
  servo1.writeMicroseconds(1500);
  servo2.writeMicroseconds(2000);

  // Attach the 3 servo's that are using TCA0)
  servo0.attach(PIN_PF0);
  servo1.attach(PIN_PF1);
  servo2.attach(PIN_PF2);

  pinMode(CPU_Pin, OUTPUT);        
}


void loop() { 
  digitalWriteFast(CPU_Pin, 1);
  digitalWriteFast(CPU_Pin, 0);
}
