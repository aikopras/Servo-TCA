//******************************************************************************************************
//
// file:      servo_TCA.cpp
// author:    Aiko Pras
// history:   2025-01-15 V1.0.0 ap initial version
//            2026-11-16 V2.0.0 ap one implementation for TCA0 and TCA1 (was servo_TCA0.cpp / servo_TCA1.cpp)
//                                 Only one class: Servo6. TCA0 and TCA1 run with a phase offset of half
//                                 a period. Bugfixes: see extras/VersionHistory.md
//
// purpose:   Code for upto three servo's that use the TCA0 timer, plus upto three servo's that use
//            the TCA1 timer (if the processor has a TCA1 timer).
//            Each servo uses its own Compare Unit, and is therefore jitter free.
//            We use Compare Units 0, 1 and 2 of each timer.
//
// AVR DA/DB/EA and MegacoreX: depending on the pin count, different pins can be used for TCA0:
// - Port A: WO0=PA0, WO1=PA1, WO2=PA2 - On all processors
// - Port B: WO0=PB0, WO1=PB1, WO2=PB2 - Only on 48/64 pin
// - Port C: WO0=PC0, WO1=PC1, WO2=PC2 - On all processors
// - Port D: WO0=PD0, WO1=PD1, WO2=PD2 - On all processors
// - Port E: WO0=PE0, WO1=PE1, WO2=PE2 - Only on 48/64 pin
// - Port F: WO0=PF0, WO1=PF1, WO2=PF2 - PF2 NOT on 28 pin
// - Port G: WO0=PG0, WO1=PG1, WO2=PG2 - Only on 64 pin
//
// AVR DD series: Ports A, C, D and F only. PD0 is not available.
// AVR DU series: Ports A, D and F only. 
//
// AVR DA/DB Processors: depending on the pincount, the following pins can be used for TCA1:
// - Port B: WO0=PB0, WO1=PB1, WO2=PB2 - This is the default setting, only on 48/64 pin
// - Port C: WO0=PC4, WO1=PC5, WO2=PC6 - Only on 48/64 pin
// - Port E: WO0=PE4, WO1=PE5, WO2=PE6 - Only on 64 pin
//
// AVR EA Processors: depending on the pincount, the following pins can be used for TCA1:
// - Port A: WO0=PA4, WO1=PA5, WO2=PA6
// - Port B: WO0=PB0, WO1=PB1, WO2=PB2 - Only on 48 pin
// - Port C: WO0=PC4, WO1=PC5, WO2=PC6 - Only on 48 pin
// - Port D: WO0=PD4, WO1=PD5, WO2=PD6
// 
// Wherever possible, code was copied from the original Arduino and DxCore servo libraries, 
// to ensure a maximum of compatibility between these libraries.
// 
//******************************************************************************************************
#include <Arduino.h>
#include "../Servo_TCA.h"


//******************************************************************************************************
// The following typedef is for internal use by this library. It is used to create an array holding  
// MAX_SERVOS channels, that act as interface between the servo objects and the processor's compare units.
// Servo objects operate on channels, which in turn are used by the TCA interrupt routine.
// After a succeful attach(), servo objects receive a "servoIndex" that can be used to access 
// the channels array, via channels[servoIndex].xxx
// Note that all variables must be declared as static, to ensure their scope remains within this unit.
//******************************************************************************************************
#define NO_TIMER       255                      // Version 2: The channel is not (yet) attached to a timer

typedef struct {
  volatile uint8_t CompareUnit = 255;  // 0, 1 or 2. 255 if mapping of channel to Compare Unit fails
  volatile uint16_t ticks;             // value for the Compare n Buffer Register
  volatile bool CMPisSet;              // true if the Compare Unit has received the latest value
  volatile bool isActive;              // true if this servo is attached
  volatile uint8_t Timer = NO_TIMER;   // Version 2: 0 (TCA0), 1 (TCA1) or NO_TIMER
} channel_t;

static channel_t channels[MAX_SERVOS];          // the array of channels


