//*****************************************************************************************************
//
// File:      Test_TCA1.ino
// Author:    Aiko Pras
// History:   2025/01/15 
//
// Tested on an AVR128DA48 and AVR64EA48 processors
// 
// Test to use the 3 servo's that are connected to TCA1. 
//
// Requires a DxCore processor that has TCA1: thus a processor with 48 or 64 pins
//
// Make sure you connect the servo's to supported pins: 
// See: extras/ProcessorsAndPins.md
//
//******************************************************************************************************
#include <Arduino.h>
#include <servo_TCA1.h>      // For objects of the servo1 class (TCA1)

#define CPU_Pin PIN_PD3      // To give the main loop something to do. 

Servo1 servo0;               // Instantiate the three servo's on TCA1
Servo1 servo1;
Servo1 servo2;


void setup() {
  Serial.begin(115200);
  Serial.println(""); 
  Serial.println("Test program for 3 servo's connected to TCA1"); 
  
  // We may initialise the pulse width before we do the attach.
  servo0.writeMicroseconds(1000);
  servo1.writeMicroseconds(1500);
  servo2.writeMicroseconds(2000);
 
  // Attach the 3 servo's that are connected to TCA1  
  servo0.attach(PIN_PB0);
  servo1.attach(PIN_PB1);
  servo2.attach(PIN_PB2);

  pinMode(CPU_Pin, OUTPUT);
}


void loop() { 
  digitalWriteFast(CPU_Pin, 1);
  digitalWriteFast(CPU_Pin, 0);
}
