//*****************************************************************************************************
//
// File:      Test_Servo6_CNano_Tests.ino
// Author:    Aiko Pras
// History:   2026/11/16 V2.0.0
//
// Tests for the Servo6 and ServoMoba6 classes on the AVR128DA48 Curiosity Nano board, to be checked
// with a logic analyser. Select the test with "#define TEST" below.
//
// Pins:
// - TCA1: PC4, PC5, PC6 (only fit TCA1). PC6 also drives the yellow user LED (active low).
// - TCA0: PD0, PD1, PD2 (only fit TCA0)
// - PB0, PB1, PB2 fit both TCA0 and TCA1 (used in TEST 4 and 5)
// - MARKER (PD7): trigger / marker for the logic analyser. See the description of each test.
// - TEST 6: power enable pins PA2..PA7 (only to show that they go high and stay high)
// Serial1 (PC0 = TX, PC1 = RX) is the virtual serial port (CDC) of the on-board debugger, 115200 baud.
// setup() first waits 3 seconds, to give the serial monitor time to connect.
//
// Unless stated otherwise, the pulse widths are 1000, 1200, 1400 us (first three servos) and
// 1600, 1800, 2000 us (last three servos), with a period of 20 ms per servo.
//
// TEST 1: TCA0 is started first (PD0..PD2), then TCA1 (PC4..PC6). Test_Servo6_CNano starts TCA1 first.
//   Expected: Serial1 shows 6 x "attached". Within one timer the pulses are 6,67 ms apart.
//   The pulses of TCA1 are shifted 3,33 ms relative to those of TCA0, and this shift stays stable.
//   Purpose: the phase offset is now calculated the other way round (including the wrap around).
//
// TEST 2: Start-up. MARKER is high from just before the first attach() till just after the last one.
//   The results are printed after MARKER went low. Trigger on the rising edge of MARKER, and look at
//   the first 100 ms.
//   Expected: MARKER is high for slightly more than 60 ms (delay(30) in the first attach() of each
//   timer). TCA1 is started first: on PC4..PC6 there are no pulses during the first 10,9 ms (the first
//   period of a timer is 0xFFFF ticks). TCA0 is started as second, with the phase offset, so its first
//   period is shorter. On all outputs, every pulse has the correct width: no short pulses, no spikes,
//   and no high level outside a pulse. The shift between TCA0 and TCA1 is 3,33 ms from the start.
//   Purpose: the (unknown) start-up problem for which delay(30) was added. The delay is now also
//   used for TCA1. Repeat this test a few times (unplug / plug the cable).
//
// TEST 3: detach() and attach() again. All 6 servos are attached. Every 2 seconds the three servos
//   on TCA1 (PC4..PC6) are alternately detached and attached again. MARKER toggles at each change.
//   Expected: after detach, PC4..PC6 stay low (no pulses). A pulse that was busy at that moment may
//   be cut short, since TCA1 is reset when it is given back to DxCore. After attach, the pulses on PC4..PC6 return
//   with the same widths, again shifted 3,33 ms relative to TCA0. The pulses on PD0..PD2 (TCA0)
//   continue without any interruption during the whole test. Serial1 shows each change.
//   Purpose: the bugfix in finISR() (a next attach() takes over the timer again).
//
// TEST 4: Pins that fit both timers. First PD0..PD2 are attached (TCA0), then PB0..PB2.
//   Expected: Serial1 shows 6 x "attached". PB0..PB2 use TCA1, since TCA0 is already in use for
//   port D. Thus the pulses on PB0..PB2 are shifted 3,33 ms relative to those on PD0..PD2.
//
// TEST 5: As TEST 4, but in the opposite order: first PB0..PB2, then PD0..PD2.
//   Expected: PB0..PB2 now use TCA0. PD0..PD2 can't use TCA0 (already in use for port B), and TCA1
//   doesn't support port D on DA processors. Therefore Serial1 shows "attach failed" for servo 4..6.
//   PD0..PD2 are not configured (they stay inputs), so they show no pulses; their level depends on
//   the inputs of the logic analyser. PB0..PB2 show pulses of 1000, 1200 and 1400 us, 6,67 ms apart.
//   Purpose: shows that the order of attach() matters for pins that fit both timers.
//
// TEST 6: ServoMoba6 on PC4..PC6 and PD0..PD2. Every 3 seconds all six servos move along curve 2
//   (move_A, stretch 6), alternately in both directions, between 1300 and 1700 us.
//   MARKER goes high at the start of each movement, and low when all six servos have completed it.
//   Expected: before the first movement the pulse lines are low (initPulse(LOW, ...)). During a
//   movement the pulse widths of all six servos change gradually: from 1300 to 1700 us in the first
//   movement, from 1700 to 1300 us in the next one, and so on. All six servos change in the same way.
//   After the movement, 10 more pulses follow (200 ms), and then the pulse lines stay low until the
//   next movement. MARKER goes low around the moment the pulses stop. The shift of 3,33 ms between
//   TCA0 and TCA1 remains. PA2..PA7 (power enable) are high all the time.
//
//******************************************************************************************************
#include <Arduino.h>
#include <Servo_TCA_MoBa.h>  // For objects of the ServoMoba6 class (also includes the Servo6 class)

#define TEST          6      // Select the test: 1..6 (see above)

#define MARKER        PIN_PD7


//******************************************************************************************************
#if (TEST >= 1) && (TEST <= 5)
//******************************************************************************************************
Servo6 servo[6];             // Instantiate six servo's. The pin determines the TCA timer