//******************************************************************************************************
// Include the file that contains the code to initialise the multiplexer and the compare unit
// Such code is different for MegaTinyCore and DxCore
//******************************************************************************************************
// The following variables must be declared before we include the files with specific code
// Version 2: Each TCA timer has its own set of variables: tca[0] for TCA0 and tca[1] for TCA1.
// The variables compareUnit0..2, CurrentCompareUnit, TCA_IsNotRunning and usedPort, which were
// previously separate static variables in servo_TCA0.cpp and servo_TCA1.cpp, are now part of tca_t.
#define NO_CHANNEL     255                      // The Compare Unit is not yet attached to a channel
#define NO_PORT        255                      // The timer is not yet connected to a port
#define NO_COMPARE_UNIT 255                     // The pin has no compare unit (used for TCA1)

typedef struct {
  uint8_t compareUnit0;                         // Point from the compare unit to the connected channel
  uint8_t compareUnit1;                         // Point from the compare unit to the connected channel
  uint8_t compareUnit2;                         // Point from the compare unit to the connected channel
  volatile uint8_t CurrentCompareUnit;          // 0, 1 or 2. Used by the ISR to switch after 20/3 ms
  boolean IsNotRunning;                         // TCA is initialised as part of the 1st attach() call
  uint8_t usedPort;                             // Set during 1st initCompareUnit
} tca_t;

#define TCA_INITIAL_VALUES {NO_CHANNEL, NO_CHANNEL, NO_CHANNEL, 0, true, NO_PORT}
#if (NUMBER_OF_TIMERS == 2)
static tca_t tca[NUMBER_OF_TIMERS] = {TCA_INITIAL_VALUES, TCA_INITIAL_VALUES};
#else
static tca_t tca[NUMBER_OF_TIMERS] = {TCA_INITIAL_VALUES};
#endif

#if defined(MEGATINYCORE_SERIES) 
#include "../TCA0/servo_Timer_TCA0_MegaTiny.h"
#else 
#include "../TCA0/servo_Timer_TCA0_DxCore.h"
#endif
#include "../TCA1/servo_Timer_TCA1.h"           // Version 2: contains stubs for processors without TCA1


//******************************************************************************************************
// Check if the library supports the current clockspeed
#if (F_CPU != 48000000) && (F_CPU != 40000000) && (F_CPU != 36000000) && (F_CPU != 32000000) \
 && (F_CPU != 28000000) && (F_CPU != 24000000) && (F_CPU != 20000000) && (F_CPU != 16000000) \
 && (F_CPU != 12000000) && (F_CPU != 10000000) && (F_CPU !=  8000000) && (F_CPU !=  5000000) \
 && (F_CPU !=  4000000) && (F_CPU !=  1000000) 
#error "Library has not been designed for this Clock Speed "
#endif


//******************************************************************************************************
// Set the prescaler, and define two functions:
// - usToTicks(us):    converts microseconds to ticks               (us = 0 ... 20000/3)
// - ticksToUs(ticks): converts from ticks back to microseconds     (ticks = 0 ... 60000)
// Note that rounding errors may occur during calculation of these functions, leading to lost precision.
//
// The sketch may run at different clock speeds: 1, 4, 5, 8, 10, 12, 16, 20, 24MHz (28, 32, 36, 40, 48)
// clockCyclesPerMicrosecond() can be: 1, 4, 5, 8, 10, 12, 16, 20, 24(, 28, 30, 32, 36, 40, 48)
// The prescaler can take the values: 1, 2, 4 and 8.
//
// Prescaler    clockCyclesPerMicrosecond()       clockCyclesPerMicrosecond() / Prescaler 
// ---------------------------------------------------------------------------------------
//     1                1, 4, 5, 8                               1, 4, 5, 8                   
//     2                10, 12, 16                                 5, 6, 8                    
//     4             20, 24, 28, 32 36                          5, 6, 7, 8, 9                 
//     8                  40, 48                                   5, 6                     
// 
// To limit rounding errors / overflows in usToTicks() and ticksToUs(), we always divide first
// clockCyclesPerMicrosecond by the prescaler.
// Note: this approach differs from the original DxCore Servo library (for TCB)!
#if (F_CPU > 36000000) // requires external clock and has not been tested
  #define PRESCALER 8 
  #define PRESCALER_GC TCA_SINGLE_CLKSEL_DIV8_gc 
