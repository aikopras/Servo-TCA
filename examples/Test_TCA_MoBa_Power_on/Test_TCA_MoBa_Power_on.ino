//*****************************************************************************************************
//
// File:      Test_TCA_MoBa_Power_on.ino
// Author:    Aiko Pras
// History:   2025/04/27
//
// Different servos behave differently when power is switched on, or when servos start moving. 
// This sketch allows for experiments to find the power and pulse related settings that fit best
// with your specific servo.

// The example sketch is an extended version of Test_TCA_Power_On.ino, which was intended for the 
// standard Servo_TCAx libraries. In contrast, this sketch uses the power and pulse settings that
// are provided by the Servo_TCAx_MoBa libraries. A button is added to reboot the processor
// in cases where the board has no reset button.
//
// In this example we assume the servo's power can be switch on and off via the SERVO_ENABLE_PIN.
// Switching servo power on (and off)  can be done by putting a MOSFET (or a USB Power Distribution
// Switch) in series with the servo's 5V power line
//
// For a specific servo, the following parameters for pulse and power may be fine-tuned:
// - pulseStartUplevel
// - idleOutput
// - pulseBeforeMoving
// - pulseAfterMoving
// - initialPulseWidth
// - idlePowerIsOff
// - powerEnableValue
// - powerOnBeforeMoving
// - powerOffAfterMoving
//
//
// 1) pulseStartUplevel
// --------------------
// This parameter determines if, after startup, the pulse signal starts with a high or low value.
// The pulse signal stays at this level, till the first call is made to moveServoAlongCurve().
// The parameter should be included by a call to constantOutput(), and issued before the attach. 
// See the two figures below:
// https://github.com/aikopras/Servo-TCA/blob/main/extras/ServoMoba/Startup-constantOutput-HIGH.png
// https://github.com/aikopras/Servo-TCA/blob/main/extras/ServoMoba/Startup-constantOutput-LOW.png
//
// 2) idleOutput
// -------------
// To move the servo, moveServoAlongCurve() should be called. After the movement is completed, the
// pulse idle signal until the next move may be HIGH, LOW or continuous.
// The following two figures show the differences between idleOutput = HIGH and idleOutput = LOW  
// https://github.com/aikopras/Servo-TCA/blob/main/extras/ServoMoba/initpulse-idleOutput-HIGH.png
// https://github.com/aikopras/Servo-TCA/blob/main/extras/ServoMoba/initpulse-idleOutput-LOW.png
//
// 3 & 5) pulseBeforeMoving / initialPulseWidth
// --------------------------------------------
// After a call is made to moveServoAlongCurve(), it is possible to add some extra pulses
// before the movement starts. The next figure shows an example where two pulses are added
// before the servo starts moving along the curve.
// The pulse width for these preceeding pulses is the same as the pulse width with which the
// previous curve ended. If this is the first servo movement after power-on, we have to explicitly
// tell the library this initialPulseWidth.
// The next figure shows the first servo movement after power-on
// https://github.com/aikopras/Servo-TCA/blob/main/extras/ServoMoba/initialPulseWidth.png
//
// 4) pulseAfterMoving
// -------------------
// After the servo has moved along a curve, some additional pulses may be added before the pulse signal
// stops (thus in cases idleOutput is HIGH or LOW). The number of additional pulses can be specified by
// the pulseAfterMoving parameter.
// 
// 6) idlePowerIsOff
// -----------------
// Should the servo power be switch off at between servo movements. This may avoid the jitter that
// is commonly seen with (poorer quality) servos and decoders. This disadvantage is that the servo
// may not hold its position, if not powered.

// 7) powerEnableValue
// -------------------
// Does the power enable hardware (MOSFET, USB Power Distribution Switch) require a HIGH or LOW signal?
//
// 8 & 9) powerOnBeforeMoving / powerOffAfterMoving 
// ------------------------------------------------
// Number of 20 ms ticks the power gets turned on before, and turned off after the actual
// movement along the curve. See the figure below:
// https://github.com/aikopras/Servo-TCA/blob/main/extras/ServoMoba/PowerEnable.png
//
//
// ******************************************************************************************************
#include <Arduino.h>
#include <Servo_TCA0_MoBa.h>             // For objects of the servo MoBa class (TCA0)

#define SERVO_PIN             PIN_PF0    // Select the pin that suits your board.
#define SERVO_ENABLE_PIN      PIN_PA1    // Select the pin that suits your board.
#define REBOOT_PIN            PIN_PA5    // For a button that reboots the controller

#define pulseStartUplevel     HIGH       // 0: low signal (0V), 1 = high signal (3,3 or 5V)
#define idleOutput            HIGH       // 0: low signal (0V), 1 = high signal (3,3 or 5V)
#define pulseBeforeMoving     3          // Number of extra pulses before the servo starts moving
#define pulseAfterMoving      3          // Number of extra pulses after the servo stopped moving

#define idlePowerIsOff        true       // should power be switch off while idle?
#define powerEnableValue      HIGH       // does the power enable hardware require a HIGH or LOW signal?
#define powerOnBeforeMoving   2          // 0.255. Steps are in 20 ms
#define powerOffAfterMoving   2          // 0.255. Steps are in 20 ms

// It may not be necessary to modify the #defines below 
#define TEST_CURVE            11         // This curve is for a complete sine wave, and lasts 800ms
#define CURVE_STRETCH         2          // 1..255. Multiplies the curve's time by this number
#define initialPulseWidth     1500       // Used by initPulse for the first pulses after startup

// ******************************************************************************************************
ServoMoba  servo0;                       // Instantiate the servo


void setup() {
  // Set both tresholds for the servo movements, and load the curve
  servo0.setTreshold1(1000);
  servo0.setTreshold2(2000);
  servo0.initCurveFromPROGMEM(TEST_CURVE, CURVE_STRETCH);

  // Set the various parameters for the pulse line
  servo0.initPulse(
    idleOutput, 
    pulseBeforeMoving, 
    pulseAfterMoving, 
    initialPulseWidth
    );

  // Set the various parameters for the power line
  servo0.initPower(
     idlePowerIsOff,
     SERVO_ENABLE_PIN,
     powerEnableValue,
     powerOnBeforeMoving,
     powerOffAfterMoving
     );

  // With attach() the compare unit starts, thus the PWM (pulse) signal is output. 
  // To avoid heavy servo movements during power on, it may be necessary to set this 
  // PWM signal to a constant (generally HIGH) output level, 
  servo0.constantOutput(pulseStartUplevel);
  servo0.attach(SERVO_PIN);

  // Pin for the reboot button
  pinMode(REBOOT_PIN, INPUT_PULLUP);
  delay(1000);
}


void checkReboot() {
  // Reboot if button is pushed
  if (!digitalRead(REBOOT_PIN)) {
    digitalWriteFast(SERVO_ENABLE_PIN, 0);
    delay(100);
    noInterrupts();
    asm volatile("  jmp 0");
    interrupts();
  };
};

// Parameters for the main loop
long lastMillis;


void loop() {
  if ((millis() - lastMillis) > 2500) {
    lastMillis = millis();
    servo0.writeMicroseconds(1500);      // NODIG? ALLEEN VOOR 1ste curve
    delay(20);
    servo0.moveServoAlongCurve(0);       // Move along curve in the normal direction
  }  
  servo0.checkServo();
  checkReboot(); }
