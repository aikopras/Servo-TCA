//******************************************************************************************************
//
// file:      Servo_TCA.h
// author:    Aiko Pras
// history:   2025-01-10 V1.0.0 ap initial version
//            2026-11-16 V2.0.0 ap one implementation for TCA0 and TCA1. Was servo_TCA0.h / servo_TCA1.h
//                                 Only one class: Servo6 (TCA0 and/or TCA1). Was Servo (TCA0) and Servo1 (TCA1)
//
// purpose:   Code for upto three servo's per TCA timer. Processors with a TCA1 timer support six servo's
//
// Wherever possible, code was copied from the original Arduino and DxCore servo libraries, to ensure
// easy migration.// 
//
// Currently supports DxCore only. Some modifications regarding multiplexer and output pin 
// would be needed to make it working for megaTinyCore.
//
// Instantiation:
// --------------
// If an object is instantiated of the class "Servo6", the minimim, maximum and default pulsewidths 
// are set, based on MIN_PULSE_WIDTH, MAX_PULSE_WIDTH and DEFAULT_PULSE_WIDTH.
// Once instatiated, obejcts of class "Servo6" can't be destroyed. Therefore it is never possible to
// instatiate more than MAX_SERVOS objects. If an attempt is made to instantiate an additional
// object, that object will be marked as INVALID_SERVO, and be ignored in the remainder of the program.
// MAX_SERVOS is 6 if the processor has a TCA1 timer, otherwise 3.
//
// Version 2: the class Servo6
// ---------------------------
// Version 1 had the classes Servo (TCA0) and Servo1 (TCA1). Version 2 has only one class: Servo6.
// The name avoids a conflict with the class Servo of other servo libraries, such as those of DxCore.
// The 6 is the maximum number of servos; on processors without a TCA1 timer the maximum is 3.
// A Servo6 object may use TCA0 or TCA1. The pin that is given to attach() determines which of the
// two is used. If the pin can be used by both timers (PB0..PB2 and PG0..PG2 on DA/DB, PB0..PB2 on EA),
// TCA0 is used, unless TCA0 is already in use for servos on another port. In that case TCA1 is used.
// Thus, if such pins should use TCA1, first attach the servos for the pins that only fit TCA0.
// See extras/migration-V1-V2.md for the changes needed in existing sketches.
//
// Attach(pin [,min, max]) 
// -----------------------
// - Pin can be an arduino pin number, but #defines like PIN_PF0 are preferred, since they generate 
//   a compiler error when a non-existing pin is chosen. Depending on the specific processor being used,
//   only certain pins are allowed!
// - Min and max are optional parameters, that may be used to modify MIN_PULSE_WIDTH and MAX_PULSE_WIDTH.
//   Such modification can be plus or minus 511 microseconds. If larger values are specified, it will
//   limited to 511us. resulotion is in steps of 4us.
//   These parameters can be used to protect the servo from moving beyond its physical limits,
//   but should not be used to set temporary limits.
// 
// Attach initialises the TCA timer, if it was not already initialized during an earlier attach for
// another servo object. Once initialised, the TCA timer will never be stopped, even if all servo
// objects got detached. Therefore TCA timer interrupts keep occuring every 6,67 milliseconds 
// (= REFRESH_INTERVAL / SERVOS_PER_TIMER).
// Version 2: A TCA timer that is not used by any of the attached servos, is not touched.
//
// If both TCA timers are used, the timer that is started as second runs half a period (20/6 ms)
// behind the other timer. Therefore pulses for servos on TCA0 and TCA1 never start at the same time.
// This limits the peak current if multiple servos move at the same time.
//
// Attach initialises the multiplexer, that connects the output of the Compare Unit to the 
// port to which the specified pin belongs. For DxCore processors, all servos that use the same
// TCA timer must use the same port.
// For TCA0, only the pins PIN_Px0, PIN_Px1 and PIN_Px2 can be used; attach sets the pin to OUTPUT.
// For TCA1, see the file servo_Timer_TCA1.h (for DA/DB: PIN_PB0..PB2 or PIN_PC4..PC6).
//  
// Attach returns with a servoIndex, which is 0 .. (MAX_SERVOS - 1) or INVALID_SERVO.
//
// Version 2: attach() should not be called while interrupts are disabled (for example from within
// an ISR), since attach() enables the interrupts again (see setPhaseOffset() in servo_TCA.cpp).
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>