#elif (F_CPU > 16000000)
  #define PRESCALER 4 
  #define PRESCALER_GC TCA_SINGLE_CLKSEL_DIV4_gc
#elif (F_CPU > 8000000) // Above 8Mhz: prescaler = 2
  #define PRESCALER 2 
  #define PRESCALER_GC TCA_SINGLE_CLKSEL_DIV2_gc
#else
  #define PRESCALER 1 
  #define PRESCALER_GC TCA_SINGLE_CLKSEL_DIV1_gc
#endif


#define CYCLES_DIV_PRESCALER   (clockCyclesPerMicrosecond() / PRESCALER)
#define ISR_PERIOD             ((int) REFRESH_INTERVAL / SERVOS_PER_TIMER)
#define SERVO_MIN()            (MIN_PULSE_WIDTH - (int8_t)this->min * 4)
#define SERVO_MAX()            (MAX_PULSE_WIDTH - this->max * 4)
#define usToTicks(_us)         (CYCLES_DIV_PRESCALER * _us) 
#define ticksToUs(_ticks)      ((unsigned) (_ticks / CYCLES_DIV_PRESCALER))
#define myServo                this->servoIndex
#define OUT_HIGH               65535            // Used to set / indicate the output at 5V


//******************************************************************************************************
// Internal objects / variables 
//******************************************************************************************************
static uint8_t ServoCount = 0;                  // number of instatiated servo objects (0 .. MAX_SERVOS)


// ******************************************************************************************************
// Local (static) functions
// ******************************************************************************************************
//******************************************************************************************************
// Version 2: tcaTimer() returns the registers of TCA0 or TCA1. The timer number is 0 or 1.
// TCA_SINGLE_t is defined in the processor's IO header (via <avr/io.h>, included by Arduino.h).
// It is a struct with all TCA registers in single (16-bit) mode, in the order of the
// "Register Summary" in the datasheet. TCA0 and TCA1 are such structs at fixed addresses
// (0x0A00 and 0x0A40), so TCA0.SINGLE.PER is the PER register of TCA0 in the datasheet.
// tcaTimer() returns a reference to TCA0.SINGLE or TCA1.SINGLE, so the same code can be used
// for both timers. Version 1 used the macro _TIMER (= TCA0.SINGLE) for the same purpose.
//******************************************************************************************************
static TCA_SINGLE_t &tcaTimer(uint8_t timerNr) {
  #if defined(TCA1)
  if (timerNr == 1) return TCA1.SINGLE;
  return TCA0.SINGLE;
  #else
  (void) timerNr;                                // Only TCA0
  return TCA0.SINGLE;
  #endif
}


