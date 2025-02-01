//******************************************************************************************************
//
// file:      servo_TCA1.cpp
// author:    Aiko Pras
// history:   2025-01-10 V1.0.0 ap initial version
//
// purpose:   Code for uptthree servo's that use the TCA1 timer. 
//            Each servo uses its own Compare Unit, and is therefore jitter free.
//            We use Compare Units 0, 1 and 2.
//
// AVR DA/DB Processors: depending on the pincount, the following pins can be used for TCA1:
// - Port B: WO0=PB0, WO1=PB1, WO2=PB2 - This is the default setting, only on 48/64 pin
// - Port C: WO0=PC4, WO1=PC5, WO2=PC6 - Only on 48/64 pin
// - Port E: WO0=PE4, WO1=PE5, WO2=PE6 - Only on 64 pin
// - Port G: WO0=PG0, WO1=PG1, WO2=PG2 - Only on 64 pin
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

#if defined(TCA1) // Skip if we don't have a TCA1 timer
#include "../servo_TCA1.h"

//******************************************************************************************************
// The following typedef is for internal use by this library. It is used to create an array holding  
// three channels, that act as interface between the servo objects and the processor's compare units.
// Servo objects operate on channels, which in turn are used by the TCA interrupt routine.
// After a succeful attach(), servo objects receive a "servoIndex" that can be used to access 
// the channels array, via channels[servoIndex].xxx
// Note that all variables must be declared as static, to ensure their scope remains within this unit.
//******************************************************************************************************
typedef struct {
  volatile uint8_t CompareUnit = 255;  // 0, 1 or 2. 255 if mapping of channel to Compare Unit fails
  volatile uint16_t ticks;             // value for the Compare n Buffer Register
  volatile bool CMPisSet;              // true if the Compare Unit has received the latest value
  volatile bool isActive;              // true if this servo is attached
} channel_t;

static channel_t channels[MAX_SERVOS];         // the array of channels

//******************************************************************************************************
// Include the file that contains the code to initialise the multiplexer and the compare unit
//******************************************************************************************************
// The following variables must be declared before we include the files with specific code
#define NO_CHANNEL     255                     // The Compare Unit is not yet attached to a channel
static uint8_t compareUnit0 = NO_CHANNEL;      // Point from the compare unit to the connected channel
static uint8_t compareUnit1 = NO_CHANNEL;      // Point from the compare unit to the connected channel
static uint8_t compareUnit2 = NO_CHANNEL;      // Point from the compare unit to the connected channel

#include "servo_Timer_TCA1.h"                  // TCA1 specific code

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
static volatile uint8_t CurrentCompareUnit = 0;   // 0, 1 or 2. Used by the ISR to switch after 20/3 ms
static uint8_t ServoCount = 0;                    // number of instatiated servo objects (0, 1, 2 or 3)
static boolean IsNotRunning = true;               // TCA is initialised as part of the 1st attach() call

// ******************************************************************************************************
// Local (static) functions
// ******************************************************************************************************
static void initTCA() {
  // STEP 1: Avoid that DxCore/Mightycore will configure TCA
  // The file included above tells if this is for TCA0 or TCA1
  takeOverTCA();               // The included file tells if this is for TCA0 or TCA1
  // STEP 2: Configure CTRL A: Set the prescaler (see above) and enable the TCA peripheral
  _TIMER.CTRLA = PRESCALER_GC | TCA_SINGLE_ENABLE_bm; 
  // STEP 3: Configure CRTL B: set the timer mode: single-slope PWM mode
  _TIMER.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; 
  // STEP 4: Configure EVCTRL: Disable event counting (which is the default)
  _TIMER.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
  // STEP 5: Configure INTCTRL: Enable the overflow interrupt
  _TIMER.INTCTRL = TCA_SINGLE_OVF_bm;
  // STEP 6: Configure the PERBUF register: PER = Period value
  // This register holds the TOP value for all three Compare Unit. We want an interrupt every 20/3 ms.
  _TIMER.PERBUF = usToTicks(ISR_PERIOD);
  IsNotRunning = false;
}

static void finISR() {
  _TIMER.INTCTRL = 0;                    // Disable interrupt
  resumeTCA();                           // Give TCA back to DxCore / MegaTinyCore
}

static boolean isTimerActive() {         // returns true if any servo is active on this timer
  for (uint8_t i = 0; i < SERVOS_PER_TIMER; i++) {
    if (channels[i].isActive == true) return true;
  }
  return false;
}

