//******************************************************************************************************
//
// file:      curves.h
// author:    Aiko Pras
// history:   2025-02-11 V1.0.0 ap initial version
//
// purpose:   Predefined curves for servos.
//            The idea is to define a number of curves; each describing a specific path that servos
//            may take. A curve consists of a series of curve points. 
//            Depending on the processor being used, these predefined servo curves may be stored
//            in PROGMEM (flash).
//            
// source:    The idea of predefined servo's curves, the curves itself, some text and parts of the code
//            is based on / copied from the servo specific source file of the opendcc decoder project.
//            That copyright (c) is with Wolfgang Kufer.
//            https://www.opendcc.de/elektronik/opendecoder/opendecoder_sw_servo.html
//
// Arrays in PROGMEM are static (permanent) variables, and can not be part of a dynamic structure,
// such as a class. Therefore they are defined as global variables.
//
// The #if directives at the beginning, are copied from the DxCore GitHub documentation:
// https://github.com/SpenceKonde/megaTinyCore/blob/master/ContributionGuidelines.md
//
// Additional information can be found at:
// https://github.com/SpenceKonde/DxCore/blob/master/megaavr/extras/Ref_PROGMEM.md
// It seems that the 32K parts always use __AVR_ARCH__ == 103 
// The 64K and 128K always seem to use PROGMEM
//
//******************************************************************************************************
#pragma once

#if __AVR_ARCH__ == 103
  #define FLASH_MEMORY
  #define lookupTime (src->time)
  #define lookupPosition (src->position)
#elif defined(FLMAP_LOCKED)
  #define FLASH_MEMORY PROGMEM_MAPPED
  #define lookupTime (src->time)
  #define lookupPosition (src->position)
#else
  #define FLASH_MEMORY PROGMEM
  #define lookupTime (pgm_read_byte(&src->time))
  #define lookupPosition (pgm_read_byte(&src->position))
#endif


#define SIZE_SERVO_CURVE 24                      // Maximum number of curve points withi a curve
#define NUMBER_OF_LAST_CURVE 10                  // For PROGMEM! Should match the number in curves.cpp.

typedef struct {                                 // A servo curve consists of a sequence of points
  uint8_t time;                                  // counter for the number of 20ms ticks
  uint8_t position;                              // A value that will be converted into microseconds
} curvePoint_t;

extern const curvePoint_t *PredefinedCurves[];   // The collection of all predefined curves