//******************************************************************************************************
// Version 2: Phase offset between TCA0 and TCA1
// If both TCA timers are used, the timer that is started as second ("timer") will run half a period
// (20/6 ms) behind the timer that was already running ("runningTimer"). Since both timers use the
// same clock and prescaler, this offset remains the same. As a result, servo pulses of TCA0 and TCA1
// never start at the same time, which limits the peak current if multiple servos move simultaneously.
//
// Example at 24 MHz: prescaler = 4, so 6 ticks per us. TOP = 6 * 6666 = 39996.
// A period lasts TOP + 1 = 39997 ticks (6,67 ms), half a period is 19998 ticks (3,33 ms).
//
// The function works in three steps:
// 1) Wait till the running timer has its final period. After takeOverTCAx() the period register
//    (PER) is 0xFFFF. The value written into PERBUF is copied to PER at the first overflow (after
//    10,9 ms at 24 MHz). Before that moment the running timer does not yet have its final period.
//    We wait at most 250 x 100 us = 25 ms.
// 2) Set the period of the new timer. This timer has just been reset as well, so its PER is also
//    0xFFFF. We write PER directly, since otherwise the new timer would first count till 0xFFFF.
// 3) Set the counter (CNT) of the new timer to the counter of the running timer plus half a
//    period. If the result is beyond TOP, one period is subtracted (wrap around).
//    Between reading and writing CNT a few ticks pass; this error is less than 1 us.
//
// Interrupts are disabled while 16 bit registers are accessed. The AVR accesses a 16 bit register
// in two 8 bit steps, using the TEMP register of that TCA. The ISR of the running timer also writes
// 16 bit registers (CMPnBUF) via that same TEMP register. If that ISR occurs between the two steps,
// the value we read or write gets corrupted.
// Note: interrupts() always enables the interrupts, also if they were disabled before. Therefore
// attach() should not be called while interrupts are disabled (for example from within an ISR).
//******************************************************************************************************
static void setPhaseOffset(TCA_SINGLE_t &timer, TCA_SINGLE_t &runningTimer) {
  const uint16_t top = usToTicks(ISR_PERIOD);              // TOP value, as set via PERBUF in initTCA()
  const uint32_t period = (uint32_t) top + 1;              // A period lasts TOP + 1 ticks
  uint16_t per;
  // Step 1: wait till PER of the running timer has its final value (TOP)
  for (uint8_t i = 0; i < 250; i++) {                      // max 250 x 100 us = 25 ms
    noInterrupts();
    per = runningTimer.PER;                                // 16 bit read, via TEMP
    interrupts();
    if (per == top) break;
    delayMicroseconds(100);                                // busy wait, doesn't depend on a timer
  }
  noInterrupts();
  // Step 2: set the period of the new timer directly (not via PERBUF)
  timer.PER = top;
  // Step 3: new CNT = CNT of the running timer + half a period, modulo one period
  uint32_t count = runningTimer.CNT + (period / 2);
  if (count >= period) count = count - period;             // wrap around
  timer.CNT = (uint16_t) count;
  interrupts();
}


static void initTCA(uint8_t timerNr) {
  TCA_SINGLE_t &timer = tcaTimer(timerNr);  // Version 2: timer replaces _TIMER
  // STEP 1: Avoid that DxCore/Mightycore will configure TCA
  if (timerNr == 0) takeOverTCA0();
  if (timerNr == 1) takeOverTCA1();          // Version 2: a stub on processors without TCA1
  // STEP 2: Configure CTRL A: Set the prescaler (see above) and enable the TCA peripheral
  timer.CTRLA = PRESCALER_GC | TCA_SINGLE_ENABLE_bm;
  // STEP 3: Configure CRTL B: set the timer mode: single-slope PWM mode
  timer.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;          // 2025/04/22 AP: |= replaced by =
  // STEP 4: Configure EVCTRL: Disable event counting (which is the default)
  timer.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
  // STEP 5: Configure INTCTRL: Enable the overflow interrupt
  timer.INTCTRL = TCA_SINGLE_OVF_bm;
  // STEP 6: Configure the PERBUF register: PER = Period value
  // This register holds the TOP value for all three Compare Unit. We want an interrupt every 20/3 ms.
  timer.PERBUF = usToTicks(ISR_PERIOD);
  tca[timerNr].IsNotRunning = false;
  // STEP 7 (Version 2): If the other TCA timer is already running, run half a period behind it.
  // On processors with only TCA0, the compiler removes this code (NUMBER_OF_TIMERS is a constant).
  if (NUMBER_OF_TIMERS == 2) {
    uint8_t otherTimerNr = 1 - timerNr;
    if (tca[otherTimerNr].IsNotRunning == false) setPhaseOffset(timer, tcaTimer(otherTimerNr));
  }
  // 2025/04/23 AP: A delay is introduced, to avoid that any compare unit gets activated before we
  // have had one complete period. This seems to be needed with (at least some variants of) dxcore
  // processors. I do not really understand why this is needed.
  // Version 2: The cause is still unknown. Version 1 used this delay only for TCA0 (servo_TCA1.cpp
  // had no such delay). As a precaution, the delay is now used for TCA1 as well.
  delay(30);
}

