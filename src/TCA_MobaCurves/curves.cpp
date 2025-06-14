//******************************************************************************************************
//
// file:      curves.cpp
// author:    Aiko Pras
// history:   2025-02-11 V1.0.0 ap initial version
//            2025-06-01 V1.0.1 ap move_A and move_B are now between 0..255
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
// Note on Scaling: 
//  x: this is the time scaled in integer steps. When the curve is
//     processed, the time is stretched by servo.timeStretch.
//  y: Independant of the actual positioning the predefined curves
//     always have a range from 0 to 255. When the curve is
//     processed, this is scaled between MIN and MAX of this servos.
//     For convienience and after whip, all curves should have
//     25 and 230 resp. as endpoints.
//
// -------------------------------------------------------------------------------------
//    Name         Decription                    min  max   start  end    time
// -------------------------------------------------------------------------------------
//  0 lin_A      | linear move, part A             0  255       0  255    80ms
//  1 lin_B      | linear move, part B             0  255     255    0    80ms
//  2 move_A     | smooth move, part A (cos)       0  255       0  255   180ms
//  3 move_B     | smooth move, part B (cos)       0  255     255    0   180ms
//  4 sine_A     | sinosoidal wave, part A       128  255     128  128   400ms
//  5 sine_B     | sinosoidal wave, part B         1  128     128  128   400ms 
//  6 whip_A     | parabola, part A              128  255     128  128   320ms
//  7 whip_B:    | parabola, part B                1  128     128  128   320ms 
//  8 sig_hp0:   | close flag, after-whip          1  240     230   25   700ms
//  9 sig_hp1:   | open flag, after-whip           1  240      25  230   700ms
// 10 sig_hp1p:  | open flag, pause, after-whip    1  240      25  230  1280ms
// 11 sine_AB:   | sinosoidal move, complete sine  1  255     128  128   800ms
//
//******************************************************************************************************
#include <Arduino.h>
#include "curves.h"


