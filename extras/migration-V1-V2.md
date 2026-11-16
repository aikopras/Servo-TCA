# <a name="Migration"></a>Migration from V1 to V2 #

Version 2 of this library has only one class for the core library (`Servo6`) and one class for the high-level library (`ServoMoba6`). Both classes use TCA0 or TCA1, depending on the pin that is given to `attach()`. Existing sketches need the changes below.

### 1. Header files ###

| Version 1                                  | Version 2                     |
|:-------------------------------------------|:------------------------------|
| `#include <Servo_TCA0.h>`                  | `#include <Servo_TCA.h>`      |
| `#include <Servo_TCA1.h>`                  | `#include <Servo_TCA.h>`      |
| `#include <Servo_TCA0_MoBa.h>`             | `#include <Servo_TCA_MoBa.h>` |
| `#include <Servo_TCA1_MoBa.h>`             | `#include <Servo_TCA_MoBa.h>` |

### 2. Classes ###

| Version 1                       | Version 2                        |
|:--------------------------------|:---------------------------------|
| `Servo` (TCA0)                  | `Servo6`                         |
| `Servo1` (TCA1)                 | `Servo6`                         |
| `ServoMoba` (TCA0)              | `ServoMoba6`                     |
| `ServoMoba1` (TCA1)             | `ServoMoba6`                     |
| `ServoMoba::low` (and `high`, `continuous`) | `ServoMoba6::low`    |

The name `Servo6` avoids a conflict with the class `Servo` of other servo libraries, such as the servo libraries that come with DxCore. The 6 is the maximum number of servos; on processors without a TCA1 timer the maximum is 3.

Example (AVR-Servo-3, file `myServo.h`):

    // Version 1
    #include <Servo_TCA0_MoBa.h>
    class MyServo: public ServoMoba {

    // Version 2
    #include <Servo_TCA_MoBa.h>
    class MyServo: public ServoMoba6 {

### 3. printCurve() ###
`printCurve()` now has the Serial to be used for the output as parameter. Version 1 always used `Serial1`.

    servo0.printCurve();           // Version 1
    servo0.printCurve(Serial1);    // Version 2, for example with "#define Monitor Serial1": printCurve(Monitor)

### 4. Selection of the timer ###
In version 1 the class determined the timer. In version 2 the pin determines the timer. Most pins can only be used by one timer, so nothing changes for these pins. The pins PB0..PB2 (DA, DB and EA) and PG0..PG2 (64 pin DA and DB) can be used by both timers. For these pins TCA0 is used, unless TCA0 is already in use for servos on another port; in that case TCA1 is used. Thus, if a version 1 sketch used `Servo1` objects on such pins, first attach the servos for the pins that can only be used by TCA0. See [Processors and Pins](ProcessorsAndPins.md).

### 5. Smaller differences ###
- The return value of `attach()` (the servoIndex) is counted over all objects, in the order in which they are instantiated: 0..5 on processors with TCA1. In version 1 the `Servo1` objects had their own range 0..2.
- On processors with TCA1, upto 6 objects can be instantiated, in any order; each timer still supports 3 servos. A 7th object (4th on processors without TCA1) is marked as `INVALID_SERVO`.
- If both timers are used, the timer that is started as second runs half a period (20/6 ms) behind the other timer.
- `attach()` waits 30 ms when it starts a timer. In version 1 this was the case for TCA0 only; now it holds for TCA1 as well.
- `attach()` should not be called while interrupts are disabled (for example from within an ISR), since it enables the interrupts again.
