# <a name="VersionHistory"></a>Version History #

## V2.0.0 (2026-11-16) ##

### Main changes ###
The main changes are described in the [README](../README.md#changes-in-version-2) and in [Migration from V1 to V2](migration-V1-V2.md). In the code, comments for these changes start with "Version 2:".

### Smaller code changes ###
These changes have no "Version 2:" comment in the code.

- 2026-11-10: ISR: `CMPisSet` was also written via `channels[255]` for compare units that are not in use (thus with 1 or 2 servos on a timer). The ISR now first tests on `NO_CHANNEL`.
- 2026-11-10: `detach()`, `attached()`, `waitTillNextPulse()` and `constantOutput()` now test on `INVALID_SERVO`. Before, an object without channel (the 4th object in version 1) wrote via `channels[255]`.
- 2026-11-10: `finISR()` now sets `IsNotRunning`. Before, a next `attach()` after detaching all servos did not take over the timer again.
- 2026-11-10: `detach()` returns immediately if the servo was never attached, or if the timer was already given back to the core.
- 2026-11-10: MegaTinyCore, 8-pin processors: PA1 (WO1) used `compareUnit0` instead of `compareUnit1`, so PA1 never got pulses.
- 2026-11-10: The library didn't compile for processors without `Serial1` (ATtiny 0 and 1-series), since `printCurve()` used `Serial1`. Now the Serial is a parameter of `printCurve()`. A side effect is that sketches that don't use `Serial1` no longer link `Serial1`: on an AVR32DA48 with 6 servos this saves around 900 bytes Flash and 150 bytes RAM.
- 2026-11-10: The header files `Servo_TCA0_MoBa.h`, the README and the examples included `Servo_TCA0.h`, while the file was called `servo_TCA0.h`. This worked on macOS, but not on Linux. The new file names are consistent.
- 2026-11-10: Example `Test_Servo_Moba`: `initPulse()` was called with 3 instead of 4 arguments.
- 2026-11-16: `servoHandler()`: the `always_inline` attribute is now in front of the function, so a separate prototype is no longer needed. The generated ISR code is identical.
- 2026-11-16: Comment in `Servo_TCA.h`: the TCA interrupts occur every 6,67 milliseconds (not microseconds).

### Known issues (not yet solved) ###
- MegaTinyCore declares `resumeTCA0()`, but doesn't define it. Therefore a sketch that calls `detach()` gives a link error on ATtiny processors. This was also the case in version 1.
- EA-Series: the TCA1 pins PA4..PA6 and PD4..PD6 are not in the pin table of `compareUnitTCA1()`, so `attach()` fails for these pins.
- `initCurveFromEEPROM()` writes beyond `myCurve[]` if an EEPROM curve has no end point `(0,0)` within the first 24 points. `printCurve()` reads beyond `myCurve[]` in the same situation.
- The Flash and RAM figures in the README (section Resources) have not been updated for version 2.

## Earlier versions ##
See the history lines in the header of each file, and the git log.
