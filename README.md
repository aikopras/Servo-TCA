# <a name="Servo_TCA"></a>Servo TCA Library #

Servo library using the TCA timer(s) of modern 8-bit AVR Microchip processors. Depending on the processor, 3 or 6 servo's can be controlled by a single processor. Supported processors are MegacoreX, megaTinyCore (ATtiny 0, 1 and 2 series) and DxCore (DA, DB, DD, DU and EA series). As opposed to existing servo libraries, the PWM servo signal is  generated by hardware, thus avoiding the jitter that may occur with existing libraries.

This library basically consists of two servo core libraries, plus two "high-level" libraries. These "high-level" libraries allow servo movement along a user defined curve. For some common curves, a couple of predefined curves have already been included. In addition, these "high-level" libraries allow the servo power and puls signals to be switch on or off.  

## Core libraries ##
There are two core libraries, one using the TCA0 timer, and another using the TCA1 timer.
Since each TCA timer has three Compare Units / Waveform Outputs (WO), each library supports up to 3 servos.
A sketch may include both libraries, thus the maximum number of servos is 6.

The TCA0 library requires the line `#include <Servo_TCA0_MoBa.h>` at the top of the user sketch, and makes available the class `Servo`. This core library can be used for all processors mentioned above.

The TCA1 library requires the line `#include <Servo_TCA1_MoBa.h>` at the top of the user sketch, and makes available the class `Servo1`. This core library can only be used with processors that have a TCA1 timer, thus the DA, DB and EA processors with 48 pins or more..  


The core libraries are upwards compatible with the standard servo libraries, and implement all the methods that can be found in these standard servo libraries, as well as some additional methods. The (public) methods provided by the `Servo` class are shown below; the `Servo1` class provides exactly the same methods.

    class Servo {
      public:
        uint8_t attach(uint8_t pin);                   // attach channel to a Compare Unit, sets pinMode, returns servoIndex or INVALID_SERVO
        uint8_t attach(uint8_t pin, int min, int max); // as above but also sets min and max values for writes.
        void detach();
        void write(uint16_t value);                    // a value < MIN_PULSE_WIDTH is treated as an angle, otherwise as pulse width in microseconds
        void writeMicroseconds(uint16_t value);        // Write pulse width in microseconds
        uint16_t readMicroseconds();                   // returns current pulse width in microseconds
        bool acceptsNewValue();                        // New for the servo_TCA library: to avoid the delays(15), as seen in several examples.
        void waitTillNextPulse();                      // New for the servo_TCA library
        void constantOutput(uint8_t on_off);           // New for the servo_TCA library: sets output signal 5V (1) or 0V (0)
    };

Compared to standard servo libraries, three new methods were added: `acceptsNewValue()`, `waitTillNextPulse()` and `constantOutput(uint8_t on_off)`. These methods were added to allow better control regarding the start and stop behavior of the attached servo's.


## High-level libraries ##
The core libraries allow servo's to move from one position to another. Such move is nearly instantaneous, which is often not desirable. A better approach would be that servos move slowly from one position to another or, even better, follow a specific curve between the start and end position. Since servos not only make noise while moving, but often also while in rest, wouldn't it be nice if we were able to switch the servo power and/or pulse signal off, once the end position is reached? This is exactly what the high-level libraries add to the core libraries.

<div style="text-align:center"><img src="https://www.opendcc.de/elektronik/opendecoder/curve_hp1p.gif" /></div>

The "high-level" libraries are strongly inspired by, and with respect to functionality, copied from the [OpenDecoder 2](https://www.opendcc.de/elektronik/opendecoder/opendecoder_sw_servo.html) of the [OpenDCC project](https://www.opendcc.de/index.html). To give an idea of how a curve may look like, above a (linked) figure from the OpenDCC project. In fact, all predefined curves in this library are copied from OpenDecoder 2.

Like the core libraries, there are two high-level libraries: one for TCA0 and the other for TCA1. Technically speaking, the first high-level library inherits the Servo class for TCA0, and the second inherits the Servo class from TCA1.

    class ServoMoba: public Servo {
      public:
        enum idlePulseDefault_t {                      // default servo signal in the idle state
          low,  
          high,
          continuous};

        void checkServo();                             // Must be called as often as possible from the main loop

        void moveServoAlongCurve(uint8_t direction);   // Start moving along the path selected with initCurve

        void initCurveFromEEPROM(                      // use a predefined curve from EEPROM
          int adresEeprom,                             // The starting address in EEPRROM of this curve
          uint8_t timeStretch);                        // 1..255

        void initCurveFromPROGMEM(                     // use a predefined curve from PROGMEM
          uint8_t indexCurve,                          // See curves.cpp for possible curves
          uint8_t timeStretch);                        // 1..255

        void initPulse(                                // What to do with the servo puls signal in idle state?
          idlePulseDefault_t idleDefault,              // low, high or continuous
          uint8_t pulseBeforeMoving,                   // 0.255. Steps are in 20 ms
          uint8_t pulseAfterMoving);                   // 0.255. Steps are in 20 ms

        void initPower(                                // What to do with the servo power signal in idle state?
          boolean idlePowerIsOff,                      // should power be switch off while idle?
          uint8_t powerEnablePin,                      // the pin used to switch the servo power on and off
          boolean powerEnableValue,                    // does the power enable hardware require a HIGH or LOW signal?
          uint8_t powerOnBeforeMoving,                 // 0.255. Steps are in 20 ms
          uint8_t powerOffAfterMoving);                // 0.255. Steps are in 20 ms

        void setTreshold1(uint16_t value);             // Treshold1 can be higher or lower than Treshold2
        void setTreshold2(uint16_t value);             // Value in us.
        uint16_t getTreshold1();                       // returns Treshold1
        uint16_t getTreshold2();                       // returns Treshold2

        void printCurve();                             // May be used for testing. Uses Serial1
    }

## Why Yet Another Servo Library? ##
Standard Arduino servo libraries rely on a single (usually 16 bit) timer to generate an interrupt (ISR) when the PWM puls for the current servo should end, and the puls for the subsequent servo should start. Within the ISR, functions like digitalWrite(), are generally used to switch the pulses on and off. This approach has as disadvantage that the exact time the pulses will change, may vary, depending on occurrence or absence of other interrupts. The standard servo PWM signal may therefore show some jitter, resulting into noise produced by the servo.

With this library, the PWM servo pulses are generated by hardware, using the Compare Unit / Waveform Output, that is available in many processors. Usage of such hardware results into a superior PWM servo signal.

The [extras](extras/) directory of this repository contains several pictures that compare the performance of this library to that of standard servo libraries. In addition, some slides are included that shows several details regarding the internals of this library.

___
## Resources
The library has been tested on the following processors: ATMEGA 4809 (Arduino Nano Every), ATtiny 1607, ATtiny 3217, ATtiny 1627, AVR128DA48, AVR64DD32 and AVR64EA48. For 1 servo, it needs around 500 bytes of Flash and 10 bytes of RAM. For 3 servo's it needs around 800 bytes of Flash and 16 bytes of RAM. For 6 servo's 1600 bytes Flash and 32 bytes of RAM are needed.

The overhead of TCA interrupts is, when a single TCA timer is used (thus 1..3 Servo's), roughly 6us every 6,67 ms. When 2 TCA timers are used (upto 6 servo's), it is 6us per 3,33ms.
For comparison: the overhead of the millis() timer is around 1,8us every 1ms.

See [possible pins ](extras/ProcessorsAndPins.md) to learn which pins can be used on which processor. Use the provided examples to test this.
