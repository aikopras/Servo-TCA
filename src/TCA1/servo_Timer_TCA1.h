//******************************************************************************************************
//
// file:      servo_Timer_TCA1.h
// author:    Aiko Pras
// history:   2024-01-15 V1.0.0 ap initial version
//            2026-11-16 V2.0.0 ap TCA0 and TCA1 code are now included in the same .cpp file. Therefore
//                                 the functions got the postfix TCA1, and the state is kept in tca[1].
//                                 The pin table moved from enableCompareUnit() to compareUnitTCA1().
//                                 New: pinFitsTCA1(). Stub functions for processors without TCA1
//
// purpose:   TCA1 specific code for DxCore and MEGACOREX
//
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>


//======================================================================================================
// The real code is only compiled if we have a TCA1 timer. Otherwise the stubs at the end are used.
#if defined(TCA1)

// Version 2: The defines _TIMER, takeOverTCA(), resumeTCA() and ServoHandler have been removed.
// servo_TCA.cpp now addresses TCA0 and TCA1 directly.

//******************************************************************************************************
// The initialisation of the multplexer depends on the processor being used.
// DxCore and MEGACOREX processors require that all pins belong to the same port
// Version 2: If onlyCheck is true, the PORTMUX register is not changed. In that case only
// the return value is of interest: does the multiplexer support this port?
//******************************************************************************************************
static boolean initMultiplexerTCA1(uint8_t port, boolean onlyCheck) {
  boolean configured = false;
  // DxCore will preset the PORTMUX to a certain port during startup. 
  // Therefore we have to clear the TCA1 bits before we can set it with new values. 
  // Sine registers are volatile variables, we have to load it to a temporary variable
  // For details, see DxCore EA48.md
  uint8_t tcaroutea = PORTMUX.TCAROUTEA; 
  tcaroutea &= ~PORTMUX_TCA1_gm;                 // mask off the (TCA1) bits we will change.
  switch (port) { 
    #ifdef PORTMUX_TCA1_PORTA_gc                 // Only on EA 
    case PA:
      tcaroutea |= PORTMUX_TCA1_PORTA_gc;
      configured = true;
      break;
    #endif
    case PB:                                    // Available on all (DA, DB, EA)
      tcaroutea |= PORTMUX_TCA1_PORTB_gc;
      configured = true;
      break;
    case PC:                                    // Available on all (DA, DB, EA)
      tcaroutea |= PORTMUX_TCA1_PORTC_gc;
      configured = true;
    break;
    #ifdef PORTMUX_TCA1_PORTD_gc                 // Only on EA 
    case PD:
      tcaroutea |= PORTMUX_TCA1_PORTD_gc;
      configured = true;
      break;
    #endif
    #ifdef PORTMUX_TCA1_PORTE_gc                 // Not available on 48 pin and EA
    case PE:
      tcaroutea |= PORTMUX_TCA1_PORTE_gc;
      configured = true;
      break;
    #endif
    #ifdef PORTMUX_TCA1_PORTG_gc                 // Not available on 48 pin 
    case PG:
      tcaroutea |= PORTMUX_TCA1_PORTG_gc;
      configured = true;
      break;
    #endif
    default:                                     // PA, PD, PF
    break;
  }  
  if (configured && !onlyCheck) PORTMUX.TCAROUTEA = tcaroutea; // write the temp variable back to the register.
  return configured;
}


//******************************************************************************************************
// Version 2: compareUnitTCA1() returns the Compare Unit (0, 1 or 2) that belongs to a pin.
// It returns NO_COMPARE_UNIT if the pin can not be used by TCA1. It does not change any register.
// The pin table was previously part of enableCompareUnit().
//******************************************************************************************************
static uint8_t compareUnitTCA1(byte pin) {
  uint8_t compareUnit = NO_COMPARE_UNIT;
  switch (pin) { 
    case PIN_PB0:
    case PIN_PC4:
      compareUnit = 0;
    break;
    case PIN_PB1: 
    case PIN_PC5:
      compareUnit = 1;
    break;
    case PIN_PB2: 
    case PIN_PC6:
      compareUnit = 2;
    break;
    //
    #if _AVR_PINCOUNT == 64
    case PIN_PE4:
    case PIN_PG0:
      compareUnit = 0;
    break;
    case PIN_PE5: 
    case PIN_PG1:
      compareUnit = 1;
    break;
    case PIN_PE6: 
    case PIN_PG2:
      compareUnit = 2;
    break;
    #endif
  }
  return compareUnit;
}


