//******************************************************************************************************
//
// file:      servo_Timer_TCA0_MegaTiny.h
// author:    Aiko Pras
// history:   2024-01-03 V1.0.0 ap initial version
//
// purpose:   TCA0 specific code for MegaTinyCore
//
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>

//======================================================================================================
// Only compile if we have a TCA0 timer, and this is MEGATINYCORE


#if defined(TCA0) && defined(MEGATINYCORE_SERIES)

// The following defines enable an easy switch between TCA0 and TCA1  
#define _TIMER        TCA0.SINGLE               // Used in the .cpp file
#define takeOverTCA() takeOverTCA0()            // Used in the .cpp file
#define resumeTCA()   resumeTCA0()              // Used in the .cpp file
#define ServoHandler  TCA0_OVF_vect             // Used in the .cpp file


//******************************************************************************************************
// The PORTMUX for the ATtiny series is different from that of the DxCore controllers.
// Within the ATtiny series, there are also differences:
// - the 0 and 1 series use the CNTRLC register for multiplexing TCA
// - the 2 series use the TCAROUTEA register for multiplexing TCA
// Finally there is a difference between the 
// - 8-pin: WO0 = PA3 (PA7), WO1 = PA1, WO2 = PA2
// - others: WO0 = PB0 (PB3), WO1 = PB1 (PB4), WO2 = PB2 (PB5) 
//           Note: PB4 and PB5 don't exist on the 14 pin versions
//******************************************************************************************************
#if defined PORTMUX_TCAROUTEA                   // 2 series
#define TCAMUX PORTMUX.TCAROUTEA
#elif defined PORTMUX_CTRLC                     // 0 and 1 series
#define TCAMUX PORTMUX.CTRLC
#endif


//
#if _AVR_PINCOUNT == 8
//====================

static boolean initCompareUnit(byte pin, uint8_t servoIndex) {
  uint8_t servoPin = digitalPinToBitPosition(pin);
  switch (servoPin) { 
    case 1: 
      compareUnit0 = servoIndex;                                    // index into the channels array
      channels[servoIndex].CompareUnit = 1;                         // attach the channel to the Compare Unit
      _TIMER.CTRLB |= TCA_SINGLE_CMP1EN_bm;                         // enable Compare Unit 1
      TCAMUX &= ~PORTMUX_TCA0_1_bm;                                 // use the default pin
    break;
    case 2: 
      compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      _TIMER.CTRLB |= TCA_SINGLE_CMP2EN_bm;                         // enable Compare Unit 2     
      TCAMUX &= ~PORTMUX_TCA0_2_bm;                                 // use the default pin
    break;
    case 3:
      compareUnit0 = servoIndex;
      channels[servoIndex].CompareUnit = 0;
      _TIMER.CTRLB |= TCA_SINGLE_CMP0EN_bm;                         // enable Compare Unit 0
      TCAMUX &= ~PORTMUX_TCA0_0_bm;                                 // use the default pin
    break;
    case 7:                                                         // Not on 14 pins processors
      compareUnit0 = servoIndex;
      channels[servoIndex].CompareUnit = 0;
      _TIMER.CTRLB |= TCA_SINGLE_CMP0EN_bm;                         // enable Compare Unit 0     
      TCAMUX |= PORTMUX_TCA0_0_bm;                                  // use the alternativ pin
    break;
    default:                                                        // PA0, PA4, PA5 and PA6 can't be used
      return false;
    break;
  }
  pinMode(pin, OUTPUT);                                             // Set the pin as output
  return true;
}

//
#else                                                               // _AVR_PINCOUNT != 8
//===

static boolean initCompareUnit(byte pin, uint8_t servoIndex) {
  uint8_t servoPin = digitalPinToBitPosition(pin);
  uint8_t servoPort = digitalPinToPort(pin);
  if (servoPort != PB) return false;
  switch (servoPin) { 
    case 0:                                                         // Compare Unit 0
      compareUnit0 = servoIndex;                                    // index into the channels array
      channels[servoIndex].CompareUnit = 0;                         // attach the channel to the Compare Unit
      _TIMER.CTRLB |= TCA_SINGLE_CMP0EN_bm;                         // enable Compare Unit 0
      TCAMUX &= ~PORTMUX_TCA0_0_bm;                                 // use the default pin
    break;
    case 1: 
      compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      _TIMER.CTRLB |= TCA_SINGLE_CMP1EN_bm;                         // enable Compare Unit 1
      TCAMUX &= ~PORTMUX_TCA0_1_bm;                                 // use the default pin
    break;
    case 2: 
      compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      _TIMER.CTRLB |= TCA_SINGLE_CMP2EN_bm;                         // enable Compare Unit 2     
      TCAMUX &= ~PORTMUX_TCA0_2_bm;                                 // use the default pin
    break;
    case 3:
      compareUnit0 = servoIndex;
      channels[servoIndex].CompareUnit = 0;
      _TIMER.CTRLB |= TCA_SINGLE_CMP0EN_bm;                         // enable Compare Unit 0
      TCAMUX |= PORTMUX_TCA0_0_bm;                                  // use the alternativ pin
    break;
    case 4:                                                         // Not on 14 pins processors
      compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      _TIMER.CTRLB |= TCA_SINGLE_CMP1EN_bm;                         // enable Compare Unit 1
      TCAMUX |= PORTMUX_TCA0_1_bm;                                  // use the alternativ pin
    break;
    case 5:                                                         // Not on 14 pins processors
      compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      _TIMER.CTRLB |= TCA_SINGLE_CMP2EN_bm;                         // enable Compare Unit 2     
      TCAMUX |= PORTMUX_TCA0_2_bm;                                  // use the alternativ pin
    break;
    default:                                                        // In case PB6 or PB7 was selected
      return false;
    break;
  }
  pinMode(pin, OUTPUT);                                             // Set the pin as output
  return true;
}

//
#endif                                                              // _AVR_PINCOUNT != 8
//====

//======================================================================================================
#else
#error "TCA0 is not defined for this processor / Board is not MEGATINYCORE"
#endif
