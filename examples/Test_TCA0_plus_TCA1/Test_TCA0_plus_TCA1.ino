//*****************************************************************************************************
//
// File:      Test_TCA0_plus_TCA1.ino
// Author:    Aiko Pras
// History:   2025/01/15 
//
// Tested on an AVR128DA48 and AVR64EA48 processors
// 
// Test to use all 6 servo's. 
//
// Requires a DxCore processor that has TCA1: thus a processor with 48 or 64 pins
//
// Make sure you connect the servo's to supported pins: 
// See: extras/ProcessorsAndPins.md
//
//******************************************************************************************************
#include <Arduino.h>
#include <servo_TCA0.h>      // For objects of the servo class (TCA0)
#include <servo_TCA1.h>      // For objects of the servo1 class (TCA1)

#define CPU_Pin PIN_PD7      // To give the main loop something to do. 

Servo  servo0;               // Instantiate the first group of three servo's on TCA0
Servo  servo1;
Servo  servo2;
Servo1 servo3;               // Instantiate the second group of three servo's on TCA1
Servo1 servo4;
Servo1 servo5;


void setup() {
  Serial.begin(115200);
  Serial.println(""); 
  Serial.println("Test program for 6 servo's: 3 using TCA0 and 3 using TCA1"); 
  
  // We may initialise the pulse width before we do the attach.
  servo0.writeMicroseconds(1000);
  servo1.writeMicroseconds(1200);
  servo2.writeMicroseconds(1400);
  servo3.writeMicroseconds(1600);
  servo4.writeMicroseconds(1800);
  servo5.writeMicroseconds(2000);

  // Attach the first 3 servo's (connected to TCA0)
  servo0.attach(PIN_PB0);
  servo1.attach(PIN_PB1);
  servo2.attach(PIN_PB2);

  // Wait 2ms, to ensure there is a time difference between the pulses for servos 0..2,
  // and servos 3..5. This difference may be useful to limit the maximum current draw.
  delay(2);

  // Attach the second 3 servo's (connected to TCA1)
  servo3.attach(PIN_PC4);
  servo4.attach(PIN_PC5);
  servo5.attach(PIN_PC6);

  pinMode(CPU_Pin, OUTPUT);
}


void loop() { 
  digitalWriteFast(CPU_Pin, 1);
  digitalWriteFast(CPU_Pin, 0);
}