static void finISR(uint8_t timerNr) {
  tcaTimer(timerNr).INTCTRL = 0;         // Disable interrupt
  if (timerNr == 0) resumeTCA0();        // Give TCA back to DxCore / MegaTinyCore
  if (timerNr == 1) resumeTCA1();        // Version 2: a stub on processors without TCA1
  tca[timerNr].IsNotRunning = true;      // A next attach() must take over TCA again
}

static boolean isTimerActive(uint8_t timerNr) { // returns true if any servo is active on this timer
  for (uint8_t i = 0; i < MAX_SERVOS; i++) {
    if ((channels[i].isActive == true) && (channels[i].Timer == timerNr)) return true;
  }
  return false;
}


//******************************************************************************************************
// Version 2: Selection of the TCA timer for a certain pin.
// selectTimer() checks, without changing any register, which of the two timers can use this pin.
// If both timers can use the pin, TCA0 is selected. TCA1 is selected if the pin doesn't fit TCA0,
// for example because TCA0 is already in use for servos on another port.
// On processors without TCA1, pinFitsTCA1() is a stub that always returns false.
// Returns 0 (TCA0), 1 (TCA1) or NO_TIMER.
//******************************************************************************************************
static uint8_t selectTimer(byte pin) {
  if (pinFitsTCA0(pin)) return 0;
  if (pinFitsTCA1(pin)) return 1;
  return NO_TIMER;
}

static boolean initCompareUnit(uint8_t timerNr, byte pin, uint8_t servoIndex) {
  if (timerNr == 1) return initCompareUnitTCA1(pin, servoIndex);   // A stub on processors without TCA1
  return initCompareUnitTCA0(pin, servoIndex);
}

//******************************************************************************************************
// The constructor is used to initialise a number of attributes.
// It would have been nicer if we could have avoided using channels, and instead directly referenced  
// the Compare Units. However, we know the associated Compare Unit's number only after the pin 
// has been provided, and that is done as part of the attach() method. To remain compatible with 
// existing servo libraries, we keep the channels (=servos) array as interface.
// We set already here the min and max values, to allow the write() to occur before the attach().
//******************************************************************************************************
Servo6::Servo6() {
  if (ServoCount < MAX_SERVOS) {
    myServo = ServoCount++;                                   // assign a channel index to this instance
    channels[myServo].ticks = usToTicks(DEFAULT_PULSE_WIDTH); // start with the default value
    this->min = 0;                                            // delta from MIN_PULSE_WIDTH
    this->max = 0;                                            // delta from MAX_PULSE_WIDTH
  } else {
    myServo = INVALID_SERVO;
  }
}


//******************************************************************************************************
// attach sets te desired pin as output, and configures the multiplexer.
// If a certain pin can not be used for compare unit output, attach returns with INVALID_SERVO (255)
//******************************************************************************************************
uint8_t Servo6::attach(byte pin, int min, int max) {
  // Overwrite the minimum and maximum values for this servo, and call attach()
  // this->min and this->max are 4us delta values, ranging from -128 .. 127 (int8_t)
  // resolution of min/max is thus 4 uS
  // If an int8_t overflow would occur, store the possible min/max value 
  if (abs((MIN_PULSE_WIDTH - min) / 4) < 128) {
    this->min = (MIN_PULSE_WIDTH - min) / 4; }
  else {
    if (MIN_PULSE_WIDTH > min) this->min = 127;
    else this->min = -127;
  }
  if (abs((MAX_PULSE_WIDTH - max) / 4) < 128) {
    this->max = (MAX_PULSE_WIDTH - max) / 4; }
  else {
    if (MAX_PULSE_WIDTH > max) this->max = 127;
    else this->max = -127;
    // this->max = (int8_t)((MAX_PULSE_WIDTH - 127) / 4);
  }
  return this->attach(pin);
}