//******************************************************************************************************
// The constructor is used to initialise a number of attributes.
// It would have been nicer if we could have avoided using channels, and instead directly referenced  
// the Compare Units. However, we know the associated Compare Unit's number only after the pin 
// has been provided, and that is done as part of the attach() method. To remain compatible with 
// existing servo libraries, we keep the channels (=servos) array as interface.
// We set already here the min and max values, to allow the write() to occur before the attach().
//******************************************************************************************************
Servo1::Servo1() {
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
uint8_t Servo1::attach(byte pin, int min, int max) {
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


uint8_t Servo1::attach(byte pin) {
  if (myServo == INVALID_SERVO) {return INVALID_SERVO;}
  if (IsNotRunning) {initTCA();}
  // Find the compare unit for this pin, and set the pin as output
  if (initCompareUnit(pin, myServo)) {
    channels[myServo].isActive = true;
    return myServo;
    }
  else return INVALID_SERVO;
}


//******************************************************************************************************
// The following four read and write methods are identical to other, existing, servo libraries
//******************************************************************************************************
void Servo1::write(unsigned int value) {
  // treat values less than MIN_PULSE_WIDTH as angles in degrees
  // treat values above MIN_PULSE_WIDTH as microseconds
  if (value < MIN_PULSE_WIDTH) {
    if (value > 180) {value = 180;}
    value = map(value, 0, 180, SERVO_MIN(), SERVO_MAX());
  }
  writeMicroseconds(value);
}


void Servo1::writeMicroseconds(uint16_t value) {
  // calculate and store the values for the given channel
  if (myServo != INVALID_SERVO) {   
    if (value < (uint16_t) SERVO_MIN()) value = SERVO_MIN();
    else if (value > (uint16_t) SERVO_MAX()) value = SERVO_MAX();
    channels[myServo].ticks = usToTicks(value);
    channels[myServo].CMPisSet = false;           // Flag for the main program
  }
}


int Servo1::read() { // return the value as degrees
  return map(readMicroseconds() + 1, SERVO_MIN(), SERVO_MAX(), 0, 180);
}


uint16_t Servo1::readMicroseconds(){
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
void Servo1::detach() {
  channels[myServo].isActive = false;
  if (isTimerActive() == false) {finISR();}
}


bool Servo1::attached() {
  return channels[myServo].isActive;
}



//******************************************************************************************************
// The acceptsNewValue() method was added to this servo_TCA library to allow a calling routine to 
// determine if the previous pulse value has already been on the output. The purpose of this method is 
// to avoid the delay() or "if (timepassed > 20ms)" constructs, that are often seen in existing servo 
// code  (such as in the traditional sweep examples).
// waitTillnextPulse() allows the calling routine to clear the flag, if there is nothing left to do 
// within the 20ms period until the next pulse is output.
//******************************************************************************************************
bool Servo1::acceptsNewValue() {
  bool ready = false;
  uint8_t index = myServo;
  if ((index != INVALID_SERVO)) {ready = channels[index].CMPisSet;}  
  return ready;
}

void Servo1::waitTillnextPulse() {
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
void Servo1::constantOutput(uint8_t on_off) {
  if (on_off == 0) {channels[myServo].ticks = 0;}
  else {channels[myServo].ticks = OUT_HIGH;}        // This value ensures a continuous high output
  channels[myServo].CMPisSet = false;               // Flag for the compare buffer 
}


//******************************************************************************************************
// The interrupt service routine is called every 20/3 ms. It's job is to activate the compare unit
// that belongs to the current servo and silence the two other compare units (by setting CMPBUF = 0)
//******************************************************************************************************
ISR(ServoHandler) {
  // An Update has just past, and triggered the execution of this ISR. This occurs every 20/3 ms
  _TIMER.INTFLAGS = TCA_SINGLE_OVF_bm;                 // The interrupt flag has to be cleared manually
  // With each ISR invocation we configure the next Compare Unit. 
  switch (CurrentCompareUnit) { 
    case 0:                                            // Handle Compare Unit 0
      if (_TIMER.CMP0 != OUT_HIGH) _TIMER.CMP0BUF = 0; // We nust test the entire 16 bit register (testing only CMPH doesn't work)
      if ((compareUnit1 != NO_CHANNEL) && (channels[compareUnit1].isActive)) {_TIMER.CMP1BUF = channels[compareUnit1].ticks;}
      if (_TIMER.CMP2 != OUT_HIGH) _TIMER.CMP2BUF = 0;      
      channels[compareUnit1].CMPisSet = true;
    break;
    case 1: 
      if (_TIMER.CMP0 != OUT_HIGH) _TIMER.CMP0BUF = 0;
      if (_TIMER.CMP1 != OUT_HIGH) _TIMER.CMP1BUF = 0;
      if ((compareUnit2 != NO_CHANNEL) && (channels[compareUnit2].isActive)) {_TIMER.CMP2BUF = channels[compareUnit2].ticks;}
      channels[compareUnit2].CMPisSet = true;
    break;
    case 2:
      if ((compareUnit0 != NO_CHANNEL) && (channels[compareUnit0].isActive)) {_TIMER.CMP0BUF = channels[compareUnit0].ticks;}
      if (_TIMER.CMP1 != OUT_HIGH) _TIMER.CMP1BUF = 0;      
      if (_TIMER.CMP2 != OUT_HIGH) _TIMER.CMP2BUF = 0;      
      channels[compareUnit0].CMPisSet = true;
    break;
  }  
  switch (CurrentCompareUnit) {                     // A switch statement is much faster than a Modulo 3 statement
    case 0: CurrentCompareUnit = 1; break;
    case 1: CurrentCompareUnit = 2; break;
    case 2: CurrentCompareUnit = 0; break;  
  }
}


#endif