# <a name="ProcessorsAndPins"></a>Supported Processors and Pins #

#### MegaTinyCore - Servo (TCA0) ####
The megaTinyCore families in the table below include a TCA0 timer, and can therefore be used with the Servo Class. Depending on the number of processor pins, servos can be connected to the following pins:

|                   | PA1..PA3 / PA7 | PB0..PB2 | PB3..PB5 |
|:-----------------:|:--------------:|:--------:|:--------:|
| ATtiny-0 / 8-Pin  |       X        |          |          |
| ATtiny-1 / 8-Pin  |       X        |          |          |
| ATtiny-0 / Others |                |     X    |    X     |
| ATtiny-1 / Others |                |     X    |    X     |
| ATtiny-2 / Others |                |     X    |    X     |

#### DxCore and MegacoreX - Servo (TCA0) ####
The processor families in the table below include a TCA0 timer, and can therefore be used with the Servo Class. The table shows the possible pins for servo outputs, on the processors with the highest pin count, within a certain series. Processors with a lower pin count, provide only a subset of all servo outputs. Note that the EB family can not be used with the Servo Class, since it it does not have any TCA timer (but instead TCE / TCF timers).

|           | PA0..PA2 | PB0..PB2 | PC0..PC2 | PD0..PD2 | PE0..PE2 | PF0..PF2 | PG0..PG2 |
|:---------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|
| DA-Series |    X     |     X    |    X     |    X     |    X     |    X     |    X     |
| DB-Series |    X     |     X    |    X     |    X     |    X     |    X     |    X     |
| DD-Series |    X     |          |    X     | PD1..PD2 |          |    X     |          |
| DU-Series |    X     |          |          |    X     |          |    X     |          |
| EA-Series |    X     |     X    |    X     |    X     |    X     |    X     |    X     |
| EB-Series |          |          |          |          |          |          |          |
| MegacoreX |    X     |     X    |    X     |    X     |    X     |    X     |          |



#### DxCore - Servo1 (TCA1) ####
The processor families in the table below include a TCA1 timer, and can therefore be used with the Servo1 Class. The table shows the possible pins for servo outputs, on the processors with the highest pin count, within a certain series. Processors with a lower pin count, provide only a subset of all servo outputs.

|           | PA4..PA6 | PB0..PB2 | PC4..PC6 | PD4..PD6 | PE4..PE6 | PG0..PG2 |
|:---------:|:--------:|:--------:|:--------:|:--------:|:--------:|:--------:|
| DA-Series |          |     X    |    X     |          |    X     |    X     |
| DB-Series |          |     X    |    X     |          |    X     |    X     |
| EA-Series |    X     |     X    |    X     |    X     |          |          |

The processor families below can not be used with the Servo1 Class, since they either do not have a TCA1 timer or not even any TCA timer (but instead TCE or TCF).
- ATtiny 0-Series: No TCA1
- ATtiny 1-Series: No TCA1
- ATtiny 2-Series: No TCA1
- DD-Series: No TCA1
- DU-Series: No TCA1
- EB-Series: No TCA
- Megacore: No TCA1
