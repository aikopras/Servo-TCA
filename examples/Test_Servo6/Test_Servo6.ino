//*****************************************************************************************************
//
// File:      Test_Servo6.ino
// Author:    Aiko Pras
// History:   2026/11/16 V2.0.0
//
// Test to use six servo's with the Servo6 class. A Servo6 object uses TCA0 or TCA1, depending
// on the pin that is given to attach().
//
// The pins below are those of the AVR Servo decoder V2.3 (AVR32DA48):
// - servo 1..3: PC4, PC5, PC6 (these pins only fit TCA1)
// - servo 4..6: PF0, PF1, PF2 (these pins only fit TCA0)
// TCA1 is started first (servo 1), and TCA0 runs half a period (20/6 ms) behind TCA1.
// With a logic analyser it can be checked that pulses of TCA0 and TCA1 never start at the same time.
// The servo enable (power) pins are not used by this sketch.
//
// Requires a DxCore processor that has TCA1: thus a processor with 48 or 64 pins
//
// Make sure you connect the servo's to supported pins:
// See: extras/ProcessorsAndPins.md
//
//******************************************************************************************************
#include <Arduino.h>
#include <Servo_TCA.h>       // For objects of the Servo6 class

#define CPU_Pin PIN_PD7      // To give the main loop something to do.

Servo6 servo[6];             // Instantiate six servo's. The pin determines the TCA timer

const uint8_t servoPin[6] = {PIN_PC4, PIN_PC5, PIN_PC6, PIN_PF0, PIN_PF1, PIN_PF2};


void setup() {
  Serial1.begin(115200);     // On the AVR Servo decoder, the serial monitor is connected to Serial1
  Serial1.println("");
  Serial1.println("Test program for 6 servo's using the Servo6 class (TCA0 and TCA1)");

  // We may initialise the pulse width before we do the attach.
  for (uint8_t i = 0; i < 6; i++) servo[i].writeMicroseconds(1000 + (i * 200));

  // Attach the servo's. The result is the servoIndex, or INVALID_SERVO if attach failed
  for (uint8_t i = 0; i < 6; i++) {
    uint8_t result = servo[i].attach(servoPin[i]);
    Serial1.print("Servo ");
    Serial1.print(i + 1);
    if (result == INVALID_SERVO) Serial1.println(": attach failed");
      else Serial1.println(": attached");
  }

  pinMode(CPU_Pin, OUTPUT);
}


void loop() {
  digitalWriteFast(CPU_Pin, 1);
  digitalWriteFast(CPU_Pin, 0);
}
