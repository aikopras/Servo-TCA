//******************************************************************************************************
//
// file:      servo_Timer_TCA1.h
// author:    Aiko Pras
// history:   2024-01-15 V1.0.0 ap initial version
//
// purpose:   TCA1 specific code for DxCore and MEGACOREX
//
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>


//======================================================================================================
// Only compile if we have a TCA1 timer
#if defined(TCA1)

// The following defines enable an easy switch between TCA0 and TCA1  
#define _TIMER        TCA1.SINGLE               // Used in the .cpp file
#define takeOverTCA() takeOverTCA1()            // Used in the .cpp file
#define resumeTCA()   resumeTCA1()              // Used in the .cpp file
#define ServoHandler  TCA1_OVF_vect             // Used in the .cpp file

//******************************************************************************************************
// The initialisation of the multplexer depends on the processor being used.
// DxCore and MEGACOREX processors require that all pins belong to the same port
//******************************************************************************************************
static boolean initMultiplexer(uint8_t port) {
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
  if (configured) PORTMUX.TCAROUTEA = tcaroutea; // write the temp variable back to the register.
  return configured;
}


static boolean enableCompareUnit(byte pin, uint8_t servoIndex) {
  boolean enabled = false;
  switch (pin) { 
    case PIN_PB0:
    case PIN_PC4:
      compareUnit0 = servoIndex;                                    // index into the channels array
      channels[servoIndex].CompareUnit = 0;                         // attach the channel to the Compare Unit
      _TIMER.CTRLB |= TCA_SINGLE_CMP0EN_bm;                         // enable Compare Unit 0
      enabled = true;
    break;
    case PIN_PB1: 
    case PIN_PC5:
      compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      _TIMER.CTRLB |= TCA_SINGLE_CMP1EN_bm;                         // enable Compare Unit 1
      enabled = true;
    break;
    case PIN_PB2: 
    case PIN_PC6:
      compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      _TIMER.CTRLB |= TCA_SINGLE_CMP2EN_bm;                         // enable Compare Unit 2     
      enabled = true;
    break;
    //
    #if _AVR_PINCOUNT == 64
    case PIN_PE4:
    case PIN_PG0:
      compareUnit0 = servoIndex;                                    // index into the channels array
      channels[servoIndex].CompareUnit = 0;                         // attach the channel to the Compare Unit
      _TIMER.CTRLB |= TCA_SINGLE_CMP0EN_bm;                         // enable Compare Unit 0
      enabled = true;
    break;
    case PIN_PE5: 
    case PIN_PG1:
      compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      _TIMER.CTRLB |= TCA_SINGLE_CMP1EN_bm;                         // enable Compare Unit 1
      enabled = true;
    break;
    case PIN_PE6: 
    case PIN_PG2:
      compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      _TIMER.CTRLB |= TCA_SINGLE_CMP2EN_bm;                         // enable Compare Unit 2     
      enabled = true;
    break;
    #endif
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
#define NO_PORT 255
static uint8_t usedPort = NO_PORT;                                     // Set during 1st initCompareUnit

  
static boolean initCompareUnit(byte pin, uint8_t servoIndex) {
  uint8_t newPort = digitalPinToPort(pin);
  if ((usedPort != NO_PORT) && (usedPort != newPort)) return false;
  if (!initMultiplexer(newPort)) return false;
  if (!enableCompareUnit(pin, servoIndex)) return false;
  pinMode(pin, OUTPUT);                                             // Set the pin as output
  usedPort = newPort;
  return true;
}


//======================================================================================================
#endif