uint8_t Servo6::attach(byte pin) {
  if (myServo == INVALID_SERVO) {return INVALID_SERVO;}
  // Version 2: First select the TCA timer for this pin. A timer is only started (and taken over
  // from DxCore / MegaTinyCore) if a servo will use it.
  uint8_t timerNr = selectTimer(pin);
  if (timerNr == NO_TIMER) {return INVALID_SERVO;}
  if (tca[timerNr].IsNotRunning) {initTCA(timerNr);}
  // Find the compare unit for this pin, and set the pin as output
  if (initCompareUnit(timerNr, pin, myServo)) {
    channels[myServo].Timer = timerNr;
    channels[myServo].isActive = true;
    return myServo;
    }
  else return INVALID_SERVO;
}


//******************************************************************************************************
// The following four read and write methods are identical to other, existing, servo libraries
//******************************************************************************************************
void Servo6::write(unsigned int value) {
  // treat values less than MIN_PULSE_WIDTH as angles in degrees
  // treat values above MIN_PULSE_WIDTH as microseconds
  if (value < MIN_PULSE_WIDTH) {
    if (value > 180) {value = 180;}
    value = map(value, 0, 180, SERVO_MIN(), SERVO_MAX());
  }
  writeMicroseconds(value);
}


void Servo6::writeMicroseconds(uint16_t value) {
  // calculate and store the values for the given channel
  if (myServo != INVALID_SERVO) {   
    if (value < (uint16_t) SERVO_MIN()) value = SERVO_MIN();
    else if (value > (uint16_t) SERVO_MAX()) value = SERVO_MAX();
    channels[myServo].ticks = usToTicks(value);
    channels[myServo].CMPisSet = false;           // Flag for the main program
  }
}


int Servo6::read() { // return the value as degrees
  return map(readMicroseconds() + 1, SERVO_MIN(), SERVO_MAX(), 0, 180);
}


uint16_t Servo6::readMicroseconds(){
  uint16_t pulsewidth;
  if (myServo != INVALID_SERVO) {pulsewidth = ticksToUs(channels[myServo].ticks);} 
    else {pulsewidth  = 0;}
  return pulsewidth;
}


//******************************************************************************************************
// Detach only sets a flag, that prevents the ISR from generating pulses. 
// As opposed to other servo libraries, it doesn't release the pin, nor does it stop the TCA timer.
// Stopping TCA doesn't make much sense in the case of AVR timers, since there is no easy way to take
// back the timer via something opposite to "TakeOverTCA".
// Detach is therefore of little use. To stop output pulses, a better way is to call constantOutput().
//******************************************************************************************************
void Servo6::detach() {
  if (myServo == INVALID_SERVO) return;
  channels[myServo].isActive = false;
  uint8_t timerNr = channels[myServo].Timer;       // Version 2: the timer used by this servo
  if (timerNr == NO_TIMER) return;                 // This servo was never attached
  if (tca[timerNr].IsNotRunning) return;           // finISR() was already called
  if (isTimerActive(timerNr) == false) {finISR(timerNr);}
}


bool Servo6::attached() {
  if (myServo == INVALID_SERVO) return false;
  return channels[myServo].isActive;
}



//******************************************************************************************************
// The acceptsNewValue() method was added to this servo_TCA library to allow a calling routine to 
// determine if the previous pulse value has already been on the output. The purpose of this method is 
// to avoid the delay() or "if (timepassed > 20ms)" constructs, that are often seen in existing servo 
// code  (such as in the traditional sweep examples).
// waitTillNextPulse() allows the calling routine to clear the flag, if there is nothing left to do 
// within the 20ms period until the next pulse is output.
//******************************************************************************************************
bool Servo6::acceptsNewValue() {
  bool ready = false;
  uint8_t index = myServo;
  if ((index != INVALID_SERVO)) {ready = channels[index].CMPisSet;}  
  return ready;
}

void Servo6::waitTillNextPulse() {
  if (myServo == INVALID_SERVO) return;
  channels[myServo].CMPisSet = false;               // Flag cleared by the main program 
}


