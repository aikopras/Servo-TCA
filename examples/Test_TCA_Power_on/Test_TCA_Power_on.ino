//*****************************************************************************************************
//
// File:      Test_TCA_Power_On.ino
// Author:    Aiko Pras
// History:   2025/04/26
//
// Different servos behave differently when power is switched on. When power is switched on, 
// many make abrupt short movements. One way to avoid / limit these startup movements, is to ensure 
// that the pulse line is high when the power s swich on. To achieve this, constantOutput(1) should
// be called before the servo is attached and the power is supplied to the servo.
//
// You can test this startup behaviour via this script: 
// - set STARTUP_LEVEL to 0, to give the pulse signal a constant low level (0V). 
// - set STARTUP_LEVEL to 1, to give the pulse signal a constant high level (3,3 or 5V). 
//
// In this example we assume the servo's power can be switch on and off via the SERVO0_ENABLE_PIN
// Switching servo power on (and off)  can be done by putting a MOSFET (or a USB Power Distribution
// Switch) in series in the servo's 5V power line
//
// Tested on AVR64DA28 (is assumed to work on all dxCore and Tiny series 0, 1 and 2 processors).
// Edit the #defines to match your board and tests.
//
// References (in German. The details are found here):
// - https://www.opendcc.de/elektronik/opendecoder/servo_erfahrungen.html
// - https://www.opendcc.de/elektronik/opendecoder/opendecoder_sw_servo.html
// - https://www.opendcc.de/info/tipps/servo_ruckeln.html
//
// Reference (in English):
// - https://www.opendcc.de/elektronik/opendecoder/opendecoder_sw_servo_e.html
//
// See the following logic analyser screenshot to see the result of this code:
// https://github.com/aikopras/Servo-TCA/blob/main/extras/Measurements-TCA-Library/Start-Up.png
//
// ******************************************************************************************************
#include <Arduino.h>
#include <Servo_TCA0.h>                  // For objects of the servo class (TCA0)


#define SERVO_PIN             PIN_PF0    // Select the pin that suits your board.
#define SERVO_ENABLE_PIN      PIN_PA1    // Select the pin that suits your board.
#define STARTUP_LEVEL         1          // 0: low signal (0V), 1 = high signal (3,3 or 5V)
#define POWER_UP_DELAY        100        // Number of milliseconds to wait before the servo gets power
#define PULSE_DELAY           100        // Number of milliseconds to wait before the servo gets pulses


// ******************************************************************************************************
Servo  servo0;                           // Instantiate the three servo's on TCA0


void setup() {
  pinMode(SERVO_ENABLE_PIN, OUTPUT);     // configure the power enable pin as output
  digitalWriteFast(SERVO_ENABLE_PIN, 0); // Make sure the servo doen't got power yet

  servo0.constantOutput(STARTUP_LEVEL);  // Make sure the servo's pulse signal is high / low
  servo0.attach(PIN_PF0);                // attach the servo

  delay(POWER_UP_DELAY);                 // Time we wait before the servo gets power 
  digitalWriteFast(SERVO_ENABLE_PIN, 1); // Enable the servo's power
   
  delay(PULSE_DELAY);                    // Time we wait before the servo gets pulses 
  servo0.writeMicroseconds(1500);
}


void loop() {
 // Nothing needed in this example
}