const curvePoint_t FLASH_MEMORY lin_A[] = {
  { 0 , 0 },
  { 2 , 128 },
  { 4 , 255 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY lin_B[] = { 
  { 0 , 255 },
  { 2 , 128 },
  { 4 , 0 },
  { 0 , 0 },
};


const curvePoint_t FLASH_MEMORY move_A[] = { 
  { 0 , 0 },
  { 1 , 5 },
  { 2 , 17 },
  { 3 , 37 },
  { 4 , 64 },
  { 6 , 128 },
  { 8 , 192 },
  { 9 , 218 },
  { 10 , 238 },
  { 11 , 250 },
  { 12 , 255 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY move_B[] = { 
  { 0 , 255 },
  { 1 , 250 },
  { 2 , 238 },
  { 3 , 218 },
  { 4 , 192 },
  { 6 , 128 },
  { 8 , 64 },
  { 9 , 37 },
  { 10 , 17 },
  { 11 , 5 },
  { 12 , 0 },
  { 0 , 0 },
};


const curvePoint_t FLASH_MEMORY sig_hp0[] = { 
  { 0 , 230 },
  { 1 , 224 },
  { 7 , 140 },
  { 9 , 89 },
  { 10 , 51 },
  { 11 , 26 },
  { 12 , 16 },
  { 13 , 15 },
  { 15 , 29 },
  { 16 , 33 },
  { 17 , 31 },
  { 19 , 21 },
  { 20 , 20 },
  { 22 , 27 },
  { 23 , 29 },
  { 25 , 26 },
  { 26 , 23 },
  { 27 , 23 },
  { 29 , 26 },
  { 30 , 27 },
  { 31 , 27 },
  { 33 , 24 },
  { 35 , 25 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY sig_hp1[] = { 
  { 0 , 26 },
  { 11 , 115 },
  { 13 , 122 },
  { 16 , 128 },
  { 28 , 122 },
  { 40 , 230 },
  { 41 , 239 },
  { 42 , 240 },
  { 44 , 226 },
  { 45 , 222 },
  { 46 , 224 },
  { 48 , 234 },
  { 49 , 235 },
  { 51 , 228 },
  { 52 , 226 },
  { 54 , 230 },
  { 55 , 232 },
  { 56 , 232 },
  { 58 , 229 },
  { 59 , 228 },
  { 60 , 228 },
  { 62 , 231 },
  { 64 , 230 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY sine_A[] = { 
  { 0 , 128 },
  { 3 , 186 },
  { 5 , 218 },
  { 7 , 241 },
  { 8 , 249 },
  { 9 , 253 },
  { 10 , 255 },
  { 11 , 253 },
  { 12 , 249 },
  { 13 , 241 },
  { 15 , 218 },
  { 17 , 186 },
  { 20 , 128 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY sine_B[] = { 
  { 0 , 128 },
  { 3 , 70 },
  { 5 , 38 },
  { 7 , 15 },
  { 8 , 7 },
  { 9 , 3 },
  { 10 , 1 },
  { 11 , 3 },
  { 12 , 7 },
  { 13 , 15 },
  { 15 , 38 },
  { 17 , 70 },
  { 20 , 128 },
  { 0 , 0 },
};
   
const curvePoint_t FLASH_MEMORY whip_A[] = { 
  { 0 , 128 },
  { 1 , 135 },
  { 2 , 145 },
  { 3 , 157 },
  { 4 , 172 },
  { 5 , 189 },
  { 6 , 208 },
  { 8 , 255 },
  { 10 , 208 },
  { 11 , 189 },
  { 12 , 172 },
  { 13 , 157 },
  { 14 , 145 },
  { 15 , 135 },
  { 16 , 128 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY whip_B[] = { 
  {  0 , 128 },
  {  1 , 121 },
  {  2 , 111 },
  {  3 , 99 },
  {  4 , 84 },
  {  5 , 67 },
  {  6 , 48 },
  {  8 , 1 },
  { 10 , 48 },
  { 11 , 67 },
  { 12 , 84 },
  { 13 , 99 },
  { 14 , 111 },
  { 15 , 121 },
  { 16 , 128 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY hp0[] = { 
  { 0 , 230 },
  { 1 , 224 },
  { 7 , 140 },
  { 9 , 89 },
  { 10 , 51 },
  { 11 , 26 },
  { 12 , 16 },
  { 13 , 15 },
  { 15 , 29 },
  { 16 , 33 },
  { 17 , 31 },
  { 19 , 21 },
  { 20 , 20 },
  { 22 , 27 },
  { 23 , 29 },
  { 25 , 26 },
  { 26 , 23 },
  { 27 , 23 },
  { 29 , 26 },
  { 30 , 27 },
  { 31 , 27 },
  { 33 , 24 },
  { 35 , 25 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY hp1[] = { 
  { 0 , 26 },
  { 1 , 31 },
  { 7 , 115 },
  { 9 , 166 },
  { 10 , 204 },
  { 11 , 230 },
  { 12 , 239 },
  { 13 , 240 },
  { 15 , 226 },
  { 16 , 222 },
  { 17 , 224 },
  { 19 , 234 },
  { 20 , 235 },
  { 22 , 228 },
  { 23 , 226 },
  { 25 , 230 },
  { 26 , 232 },
  { 27 , 232 },
  { 29 , 229 },
  { 30 , 228 },
  { 31 , 228 },
  { 33 , 231 },
  { 35 , 230 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY hp1p[] = { 
  { 0 , 26 },
  { 11 , 115 },
  { 13 , 122 },
  { 16 , 128 },
  { 28 , 122 },
  { 40 , 230 },
  { 41 , 239 },
  { 42 , 240 },
  { 44 , 226 },
  { 45 , 222 },
  { 46 , 224 },
  { 48 , 234 },
  { 49 , 235 },
  { 51 , 228 },
  { 52 , 226 },
  { 54 , 230 },
  { 55 , 232 },
  { 56 , 232 },
  { 58 , 229 },
  { 59 , 228 },
  { 60 , 228 },
  { 62 , 231 },
  { 64 , 230 },
  { 0 , 0 },
};

const curvePoint_t FLASH_MEMORY sine_AB[] = { 
  { 0 , 128 },
  { 3 , 186 },
  { 5 , 218 },
  { 7 , 241 },
  { 9 , 253 },
  { 10 , 255 },
  { 11 , 253 },
  { 13 , 241 },
  { 15 , 218 },
  { 17 , 186 },
  { 20 , 128 },
  { 23 , 70 },
  { 25 , 38 },
  { 27 , 15 },
  { 29 , 3 },
  { 30 , 1 },
  { 31 , 3 },
  { 33 , 15 },
  { 35 , 38 },
  { 37 , 70 },
  { 40 , 128 },
  { 0 , 0 },
};

// -----------------------------------------------------------------------------------------------
// The array with (pointers to) the predefined curves differs from that of OpenDCC - OpenDecoder2.
// In contrast to OpenDCC, the array below does NOT include the EEPROM curves.
// If the array below gets modified, dom't forget to modify NUMBER_OF_LAST_CURVE in curves.h as well
//
// - 2025/04/27: sine_AB added (may be used for testing purposes)
//
const curvePoint_t *PredefinedCurves[] = {
  lin_A,    // 0
  lin_B,    // 1
  move_A,   // 2
  move_B,   // 3
  sine_A,   // 4
  sine_B,   // 5
  whip_A,   // 6
  whip_B,   // 7
  sig_hp0,  // 8
  sig_hp1,  // 9
  hp1p,     // 10
  sine_AB,  // 11
};