//******************************************************************************************************
// The constantOutput() method was added to this servo_TCA library to allow a calling routine to 
// temporary stop the sending of pulses on the output pin. Calling constantOutput(0) forces the output
// pin to become 0V; calling constantOutput(1) forces the output to become high (5V).
// This method can be used to implement various forms of soft-start methods for the servo; for details
// see: https://www.opendcc.de/elektronik/opendecoder/servo_erfahrungen.html
// It is possible to resume the sending of pulses by calling write() or writeMicroseconds(). 
// An alternative approach to stop the sending of pulses on the output pin, would have been to call the 
// detach() method. However, after a detach(), a new attach() would be required.
//******************************************************************************************************
void Servo6::constantOutput(uint8_t on_off) {
  if (myServo == INVALID_SERVO) return;
  if (on_off == 0) {channels[myServo].ticks = 0;}
  else {channels[myServo].ticks = OUT_HIGH;}        // This value ensures a continuous high output
  channels[myServo].CMPisSet = false;               // Flag for the compare buffer 
}


//******************************************************************************************************
// The interrupt service routine is called every 20/3 ms. It's job is to activate the compare unit
// that belongs to the current servo and silence the two other compare units (by setting CMPBUF = 0)
//******************************************************************************************************
// Version 2: servoHandler() is used by the ISRs of TCA0 and TCA1. Since it is always inlined,
// the compiler uses the fixed register addresses of each timer, as before with _TIMER.
// Without always_inline the compiler may decide to make it a real function, that is called by both
// ISRs. Such a call from an ISR is expensive on AVR, since all call-clobbered registers must be saved.
static inline __attribute__((always_inline)) void servoHandler(TCA_SINGLE_t &timer, tca_t &state) {
  // An Update has just past, and triggered the execution of this ISR. This occurs every 20/3 ms
  timer.INTFLAGS = TCA_SINGLE_OVF_bm;                  // The interrupt flag has to be cleared manually
  // With each ISR invocation we configure the next Compare Unit. 
  switch (state.CurrentCompareUnit) {
    case 0:                                            // Handle Compare Unit 0
    if (timer.CMP0 != OUT_HIGH) timer.CMP0BUF = 0;   // We nust test the entire 16 bit register (testing only CMPH doesn't work)
      if ((state.compareUnit1 != NO_CHANNEL) && (channels[state.compareUnit1].isActive)) {timer.CMP1BUF = channels[state.compareUnit1].ticks;}
      if (timer.CMP2 != OUT_HIGH) timer.CMP2BUF = 0;
      if (state.compareUnit1 != NO_CHANNEL) {channels[state.compareUnit1].CMPisSet = true;}
    break;
    case 1: 
      if (timer.CMP0 != OUT_HIGH) timer.CMP0BUF = 0;
      if (timer.CMP1 != OUT_HIGH) timer.CMP1BUF = 0;
      if ((state.compareUnit2 != NO_CHANNEL) && (channels[state.compareUnit2].isActive)) {timer.CMP2BUF = channels[state.compareUnit2].ticks;}
      if (state.compareUnit2 != NO_CHANNEL) {channels[state.compareUnit2].CMPisSet = true;}
    break;
    case 2:
      if ((state.compareUnit0 != NO_CHANNEL) && (channels[state.compareUnit0].isActive)) {timer.CMP0BUF = channels[state.compareUnit0].ticks;}
      if (timer.CMP1 != OUT_HIGH) timer.CMP1BUF = 0;
      if (timer.CMP2 != OUT_HIGH) timer.CMP2BUF = 0;
      if (state.compareUnit0 != NO_CHANNEL) {channels[state.compareUnit0].CMPisSet = true;}
    break;
  }  
  switch (state.CurrentCompareUnit) {                  // A switch statement is much faster than a Modulo 3 statement
    case 0: state.CurrentCompareUnit = 1; break;
    case 1: state.CurrentCompareUnit = 2; break;
    case 2: state.CurrentCompareUnit = 0; break;
  }
}


ISR(TCA0_OVF_vect) {servoHandler(TCA0.SINGLE, tca[0]);}

#if defined(TCA1)
ISR(TCA1_OVF_vect) {servoHandler(TCA1.SINGLE, tca[1]);}
#endif
