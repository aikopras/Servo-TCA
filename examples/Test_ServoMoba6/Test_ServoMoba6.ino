//*****************************************************************************************************
//
// File:      Test_ServoMoba6.ino
// Author:    Aiko Pras
// History:   2026/11/16 V2.0.0
//
// Skeleton sketch for six ServoMoba6 servo's, that all move at the same time along the same curve.
// A ServoMoba6 object uses TCA0 or TCA1, depending on the pin that is given to attach().
//
// The pins below are those of the AVR Servo decoder V2.3 (AVR32DA48):
// - servo 1..3: pulse PC4, PC5, PC6 (TCA1) / power enable PD2, PD3, PD4
// - servo 4..6: pulse PF0, PF1, PF2 (TCA0) / power enable PF3, PF4, PF5
// TCA1 is started first (servo 1), and TCA0 runs half a period (20/6 ms) behind TCA1. Therefore
// the pulses for servo 1..3 and servo 4..6 never start at the same time.
//
// The servo power stays on, also between servo movements (first parameter of initPower() is false).
// Measurements on the AVR Servo decoder boards showed that switching the servo power on and off
// via the power switch ICs may disturb the power supply.
//
// Requires a DxCore processor that has TCA1: thus a processor with 48 or 64 pins
//
// For details regarding the implementation of the Servo_Moba classes, see ../extras/ServoMoba
//
// Make sure you connect the servo's to supported pins:
// See: extras/ProcessorsAndPins.md
//
//******************************************************************************************************
#include <Arduino.h>
#include <Servo_TCA_MoBa.h>  // For objects of the ServoMoba6 class

#define CURVE                 2          // move_A (see src/TCA_MobaCurves/curves.cpp)
#define CURVE_STRETCH         6          // 1..255. Multiplies the curve's time by this number
#define TRESHOLD1             1300       // in us
#define TRESHOLD2             1700       // in us
#define INITIAL_PULSE_WIDTH   1500       // Used by initPulse for the first pulses after startup

ServoMoba6 servo[6];                     // Instantiate six servo's. The pin determines the TCA timer

const uint8_t servoPin[6]       = {PIN_PC4, PIN_PC5, PIN_PC6, PIN_PF0, PIN_PF1, PIN_PF2};
const uint8_t servoEnablePin[6] = {PIN_PD2, PIN_PD3, PIN_PD4, PIN_PF3, PIN_PF4, PIN_PF5};


void setup() {
  for (uint8_t i = 0; i < 6; i++) {
    // Set both tresholds for the servo movements, and load the curve
    servo[i].setTreshold1(TRESHOLD1);
    servo[i].setTreshold2(TRESHOLD2);
    servo[i].initCurveFromPROGMEM(CURVE, CURVE_STRETCH);
    // Pulse line: low between movements, 0 extra pulses before and 10 extra pulses after moving
    servo[i].initPulse(LOW, 0, 10, INITIAL_PULSE_WIDTH);
    // Power line: power stays on (also while idle), enable with a HIGH signal
    servo[i].initPower(false, servoEnablePin[i], HIGH, 0, 0);
    servo[i].attach(servoPin[i]);
  }
}


// Parameters for the main loop
unsigned long lastMillis;
uint8_t dir;

void loop() {
  if ((millis() - lastMillis) > 3000) {
    lastMillis = millis();
    for (uint8_t i = 0; i < 6; i++) servo[i].moveServoAlongCurve(dir);
    if (dir > 0) dir = 0;
      else dir = 1;
  }
  for (uint8_t i = 0; i < 6; i++) servo[i].checkServo();
}