static boolean enableCompareUnitTCA1(uint8_t compareUnit, uint8_t servoIndex) {
  boolean enabled = false;
  switch (compareUnit) {
    case 0:                                                         // Compare Unit 0
      tca[1].compareUnit0 = servoIndex;                             // index into the channels array
      channels[servoIndex].CompareUnit = 0;                         // attach the channel to the Compare Unit
      TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;                    // enable Compare Unit 0
      enabled = true;
    break;
    case 1:
      tca[1].compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;                    // enable Compare Unit 1
      enabled = true;
    break;
    case 2:
      tca[1].compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      TCA1.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;                    // enable Compare Unit 2
      enabled = true;
    break;
  }
  return enabled;
}


//******************************************************************************************************
// 1) Determine the port to which the pin belongs, and configure the multiplexer.
//    With DxCore, all servo pins should belong to the same port.  
//    The first call to InitCompareUnit() stores the port number. If the pin in a subsequent call 
//    belongs to a different port, the function returns immediately.
//    If we know the port, the port multiplexer can be configured.
//    digitalPinToPort() is used to determine the port to which a specific pin belongs:  
//    Px0 becomes 0, Px1 becomes 1 etc
// 2) Enablermine the compare unit, that is attached to that pin. 
//    For the TCA1 timer the number of options is relatively low, which means that a case statement
//    can be used.
//******************************************************************************************************
// NO_PORT is defined in servo_TCA.cpp. usedPort is tca[1].usedPort.
  
static boolean initCompareUnitTCA1(byte pin, uint8_t servoIndex) {
  uint8_t newPort = digitalPinToPort(pin);
  if ((tca[1].usedPort != NO_PORT) && (tca[1].usedPort != newPort)) return false;
  if (!initMultiplexerTCA1(newPort, false)) return false;
  if (!enableCompareUnitTCA1(compareUnitTCA1(pin), servoIndex)) return false;
  pinMode(pin, OUTPUT);                                             // Set the pin as output
  tca[1].usedPort = newPort;
  return true;
}


//******************************************************************************************************
// Version 2: pinFitsTCA1() performs the same tests as initCompareUnitTCA1(), but doesn't change
// any register. It is used by attach() to decide if the servo can use TCA1 for this pin.
//******************************************************************************************************
static boolean pinFitsTCA1(byte pin) {
  uint8_t newPort = digitalPinToPort(pin);
  if (compareUnitTCA1(pin) == NO_COMPARE_UNIT) return false;         // Pin has no TCA1 compare unit
  if ((tca[1].usedPort != NO_PORT) && (tca[1].usedPort != newPort)) return false;
  return initMultiplexerTCA1(newPort, true);                         // Is the port supported?
}


//======================================================================================================
#else
//******************************************************************************************************
// Version 2: Stub functions for processors without a TCA1 timer
// servo_TCA.cpp calls the four functions below without "#if defined(TCA1)". On processors without
// TCA1 they do nothing: pinFitsTCA1() and initCompareUnitTCA1() return false, so attach() never
// selects TCA1. The compiler removes the calls, so these stubs cost no Flash.
// DxCore and MegaTinyCore declare takeOverTCA1() and resumeTCA1() in Arduino.h, but only define them
// for processors with TCA1. The same approach is used for takeOverTCA0() and resumeTCA0() on
// MegaCoreX (see servo_Timer_TCA0_DxCore.h).
//******************************************************************************************************
void takeOverTCA1() {}
void resumeTCA1() {}

static boolean pinFitsTCA1(byte pin) {
  (void) pin;                                                       // pin is not needed here
  return false;
}

static boolean initCompareUnitTCA1(byte pin, uint8_t servoIndex) {
  (void) pin;                                                       // pin is not needed here
  (void) servoIndex;                                                // servoIndex is not needed here
  return false;
}

//======================================================================================================
#endif