//******************************************************************************************************
// The following #defines allow the calling sketch te determine if it can use these new methods 
#define ACCEPTS_NEW_VALUES               // for the acceptsNewValue()
#define CONSTANT_OUTPUT                  // for the constantOutput() method

// Same defines as in the standard servo libraries. Note that we can have only 3 servos per TCA timer
#define Servo_VERSION              2     // software version of this library (same as existing libraries)
#define MIN_PULSE_WIDTH          544     // the shortest pulse sent to a servo
#define MAX_PULSE_WIDTH         2400     // the longest pulse sent to a servo
#define DEFAULT_PULSE_WIDTH     1500     // default pulse width when servo is attached
#define REFRESH_INTERVAL       19999L   // minumim time to refresh servos in microseconds (UL is needed to avoid overflow)
#define SERVOS_PER_TIMER           3     // the maximum number of servos controlled by one TCA timer
#define INVALID_SERVO            255     // flag indicating an invalid servo index

// Version 2: Number of TCA timers. MAX_SERVOS equals the number of Compare Units on these timers
#if defined(TCA1)
  #define NUMBER_OF_TIMERS         2     // TCA0 and TCA1
#else
  #define NUMBER_OF_TIMERS         1     // TCA0 only
#endif
#define MAX_SERVOS (SERVOS_PER_TIMER * NUMBER_OF_TIMERS)




//******************************************************************************************************
// The following class is for an individual servo. Each servo instantiates one object of this class.
// The class (implementation) is compatible with existing servo libraries, but has two extra methods:
//
// acceptsNewValue() has been added to this servo_TCA library, and tells if the Compare Unit is ready  
// to receive a new value from the main loop. It has been added as flag, to avoid statements like  
// "delays(15)" in the main loop, as can be seen in several servo examples (such as sweep).
// waitTillNextPulse() has also been added, and can be used by the main loop if it knows there is
// nothing left it can do until the next pulse has been output.
//
// constantOutput() has been added to this servo_TCA library, and tells the Compare Unit to stop 
// setting pulses on the output, and instead make the output signal HIGH (1) or LOW (0). 
// This can be useful to implement different types of soft-start for servo's. 
// A subsequent write() / writeMicroseconds() can be used to resume the output of pulses.
//
// A user sketch can check the existance of these new methods, by checking if ACCEPTS_NEW_VALUES and/or
// CONSTANT_OUTPUT are defined.
//******************************************************************************************************
class Servo6 {
 
  public:
    Servo6();                                      // constructor
    uint8_t attach(uint8_t pin);                   // attach channel to a Compare Unit, sets pinMode, returns servoIndex or INVALID_SERVO
    uint8_t attach(uint8_t pin, int min, int max); // as above but also sets min and max values (in us) for writes.
    void detach();
    void write(uint16_t value);                    // a value < MIN_PULSE_WIDTH is treated as an angle, otherwise as pulse width in microseconds
    void writeMicroseconds(uint16_t value);        // Write pulse width in microseconds
    int read();                                    // returns current pulse width as an angle between 0 and 180 degrees
    uint16_t readMicroseconds();                   // returns current pulse width in microseconds
    bool attached();                               // return true if this servo is attached, otherwise false
    bool acceptsNewValue();                        // New for the servo_TCA library: to avoid the delays(15), as seen in several examples.
    void waitTillNextPulse();                      // New for the servo_TCA library
    void constantOutput(uint8_t on_off);           // New for the servo_TCA library: sets output signal 5V (1) or 0V (0)

  private:
    uint8_t servoIndex;                            // index into the channels[] array
    int8_t min;                                    // minimum is this value times 4 added to MIN_PULSE_WIDTH
    int8_t max;                                    // maximum is this value times 4 added to MAX_PULSE_WIDTH
};