#if (TEST == 1)
const uint8_t servoPin[6] = {PIN_PD0, PIN_PD1, PIN_PD2, PIN_PC4, PIN_PC5, PIN_PC6};
#elif (TEST == 2) || (TEST == 3)
const uint8_t servoPin[6] = {PIN_PC4, PIN_PC5, PIN_PC6, PIN_PD0, PIN_PD1, PIN_PD2};
#elif (TEST == 4)
const uint8_t servoPin[6] = {PIN_PD0, PIN_PD1, PIN_PD2, PIN_PB0, PIN_PB1, PIN_PB2};
#elif (TEST == 5)
const uint8_t servoPin[6] = {PIN_PB0, PIN_PB1, PIN_PB2, PIN_PD0, PIN_PD1, PIN_PD2};
#endif

uint8_t result[6];           // Result of attach(): the servoIndex or INVALID_SERVO

void printResult(uint8_t i) {
  Serial1.print("Servo ");
  Serial1.print(i + 1);
  if (result[i] == INVALID_SERVO) Serial1.println(": attach failed");
  else {
    Serial1.print(": attached, servoIndex = ");
    Serial1.println(result[i]);
  }
}

void setup() {
    pinMode(PIN_PC6, OUTPUT);              // Test: PC6 low before attach()
  pinMode(MARKER, OUTPUT);
  digitalWriteFast(MARKER, 0);
  Serial1.begin(115200);     // On the Curiosity Nano, Serial1 is connected to the USB CDC port
  delay(3000);               // Wait till the serial monitor is connected
  Serial1.println("");
  Serial1.print("Servo-TCA test ");
  Serial1.println(TEST);

  // We may initialise the pulse width before we do the attach.
  for (uint8_t i = 0; i < 6; i++) servo[i].writeMicroseconds(1000 + (i * 200));

  digitalWriteFast(MARKER, 1);                     // TEST 2: start of the attach() calls
  for (uint8_t i = 0; i < 6; i++) result[i] = servo[i].attach(servoPin[i]);
  digitalWriteFast(MARKER, 0);                     // TEST 2: end of the attach() calls
  for (uint8_t i = 0; i < 6; i++) printResult(i);  // Printed afterwards, to keep the attach() calls short
}

void loop() {
  #if (TEST == 3)
  static unsigned long lastMillis = 0;
  static bool attached = true;
  if ((millis() - lastMillis) > 2000) {
    lastMillis = millis();
    digitalWriteFast(MARKER, !digitalReadFast(MARKER));
    if (attached) {
      for (uint8_t i = 0; i < 3; i++) servo[i].detach();
      Serial1.println("PC4..PC6 (TCA1) detached");
    }
    else {
      for (uint8_t i = 0; i < 3; i++) result[i] = servo[i].attach(servoPin[i]);
      for (uint8_t i = 0; i < 3; i++) printResult(i);
    }
    attached = !attached;
  }
  #endif
}


//******************************************************************************************************
#elif (TEST == 6)
//******************************************************************************************************
#define CURVE                 2          // move_A (see src/TCA_MobaCurves/curves.cpp)
#define CURVE_STRETCH         6          // 1..255. Multiplies the curve's time by this number
#define TRESHOLD1             1300       // in us
#define TRESHOLD2             1700       // in us
#define INITIAL_PULSE_WIDTH   1500       // Used by initPulse for the first pulses after startup

ServoMoba6 servo[6];                     // Instantiate six servo's. The pin determines the TCA timer

const uint8_t servoPin[6]       = {PIN_PC4, PIN_PC5, PIN_PC6, PIN_PD0, PIN_PD1, PIN_PD2};
const uint8_t servoEnablePin[6] = {PIN_PA2, PIN_PA3, PIN_PA4, PIN_PA5, PIN_PA6, PIN_PA7};

void setup() {
  pinMode(MARKER, OUTPUT);
  digitalWriteFast(MARKER, 0);
  Serial1.begin(115200);     // On the Curiosity Nano, Serial1 is connected to the USB CDC port
  delay(3000);               // Wait till the serial monitor is connected
  Serial1.println("");
  Serial1.println("Servo-TCA test 6 (ServoMoba6)");
  for (uint8_t i = 0; i < 6; i++) {
    // Set both tresholds for the servo movements, and load the curve
    servo[i].setTreshold1(TRESHOLD1);
    servo[i].setTreshold2(TRESHOLD2);
    servo[i].initCurveFromPROGMEM(CURVE, CURVE_STRETCH);
    // Pulse line: low between movements, 0 extra pulses before and 10 extra pulses after moving
    servo[i].initPulse(LOW, 0, 10, INITIAL_PULSE_WIDTH);
    // Power line: power stays on (also while idle), enable with a HIGH signal
    servo[i].initPower(false, servoEnablePin[i], HIGH, 0, 0);
    uint8_t result = servo[i].attach(servoPin[i]);
    Serial1.print("Servo ");
    Serial1.print(i + 1);
    if (result == INVALID_SERVO) Serial1.println(": attach failed");
      else Serial1.println(": attached");
  }
}

unsigned long lastMillis;
uint8_t dir;

void loop() {
  if ((millis() - lastMillis) > 3000) {
    lastMillis = millis();
    digitalWriteFast(MARKER, 1);                   // Start of the movement
    for (uint8_t i = 0; i < 6; i++) servo[i].moveServoAlongCurve(dir);
    if (dir > 0) dir = 0;
      else dir = 1;
  }
  bool allCompleted = true;
  for (uint8_t i = 0; i < 6; i++) {
    servo[i].checkServo();
    if (!servo[i].movementCompleted) allCompleted = false;
  }
  if (allCompleted) digitalWriteFast(MARKER, 0);   // All six servos have completed the movement
}


//******************************************************************************************************
#else
#error "TEST should be 1..6"
#endif
