# <a name="Servo_TCA"></a>Servo_TCA #

To be provided.

## Relation to existing Arduino servo libraries ##
To be provided.

___
## Resources
1 servo: 500 bytes Flash abd 10 bytes RAM<br>
3 servo's: 800 bytes Flash abd 16 bytes RAM<br>
3 servo's: 1600 bytes Flash abd 32 bytes RAM

## ISR Overhead
The overhead of TCA interrupts is, when a single TCA timer is used (thus 1..3 Servo's), roughly 6us every 6,67 ms. When 2 TCA timers are used (upto 6 servo's), it is 6us per 3,33ms.
For comparison: the overhead of the millis() timer is around 1,8us every 1ms.

## Supported Processors / Pins ##
The Servo TCA library can be used on megaTinyCore, DxCore and MegacoreX processors. For details, see [Processors and Pins](extras/ProcessorsAndPins.md). It has been tested on the ATtiny1607, ATtiny3217, ATtiny1627, AVR128DA48, AVR64DD32, AVR64EA48 and AVR4809 processors.
