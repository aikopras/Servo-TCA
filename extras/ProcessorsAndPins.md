# <a name="ProcessorsAndPins"></a>Supported Processors and Pins #

The classes `Servo6` and `ServoMoba6` use the TCA0 timer and, if the processor has one, the TCA1 timer. The pin that is given to `attach()` determines which of the two timers is used. The tables below show which pins can be used for each timer.

#### MegaTinyCore - TCA0 ####
The megaTinyCore families in the table below include a TCA0 timer, and can therefore be used with the Servo6 Class. Depending on the number of processor pins, servos can be connected to the following pins:

|                   | PA1..PA3 / PA7 | PB0..PB2 | PB3..PB5 |
|:-----------------:|:--------------:|:--------:|:--------:|
| ATtiny-0 / 8-Pin  |       X        |          |          |
| ATtiny-1 / 8-Pin  |       X        |          |          |
| ATtiny-0 / Others |                |     X    |    X     |
| ATtiny-1 / Others |                |     X    |    X     |
| ATtiny-2 / Others |                |     X    |    X     |

#### DxCore and MegacoreX - TCA0 ####
The processor families in the table below include a TCA0 timer, and can therefore be used with the Servo6 Class. The table shows the possible pins for servo outputs, on the processors with the highest pin count, within a certain series. Processors with a lower pin count, provide only a subset of all servo outputs. Note that the EB family can not be used with the Servo6 Class, since it it does not have any TCA timer (but instead TCE / TCF timers).

|           | PA0..PA2 | PB0..PB2 | PC0..PC2 | PD0..PD2 | PE0..PE2 | PF0..PF2 | PG0..PG2 |
|:---------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|
| DA-Series |    X     |     X    |    X     |    X     |    X     |    X     |    X     |
| DB-Series |    X     |     X    |    X     |    X     |    X     |    X     |    X     |
| DD-Series |    X     |          |    X     | PD1..PD2 |          |    X     |          |
| DU-Series |    X     |          |          |    X     |          |    X     |          |
| EA-Series |    X     |     X    |    X     |    X     |    X     |    X     |    X     |
| EB-Series |          |          |          |          |          |          |          |
| MegacoreX |    X     |     X    |    X     |    X     |    X     |    X     |          |



#### DxCore - TCA1 ####
The processor families in the table below include a TCA1 timer. On these processors the Servo6 Class can use the pins below as well, which gives a maximum of 6 servos. The table shows the possible pins for servo outputs, on the processors with the highest pin count, within a certain series. Processors with a lower pin count, provide only a subset of all servo outputs.

|           | PA4..PA6 | PB0..PB2 | PC4..PC6 | PD4..PD6 | PE4..PE6 | PG0..PG2 |
|:---------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|
| DA-Series |          |     X    |    X     |          |    X     |    X     |
| DB-Series |          |     X    |    X     |          |    X     |    X     |
| EA-Series |    X     |     X    |    X     |    X     |          |          |

The processor families below do not have a TCA1 timer (or not even any TCA timer, but instead TCE or TCF). On these processors the Servo6 Class can only use the TCA0 pins, which gives a maximum of 3 servos.
- ATtiny 0-Series: No TCA1
- ATtiny 1-Series: No TCA1
- ATtiny 2-Series: No TCA1
- DD-Series: No TCA1
- DU-Series: No TCA1
- EB-Series: No TCA
- Megacore: No TCA1


#### DxCore - Selection of the timer (TCA0 or TCA1) ####
The Servo6 Class (and the ServoMoba6 Class) may use all pins of the TCA0 and TCA1 tables above. Which TCA timer is used, is determined by the pin that is given to `attach()`. Some pins can be used by both timers: PB0..PB2 (DA, DB and EA) and PG0..PG2 (64 pin DA and DB). For such pins TCA0 is used, unless TCA0 is already in use for servos on another port; in that case TCA1 is used.

All servos that use the same TCA timer must be connected to the same port. On processors without TCA1, the Servo6 Class can only use the pins of the TCA0 tables.

Note: on the EA-Series the TCA1 pins PA4..PA6 and PD4..PD6 are not yet supported by the library (see `compareUnitTCA1()` in `src/TCA1/servo_Timer_TCA1.h`).

Example: the AVR Servo decoder V2.3 (AVR32DA48) uses PC4..PC6 (TCA1) for servo 1..3, and PF0..PF2 (TCA0) for servo 4..6.
