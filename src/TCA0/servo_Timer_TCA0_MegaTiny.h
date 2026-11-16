//******************************************************************************************************
//
// file:      servo_Timer_TCA0_MegaTiny.h
// author:    Aiko Pras
// history:   2024-01-03 V1.0.0 ap initial version
//            2026-11-16 V2.0.0 ap 8-pin: PA1 (WO1) now uses compareUnit1
//                                 the functions got the postfix TCA0, and the state is kept in tca[0].
//                                 New: pinFitsTCA0()
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

// Version 2: The defines _TIMER, takeOverTCA(), resumeTCA() and ServoHandler have been removed.
// servo_TCA.cpp now addresses TCA0 directly.


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

static boolean initCompareUnitTCA0(byte pin, uint8_t servoIndex) {
  uint8_t servoPin = digitalPinToBitPosition(pin);
  switch (servoPin) { 
    case 1: 
      tca[0].compareUnit1 = servoIndex;                             // index into the channels array
      channels[servoIndex].CompareUnit = 1;                         // attach the channel to the Compare Unit
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;                    // enable Compare Unit 1
      TCAMUX &= ~PORTMUX_TCA0_1_bm;                                 // use the default pin
    break;
    case 2: 
      tca[0].compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;                    // enable Compare Unit 2
      TCAMUX &= ~PORTMUX_TCA0_2_bm;                                 // use the default pin
    break;
    case 3:
      tca[0].compareUnit0 = servoIndex;
      channels[servoIndex].CompareUnit = 0;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;                    // enable Compare Unit 0
      TCAMUX &= ~PORTMUX_TCA0_0_bm;                                 // use the default pin
    break;
    case 7:                                                         // Not on 14 pins processors
      tca[0].compareUnit0 = servoIndex;
      channels[servoIndex].CompareUnit = 0;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;                    // enable Compare Unit 0
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

static boolean initCompareUnitTCA0(byte pin, uint8_t servoIndex) {
  uint8_t servoPin = digitalPinToBitPosition(pin);
  uint8_t servoPort = digitalPinToPort(pin);
  if (servoPort != PB) return false;
  switch (servoPin) { 
    case 0:                                                         // Compare Unit 0
      tca[0].compareUnit0 = servoIndex;                             // index into the channels array
      channels[servoIndex].CompareUnit = 0;                         // attach the channel to the Compare Unit
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;                    // enable Compare Unit 0
      TCAMUX &= ~PORTMUX_TCA0_0_bm;                                 // use the default pin
    break;
    case 1: 
      tca[0].compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;                    // enable Compare Unit 1
      TCAMUX &= ~PORTMUX_TCA0_1_bm;                                 // use the default pin
    break;
    case 2: 
      tca[0].compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;                    // enable Compare Unit 2
      TCAMUX &= ~PORTMUX_TCA0_2_bm;                                 // use the default pin
    break;
    case 3:
      tca[0].compareUnit0 = servoIndex;
      channels[servoIndex].CompareUnit = 0;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;                    // enable Compare Unit 0
      TCAMUX |= PORTMUX_TCA0_0_bm;                                  // use the alternativ pin
    break;
    case 4:                                                         // Not on 14 pins processors
      tca[0].compareUnit1 = servoIndex;
      channels[servoIndex].CompareUnit = 1;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;                    // enable Compare Unit 1
      TCAMUX |= PORTMUX_TCA0_1_bm;                                  // use the alternativ pin
    break;
    case 5:                                                         // Not on 14 pins processors
      tca[0].compareUnit2 = servoIndex;
      channels[servoIndex].CompareUnit = 2;
      TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;                    // enable Compare Unit 2
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


//******************************************************************************************************
// Version 2: pinFitsTCA0() is used by attach() to decide if the servo can use TCA0 for this pin.
// The ATtiny processors have only TCA0. Therefore initCompareUnitTCA0() decides, as before.
//******************************************************************************************************
static boolean pinFitsTCA0(byte pin) {
  (void) pin;                                                       // pin is not needed here
  return true;
}

//======================================================================================================
#else
#error "TCA0 is not defined for this processor / Board is not MEGATINYCORE"
#endif
