//******************************************************************************************************
//
// file:      servo_Timer_TCA0.h
// author:    Aiko Pras
// history:   2024-01-15 V1.0.0 ap initial version
//            2026-11-16 V2.0.0 ap TCA0 and TCA1 code are now included in the same .cpp file. Therefore
//                                 the functions got the postfix TCA0, and the state is kept in tca[0].
//                                 New: pinFitsTCA0()
//
// purpose:   TCA0 specific code for DxCore and MEGACOREX
//
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>


//======================================================================================================
// Only compile if we have a TCA0 timer, and this is not a MEGATINYCORE
#if defined(TCA0) && !defined(MEGATINYCORE_SERIES)

// Version 2: The defines _TIMER, takeOverTCA(), resumeTCA() and ServoHandler have been removed.
// servo_TCA.cpp now addresses TCA0 and TCA1 directly.

//******************************************************************************************************
// The initialisation of the multplexer depends on the processor being used.
// DxCore and MEGACOREX processors require that all pins belong to the same port
// Version 2: If onlyCheck is true, the PORTMUX register is not changed. In that case only
// the return value is of interest: does the multiplexer support this port?
//******************************************************************************************************
static boolean initMultiplexerTCA0(uint8_t port, boolean onlyCheck) {
  boolean configured = false;
  // DxCore will preset the PORTMUX to a certain port during startup. 
  // Therefore we have to clear the TCA0 bits before we can set it with new values. 
  // Sine registers are volatile variables, we have to load it to a temporary variable
  // For details, see DxCore EA48.md
  uint8_t tcaroutea = PORTMUX.TCAROUTEA; 
  tcaroutea &= ~PORTMUX_TCA0_gm;                 // mask off the (TCA0) bits we will change.
  switch (port) { 
    case PA: 
      tcaroutea |= PORTMUX_TCA0_PORTA_gc;
      configured = true;
    break;
    #ifdef PORTB                                 // PORTB is not available on 28/32 pin processors
    case PB:
      tcaroutea |= PORTMUX_TCA0_PORTB_gc;
      configured = true;
    break;
    #endif
    case PC:
      tcaroutea |= PORTMUX_TCA0_PORTC_gc;
      configured = true;
    break;
    case PD: 
      tcaroutea |= PORTMUX_TCA0_PORTD_gc;
      configured = true;
    break;
    #ifdef PORTE                                 // PORTE is not available on 28/32 pin processors
    case PE:
      tcaroutea |= PORTMUX_TCA0_PORTE_gc;
      configured = true;
    break;
    #endif
    case PF:
      tcaroutea |= PORTMUX_TCA0_PORTF_gc;
      configured = true;      
    break;
    #ifdef PORTG                                 // PORTG is only available on 64 pin processors
    case PG:
      tcaroutea |= PORTMUX_TCA0_PORTG_gc;
      configured = true;
    break;
    #endif
  }  
  if (configured && !onlyCheck) PORTMUX.TCAROUTEA = tcaroutea; // write the temp variable back to the register.
  return configured;
}


static boolean enableCompareUnitTCA0(uint8_t compareUnit, uint8_t servoIndex) {
  boolean enabled = false;
  switch (compareUnit) { 
    case 0:                                                         // Compare Unit 0
      tca[0].compareUnit0 = servoIndex;                             // index into the channels array
      channels[servoIndex].CompareUnit = 0;                         // attach the channel to the Compare Unit
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;                    // enable Compare Unit 0
      enabled = true;
    break;
    case 1: 
      tca[0].compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;                    // enable Compare Unit 1
      enabled = true;
    break;
    case 2: 
      tca[0].compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;                    // enable Compare Unit 2
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
// 2) Determine the compare unit, that is attached to that pin. 
//    For the TCA0 timer this is easy: the first three pins of every port map upon a compare unit.
//    Px0 = Compare Unit 0, Px1 = Compare Unit 1 and Px2 = Compare Unit 2.
//    digitalPinToBitPosition() is used to determine the position of a specific pin on that port:  
//    Px0 becomes 0, Px1 becomes 1 etc
//******************************************************************************************************
// NO_PORT is defined in servo_TCA.cpp. usedPort is tca[0].usedPort.
  
static boolean initCompareUnitTCA0(byte pin, uint8_t servoIndex) {
  uint8_t compareUnit = digitalPinToBitPosition(pin);
  uint8_t newPort = digitalPinToPort(pin);
  if (tca[0].usedPort == NO_PORT) {                // First call to initCompareUnit?
    if (!initMultiplexerTCA0(newPort, false)) return false; // Return if the multiplexer doesn't support the port
  }
  else {
    if (tca[0].usedPort != newPort) return false;  // Return if a subsequent servo request a different port
  }
  if (!enableCompareUnitTCA0(compareUnit, servoIndex)) return false;
  pinMode(pin, OUTPUT);                            // Set the pin as output
  tca[0].usedPort = newPort;
  return true;
}


//******************************************************************************************************
// Version 2: pinFitsTCA0() performs the same tests as initCompareUnitTCA0(), but doesn't change
// any register. It is used by attach() to decide if the servo can use TCA0 for this pin.
//******************************************************************************************************
static boolean pinFitsTCA0(byte pin) {
  uint8_t compareUnit = digitalPinToBitPosition(pin);
  uint8_t newPort = digitalPinToPort(pin);
  if (compareUnit > 2) return false;                      // Only Px0, Px1 and Px2 have a compare unit
  if (tca[0].usedPort == NO_PORT) return initMultiplexerTCA0(newPort, true);  // Is the port supported?
  return (tca[0].usedPort == newPort);                    // All TCA0 servos must use the same port
}


//******************************************************************************************************
// MegaCoreX (ATmega4809, ATmega4808, ATmega3209, ATmega3208, ATmega1609, ATmega1608, ATmega809 and 
// ATmega808) doesn't define takeOverTCA(). Therefore we copied the code from DxCore here, 
// and removed several lines that are not needed by MegaCoreX.
//******************************************************************************************************
#if defined(MEGACOREX)
void takeOverTCA0() {
  TCA0.SPLIT.CTRLA = 0;          // Stop TCA0
  TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc | 0x03; // Reset TCA0
}

void resumeTCA0() {}

#endif   // MEGACOREX
#endif   // TCA and not MegaTinyCore
