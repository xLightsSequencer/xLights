/*
  WS2812FX.h - Library for WS2812 LED effects.
  Harm Aldick - 2016
  www.aldick.org
  LICENSE
  The MIT License (MIT)
  Copyright (c) 2016  Harm Aldick
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  Modified for WLED

  Modified for xLights
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#ifndef XLIGHTS_FX
#include "const.h"

#define FASTLED_INTERNAL //remove annoying pragma messages
#define USE_GET_MILLISECOND_TIMER
#include "FastLED.h"
#else
// I am attempting here to minimise the change from the WLED versions of these files
// These are brought in from the other header files in WLED where required

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../Color.h"

#define PROGMEM
#define byte uint8_t
#define fract8 uint8_t
#define fract16 uint16_t
#define saccum87 int16_t
typedef uint16_t accum88;
class RenderBuffer;
#define CRGB xlCRGB
#define instance _fx

#define scale8_LEAVING_R1_DIRTY ::scale8
uint8_t scale8(uint8_t a, fract8 b);
int16_t sin16(uint16_t theta);
uint8_t sin8(uint8_t theta);
uint16_t scale16(uint16_t i, fract16 scale);

//Segment option byte bits
#define SEG_OPTION_SELECTED 0
#define SEG_OPTION_REVERSED 1
#define SEG_OPTION_ON 2
#define SEG_OPTION_MIRROR 3   //Indicates that the effect will be mirrored within the segment
#define SEG_OPTION_NONUNITY 4 //Indicates that the effect does not use FRAMETIME or needs getPixelColor
#define SEG_OPTION_FREEZE 5   //Segment contents will not be refreshed
#define SEG_OPTION_TRANSITIONAL 7

//Segment differs return byte
#define SEG_DIFFERS_BRI 0x01
#define SEG_DIFFERS_OPT 0x02
#define SEG_DIFFERS_COL 0x04
#define SEG_DIFFERS_FX 0x08
#define SEG_DIFFERS_BOUNDS 0x10
#define SEG_DIFFERS_GSO 0x20 

#define GRADIENT_PALETTE_COUNT 58

const byte ib_jul01_gp[] PROGMEM = {
    0, 194, 1, 1,
    94, 1, 29, 18,
    132, 57, 131, 28,
    255, 113, 1, 1
};

// Gradient palette "es_vintage_57_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_57.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte es_vintage_57_gp[] PROGMEM = {
    0, 2, 1, 1,
    53, 18, 1, 0,
    104, 69, 29, 1,
    153, 167, 135, 10,
    255, 46, 56, 4
};

// Gradient palette "es_vintage_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte es_vintage_01_gp[] PROGMEM = {
    0, 4, 1, 1,
    51, 16, 0, 1,
    76, 97, 104, 3,
    101, 255, 131, 19,
    127, 67, 9, 4,
    153, 16, 0, 1,
    229, 4, 1, 1,
    255, 4, 1, 1
};

// Gradient palette "es_rivendell_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/rivendell/tn/es_rivendell_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte es_rivendell_15_gp[] PROGMEM = {
    0, 1, 14, 5,
    101, 16, 36, 14,
    165, 56, 68, 30,
    242, 150, 156, 99,
    255, 150, 156, 99
};

// Gradient palette "rgi_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/rgi/tn/rgi_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.
// Edited to be brighter

const byte rgi_15_gp[] PROGMEM = {
    0, 4, 1, 70,
    31, 55, 1, 30,
    63, 255, 4, 7,
    95, 59, 2, 29,
    127, 11, 3, 50,
    159, 39, 8, 60,
    191, 112, 19, 40,
    223, 78, 11, 39,
    255, 29, 8, 59
};

// Gradient palette "retro2_16_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/retro2/tn/retro2_16.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

const byte retro2_16_gp[] PROGMEM = {
    0, 188, 135, 1,
    255, 46, 7, 1
};

// Gradient palette "Analogous_1_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/red/tn/Analogous_1.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte Analogous_1_gp[] PROGMEM = {
    0, 3, 0, 255,
    63, 23, 0, 255,
    127, 67, 0, 255,
    191, 142, 0, 45,
    255, 255, 0, 0
};

// Gradient palette "es_pinksplash_08_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/pink_splash/tn/es_pinksplash_08.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte es_pinksplash_08_gp[] PROGMEM = {
    0, 126, 11, 255,
    127, 197, 1, 22,
    175, 210, 157, 172,
    221, 157, 3, 112,
    255, 157, 3, 112
};

// Gradient palette "es_ocean_breeze_036_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_036.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

const byte es_ocean_breeze_036_gp[] PROGMEM = {
    0, 1, 6, 7,
    89, 1, 99, 111,
    153, 144, 209, 255,
    255, 0, 73, 82
};

// Gradient palette "departure_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/mjf/tn/departure.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 88 bytes of program space.

const byte departure_gp[] PROGMEM = {
    0, 8, 3, 0,
    42, 23, 7, 0,
    63, 75, 38, 6,
    84, 169, 99, 38,
    106, 213, 169, 119,
    116, 255, 255, 255,
    138, 135, 255, 138,
    148, 22, 255, 24,
    170, 0, 255, 0,
    191, 0, 136, 0,
    212, 0, 55, 0,
    255, 0, 55, 0
};

// Gradient palette "es_landscape_64_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_64.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

const byte es_landscape_64_gp[] PROGMEM = {
    0, 0, 0, 0,
    37, 2, 25, 1,
    76, 15, 115, 5,
    127, 79, 213, 1,
    128, 126, 211, 47,
    130, 188, 209, 247,
    153, 144, 182, 205,
    204, 59, 117, 250,
    255, 1, 37, 192
};

// Gradient palette "es_landscape_33_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_33.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte es_landscape_33_gp[] PROGMEM = {
    0, 1, 5, 0,
    19, 32, 23, 1,
    38, 161, 55, 1,
    63, 229, 144, 1,
    66, 39, 142, 74,
    255, 1, 4, 1
};

// Gradient palette "rainbowsherbet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/icecream/tn/rainbowsherbet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte rainbowsherbet_gp[] PROGMEM = {
    0, 255, 33, 4,
    43, 255, 68, 25,
    86, 255, 7, 25,
    127, 255, 82, 103,
    170, 255, 255, 242,
    209, 42, 255, 22,
    255, 87, 255, 65
};

// Gradient palette "gr65_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr65_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte gr65_hult_gp[] PROGMEM = {
    0, 247, 176, 247,
    48, 255, 136, 255,
    89, 220, 29, 226,
    160, 7, 82, 178,
    216, 1, 124, 109,
    255, 1, 124, 109
};

// Gradient palette "gr64_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr64_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte gr64_hult_gp[] PROGMEM = {
    0, 1, 124, 109,
    66, 1, 93, 79,
    104, 52, 65, 1,
    130, 115, 127, 1,
    150, 52, 65, 1,
    201, 1, 86, 72,
    239, 0, 55, 45,
    255, 0, 55, 45
};

// Gradient palette "GMT_drywet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_drywet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte GMT_drywet_gp[] PROGMEM = {
    0, 47, 30, 2,
    42, 213, 147, 24,
    84, 103, 219, 52,
    127, 3, 219, 207,
    170, 1, 48, 214,
    212, 1, 1, 111,
    255, 1, 7, 33
};

// Gradient palette "ib15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ing/general/tn/ib15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte ib15_gp[] PROGMEM = {
    0, 113, 91, 147,
    72, 157, 88, 78,
    89, 208, 85, 33,
    107, 255, 29, 11,
    141, 137, 31, 39,
    255, 59, 33, 89
};

// Gradient palette "Tertiary_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/vermillion/tn/Tertiary_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte Tertiary_01_gp[] PROGMEM = {
    0, 0, 1, 255,
    63, 3, 68, 45,
    127, 23, 255, 0,
    191, 100, 68, 1,
    255, 255, 1, 4
};

// Gradient palette "lava_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/lava.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

const byte lava_gp[] PROGMEM = {
    0, 0, 0, 0,
    46, 18, 0, 0,
    96, 113, 0, 0,
    108, 142, 3, 1,
    119, 175, 17, 1,
    146, 213, 44, 2,
    174, 255, 82, 4,
    188, 255, 115, 4,
    202, 255, 156, 4,
    218, 255, 203, 4,
    234, 255, 255, 4,
    244, 255, 255, 71,
    255, 255, 255, 255
};

// Gradient palette "fierce_ice_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/fierce-ice.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte fierce_ice_gp[] PROGMEM = {
    0, 0, 0, 0,
    59, 0, 9, 45,
    119, 0, 38, 255,
    149, 3, 100, 255,
    180, 23, 199, 255,
    217, 100, 235, 255,
    255, 255, 255, 255
};

// Gradient palette "Colorfull_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Colorfull.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

const byte Colorfull_gp[] PROGMEM = {
    0, 10, 85, 5,
    25, 29, 109, 18,
    60, 59, 138, 42,
    93, 83, 99, 52,
    106, 110, 66, 64,
    109, 123, 49, 65,
    113, 139, 35, 66,
    116, 192, 117, 98,
    124, 255, 255, 137,
    168, 100, 180, 155,
    255, 22, 121, 174
};

// Gradient palette "Pink_Purple_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Pink_Purple.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

const byte Pink_Purple_gp[] PROGMEM = {
    0, 19, 2, 39,
    25, 26, 4, 45,
    51, 33, 6, 52,
    76, 68, 62, 125,
    102, 118, 187, 240,
    109, 163, 215, 247,
    114, 217, 244, 255,
    122, 159, 149, 221,
    149, 113, 78, 188,
    183, 128, 57, 155,
    255, 146, 40, 123
};

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte Sunset_Real_gp[] PROGMEM = {
    0, 120, 0, 0,
    22, 179, 22, 0,
    51, 255, 104, 0,
    85, 167, 22, 18,
    135, 100, 0, 103,
    198, 16, 0, 130,
    255, 0, 0, 160
};

// Gradient palette "Sunset_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

const byte Sunset_Yellow_gp[] PROGMEM = {
    0, 10, 62, 123,
    36, 56, 130, 103,
    87, 153, 225, 85,
    100, 199, 217, 68,
    107, 255, 207, 54,
    115, 247, 152, 57,
    120, 239, 107, 61,
    128, 247, 152, 57,
    180, 255, 207, 54,
    223, 255, 227, 48,
    255, 255, 248, 42
};

// Gradient palette "Beech_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Beech.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 60 bytes of program space.

const byte Beech_gp[] PROGMEM = {
    0, 255, 252, 214,
    12, 255, 252, 214,
    22, 255, 252, 214,
    26, 190, 191, 115,
    28, 137, 141, 52,
    28, 112, 255, 205,
    50, 51, 246, 214,
    71, 17, 235, 226,
    93, 2, 193, 199,
    120, 0, 156, 174,
    133, 1, 101, 115,
    136, 1, 59, 71,
    136, 7, 131, 170,
    208, 1, 90, 151,
    255, 0, 56, 133
};

// Gradient palette "Another_Sunset_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Another_Sunset.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte Another_Sunset_gp[] PROGMEM = {
    0, 110, 49, 11,
    29, 55, 34, 10,
    68, 22, 22, 9,
    68, 239, 124, 8,
    97, 220, 156, 27,
    124, 203, 193, 61,
    178, 33, 53, 56,
    255, 0, 1, 52
};

// Gradient palette "es_autumn_19_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/autumn/tn/es_autumn_19.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

const byte es_autumn_19_gp[] PROGMEM = {
    0, 26, 1, 1,
    51, 67, 4, 1,
    84, 118, 14, 1,
    104, 137, 152, 52,
    112, 113, 65, 1,
    122, 133, 149, 59,
    124, 137, 152, 52,
    135, 113, 65, 1,
    142, 139, 154, 46,
    163, 113, 13, 1,
    204, 55, 3, 1,
    249, 17, 1, 1,
    255, 17, 1, 1
};

// Gradient palette "BlacK_Blue_Magenta_White_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Blue_Magenta_White.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte BlacK_Blue_Magenta_White_gp[] PROGMEM = {
    0, 0, 0, 0,
    42, 0, 0, 45,
    84, 0, 0, 255,
    127, 42, 0, 255,
    170, 255, 0, 255,
    212, 255, 55, 255,
    255, 255, 255, 255
};

// Gradient palette "BlacK_Magenta_Red_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Magenta_Red.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte BlacK_Magenta_Red_gp[] PROGMEM = {
    0, 0, 0, 0,
    63, 42, 0, 45,
    127, 255, 0, 255,
    191, 255, 0, 45,
    255, 255, 0, 0
};

// Gradient palette "BlacK_Red_Magenta_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Red_Magenta_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

const byte BlacK_Red_Magenta_Yellow_gp[] PROGMEM = {
    0, 0, 0, 0,
    42, 42, 0, 0,
    84, 255, 0, 0,
    127, 255, 0, 45,
    170, 255, 0, 255,
    212, 255, 55, 45,
    255, 255, 255, 0
};

// Gradient palette "Blue_Cyan_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/Blue_Cyan_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte Blue_Cyan_Yellow_gp[] PROGMEM = {
    0, 0, 0, 255,
    63, 0, 55, 255,
    127, 0, 255, 255,
    191, 42, 255, 45,
    255, 255, 255, 0
};

//Custom palette by Aircoookie

const byte Orange_Teal_gp[] PROGMEM = {
    0, 0, 150, 92,
    55, 0, 150, 92,
    200, 255, 72, 0,
    255, 255, 72, 0
};

//Custom palette by Aircoookie

const byte Tiamat_gp[] PROGMEM = {
    0, 1, 2, 14,        //gc
    33, 2, 5, 35,       //gc from 47, 61,126
    100, 13, 135, 92,   //gc from 88,242,247
    120, 43, 255, 193,  //gc from 135,255,253
    140, 247, 7, 249,   //gc from 252, 69,253
    160, 193, 17, 208,  //gc from 231, 96,237
    180, 39, 255, 154,  //gc from 130, 77,213
    200, 4, 213, 236,   //gc from 57,122,248
    220, 39, 252, 135,  //gc from 177,254,255
    240, 193, 213, 253, //gc from 203,239,253
    255, 255, 249, 255
};

//Custom palette by Aircoookie

const byte April_Night_gp[] PROGMEM = {
    0, 1, 5, 45, //deep blue
    10, 1, 5, 45,
    25, 5, 169, 175, //light blue
    40, 1, 5, 45,
    61, 1, 5, 45,
    76, 45, 175, 31, //green
    91, 1, 5, 45,
    112, 1, 5, 45,
    127, 249, 150, 5, //yellow
    143, 1, 5, 45,
    162, 1, 5, 45,
    178, 255, 92, 0, //pastel orange
    193, 1, 5, 45,
    214, 1, 5, 45,
    229, 223, 45, 72, //pink
    244, 1, 5, 45,
    255, 1, 5, 45
};

const byte Orangery_gp[] PROGMEM = {
    0, 255, 95, 23,
    30, 255, 82, 0,
    60, 223, 13, 8,
    90, 144, 44, 2,
    120, 255, 110, 17,
    150, 255, 69, 0,
    180, 158, 13, 11,
    210, 241, 82, 17,
    255, 213, 37, 4
};

//inspired by Mark Kriegsman https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
const byte C9_gp[] PROGMEM = {
    0, 184, 4, 0, //red
    60, 184, 4, 0,
    65, 144, 44, 2, //amber
    125, 144, 44, 2,
    130, 4, 96, 2, //green
    190, 4, 96, 2,
    195, 7, 7, 88, //blue
    255, 7, 7, 88
};

const byte Sakura_gp[] PROGMEM = {
    0, 196, 19, 10,
    65, 255, 69, 45,
    130, 223, 45, 72,
    195, 255, 82, 103,
    255, 223, 13, 17
};

const byte Aurora_gp[] PROGMEM = {
    0, 1, 5, 45, //deep blue
    64, 0, 200, 23,
    128, 0, 255, 0, //green
    170, 0, 243, 45,
    200, 0, 135, 7,
    255, 1, 5, 45
}; //deep blue

const byte Atlantica_gp[] PROGMEM = {
    0, 0, 28, 112,   //#001C70
    50, 32, 96, 255, //#2060FF
    100, 0, 243, 45,
    150, 12, 95, 82,  //#0C5F52
    200, 25, 190, 95, //#19BE5F
    255, 40, 170, 80
}; //#28AA50

const byte C9_2_gp[] PROGMEM = {
    0, 6, 126, 2, //green
    45, 6, 126, 2,
    45, 4, 30, 114, //blue
    90, 4, 30, 114,
    90, 255, 5, 0, //red
    135, 255, 5, 0,
    135, 196, 57, 2, //amber
    180, 196, 57, 2,
    180, 137, 85, 2, //yellow
    255, 137, 85, 2
};

//C9, but brighter and with a less purple blue
const byte C9_new_gp[] PROGMEM = {
    0, 255, 5, 0, //red
    60, 255, 5, 0,
    60, 196, 57, 2, //amber (start 61?)
    120, 196, 57, 2,
    120, 6, 126, 2, //green (start 126?)
    180, 6, 126, 2,
    180, 4, 30, 114, //blue (start 191?)
    255, 4, 30, 114
};

// Gradient palette "temperature_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/arendal/tn/temperature.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 144 bytes of program space.

const byte temperature_gp[] PROGMEM = {
    0, 1, 27, 105,
    14, 1, 40, 127,
    28, 1, 70, 168,
    42, 1, 92, 197,
    56, 1, 119, 221,
    70, 3, 130, 151,
    84, 23, 156, 149,
    99, 67, 182, 112,
    113, 121, 201, 52,
    127, 142, 203, 11,
    141, 224, 223, 1,
    155, 252, 187, 2,
    170, 247, 147, 1,
    184, 237, 87, 1,
    198, 229, 43, 1,
    226, 171, 2, 2,
    240, 80, 3, 3,
    255, 80, 3, 3
};

const byte Aurora2_gp[] PROGMEM = {
    0, 17, 177, 13,    //Greenish
    64, 121, 242, 5,   //Greenish
    128, 25, 173, 121, //Turquoise
    192, 250, 77, 127, //Pink
    255, 171, 101, 221 //Purple
};

// Gradient palette "bhw1_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 12 bytes of program space.

const byte retro_clown_gp[] PROGMEM = {
    0, 227, 101, 3,
    117, 194, 18, 19,
    255, 92, 8, 192
};

// Gradient palette "bhw1_04_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_04.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

const byte candy_gp[] PROGMEM = {
    0, 229, 227, 1,
    15, 227, 101, 3,
    142, 40, 1, 80,
    198, 17, 1, 79,
    255, 0, 0, 45
};

// Gradient palette "bhw1_05_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_05.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

const byte toxy_reaf_gp[] PROGMEM = {
    0, 1, 221, 53,
    255, 73, 3, 178
};

// Gradient palette "bhw1_06_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_06.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

const byte fairy_reaf_gp[] PROGMEM = {
    0, 184, 1, 128,
    160, 1, 193, 182,
    219, 153, 227, 190,
    255, 255, 255, 255
};

// Gradient palette "bhw1_14_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_14.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

const byte semi_blue_gp[] PROGMEM = {
    0, 0, 0, 0,
    12, 1, 1, 3,
    53, 8, 1, 22,
    80, 4, 6, 89,
    119, 2, 25, 216,
    145, 7, 10, 99,
    186, 15, 2, 31,
    233, 2, 1, 5,
    255, 0, 0, 0
};

// Gradient palette "bhw1_three_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_three.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte pink_candy_gp[] PROGMEM = {
    0, 255, 255, 255,
    45, 7, 12, 255,
    112, 227, 1, 127,
    112, 227, 1, 127,
    140, 255, 255, 255,
    155, 227, 1, 127,
    196, 45, 1, 99,
    255, 255, 255, 255
};

// Gradient palette "bhw1_w00t_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_w00t.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

const byte red_reaf_gp[] PROGMEM = {
    0, 3, 13, 43,
    104, 78, 141, 240,
    188, 255, 0, 0,
    255, 28, 1, 1
};

// Gradient palette "bhw2_23_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_23.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Red & Flash in SR
// Size: 28 bytes of program space.

const byte aqua_flash_gp[] PROGMEM = {
    0, 0, 0, 0,
    66, 57, 227, 233,
    96, 255, 255, 8,
    124, 255, 255, 255,
    153, 255, 255, 8,
    188, 57, 227, 233,
    255, 0, 0, 0
};

// Gradient palette "bhw2_xc_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_xc.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// YBlue in SR
// Size: 28 bytes of program space.

const byte yelblu_hot_gp[] PROGMEM = {
    0, 4, 2, 9,
    58, 16, 0, 47,
    122, 24, 0, 16,
    158, 144, 9, 1,
    183, 179, 45, 1,
    219, 220, 114, 2,
    255, 234, 237, 1
};

// Gradient palette "bhw2_45_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_45.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

const byte lite_light_gp[] PROGMEM = {
    0, 0, 0, 0,
    9, 1, 1, 1,
    40, 5, 5, 6,
    66, 5, 5, 6,
    101, 10, 1, 12,
    255, 0, 0, 0
};

// Gradient palette "bhw2_22_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw2/tn/bhw2_22.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Pink Plasma in SR
// Size: 20 bytes of program space.

const byte red_flash_gp[] PROGMEM = {
    0, 0, 0, 0,
    99, 227, 1, 1,
    130, 249, 199, 95,
    155, 227, 1, 1,
    255, 0, 0, 0
};

// Gradient palette "bhw3_40_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw3/tn/bhw3_40.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

const byte blink_red_gp[] PROGMEM = {
    0, 1, 1, 1,
    43, 4, 1, 11,
    76, 10, 1, 3,
    109, 161, 4, 29,
    127, 255, 86, 123,
    165, 125, 16, 160,
    204, 35, 13, 223,
    255, 18, 2, 18
};

// Gradient palette "bhw3_52_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw3/tn/bhw3_52.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Yellow2Blue in SR
// Size: 28 bytes of program space.

const byte red_shift_gp[] PROGMEM = {
    0, 31, 1, 27,
    45, 34, 1, 16,
    99, 137, 5, 9,
    132, 213, 128, 10,
    175, 199, 22, 1,
    201, 199, 9, 6,
    255, 1, 0, 1
};

// Gradient palette "bhw4_097_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw4/tn/bhw4_097.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Yellow2Red in SR
// Size: 44 bytes of program space.

const byte red_tide_gp[] PROGMEM = {
    0, 247, 5, 0,
    28, 255, 67, 1,
    43, 234, 88, 11,
    58, 234, 176, 51,
    84, 229, 28, 1,
    114, 113, 12, 1,
    140, 255, 225, 44,
    168, 113, 12, 1,
    196, 244, 209, 88,
    216, 255, 28, 1,
    255, 53, 1, 1
};

// Gradient palette "bhw4_017_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw4/tn/bhw4_017.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

const byte candy2_gp[] PROGMEM = {
    0, 39, 33, 34,
    25, 4, 6, 15,
    48, 49, 29, 22,
    73, 224, 173, 1,
    89, 177, 35, 5,
    130, 4, 6, 15,
    163, 255, 114, 6,
    186, 224, 173, 1,
    211, 39, 33, 34,
    255, 1, 1, 1
};

// Single array of defined cpt-city color palettes.
// This will let us programmatically choose one based on
// a number, rather than having to activate each explicitly
// by name every time.
const byte* const gGradientPalettes[] PROGMEM = {
    Sunset_Real_gp,              //13-00 Sunset
    es_rivendell_15_gp,          //14-01 Rivendell
    es_ocean_breeze_036_gp,      //15-02 Breeze
    rgi_15_gp,                   //16-03 Red & Blue
    retro2_16_gp,                //17-04 Yellowout
    Analogous_1_gp,              //18-05 Analogous
    es_pinksplash_08_gp,         //19-06 Splash
    Sunset_Yellow_gp,            //20-07 Pastel
    Another_Sunset_gp,           //21-08 Sunset2
    Beech_gp,                    //22-09 Beech
    es_vintage_01_gp,            //23-10 Vintage
    departure_gp,                //24-11 Departure
    es_landscape_64_gp,          //25-12 Landscape
    es_landscape_33_gp,          //26-13 Beach
    rainbowsherbet_gp,           //27-14 Sherbet
    gr65_hult_gp,                //28-15 Hult
    gr64_hult_gp,                //29-16 Hult64
    GMT_drywet_gp,               //30-17 Drywet
    ib_jul01_gp,                 //31-18 Jul
    es_vintage_57_gp,            //32-19 Grintage
    ib15_gp,                     //33-20 Rewhi
    Tertiary_01_gp,              //34-21 Tertiary
    lava_gp,                     //35-22 Fire
    fierce_ice_gp,               //36-23 Icefire
    Colorfull_gp,                //37-24 Cyane
    Pink_Purple_gp,              //38-25 Light Pink
    es_autumn_19_gp,             //39-26 Autumn
    BlacK_Blue_Magenta_White_gp, //40-27 Magenta
    BlacK_Magenta_Red_gp,        //41-28 Magred
    BlacK_Red_Magenta_Yellow_gp, //42-29 Yelmag
    Blue_Cyan_Yellow_gp,         //43-30 Yelblu
    Orange_Teal_gp,              //44-31 Orange & Teal
    Tiamat_gp,                   //45-32 Tiamat
    April_Night_gp,              //46-33 April Night
    Orangery_gp,                 //47-34 Orangery
    C9_gp,                       //48-35 C9
    Sakura_gp,                   //49-36 Sakura
    Aurora_gp,                   //50-37 Aurora
    Atlantica_gp,                //51-38 Atlantica
    C9_2_gp,                     //52-39 C9 2
    C9_new_gp,                   //53-40 C9 New
    temperature_gp,              //54-41 Temperature
    Aurora2_gp,                  //55-42 Aurora 2
    retro_clown_gp,              //56-43 Retro Clown
    candy_gp,                    //57-44 Candy
    toxy_reaf_gp,                //58-45 Toxy Reaf
    fairy_reaf_gp,               //59-46 Fairy Reaf
    semi_blue_gp,                //60-47 Semi Blue
    pink_candy_gp,               //61-48 Pink Candy
    red_reaf_gp,                 //62-49 Red Reaf
    aqua_flash_gp,               //63-50 Aqua Flash
    yelblu_hot_gp,               //64-51 Yelblu Hot
    lite_light_gp,               //65-52 Lite Light
    red_flash_gp,                //66-53 Red Flash
    blink_red_gp,                //67-54 Blink Red
    red_shift_gp,                //68-55 Red Shift
    red_tide_gp,                 //69-56 Red Tide
    candy2_gp                    //70-57 Candy2
};

struct CHSV {
    union {
        struct {
            union {
                uint8_t hue;
                uint8_t h;
            };
            union {
                uint8_t saturation;
                uint8_t sat;
                uint8_t s;
            };
            union {
                uint8_t value;
                uint8_t val;
                uint8_t v;
            };
        };
        uint8_t raw[3];
    };

    /// Array access operator to index into the chsv object
    inline uint8_t& operator[](uint8_t x) //__attribute__((always_inline))
    {
        return raw[x];
    }

    /// Array access operator to index into the chsv object
    inline const uint8_t& operator[](uint8_t x) const //__attribute__((always_inline))
    {
        return raw[x];
    }

    /// default values are UNITIALIZED
    inline CHSV() //__attribute__((always_inline)) 
        = default;

    /// allow construction from H, S, V
    inline CHSV(uint8_t ih, uint8_t is, uint8_t iv) //__attribute__((always_inline)) 
        : h(ih), s(is), v(iv)
    {
    }

    /// allow copy construction
    inline CHSV(const CHSV& rhs) //__attribute__((always_inline)) 
        = default;

    inline CHSV& operator=(const CHSV& rhs) //__attribute__((always_inline)) 
        = default;

    inline CHSV& setHSV(uint8_t ih, uint8_t is, uint8_t iv) //__attribute__((always_inline))
    {
        h = ih;
        s = is;
        v = iv;
        return *this;
    }
};

struct CRGB;

void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb);
void nscale8x3_video(uint8_t& r, uint8_t& g, uint8_t& b, fract8 scale);
void nscale8x3(uint8_t& r, uint8_t& g, uint8_t& b, fract8 scale);
uint8_t DecodeMode(const std::string& mode);
uint8_t DecodePalette(const std::string& palette);
uint32_t convertColour(const xlColor& c);

typedef enum { FORWARD_HUES,
               BACKWARD_HUES,
               SHORTEST_HUES,
               LONGEST_HUES } TGradientDirectionCode;

inline void memmove8(void* dest, const void* src, size_t size)
{
    // this copies memory from src to dest but they may overlap so you need to ensure nothing is lost
    if (src > dest) {
        memcpy(dest, src, size);
    } else {
        uint8_t* d = (uint8_t*)dest + size - 1;
        const uint8_t* s = (uint8_t*)src + size - 1;
        for (size_t i = 0; i < size; i++) {
            *d = *s;
            d--;
            s--;
        }
    }
}

/// add one byte to another, saturating at 0xFF
/// @param i - first byte to add
/// @param j - second byte to add
/// @returns the sum of i & j, capped at 0xFF
/// 
inline uint8_t qadd8(uint8_t i, uint8_t j)
{
    uint16_t res = (uint16_t)i + (uint16_t)j;
    if (res > 255)
        res = 255;
    return (uint8_t)res;
}

/// subtract one byte from another, saturating at 0x00
/// @returns i - j with a floor of 0
inline uint8_t qsub8(uint8_t i, uint8_t j)
{
    int res = (int)i - (int)j;
    if (res < 0)
        res = 0;
    return (uint8_t)res;
}

/// saturating 8x8 bit multiplication, with 8 bit result
/// @returns the product of i * j, capping at 0xFF
inline uint8_t qmul8(uint8_t i, uint8_t j)
{
    uint16_t res = (uint16_t)i * (uint16_t)j;
    if (res > 255)
        res = 255;
    return (uint8_t)res;
}

struct CRGB {
    union {
        struct {
            union {
                uint8_t r;
                uint8_t red;
            };
            union {
                uint8_t g;
                uint8_t green;
            };
            union {
                uint8_t b;
                uint8_t blue;
            };
        };
        uint8_t raw[3];
    };
    /// Array access operator to index into the crgb object
    inline uint8_t& operator[](uint8_t x) //__attribute__((always_inline))
    {
        return raw[x];
    }

    /// Array access operator to index into the crgb object
    inline const uint8_t& operator[](uint8_t x) const //__attribute__((always_inline))
    {
        return raw[x];
    }

    // default values are UNINITIALIZED
    inline CRGB() //__attribute__((always_inline))
        = default;

    /// allow construction from R, G, B
    inline CRGB(uint8_t ir, uint8_t ig, uint8_t ib) //__attribute__((always_inline)) 
        : r(ir), g(ig), b(ib)
    {
    }

    /// allow construction from 32-bit (really 24-bit) bit 0xRRGGBB color code
    inline CRGB(uint32_t colorcode) //__attribute__((always_inline)) 
        : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    {
    }

#ifndef XLIGHTS_FX
    /// allow construction from a LEDColorCorrection enum
    inline CRGB(LEDColorCorrection colorcode) //__attribute__((always_inline)) 
        : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    {
    }

    /// allow construction from a ColorTemperature enum
    inline CRGB(ColorTemperature colorcode) //__attribute__((always_inline)) 
        : r((colorcode >> 16) & 0xFF), g((colorcode >> 8) & 0xFF), b((colorcode >> 0) & 0xFF)
    {
    }
#endif

    /// allow copy construction
    inline CRGB(const CRGB& rhs) //__attribute__((always_inline)) 
        = default;

    /// allow construction from HSV color
    inline CRGB(const CHSV& rhs) //__attribute__((always_inline))
    {
        hsv2rgb_rainbow(rhs, *this);
    }

    /// allow assignment from one RGB struct to another
    inline CRGB& operator=(const CRGB& rhs) //__attribute__((always_inline))
        = default;

    /// allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
    inline CRGB& operator=(const uint32_t colorcode) //__attribute__((always_inline))
    {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = (colorcode >> 0) & 0xFF;
        return *this;
    }

    /// allow assignment from R, G, and B
    inline CRGB& setRGB(uint8_t nr, uint8_t ng, uint8_t nb) //__attribute__((always_inline))
    {
        r = nr;
        g = ng;
        b = nb;
        return *this;
    }

#ifndef XLIGHTS_FX
    /// allow assignment from H, S, and V
    inline CRGB& setHSV(uint8_t hue, uint8_t sat, uint8_t val) //__attribute__((always_inline))
    {
        hsv2rgb_rainbow(CHSV(hue, sat, val), *this);
        return *this;
    }

    /// allow assignment from just a Hue, saturation and value automatically at max.
    inline CRGB& setHue(uint8_t hue) //__attribute__((always_inline))
    {
        hsv2rgb_rainbow(CHSV(hue, 255, 255), *this);
        return *this;
    }

    /// allow assignment from HSV color
    inline CRGB& operator=(const CHSV& rhs) //__attribute__((always_inline))
    {
        hsv2rgb_rainbow(rhs, *this);
        return *this;
    }
#endif

    /// allow assignment from 32-bit (really 24-bit) 0xRRGGBB color code
    inline CRGB& setColorCode(uint32_t colorcode) //__attribute__((always_inline))
    {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = (colorcode >> 0) & 0xFF;
        return *this;
    }

    /// add one RGB to another, saturating at 0xFF for each channel
    inline CRGB& operator+=(const CRGB& rhs)
    {
        r = qadd8(r, rhs.r);
        g = qadd8(g, rhs.g);
        b = qadd8(b, rhs.b);
        return *this;
    }

    /// add a contstant to each channel, saturating at 0xFF
    /// this is NOT an operator+= overload because the compiler
    /// can't usefully decide when it's being passed a 32-bit
    /// constant (e.g. CRGB::Red) and an 8-bit one (CRGB::Blue)
    inline CRGB& addToRGB(uint8_t d)
    {
        r = qadd8(r, d);
        g = qadd8(g, d);
        b = qadd8(b, d);
        return *this;
    }

    /// subtract one RGB from another, saturating at 0x00 for each channel
    inline CRGB& operator-=(const CRGB& rhs)
    {
        r = qsub8(r, rhs.r);
        g = qsub8(g, rhs.g);
        b = qsub8(b, rhs.b);
        return *this;
    }

    /// subtract a constant from each channel, saturating at 0x00
    /// this is NOT an operator+= overload because the compiler
    /// can't usefully decide when it's being passed a 32-bit
    /// constant (e.g. CRGB::Red) and an 8-bit one (CRGB::Blue)
    inline CRGB& subtractFromRGB(uint8_t d)
    {
        r = qsub8(r, d);
        g = qsub8(g, d);
        b = qsub8(b, d);
        return *this;
    }

    /// subtract a constant of '1' from each channel, saturating at 0x00
    inline CRGB& operator--() //__attribute__((always_inline))
    {
        subtractFromRGB(1);
        return *this;
    }

    /// subtract a constant of '1' from each channel, saturating at 0x00
    inline CRGB operator--(int) //__attribute__((always_inline))
    {
        CRGB retval(*this);
        --(*this);
        return retval;
    }

    /// add a constant of '1' from each channel, saturating at 0xFF
    inline CRGB& operator++() //__attribute__((always_inline))
    {
        addToRGB(1);
        return *this;
    }

    /// add a constant of '1' from each channel, saturating at 0xFF
    inline CRGB operator++(int) //__attribute__((always_inline))
    {
        CRGB retval(*this);
        ++(*this);
        return retval;
    }

    /// divide each of the channels by a constant
    inline CRGB& operator/=(uint8_t d)
    {
        r /= d;
        g /= d;
        b /= d;
        return *this;
    }

    /// right shift each of the channels by a constant
    inline CRGB& operator>>=(uint8_t d)
    {
        r >>= d;
        g >>= d;
        b >>= d;
        return *this;
    }

    /// multiply each of the channels by a constant,
    /// saturating each channel at 0xFF
    inline CRGB& operator*=(uint8_t d)
    {
        r = qmul8(r, d);
        g = qmul8(g, d);
        b = qmul8(b, d);
        return *this;
    }

    /// scale down a RGB to N 256ths of it's current brightness, using
    /// 'video' dimming rules, which means that unless the scale factor is ZERO
    /// each channel is guaranteed NOT to dim down to zero.  If it's already
    /// nonzero, it'll stay nonzero, even if that means the hue shifts a little
    /// at low brightness levels.
    inline CRGB& nscale8_video(uint8_t scaledown)
    {
        nscale8x3_video(r, g, b, scaledown);
        return *this;
    }

#ifndef XLIGHTS_FX
    /// %= is a synonym for nscale8_video.  Think of it is scaling down
    /// by "a percentage"
    inline CRGB& operator%=(uint8_t scaledown)
    {
        nscale8x3_video(r, g, b, scaledown);
        return *this;
    }

    /// fadeLightBy is a synonym for nscale8_video( ..., 255-fadefactor)
    inline CRGB& fadeLightBy(uint8_t fadefactor)
    {
        nscale8x3_video(r, g, b, 255 - fadefactor);
        return *this;
    }
#endif
    /// scale down a RGB to N 256ths of it's current brightness, using
    /// 'plain math' dimming rules, which means that if the low light levels
    /// may dim all the way to 100% black.
    inline CRGB& nscale8(uint8_t scaledown)
    {
        nscale8x3(r, g, b, scaledown);
        return *this;
    }

    /// scale down a RGB to N 256ths of it's current brightness, using
    /// 'plain math' dimming rules, which means that if the low light levels
    /// may dim all the way to 100% black.
    inline CRGB& nscale8(const CRGB& scaledown)
    {
        r = ::scale8(r, scaledown.r);
        g = ::scale8(g, scaledown.g);
        b = ::scale8(b, scaledown.b);
        return *this;
    }

    /// return a CRGB object that is a scaled down version of this object
    inline CRGB scale8(uint8_t scaledown) const
    {
        CRGB out = *this;
        nscale8x3(out.r, out.g, out.b, scaledown);
        return out;
    }

    /// return a CRGB object that is a scaled down version of this object
    inline CRGB scale8(const CRGB& scaledown) const
    {
        CRGB out;
        out.r = ::scale8(r, scaledown.r);
        out.g = ::scale8(g, scaledown.g);
        out.b = ::scale8(b, scaledown.b);
        return out;
    }

#ifndef XLIGHTS_FX
    /// fadeToBlackBy is a synonym for nscale8( ..., 255-fadefactor)
    inline CRGB& fadeToBlackBy(uint8_t fadefactor)
    {
        nscale8x3(r, g, b, 255 - fadefactor);
        return *this;
    }
#endif

    /// "or" operator brings each channel up to the higher of the two values
    inline CRGB& operator|=(const CRGB& rhs)
    {
        if (rhs.r > r)
            r = rhs.r;
        if (rhs.g > g)
            g = rhs.g;
        if (rhs.b > b)
            b = rhs.b;
        return *this;
    }

    /// "or" operator brings each channel up to the higher of the two values
    inline CRGB& operator|=(uint8_t d)
    {
        if (d > r)
            r = d;
        if (d > g)
            g = d;
        if (d > b)
            b = d;
        return *this;
    }

    /// "and" operator brings each channel down to the lower of the two values
    inline CRGB& operator&=(const CRGB& rhs)
    {
        if (rhs.r < r)
            r = rhs.r;
        if (rhs.g < g)
            g = rhs.g;
        if (rhs.b < b)
            b = rhs.b;
        return *this;
    }

    /// "and" operator brings each channel down to the lower of the two values
    inline CRGB& operator&=(uint8_t d)
    {
        if (d < r)
            r = d;
        if (d < g)
            g = d;
        if (d < b)
            b = d;
        return *this;
    }

    /// this allows testing a CRGB for zero-ness
    inline operator bool() const //__attribute__((always_inline))
    {
        return r || g || b;
    }

    /// invert each channel
    inline CRGB operator-() const
    {
        CRGB retval;
        retval.r = 255 - r;
        retval.g = 255 - g;
        retval.b = 255 - b;
        return retval;
    }

#if (defined SmartMatrix_h || defined SmartMatrix3_h)
    operator rgb24() const
    {
        rgb24 ret;
        ret.red = r;
        ret.green = g;
        ret.blue = b;
        return ret;
    }
#endif

#ifndef XLIGHTS_FX
    /// Get the 'luma' of a CRGB object - aka roughly how much light the
    /// CRGB pixel is putting out (from 0 to 255).
    inline uint8_t getLuma() const
    {
        //Y' = 0.2126 R' + 0.7152 G' + 0.0722 B'
        //     54            183       18 (!)

        uint8_t luma = scale8_LEAVING_R1_DIRTY(r, 54) +
                       scale8_LEAVING_R1_DIRTY(g, 183) +
                       scale8_LEAVING_R1_DIRTY(b, 18);
        cleanup_R1();
        return luma;
    }
#endif

    /// Get the average of the R, G, and B values
    inline uint8_t getAverageLight() const
    {
#if FASTLED_SCALE8_FIXED == 1
        const uint8_t eightyfive = 85;
#else
        const uint8_t eightyfive = 86;
#endif
        uint8_t avg = scale8_LEAVING_R1_DIRTY(r, eightyfive) +
                      scale8_LEAVING_R1_DIRTY(g, eightyfive) +
                      scale8_LEAVING_R1_DIRTY(b, eightyfive);
#ifndef XLIGHTS_FX
        cleanup_R1();
#endif
        return avg;
    }

#ifndef XLIGHTS_FX
    /// maximize the brightness of this CRGB object
    inline void maximizeBrightness(uint8_t limit = 255)
    {
        uint8_t max = red;
        if (green > max)
            max = green;
        if (blue > max)
            max = blue;

        // stop div/0 when color is black
        if (max > 0) {
            uint16_t factor = ((uint16_t)(limit)*256) / max;
            red = (red * factor) / 256;
            green = (green * factor) / 256;
            blue = (blue * factor) / 256;
        }
    }

    /// return a new CRGB object after performing a linear interpolation between this object and the passed in object
    inline CRGB lerp8(const CRGB& other, fract8 frac) const
    {
        CRGB ret;

        ret.r = lerp8by8(r, other.r, frac);
        ret.g = lerp8by8(g, other.g, frac);
        ret.b = lerp8by8(b, other.b, frac);

        return ret;
    }

    /// return a new CRGB object after performing a linear interpolation between this object and the passed in object
    inline CRGB lerp16(const CRGB& other, fract16 frac) const
    {
        CRGB ret;

        ret.r = lerp16by16(r << 8, other.r << 8, frac) >> 8;
        ret.g = lerp16by16(g << 8, other.g << 8, frac) >> 8;
        ret.b = lerp16by16(b << 8, other.b << 8, frac) >> 8;

        return ret;
    }

    /// getParity returns 0 or 1, depending on the
    /// lowest bit of the sum of the color components.
    inline uint8_t getParity()
    {
        uint8_t sum = r + g + b;
        return (sum & 0x01);
    }

    /// setParity adjusts the color in the smallest
    /// way possible so that the parity of the color
    /// is now the desired value.  This allows you to
    /// 'hide' one bit of information in the color.
    ///
    /// Ideally, we find one color channel which already
    /// has data in it, and modify just that channel by one.
    /// We don't want to light up a channel that's black
    /// if we can avoid it, and if the pixel is 'grayscale',
    /// (meaning that R==G==B), we modify all three channels
    /// at once, to preserve the neutral hue.
    ///
    /// There's no such thing as a free lunch; in many cases
    /// this 'hidden bit' may actually be visible, but this
    /// code makes reasonable efforts to hide it as much
    /// as is reasonably possible.
    ///
    /// Also, an effort is made to have make it such that
    /// repeatedly setting the parity to different values
    /// will not cause the color to 'drift'.  Toggling
    /// the parity twice should generally result in the
    /// original color again.
    ///
    inline void setParity(uint8_t parity)
    {
        uint8_t curparity = getParity();

        if (parity == curparity)
            return;

        if (parity) {
            // going 'up'
            if ((b > 0) && (b < 255)) {
                if (r == g && g == b) {
                    ++r;
                    ++g;
                }
                ++b;
            } else if ((r > 0) && (r < 255)) {
                ++r;
            } else if ((g > 0) && (g < 255)) {
                ++g;
            } else {
                if (r == g && g == b) {
                    r ^= 0x01;
                    g ^= 0x01;
                }
                b ^= 0x01;
            }
        } else {
            // going 'down'
            if (b > 1) {
                if (r == g && g == b) {
                    --r;
                    --g;
                }
                --b;
            } else if (g > 1) {
                --g;
            } else if (r > 1) {
                --r;
            } else {
                if (r == g && g == b) {
                    r ^= 0x01;
                    g ^= 0x01;
                }
                b ^= 0x01;
            }
        }
    }
#endif

    /// Predefined RGB colors
    typedef enum {
        AliceBlue = 0xF0F8FF,
        Amethyst = 0x9966CC,
        AntiqueWhite = 0xFAEBD7,
        Aqua = 0x00FFFF,
        Aquamarine = 0x7FFFD4,
        Azure = 0xF0FFFF,
        Beige = 0xF5F5DC,
        Bisque = 0xFFE4C4,
        Black = 0x000000,
        BlanchedAlmond = 0xFFEBCD,
        Blue = 0x0000FF,
        BlueViolet = 0x8A2BE2,
        Brown = 0xA52A2A,
        BurlyWood = 0xDEB887,
        CadetBlue = 0x5F9EA0,
        Chartreuse = 0x7FFF00,
        Chocolate = 0xD2691E,
        Coral = 0xFF7F50,
        CornflowerBlue = 0x6495ED,
        Cornsilk = 0xFFF8DC,
        Crimson = 0xDC143C,
        Cyan = 0x00FFFF,
        DarkBlue = 0x00008B,
        DarkCyan = 0x008B8B,
        DarkGoldenrod = 0xB8860B,
        DarkGray = 0xA9A9A9,
        DarkGrey = 0xA9A9A9,
        DarkGreen = 0x006400,
        DarkKhaki = 0xBDB76B,
        DarkMagenta = 0x8B008B,
        DarkOliveGreen = 0x556B2F,
        DarkOrange = 0xFF8C00,
        DarkOrchid = 0x9932CC,
        DarkRed = 0x8B0000,
        DarkSalmon = 0xE9967A,
        DarkSeaGreen = 0x8FBC8F,
        DarkSlateBlue = 0x483D8B,
        DarkSlateGray = 0x2F4F4F,
        DarkSlateGrey = 0x2F4F4F,
        DarkTurquoise = 0x00CED1,
        DarkViolet = 0x9400D3,
        DeepPink = 0xFF1493,
        DeepSkyBlue = 0x00BFFF,
        DimGray = 0x696969,
        DimGrey = 0x696969,
        DodgerBlue = 0x1E90FF,
        FireBrick = 0xB22222,
        FloralWhite = 0xFFFAF0,
        ForestGreen = 0x228B22,
        Fuchsia = 0xFF00FF,
        Gainsboro = 0xDCDCDC,
        GhostWhite = 0xF8F8FF,
        Gold = 0xFFD700,
        Goldenrod = 0xDAA520,
        Gray = 0x808080,
        Grey = 0x808080,
        Green = 0x008000,
        GreenYellow = 0xADFF2F,
        Honeydew = 0xF0FFF0,
        HotPink = 0xFF69B4,
        IndianRed = 0xCD5C5C,
        Indigo = 0x4B0082,
        Ivory = 0xFFFFF0,
        Khaki = 0xF0E68C,
        Lavender = 0xE6E6FA,
        LavenderBlush = 0xFFF0F5,
        LawnGreen = 0x7CFC00,
        LemonChiffon = 0xFFFACD,
        LightBlue = 0xADD8E6,
        LightCoral = 0xF08080,
        LightCyan = 0xE0FFFF,
        LightGoldenrodYellow = 0xFAFAD2,
        LightGreen = 0x90EE90,
        LightGrey = 0xD3D3D3,
        LightPink = 0xFFB6C1,
        LightSalmon = 0xFFA07A,
        LightSeaGreen = 0x20B2AA,
        LightSkyBlue = 0x87CEFA,
        LightSlateGray = 0x778899,
        LightSlateGrey = 0x778899,
        LightSteelBlue = 0xB0C4DE,
        LightYellow = 0xFFFFE0,
        Lime = 0x00FF00,
        LimeGreen = 0x32CD32,
        Linen = 0xFAF0E6,
        Magenta = 0xFF00FF,
        Maroon = 0x800000,
        MediumAquamarine = 0x66CDAA,
        MediumBlue = 0x0000CD,
        MediumOrchid = 0xBA55D3,
        MediumPurple = 0x9370DB,
        MediumSeaGreen = 0x3CB371,
        MediumSlateBlue = 0x7B68EE,
        MediumSpringGreen = 0x00FA9A,
        MediumTurquoise = 0x48D1CC,
        MediumVioletRed = 0xC71585,
        MidnightBlue = 0x191970,
        MintCream = 0xF5FFFA,
        MistyRose = 0xFFE4E1,
        Moccasin = 0xFFE4B5,
        NavajoWhite = 0xFFDEAD,
        Navy = 0x000080,
        OldLace = 0xFDF5E6,
        Olive = 0x808000,
        OliveDrab = 0x6B8E23,
        Orange = 0xFFA500,
        OrangeRed = 0xFF4500,
        Orchid = 0xDA70D6,
        PaleGoldenrod = 0xEEE8AA,
        PaleGreen = 0x98FB98,
        PaleTurquoise = 0xAFEEEE,
        PaleVioletRed = 0xDB7093,
        PapayaWhip = 0xFFEFD5,
        PeachPuff = 0xFFDAB9,
        Peru = 0xCD853F,
        Pink = 0xFFC0CB,
        Plaid = 0xCC5533,
        Plum = 0xDDA0DD,
        PowderBlue = 0xB0E0E6,
        Purple = 0x800080,
        Red = 0xFF0000,
        RosyBrown = 0xBC8F8F,
        RoyalBlue = 0x4169E1,
        SaddleBrown = 0x8B4513,
        Salmon = 0xFA8072,
        SandyBrown = 0xF4A460,
        SeaGreen = 0x2E8B57,
        Seashell = 0xFFF5EE,
        Sienna = 0xA0522D,
        Silver = 0xC0C0C0,
        SkyBlue = 0x87CEEB,
        SlateBlue = 0x6A5ACD,
        SlateGray = 0x708090,
        SlateGrey = 0x708090,
        Snow = 0xFFFAFA,
        SpringGreen = 0x00FF7F,
        SteelBlue = 0x4682B4,
        Tan = 0xD2B48C,
        Teal = 0x008080,
        Thistle = 0xD8BFD8,
        Tomato = 0xFF6347,
        Turquoise = 0x40E0D0,
        Violet = 0xEE82EE,
        Wheat = 0xF5DEB3,
        White = 0xFFFFFF,
        WhiteSmoke = 0xF5F5F5,
        Yellow = 0xFFFF00,
        YellowGreen = 0x9ACD32,

        // LED RGB color that roughly approximates
        // the color of incandescent fairy lights,
        // assuming that you're using FastLED
        // color correction on your LEDs (recommended).
        FairyLight = 0xFFE42D,
        // If you are using no color correction, use this
        FairyLightNCC = 0xFF9D2A

    } HTMLColorCode;
};

class CRGBPalette16
{
public:
    CRGB entries[16];
    CRGBPalette16(){};
    CRGBPalette16(const CRGB& c00, const CRGB& c01, const CRGB& c02, const CRGB& c03,
                  const CRGB& c04, const CRGB& c05, const CRGB& c06, const CRGB& c07,
                  const CRGB& c08, const CRGB& c09, const CRGB& c10, const CRGB& c11,
                  const CRGB& c12, const CRGB& c13, const CRGB& c14, const CRGB& c15)
    {
        entries[0] = c00;
        entries[1] = c01;
        entries[2] = c02;
        entries[3] = c03;
        entries[4] = c04;
        entries[5] = c05;
        entries[6] = c06;
        entries[7] = c07;
        entries[8] = c08;
        entries[9] = c09;
        entries[10] = c10;
        entries[11] = c11;
        entries[12] = c12;
        entries[13] = c13;
        entries[14] = c14;
        entries[15] = c15;
    };

    CRGBPalette16(const CRGBPalette16& rhs)
    {
        memmove8((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
    }
    CRGBPalette16(const CRGB rhs[16])
    {
        memmove8((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
    }
    CRGBPalette16& operator=(const CRGBPalette16& rhs)
    {
        memmove8((void*)&(entries[0]), &(rhs.entries[0]), sizeof(entries));
        return *this;
    }
    CRGBPalette16& operator=(const CRGB rhs[16])
    {
        memmove8((void*)&(entries[0]), &(rhs[0]), sizeof(entries));
        return *this;
    }

#ifndef XLIGHTS_FX
    CRGBPalette16(const CHSVPalette16& rhs)
    {
        for (uint8_t i = 0; i < 16; ++i) {
            entries[i] = rhs.entries[i]; // implicit HSV-to-RGB conversion
        }
    }
#endif

    CRGBPalette16(const CHSV rhs[16])
    {
        for (uint8_t i = 0; i < 16; ++i) {
#ifndef XLIGHTS_FX
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
#else
            entries[i].r = rhs[i].h; // implicit HSV-to-RGB conversion
            entries[i].g = rhs[i].s; // implicit HSV-to-RGB conversion
            entries[i].b = rhs[i].v; // implicit HSV-to-RGB conversion
#endif
        }
    }
#ifndef XLIGHTS_FX
    CRGBPalette16& operator=(const CHSVPalette16& rhs)
    {
        for (uint8_t i = 0; i < 16; ++i) {
#ifndef XLIGHTS_FX
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
#else
            entries[i].r = rhs[i].h; // implicit HSV-to-RGB conversion
            entries[i].g = rhs[i].s; // implicit HSV-to-RGB conversion
            entries[i].b = rhs[i].v; // implicit HSV-to-RGB conversion
#endif
        }
        return *this;
    }
#endif

    CRGBPalette16& operator=(const CHSV rhs[16])
    {
        for (uint8_t i = 0; i < 16; ++i) {
#ifndef XLIGHTS_FX
            entries[i] = rhs[i]; // implicit HSV-to-RGB conversion
#else
            entries[i].r = rhs[i].h; // implicit HSV-to-RGB conversion
            entries[i].g = rhs[i].s; // implicit HSV-to-RGB conversion
            entries[i].b = rhs[i].v; // implicit HSV-to-RGB conversion
#endif
        }
        return *this;
    }

#ifndef XLIGHTS_FX
    CRGBPalette16(const TProgmemRGBPalette16& rhs)
    {
        for (uint8_t i = 0; i < 16; ++i) {
            entries[i] = FL_PGM_READ_DWORD_NEAR(rhs + i);
        }
    }
    CRGBPalette16& operator=(const TProgmemRGBPalette16& rhs)
    {
        for (uint8_t i = 0; i < 16; ++i) {
            entries[i] = FL_PGM_READ_DWORD_NEAR(rhs + i);
        }
        return *this;
    }
#endif

    void fill_solid(struct CRGB* leds, int numToFill,
                    const struct CRGB& color)
    {
        for (int i = 0; i < numToFill; ++i) {
            leds[i] = color;
        }
    }

    void fill_solid(struct CHSV* targetArray, int numToFill,
                    const struct CHSV& hsvColor)
    {
        for (int i = 0; i < numToFill; ++i) {
            targetArray[i] = hsvColor;
        }
    }

    bool operator==(const CRGBPalette16 rhs)
    {
        const uint8_t* p = (const uint8_t*)(&(this->entries[0]));
        const uint8_t* q = (const uint8_t*)(&(rhs.entries[0]));
        if (p == q)
            return true;
        for (uint8_t i = 0; i < (sizeof(entries)); ++i) {
            if (*p != *q)
                return false;
            ++p;
            ++q;
        }
        return true;
    }
    bool operator!=(const CRGBPalette16 rhs)
    {
        return !(*this == rhs);
    }

    inline CRGB& operator[](uint8_t x) //__attribute__((always_inline))
    {
        return entries[x];
    }
    inline const CRGB& operator[](uint8_t x) const //__attribute__((always_inline))
    {
        return entries[x];
    }

    inline CRGB& operator[](int x) //__attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }
    inline const CRGB& operator[](int x) const //__attribute__((always_inline))
    {
        return entries[(uint8_t)x];
    }

    operator CRGB*()
    {
        return &(entries[0]);
    }

    template<typename T>
    void fill_gradient(T* targetArray,
                       uint16_t startpos, CHSV startcolor,
                       uint16_t endpos, CHSV endcolor,
                       TGradientDirectionCode directionCode = SHORTEST_HUES)
    {
        // if the points are in the wrong order, straighten them
        if (endpos < startpos) {
            uint16_t t = endpos;
            CHSV tc = endcolor;
            endcolor = startcolor;
            endpos = startpos;
            startpos = t;
            startcolor = tc;
        }

        // If we're fading toward black (val=0) or white (sat=0),
        // then set the endhue to the starthue.
        // This lets us ramp smoothly to black or white, regardless
        // of what 'hue' was set in the endcolor (since it doesn't matter)
        if (endcolor.value == 0 || endcolor.saturation == 0) {
            endcolor.hue = startcolor.hue;
        }

        // Similarly, if we're fading in from black (val=0) or white (sat=0)
        // then set the starthue to the endhue.
        // This lets us ramp smoothly up from black or white, regardless
        // of what 'hue' was set in the startcolor (since it doesn't matter)
        if (startcolor.value == 0 || startcolor.saturation == 0) {
            startcolor.hue = endcolor.hue;
        }

        saccum87 huedistance87;
        saccum87 satdistance87;
        saccum87 valdistance87;

        satdistance87 = (endcolor.sat - startcolor.sat) << 7;
        valdistance87 = (endcolor.val - startcolor.val) << 7;

        uint8_t huedelta8 = endcolor.hue - startcolor.hue;

        if (directionCode == SHORTEST_HUES) {
            directionCode = FORWARD_HUES;
            if (huedelta8 > 127) {
                directionCode = BACKWARD_HUES;
            }
        }

        if (directionCode == LONGEST_HUES) {
            directionCode = FORWARD_HUES;
            if (huedelta8 < 128) {
                directionCode = BACKWARD_HUES;
            }
        }

        if (directionCode == FORWARD_HUES) {
            huedistance87 = huedelta8 << 7;
        } else /* directionCode == BACKWARD_HUES */
        {
            huedistance87 = (uint8_t)(256 - huedelta8) << 7;
            huedistance87 = -huedistance87;
        }

        uint16_t pixeldistance = endpos - startpos;
        int16_t divisor = pixeldistance ? pixeldistance : 1;

        saccum87 huedelta87 = huedistance87 / divisor;
        saccum87 satdelta87 = satdistance87 / divisor;
        saccum87 valdelta87 = valdistance87 / divisor;

        huedelta87 *= 2;
        satdelta87 *= 2;
        valdelta87 *= 2;

        accum88 hue88 = startcolor.hue << 8;
        accum88 sat88 = startcolor.sat << 8;
        accum88 val88 = startcolor.val << 8;
        for (uint16_t i = startpos; i <= endpos; ++i) {
            targetArray[i] = CHSV(hue88 >> 8, sat88 >> 8, val88 >> 8);
            hue88 += huedelta87;
            sat88 += satdelta87;
            val88 += valdelta87;
        }
    }

    template<typename T>
    void fill_gradient(T* targetArray, uint16_t numLeds,
                       const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4,
                       TGradientDirectionCode directionCode = SHORTEST_HUES)
    {
        uint16_t onethird = (numLeds / 3);
        uint16_t twothirds = ((numLeds * 2) / 3);
        uint16_t last = numLeds - 1;
        fill_gradient(targetArray, 0, c1, onethird, c2, directionCode);
        fill_gradient(targetArray, onethird, c2, twothirds, c3, directionCode);
        fill_gradient(targetArray, twothirds, c3, last, c4, directionCode);
    }

    template<typename T>
    void fill_gradient(T* targetArray, uint16_t numLeds, const CHSV& c1, const CHSV& c2,
                       TGradientDirectionCode directionCode = SHORTEST_HUES)
    {
        uint16_t last = numLeds - 1;
        fill_gradient(targetArray, 0, c1, last, c2, directionCode);
    }

    template<typename T>
    void fill_gradient(T* targetArray, uint16_t numLeds,
                       const CHSV& c1, const CHSV& c2, const CHSV& c3,
                       TGradientDirectionCode directionCode = SHORTEST_HUES)
    {
        uint16_t half = (numLeds / 2);
        uint16_t last = numLeds - 1;
        fill_gradient(targetArray, 0, c1, half, c2, directionCode);
        fill_gradient(targetArray, half, c2, last, c3, directionCode);
    }

    CRGBPalette16(const CHSV& c1)
    {
        fill_solid(&(entries[0]), 16, c1);
    }
    CRGBPalette16(const CHSV& c1, const CHSV& c2)
    {
        fill_gradient(&(entries[0]), 16, c1, c2);
    }
    CRGBPalette16(const CHSV& c1, const CHSV& c2, const CHSV& c3)
    {
        fill_gradient(&(entries[0]), 16, c1, c2, c3);
    }
    CRGBPalette16(const CHSV& c1, const CHSV& c2, const CHSV& c3, const CHSV& c4)
    {
        fill_gradient(&(entries[0]), 16, c1, c2, c3, c4);
    }
    CRGBPalette16(const CRGB& c1)
    {
        fill_solid(&(entries[0]), 16, c1);
    }
    CRGBPalette16(const CRGB& c1, const CRGB& c2)
    {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2);
    }
    CRGBPalette16(const CRGB& c1, const CRGB& c2, const CRGB& c3)
    {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3);
    }
    CRGBPalette16(const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4)
    {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3, c4);
    }
    CRGBPalette16(const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5)
    {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3, c4, c5);
    }
    CRGBPalette16(const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5, const CRGB& c6)
    {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3, c4, c5, c6);
    }
    CRGBPalette16(const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5, const CRGB& c6, const CRGB& c7)
    {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3, c4, c5, c6, c7);
    }
    CRGBPalette16(const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5, const CRGB& c6, const CRGB& c7, const CRGB& c8)
    {
        fill_gradient_RGB(&(entries[0]), 16, c1, c2, c3, c4, c5, c6, c7, c8);
    }

#ifndef XLIGHTS_FX
    // Gradient palettes are loaded into CRGB16Palettes in such a way
    // that, if possible, every color represented in the gradient palette
    // is also represented in the CRGBPalette16.
    // For example, consider a gradient palette that is all black except
    // for a single, one-element-wide (1/256th!) spike of red in the middle:
    //     0,   0,0,0
    //   124,   0,0,0
    //   125, 255,0,0  // one 1/256th-palette-wide red stripe
    //   126,   0,0,0
    //   255,   0,0,0
    // A naive conversion of this 256-element palette to a 16-element palette
    // might accidentally completely eliminate the red spike, rendering the
    // palette completely black.
    // However, the conversions provided here would attempt to include a
    // the red stripe in the output, more-or-less as faithfully as possible.
    // So in this case, the resulting CRGBPalette16 palette would have a red
    // stripe in the middle which was 1/16th of a palette wide -- the
    // narrowest possible in a CRGBPalette16.
    // This means that the relative width of stripes in a CRGBPalette16
    // will be, by definition, different from the widths in the gradient
    // palette.  This code attempts to preserve "all the colors", rather than
    // the exact stripe widths at the expense of dropping some colors.
    CRGBPalette16(TProgmemRGBGradientPalette_bytes progpal)
    {
        *this = progpal;
    }
    CRGBPalette16& operator=(TProgmemRGBGradientPalette_bytes progpal)
    {
        TRGBGradientPaletteEntryUnion* progent = (TRGBGradientPaletteEntryUnion*)(progpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u.dword = FL_PGM_READ_DWORD_NEAR(progent + count);
            ++count;
        } while (u.index != 255);

        int8_t lastSlotUsed = -1;

        u.dword = FL_PGM_READ_DWORD_NEAR(progent);
        CRGB rgbstart(u.r, u.g, u.b);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while (indexstart < 255) {
            ++progent;
            u.dword = FL_PGM_READ_DWORD_NEAR(progent);
            int indexend = u.index;
            CRGB rgbend(u.r, u.g, u.b);
            istart8 = indexstart / 16;
            iend8 = indexend / 16;
            if (count < 16) {
                if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
                    istart8 = lastSlotUsed + 1;
                    if (iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB(&(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
#endif

void fill_gradient_RGB(CRGB* leds,
                           uint16_t startpos, CRGB startcolor,
                           uint16_t endpos, CRGB endcolor)
    {
        // if the points are in the wrong order, straighten them
        if (endpos < startpos) {
            uint16_t t = endpos;
            CRGB tc = endcolor;
            endcolor = startcolor;
            endpos = startpos;
            startpos = t;
            startcolor = tc;
        }

        saccum87 rdistance87;
        saccum87 gdistance87;
        saccum87 bdistance87;

        rdistance87 = (endcolor.r - startcolor.r) << 7;
        gdistance87 = (endcolor.g - startcolor.g) << 7;
        bdistance87 = (endcolor.b - startcolor.b) << 7;

        uint16_t pixeldistance = endpos - startpos;
        int16_t divisor = pixeldistance ? pixeldistance : 1;

        saccum87 rdelta87 = rdistance87 / divisor;
        saccum87 gdelta87 = gdistance87 / divisor;
        saccum87 bdelta87 = bdistance87 / divisor;

        rdelta87 *= 2;
        gdelta87 *= 2;
        bdelta87 *= 2;

        accum88 r88 = startcolor.r << 8;
        accum88 g88 = startcolor.g << 8;
        accum88 b88 = startcolor.b << 8;
        for (uint16_t i = startpos; i <= endpos; ++i) {
            leds[i] = CRGB(r88 >> 8, g88 >> 8, b88 >> 8);
            r88 += rdelta87;
            g88 += gdelta87;
            b88 += bdelta87;
        }
    }

    typedef union {
        struct {
            uint8_t index;
            uint8_t r;
            uint8_t g;
            uint8_t b;
        };
        uint32_t dword;
        uint8_t bytes[4];
    } TRGBGradientPaletteEntryUnion;

    CRGBPalette16& loadDynamicGradientPalette(const byte* gpal)
    {
        TRGBGradientPaletteEntryUnion* ent = (TRGBGradientPaletteEntryUnion*)(gpal);
        TRGBGradientPaletteEntryUnion u;

        // Count entries
        uint16_t count = 0;
        do {
            u = *(ent + count);
            ++count;
        } while (u.index != 255);

        int8_t lastSlotUsed = -1;

        u = *ent;
        CRGB rgbstart(u.r, u.g, u.b);

        int indexstart = 0;
        uint8_t istart8 = 0;
        uint8_t iend8 = 0;
        while (indexstart < 255) {
            ++ent;
            u = *ent;
            int indexend = u.index;
            CRGB rgbend(u.r, u.g, u.b);
            istart8 = indexstart / 16;
            iend8 = indexend / 16;
            if (count < 16) {
                if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
                    istart8 = lastSlotUsed + 1;
                    if (iend8 < istart8) {
                        iend8 = istart8;
                    }
                }
                lastSlotUsed = iend8;
            }
            fill_gradient_RGB(&(entries[0]), istart8, rgbstart, iend8, rgbend);
            indexstart = indexend;
            rgbstart = rgbend;
        }
        return *this;
    }
    void fill_gradient_RGB(CRGB* leds, uint16_t numLeds, const CRGB& c1, const CRGB& c2)
    {
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, last, c2);
    }

    void fill_gradient_RGB(CRGB* leds, uint16_t numLeds, const CRGB& c1, const CRGB& c2, const CRGB& c3)
    {
        uint16_t half = (numLeds / 2);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, half, c2);
        fill_gradient_RGB(leds, half, c2, last, c3);
    }

    void fill_gradient_RGB(CRGB* leds, uint16_t numLeds, const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4)
    {
        uint16_t onethird = (numLeds / 3);
        uint16_t twothirds = ((numLeds * 2) / 3);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, onethird, c2);
        fill_gradient_RGB(leds, onethird, c2, twothirds, c3);
        fill_gradient_RGB(leds, twothirds, c3, last, c4);
    }

    void fill_gradient_RGB(CRGB* leds, uint16_t numLeds, const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5)
    {
        uint16_t onequarter = (numLeds / 4);
        uint16_t onehalf = (numLeds / 2);
        uint16_t threequarters = ((numLeds * 3) / 4);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, onequarter, c2);
        fill_gradient_RGB(leds, onequarter, c2, onehalf, c3);
        fill_gradient_RGB(leds, onehalf, c3, threequarters, c4);
        fill_gradient_RGB(leds, threequarters, c4, last, c5);
    }

    void fill_gradient_RGB(CRGB* leds, uint16_t numLeds, const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5, const CRGB& c6)
    {
        uint16_t onefifth = (numLeds / 5);
        uint16_t twofifths = ((numLeds * 2) / 5);
        uint16_t threefifths = ((numLeds * 3) / 5);
        uint16_t fourfifths = ((numLeds * 5) / 5);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, onefifth, c2);
        fill_gradient_RGB(leds, onefifth, c2, twofifths, c3);
        fill_gradient_RGB(leds, twofifths, c3, threefifths, c4);
        fill_gradient_RGB(leds, threefifths, c4, fourfifths, c5);
        fill_gradient_RGB(leds, fourfifths, c5, last, c6);
    }

    void fill_gradient_RGB(CRGB* leds, uint16_t numLeds, const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5, const CRGB& c6, const CRGB& c7)
    {
        uint16_t onesixth = (numLeds / 6);
        uint16_t twosixths = ((numLeds * 2) / 6);
        uint16_t threesixths = ((numLeds * 3) / 6);
        uint16_t foursixths = ((numLeds * 4) / 6);
        uint16_t fivesixths = ((numLeds * 5) / 6);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, onesixth, c2);
        fill_gradient_RGB(leds, onesixth, c2, twosixths, c3);
        fill_gradient_RGB(leds, twosixths, c3, threesixths, c4);
        fill_gradient_RGB(leds, threesixths, c4, foursixths, c5);
        fill_gradient_RGB(leds, foursixths, c5, fivesixths, c6);
        fill_gradient_RGB(leds, fivesixths, c6, last, c7);
    }

    void fill_gradient_RGB(CRGB* leds, uint16_t numLeds, const CRGB& c1, const CRGB& c2, const CRGB& c3, const CRGB& c4, const CRGB& c5, const CRGB& c6, const CRGB& c7, const CRGB& c8)
    {
        uint16_t oneseventh = (numLeds / 7);
        uint16_t twosevenths = ((numLeds * 2) / 7);
        uint16_t threesevenths = ((numLeds * 3) / 7);
        uint16_t foursevenths = ((numLeds * 4) / 7);
        uint16_t fivesevenths = ((numLeds * 5) / 7);
        uint16_t sixsevenths = ((numLeds * 6) / 7);
        uint16_t last = numLeds - 1;
        fill_gradient_RGB(leds, 0, c1, oneseventh, c2);
        fill_gradient_RGB(leds, oneseventh, c2, twosevenths, c3);
        fill_gradient_RGB(leds, twosevenths, c3, threesevenths, c4);
        fill_gradient_RGB(leds, threesevenths, c4, foursevenths, c5);
        fill_gradient_RGB(leds, foursevenths, c5, fivesevenths, c6);
        fill_gradient_RGB(leds, fivesevenths, c6, sixsevenths, c7);
        fill_gradient_RGB(leds, sixsevenths, c7, last, c8);
    }
};

//#define UINT16_MAX ((uint16_t)0xFFFF)
#endif

#define DEFAULT_BRIGHTNESS (uint8_t)127
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint8_t)128
#define DEFAULT_INTENSITY  (uint8_t)128
#define DEFAULT_COLOR      (uint32_t)0
// Default colour was 0xFFAA00

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

/* Not used in all effects yet */
#define WLED_FPS         42
#define FRAMETIME_FIXED  (1000/WLED_FPS)
#define FRAMETIME        _frametime

/* each segment uses 52 bytes of SRAM memory, so if you're application fails because of
  insufficient memory, decreasing MAX_NUM_SEGMENTS may help */
#ifdef ESP8266
  #define MAX_NUM_SEGMENTS    16
  /* How many color transitions can run at once */
  #define MAX_NUM_TRANSITIONS  8
  /* How much data bytes all segments combined may allocate */
  #define MAX_SEGMENT_DATA  4096
#elif defined(XLIGHTS_FX)
    #define MAX_NUM_SEGMENTS 1
    #define MAX_NUM_TRANSITIONS 24
    #define MAX_SEGMENT_DATA 20480
#else
  #ifndef MAX_NUM_SEGMENTS
    #define MAX_NUM_SEGMENTS  32
  #endif
  #define MAX_NUM_TRANSITIONS 24
  #define MAX_SEGMENT_DATA  20480
#endif

/* How much data bytes each segment should max allocate to leave enough space for other segments,
  assuming each segment uses the same amount of data. 256 for ESP8266, 640 for ESP32. */
#define FAIR_DATA_PER_SEG (MAX_SEGMENT_DATA / MAX_NUM_SEGMENTS)

#define LED_SKIP_AMOUNT  1
#define MIN_SHOW_DELAY   (_frametime < 16 ? 8 : 15)

#define NUM_COLORS       8 /* number of colors per segment */
#define SEGMENT          _segments[_segment_index]
#define SEGCOLOR(x)      _colors_t[x]
#define SEGENV           _segment_runtimes[_segment_index]
#define SEGLEN           _virtualSegmentLength
#define SEGACT           SEGMENT.stop
#define SPEED_FORMULA_L  5U + (50U*(255U - SEGMENT.speed))/SEGLEN
#define RESET_RUNTIME    memset(_segment_runtimes, 0, sizeof(_segment_runtimes))

// some common colors
#define RED        (uint32_t)0xFF0000
#define GREEN      (uint32_t)0x00FF00
#define BLUE       (uint32_t)0x0000FF
#define WHITE      (uint32_t)0xFFFFFF
#define BLACK      (uint32_t)0x000000
#define YELLOW     (uint32_t)0xFFFF00
#define CYAN       (uint32_t)0x00FFFF
#define MAGENTA    (uint32_t)0xFF00FF
#define PURPLE     (uint32_t)0x400080
#define ORANGE     (uint32_t)0xFF3000
#define PINK       (uint32_t)0xFF1493
#define ULTRAWHITE (uint32_t)0xFFFFFFFF

// options
// bit    7: segment is in transition mode
// bits 4-6: TBD
// bit    3: mirror effect within segment
// bit    2: segment is on
// bit    1: reverse segment
// bit    0: segment is selected
#define NO_OPTIONS   (uint8_t)0x00
#define TRANSITIONAL (uint8_t)0x80
#define MIRROR       (uint8_t)0x08
#define SEGMENT_ON   (uint8_t)0x04
#define REVERSE      (uint8_t)0x02
#define SELECTED     (uint8_t)0x01
#define IS_TRANSITIONAL ((SEGMENT.options & TRANSITIONAL) == TRANSITIONAL)
#define IS_MIRROR       ((SEGMENT.options & MIRROR      ) == MIRROR      )
#define IS_SEGMENT_ON   ((SEGMENT.options & SEGMENT_ON  ) == SEGMENT_ON  )
#define IS_REVERSE      ((SEGMENT.options & REVERSE     ) == REVERSE     )
#define IS_SELECTED     ((SEGMENT.options & SELECTED    ) == SELECTED    )

#define MODE_COUNT  118

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_RANDOM        4
#define FX_MODE_RANDOM_COLOR             5
#define FX_MODE_COLOR_SWEEP              6
#define FX_MODE_DYNAMIC                  7
#define FX_MODE_RAINBOW                  8
#define FX_MODE_RAINBOW_CYCLE            9
#define FX_MODE_SCAN                    10
#define FX_MODE_DUAL_SCAN               11
#define FX_MODE_FADE                    12
#define FX_MODE_THEATER_CHASE           13
#define FX_MODE_THEATER_CHASE_RAINBOW   14
#define FX_MODE_RUNNING_LIGHTS          15
#define FX_MODE_SAW                     16
#define FX_MODE_TWINKLE                 17
#define FX_MODE_DISSOLVE                18
#define FX_MODE_DISSOLVE_RANDOM         19
#define FX_MODE_SPARKLE                 20
#define FX_MODE_FLASH_SPARKLE           21
#define FX_MODE_HYPER_SPARKLE           22
#define FX_MODE_STROBE                  23
#define FX_MODE_STROBE_RAINBOW          24
#define FX_MODE_MULTI_STROBE            25
#define FX_MODE_BLINK_RAINBOW           26
#define FX_MODE_ANDROID                 27
#define FX_MODE_CHASE_COLOR             28
#define FX_MODE_CHASE_RANDOM            29
#define FX_MODE_CHASE_RAINBOW           30
#define FX_MODE_CHASE_FLASH             31
#define FX_MODE_CHASE_FLASH_RANDOM      32
#define FX_MODE_CHASE_RAINBOW_WHITE     33
#define FX_MODE_COLORFUL                34
#define FX_MODE_TRAFFIC_LIGHT           35
#define FX_MODE_COLOR_SWEEP_RANDOM      36
#define FX_MODE_RUNNING_COLOR           37
#define FX_MODE_AURORA                  38
#define FX_MODE_RUNNING_RANDOM          39
#define FX_MODE_LARSON_SCANNER          40
#define FX_MODE_COMET                   41
#define FX_MODE_FIREWORKS               42
#define FX_MODE_RAIN                    43
#define FX_MODE_TETRIX                  44  //was Merry Christmas prior to 0.12.0 (use "Chase 2" with Red/Green)
#define FX_MODE_FIRE_FLICKER            45
#define FX_MODE_GRADIENT                46
#define FX_MODE_LOADING                 47
#define FX_MODE_POLICE                  48  // candidate for removal (after below three)
#define FX_MODE_FAIRY                   49  //was Police All prior to 0.13.0-b6 (use "Two Dots" with Red/Blue and full intensity)
#define FX_MODE_TWO_DOTS                50
#define FX_MODE_FAIRYTWINKLE            51  //was Two Areas prior to 0.13.0-b6 (use "Two Dots" with full intensity)
#define FX_MODE_RUNNING_DUAL            52
#define FX_MODE_HALLOWEEN               53  // candidate for removal
#define FX_MODE_TRICOLOR_CHASE          54
#define FX_MODE_TRICOLOR_WIPE           55
#define FX_MODE_TRICOLOR_FADE           56
#define FX_MODE_LIGHTNING               57
#define FX_MODE_ICU                     58
#define FX_MODE_MULTI_COMET             59
#define FX_MODE_DUAL_LARSON_SCANNER     60
#define FX_MODE_RANDOM_CHASE            61
#define FX_MODE_OSCILLATE               62
#define FX_MODE_PRIDE_2015              63
#define FX_MODE_JUGGLE                  64
#define FX_MODE_PALETTE                 65
#define FX_MODE_FIRE_2012               66
#define FX_MODE_COLORWAVES              67
#define FX_MODE_BPM                     68
#define FX_MODE_FILLNOISE8              69
#define FX_MODE_NOISE16_1               70
#define FX_MODE_NOISE16_2               71
#define FX_MODE_NOISE16_3               72
#define FX_MODE_NOISE16_4               73
#define FX_MODE_COLORTWINKLE            74
#define FX_MODE_LAKE                    75
#define FX_MODE_METEOR                  76
#define FX_MODE_METEOR_SMOOTH           77
#define FX_MODE_RAILWAY                 78
#define FX_MODE_RIPPLE                  79
#define FX_MODE_TWINKLEFOX              80
#define FX_MODE_TWINKLECAT              81
#define FX_MODE_HALLOWEEN_EYES          82
#define FX_MODE_STATIC_PATTERN          83
#define FX_MODE_TRI_STATIC_PATTERN      84
#define FX_MODE_SPOTS                   85
#define FX_MODE_SPOTS_FADE              86
#define FX_MODE_GLITTER                 87
#define FX_MODE_CANDLE                  88
#define FX_MODE_STARBURST               89
#define FX_MODE_EXPLODING_FIREWORKS     90
#define FX_MODE_BOUNCINGBALLS           91
#define FX_MODE_SINELON                 92
#define FX_MODE_SINELON_DUAL            93
#define FX_MODE_SINELON_RAINBOW         94
#define FX_MODE_POPCORN                 95
#define FX_MODE_DRIP                    96
#define FX_MODE_PLASMA                  97
#define FX_MODE_PERCENT                 98
#define FX_MODE_RIPPLE_RAINBOW          99
#define FX_MODE_HEARTBEAT              100
#define FX_MODE_PACIFICA               101
#define FX_MODE_CANDLE_MULTI           102
#define FX_MODE_SOLID_GLITTER          103
#define FX_MODE_SUNRISE                104
#define FX_MODE_PHASED                 105
#define FX_MODE_TWINKLEUP              106
#define FX_MODE_NOISEPAL               107
#define FX_MODE_SINEWAVE               108
#define FX_MODE_PHASEDNOISE            109
#define FX_MODE_FLOW                   110
#define FX_MODE_CHUNCHUN               111
#define FX_MODE_DANCING_SHADOWS        112
#define FX_MODE_WASHING_MACHINE        113
#define FX_MODE_CANDY_CANE             114  // candidate for removal
#define FX_MODE_BLENDS                 115
#define FX_MODE_TV_SIMULATOR           116
#define FX_MODE_DYNAMIC_SMOOTH         117


class WS2812FX {

#ifdef XLIGHTS_FX
    RenderBuffer* _buffer = nullptr;
    static uint8_t random8(uint8_t limit = 0xFF);
    static uint8_t random8(uint8_t low, uint8_t high);
    void copyPixels(uint16_t dest, uint16_t src, uint16_t count);

    typedef uint16_t accum88;
    uint16_t beat88(accum88 beats_per_minute_88, uint32_t timebase = 0)
    {
        // BPM is 'beats per minute', or 'beats per 60000ms'.
        // To avoid using the (slower) division operator, we
        // want to convert 'beats per 60000ms' to 'beats per 65536ms',
        // and then use a simple, fast bit-shift to divide by 65536.
        //
        // The ratio 65536:60000 is 279.620266667:256; we'll call it 280:256.
        // The conversion is accurate to about 0.05%, more or less,
        // e.g. if you ask for "120 BPM", you'll get about "119.93".
        return (((millis()) - timebase) * beats_per_minute_88 * 280) >> 16;
    }

    uint16_t beat16(accum88 beats_per_minute, uint32_t timebase = 0)
    {
        // Convert simple 8-bit BPM's to full Q8.8 accum88's if needed
        if (beats_per_minute < 256)
            beats_per_minute <<= 8;
        return beat88(beats_per_minute, timebase);
    }

    uint16_t beatsin16(accum88 beats_per_minute, uint16_t lowest = 0, uint16_t highest = 65535,
                       uint32_t timebase = 0, uint16_t phase_offset = 0)
    {
        uint16_t beat = beat16(beats_per_minute, timebase);
        uint16_t beatsin = (sin16(beat + phase_offset) + 32768);
        uint16_t rangewidth = highest - lowest;
        uint16_t scaledbeat = scale16(beatsin, rangewidth);
        uint16_t result = lowest + scaledbeat;
        return result;
    }

    uint16_t beatsin88(accum88 beats_per_minute_88, uint16_t lowest = 0, uint16_t highest = 65535,
                       uint32_t timebase = 0, uint16_t phase_offset = 0)
    {
        uint16_t beat = beat88(beats_per_minute_88, timebase);
        uint16_t beatsin = (sin16(beat + phase_offset) + 32768);
        uint16_t rangewidth = highest - lowest;
        uint16_t scaledbeat = scale16(beatsin, rangewidth);
        uint16_t result = lowest + scaledbeat;
        return result;
    }

    uint8_t beat8(accum88 beats_per_minute, uint32_t timebase = 0)
    {
        return beat16(beats_per_minute, timebase) >> 8;
    }

    uint8_t beatsin8(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255,
                     uint32_t timebase = 0, uint8_t phase_offset = 0)
    {
        uint8_t beat = beat8(beats_per_minute, timebase);
        uint8_t beatsin = sin8(beat + phase_offset);
        uint8_t rangewidth = highest - lowest;
        uint8_t scaledbeat = scale8(beatsin, rangewidth);
        uint8_t result = lowest + scaledbeat;
        return result;
    }

    const CRGBPalette16 CloudColors_p = {
        CRGB::Blue,
        CRGB::DarkBlue,
        CRGB::DarkBlue,
        CRGB::DarkBlue,

        CRGB::DarkBlue,
        CRGB::DarkBlue,
        CRGB::DarkBlue,
        CRGB::DarkBlue,

        CRGB::Blue,
        CRGB::DarkBlue,
        CRGB::SkyBlue,
        CRGB::SkyBlue,

        CRGB::LightBlue,
        CRGB::White,
        CRGB::LightBlue,
        CRGB::SkyBlue
    };

    const CRGBPalette16 LavaColors_p = {
        CRGB::Black,
        CRGB::Maroon,
        CRGB::Black,
        CRGB::Maroon,

        CRGB::DarkRed,
        CRGB::DarkRed,
        CRGB::Maroon,
        CRGB::DarkRed,

        CRGB::DarkRed,
        CRGB::DarkRed,
        CRGB::Red,
        CRGB::Orange,

        CRGB::White,
        CRGB::Orange,
        CRGB::Red,
        CRGB::DarkRed
    };

    const CRGBPalette16 OceanColors_p = {
        CRGB::MidnightBlue,
        CRGB::DarkBlue,
        CRGB::MidnightBlue,
        CRGB::Navy,

        CRGB::DarkBlue,
        CRGB::MediumBlue,
        CRGB::SeaGreen,
        CRGB::Teal,

        CRGB::CadetBlue,
        CRGB::Blue,
        CRGB::DarkCyan,
        CRGB::CornflowerBlue,

        CRGB::Aquamarine,
        CRGB::SeaGreen,
        CRGB::Aqua,
        CRGB::LightSkyBlue
    };

    const CRGBPalette16 ForestColors_p = {
        CRGB::DarkGreen,
        CRGB::DarkGreen,
        CRGB::DarkOliveGreen,
        CRGB::DarkGreen,

        CRGB::Green,
        CRGB::ForestGreen,
        CRGB::OliveDrab,
        CRGB::Green,

        CRGB::SeaGreen,
        CRGB::MediumAquamarine,
        CRGB::LimeGreen,
        CRGB::YellowGreen,

        CRGB::LightGreen,
        CRGB::LawnGreen,
        CRGB::MediumAquamarine,
        CRGB::ForestGreen
    };

    /// HSV Rainbow
    const CRGBPalette16 RainbowColors_p = {
        0xFF0000, 0xD52A00, 0xAB5500, 0xAB7F00,
        0xABAB00, 0x56D500, 0x00FF00, 0x00D52A,
        0x00AB55, 0x0056AA, 0x0000FF, 0x2A00D5,
        0x5500AB, 0x7F0081, 0xAB0055, 0xD5002B
    };

/// HSV Rainbow colors with alternatating stripes of black
#define RainbowStripesColors_p RainbowStripeColors_p
    const CRGBPalette16 RainbowStripeColors_p = {
        0xFF0000, 0x000000, 0xAB5500, 0x000000,
        0xABAB00, 0x000000, 0x00FF00, 0x000000,
        0x00AB55, 0x000000, 0x0000FF, 0x000000,
        0x5500AB, 0x000000, 0xAB0055, 0x000000
    };

    /// HSV color ramp: blue purple ping red orange yellow (and back)
    /// Basically, everything but the greens, which tend to make
    /// people's skin look unhealthy.  This palette is good for
    /// lighting at a club or party, where it'll be shining on people.
    const CRGBPalette16 PartyColors_p = {
        0x5500AB, 0x84007C, 0xB5004B, 0xE5001B,
        0xE81700, 0xB84700, 0xAB7700, 0xABAB00,
        0xAB5500, 0xDD2200, 0xF2000E, 0xC2003E,
        0x8F0071, 0x5F00A1, 0x2F00D0, 0x0007F9
    };

    /// Approximate "black body radiation" palette, akin to
    /// the FastLED 'HeatColor' function.
    /// Recommend that you use values 0-240 rather than
    /// the usual 0-255, as the last 15 colors will be
    /// 'wrapping around' from the hot end to the cold end,
    /// which looks wrong.
    const CRGBPalette16 HeatColors_p = {
        0x000000,
        0x330000, 0x660000, 0x990000, 0xCC0000, 0xFF0000,
        0xFF3300, 0xFF6600, 0xFF9900, 0xFFCC00, 0xFFFF00,
        0xFFFF33, 0xFFFF66, 0xFFFF99, 0xFFFFCC, 0xFFFFFF
    };

    public:
    uint32_t millis() const;
    void SetBuffer(RenderBuffer* buffer)
    {
        _buffer = buffer;
    }
    private:

#endif

  typedef uint16_t (WS2812FX::*mode_ptr)(void);

  // pre show callback
  typedef void (*show_callback) (void);

#ifndef XLIGHTS_FX
  static WS2812FX* instance;
#endif
  
  // segment parameters
  public:
    typedef struct Segment { // 30 (32 in memory) bytes
      WS2812FX* _fx = nullptr;
      uint16_t start;
      uint16_t stop; //segment invalid if stop == 0
      uint16_t offset;
      uint8_t  speed;
      uint8_t  intensity;
      uint8_t  palette;
      uint8_t  mode;
      uint8_t  options; //bit pattern: msb first: transitional needspixelstate tbd tbd (paused) on reverse selected
      uint8_t  grouping, spacing;
      uint8_t  opacity;
      uint32_t colors[NUM_COLORS];
      uint8_t  cct; //0==1900K, 255==10091K
      char *name;
      bool setColor(uint8_t slot, uint32_t c, uint8_t segn) { //returns true if changed
        if (slot >= NUM_COLORS || segn >= MAX_NUM_SEGMENTS) return false;
        if (c == colors[slot]) return false;
        uint8_t b = (slot == 1) ? cct : opacity;
        ColorTransition::startTransition(_fx, b, colors[slot], instance->_transitionDur, segn, slot);
        colors[slot] = c; return true;
      }
      int NumActualColours()
      {
          int num = NUM_COLORS - 1;
          while (num > 0 && colors[num] == 0xFF000000) {
              num--;
          }
          if (num < 0)
              num = 0;
          return num + 1;
      }
      void setCCT(uint16_t k, uint8_t segn) {
        if (segn >= MAX_NUM_SEGMENTS) return;
        if (k > 255) { //kelvin value, convert to 0-255
          if (k < 1900)  k = 1900;
          if (k > 10091) k = 10091;
          k = (k - 1900) >> 5;
        }
        if (cct == k) return;
        ColorTransition::startTransition(_fx, cct, colors[1], instance->_transitionDur, segn, 1);
        cct = k;
      }
      void setOpacity(uint8_t o, uint8_t segn) {
        if (segn >= MAX_NUM_SEGMENTS) return;
        if (opacity == o) return;
        ColorTransition::startTransition(_fx, opacity, colors[0], instance->_transitionDur, segn, 0);
        opacity = o;
      }
      void setOption(uint8_t n, bool val, uint8_t segn = 255)
      {
        bool prevOn = false;
        if (n == SEG_OPTION_ON) {
          prevOn = getOption(SEG_OPTION_ON);
          if (!val && prevOn) { //fade off
            ColorTransition::startTransition(_fx, opacity, colors[0], instance->_transitionDur, segn, 0);
          }
        }

        if (val) {
          options |= 0x01 << n;
        } else
        {
          options &= ~(0x01 << n);
        }

        if (n == SEG_OPTION_ON && val && !prevOn) { //fade on
          ColorTransition::startTransition(_fx, 0, colors[0], instance->_transitionDur, segn, 0);
        }
      }
      bool getOption(uint8_t n)
      {
        return ((options >> n) & 0x01);
      }
      inline bool isSelected()
      {
        return getOption(0);
      }
      inline bool isActive()
      {
        return stop > start;
      }
      inline uint16_t length()
      {
        return stop - start;
      }
      inline uint16_t groupLength()
      {
        return grouping + spacing;
      }
      uint16_t virtualLength()
      {
        uint16_t groupLen = groupLength();
        uint16_t vLength = (length() + groupLen - 1) / groupLen;
        if (options & MIRROR)
          vLength = (vLength + 1) /2;  // divide by 2 if mirror, leave at least a single LED
        return vLength;
      }
      uint8_t differs(Segment& b) {
        uint8_t d = 0;
        if (start != b.start)         d |= SEG_DIFFERS_BOUNDS;
        if (stop != b.stop)           d |= SEG_DIFFERS_BOUNDS;
        if (offset != b.offset)       d |= SEG_DIFFERS_GSO;
        if (grouping != b.grouping)   d |= SEG_DIFFERS_GSO;
        if (spacing != b.spacing)     d |= SEG_DIFFERS_GSO;
        if (opacity != b.opacity)     d |= SEG_DIFFERS_BRI;
        if (mode != b.mode)           d |= SEG_DIFFERS_FX;
        if (speed != b.speed)         d |= SEG_DIFFERS_FX;
        if (intensity != b.intensity) d |= SEG_DIFFERS_FX;
        if (palette != b.palette)     d |= SEG_DIFFERS_FX;

        if ((options & 0b00101111) != (b.options & 0b00101111)) d |= SEG_DIFFERS_OPT;
        for (uint8_t i = 0; i < NUM_COLORS; i++)
        {
          if (colors[i] != b.colors[i]) d |= SEG_DIFFERS_COL;
        }

        return d;
      }
    } segment;

  // segment runtime parameters
    typedef struct Segment_runtime { // 28 bytes
      WS2812FX* _fx = nullptr;
      unsigned long next_time;  // millis() of next update
      uint32_t step;  // custom "step" var
      uint32_t call;  // call counter
      uint16_t aux0;  // custom var
      uint16_t aux1;  // custom var
      byte* data = nullptr;
      bool allocateData(uint16_t len){
        if (data && _dataLen == len) return true; //already allocated
        deallocateData();
        //if (instance->_usedSegmentData + len > MAX_SEGMENT_DATA) return false; //not enough memory
        // if possible use SPI RAM on ESP32
        #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_PSRAM)
        if (psramFound())
          data = (byte*) ps_malloc(len);
        else
        #endif
          data = (byte*) malloc(len);
        if (!data) return false; //allocation failed
        instance->_usedSegmentData += len;
        _dataLen = len;
        memset(data, 0, len);
        return true;
      }
      void deallocateData(){
        free(data);
        data = nullptr;
        instance->_usedSegmentData -= _dataLen;
        _dataLen = 0;
      }

      /** 
       * If reset of this segment was request, clears runtime
       * settings of this segment.
       * Must not be called while an effect mode function is running
       * because it could access the data buffer and this method 
       * may free that data buffer.
       */
      void resetIfRequired() {
        if (_requiresReset) {
          next_time = 0; step = 0; call = 0; aux0 = 0; aux1 = 0; 
          deallocateData();
          _requiresReset = false;
        }
      }

      /** 
       * Flags that before the next effect is calculated,
       * the internal segment state should be reset. 
       * Call resetIfRequired before calling the next effect function.
       */
      inline void reset() { _requiresReset = true; }
      private:
        uint16_t _dataLen = 0;
        bool _requiresReset = false;
    } segment_runtime;

    typedef struct ColorTransition { // 12 bytes
#ifdef XLIGHTS_FX
        WS2812FX* _fx = nullptr;
#endif
      uint32_t colorOld = 0;
      uint32_t transitionStart;
      uint16_t transitionDur;
      uint8_t segment = 0xFF; //lower 6 bits: the segment this transition is for (255 indicates transition not in use/available) upper 2 bits: color channel
      uint8_t briOld = 0;
      static void startTransition(WS2812FX* fx, uint8_t oldBri, uint32_t oldCol, uint16_t dur, uint8_t segn, uint8_t slot) {
        if (segn >= MAX_NUM_SEGMENTS || slot >= NUM_COLORS || dur == 0) return;
        if (fx->_brightness == 0) return; //do not need transitions if master bri is off
        if (!fx->_segments[segn].getOption(SEG_OPTION_ON)) return; //not if segment is off either
        uint8_t tIndex = 0xFF; //none found
        uint16_t tProgression = 0;
        uint8_t s = segn + (slot << 6); //merge slot and segment into one byte

        for (uint8_t i = 0; i < MAX_NUM_TRANSITIONS; i++) {
          uint8_t tSeg = fx->transitions[i].segment;
          //see if this segment + color already has a running transition
          if (tSeg == s) {
            tIndex = i; break;
          }
          if (tSeg == 0xFF) { //free transition
            tIndex = i; tProgression = 0xFFFF;
          }
        }

        if (tIndex == 0xFF) { //no slot found yet
          for (uint8_t i = 0; i < MAX_NUM_TRANSITIONS; i++) {
            //find most progressed transition to overwrite
            uint16_t prog = fx->transitions[i].progress();
            if (prog > tProgression) {
              tIndex = i; tProgression = prog;
            }
          }
        }

        ColorTransition& t = fx->transitions[tIndex];
        if (t.segment == s) //this is an active transition on the same segment+color
        {
          bool wasTurningOff = (oldBri == 0);
          t.briOld = t.currentBri(wasTurningOff, slot);
          t.colorOld = t.currentColor(oldCol);
        } else {
          t.briOld = oldBri;
          t.colorOld = oldCol;
          uint8_t prevSeg = t.segment & 0x3F;
          if (prevSeg < MAX_NUM_SEGMENTS) fx->_segments[prevSeg].setOption(SEG_OPTION_TRANSITIONAL, false);
        }
        t.transitionDur = dur;
        t.transitionStart = fx->millis();
        t.segment = s;
        fx->_segments[segn].setOption(SEG_OPTION_TRANSITIONAL, true);
        //refresh immediately, required for Solid mode
        if (fx->_segment_runtimes[segn].next_time > t.transitionStart + 22) fx->_segment_runtimes[segn].next_time = t.transitionStart;
      }
      uint16_t progress(bool allowEnd = false) { //transition progression between 0-65535
        uint32_t timeNow = _fx->millis();
        if (timeNow - transitionStart > transitionDur) {
          if (allowEnd) {
            uint8_t segn = segment & 0x3F;
            if (segn < MAX_NUM_SEGMENTS) instance->_segments[segn].setOption(SEG_OPTION_TRANSITIONAL, false);
            segment = 0xFF;
          }
          return 0xFFFF;
        }
        uint32_t elapsed = timeNow - transitionStart;
        uint32_t prog = elapsed * 0xFFFF / transitionDur;
        return (prog > 0xFFFF) ? 0xFFFF : prog;
      }
      uint32_t currentColor(uint32_t colorNew) {
        return instance->color_blend(colorOld, colorNew, progress(true), true);
      }
      uint8_t currentBri(bool turningOff = false, uint8_t slot = 0) {
        uint8_t segn = segment & 0x3F;
        if (segn >= MAX_NUM_SEGMENTS) return 0;
        uint8_t briNew = instance->_segments[segn].opacity;
        if (slot == 0) {
          if (!instance->_segments[segn].getOption(SEG_OPTION_ON) || turningOff) briNew = 0;
        } else { //transition slot 1 brightness for CCT transition
          briNew = instance->_segments[segn].cct;
        }
        uint32_t prog = progress() + 1;
        return ((briNew * prog) + (briOld * (0x10000 - prog))) >> 16;
      }
    } color_transition;

    WS2812FX() {
      //WS2812FX::instance = this;
      //assign each member of the _mode[] array to its respective function reference 
      _mode[FX_MODE_STATIC]                  = &WS2812FX::mode_static;
      _mode[FX_MODE_BLINK]                   = &WS2812FX::mode_blink;
      _mode[FX_MODE_COLOR_WIPE]              = &WS2812FX::mode_color_wipe;
      _mode[FX_MODE_COLOR_WIPE_RANDOM]       = &WS2812FX::mode_color_wipe_random;
      _mode[FX_MODE_RANDOM_COLOR]            = &WS2812FX::mode_random_color;
      _mode[FX_MODE_COLOR_SWEEP]             = &WS2812FX::mode_color_sweep;
      _mode[FX_MODE_DYNAMIC]                 = &WS2812FX::mode_dynamic;
      _mode[FX_MODE_RAINBOW]                 = &WS2812FX::mode_rainbow;
      _mode[FX_MODE_RAINBOW_CYCLE]           = &WS2812FX::mode_rainbow_cycle;
      _mode[FX_MODE_SCAN]                    = &WS2812FX::mode_scan;
      _mode[FX_MODE_DUAL_SCAN]               = &WS2812FX::mode_dual_scan;
      _mode[FX_MODE_FADE]                    = &WS2812FX::mode_fade;
      _mode[FX_MODE_THEATER_CHASE]           = &WS2812FX::mode_theater_chase;
      _mode[FX_MODE_THEATER_CHASE_RAINBOW]   = &WS2812FX::mode_theater_chase_rainbow;
      _mode[FX_MODE_SAW]                     = &WS2812FX::mode_saw;
      _mode[FX_MODE_TWINKLE]                 = &WS2812FX::mode_twinkle;
      _mode[FX_MODE_DISSOLVE]                = &WS2812FX::mode_dissolve;
      _mode[FX_MODE_DISSOLVE_RANDOM]         = &WS2812FX::mode_dissolve_random;
      _mode[FX_MODE_SPARKLE]                 = &WS2812FX::mode_sparkle;
      _mode[FX_MODE_FLASH_SPARKLE]           = &WS2812FX::mode_flash_sparkle;
      _mode[FX_MODE_HYPER_SPARKLE]           = &WS2812FX::mode_hyper_sparkle;
      _mode[FX_MODE_STROBE]                  = &WS2812FX::mode_strobe;
      _mode[FX_MODE_STROBE_RAINBOW]          = &WS2812FX::mode_strobe_rainbow;
      _mode[FX_MODE_MULTI_STROBE]            = &WS2812FX::mode_multi_strobe;
      _mode[FX_MODE_BLINK_RAINBOW]           = &WS2812FX::mode_blink_rainbow;
      _mode[FX_MODE_ANDROID]                 = &WS2812FX::mode_android;
      _mode[FX_MODE_CHASE_COLOR]             = &WS2812FX::mode_chase_color;
      _mode[FX_MODE_CHASE_RANDOM]            = &WS2812FX::mode_chase_random;
      _mode[FX_MODE_CHASE_RAINBOW]           = &WS2812FX::mode_chase_rainbow;
      _mode[FX_MODE_CHASE_FLASH]             = &WS2812FX::mode_chase_flash;
      _mode[FX_MODE_CHASE_FLASH_RANDOM]      = &WS2812FX::mode_chase_flash_random;
      _mode[FX_MODE_CHASE_RAINBOW_WHITE]     = &WS2812FX::mode_chase_rainbow_white;
      _mode[FX_MODE_COLORFUL]                = &WS2812FX::mode_colorful;
      _mode[FX_MODE_TRAFFIC_LIGHT]           = &WS2812FX::mode_traffic_light;
      _mode[FX_MODE_COLOR_SWEEP_RANDOM]      = &WS2812FX::mode_color_sweep_random;
      _mode[FX_MODE_RUNNING_COLOR]           = &WS2812FX::mode_running_color;
      _mode[FX_MODE_AURORA]                  = &WS2812FX::mode_aurora;
      _mode[FX_MODE_RUNNING_RANDOM]          = &WS2812FX::mode_running_random;
      _mode[FX_MODE_LARSON_SCANNER]          = &WS2812FX::mode_larson_scanner;
      _mode[FX_MODE_COMET]                   = &WS2812FX::mode_comet;
      _mode[FX_MODE_FIREWORKS]               = &WS2812FX::mode_fireworks;
      _mode[FX_MODE_RAIN]                    = &WS2812FX::mode_rain;
      _mode[FX_MODE_TETRIX]                  = &WS2812FX::mode_tetrix;
      _mode[FX_MODE_FIRE_FLICKER]            = &WS2812FX::mode_fire_flicker;
      _mode[FX_MODE_GRADIENT]                = &WS2812FX::mode_gradient;
      _mode[FX_MODE_LOADING]                 = &WS2812FX::mode_loading;
      _mode[FX_MODE_POLICE]                  = &WS2812FX::mode_police;
      _mode[FX_MODE_FAIRY]                   = &WS2812FX::mode_fairy;
      _mode[FX_MODE_TWO_DOTS]                = &WS2812FX::mode_two_dots;
      _mode[FX_MODE_FAIRYTWINKLE]            = &WS2812FX::mode_fairytwinkle;
      _mode[FX_MODE_RUNNING_DUAL]            = &WS2812FX::mode_running_dual;
      _mode[FX_MODE_HALLOWEEN]               = &WS2812FX::mode_halloween;
      _mode[FX_MODE_TRICOLOR_CHASE]          = &WS2812FX::mode_tricolor_chase;
      _mode[FX_MODE_TRICOLOR_WIPE]           = &WS2812FX::mode_tricolor_wipe;
      _mode[FX_MODE_TRICOLOR_FADE]           = &WS2812FX::mode_tricolor_fade;
      _mode[FX_MODE_BREATH]                  = &WS2812FX::mode_breath;
      _mode[FX_MODE_RUNNING_LIGHTS]          = &WS2812FX::mode_running_lights;
      _mode[FX_MODE_LIGHTNING]               = &WS2812FX::mode_lightning;
      _mode[FX_MODE_ICU]                     = &WS2812FX::mode_icu;
      _mode[FX_MODE_MULTI_COMET]             = &WS2812FX::mode_multi_comet;
      _mode[FX_MODE_DUAL_LARSON_SCANNER]     = &WS2812FX::mode_dual_larson_scanner;
      _mode[FX_MODE_RANDOM_CHASE]            = &WS2812FX::mode_random_chase;
      _mode[FX_MODE_OSCILLATE]               = &WS2812FX::mode_oscillate;
      _mode[FX_MODE_FIRE_2012]               = &WS2812FX::mode_fire_2012;
      _mode[FX_MODE_PRIDE_2015]              = &WS2812FX::mode_pride_2015;
      _mode[FX_MODE_BPM]                     = &WS2812FX::mode_bpm;
      _mode[FX_MODE_JUGGLE]                  = &WS2812FX::mode_juggle;
      _mode[FX_MODE_PALETTE]                 = &WS2812FX::mode_palette;
      _mode[FX_MODE_COLORWAVES]              = &WS2812FX::mode_colorwaves;
      _mode[FX_MODE_FILLNOISE8]              = &WS2812FX::mode_fillnoise8;
      _mode[FX_MODE_NOISE16_1]               = &WS2812FX::mode_noise16_1;
      _mode[FX_MODE_NOISE16_2]               = &WS2812FX::mode_noise16_2;
      _mode[FX_MODE_NOISE16_3]               = &WS2812FX::mode_noise16_3;
      _mode[FX_MODE_NOISE16_4]               = &WS2812FX::mode_noise16_4;
      _mode[FX_MODE_COLORTWINKLE]            = &WS2812FX::mode_colortwinkle;
      _mode[FX_MODE_LAKE]                    = &WS2812FX::mode_lake;
      _mode[FX_MODE_METEOR]                  = &WS2812FX::mode_meteor;
      _mode[FX_MODE_METEOR_SMOOTH]           = &WS2812FX::mode_meteor_smooth;
      _mode[FX_MODE_RAILWAY]                 = &WS2812FX::mode_railway;
      _mode[FX_MODE_RIPPLE]                  = &WS2812FX::mode_ripple;
      _mode[FX_MODE_TWINKLEFOX]              = &WS2812FX::mode_twinklefox;
      _mode[FX_MODE_TWINKLECAT]              = &WS2812FX::mode_twinklecat;
      _mode[FX_MODE_HALLOWEEN_EYES]          = &WS2812FX::mode_halloween_eyes;
      _mode[FX_MODE_STATIC_PATTERN]          = &WS2812FX::mode_static_pattern;
      _mode[FX_MODE_TRI_STATIC_PATTERN]      = &WS2812FX::mode_tri_static_pattern;
      _mode[FX_MODE_SPOTS]                   = &WS2812FX::mode_spots;
      _mode[FX_MODE_SPOTS_FADE]              = &WS2812FX::mode_spots_fade;
      _mode[FX_MODE_GLITTER]                 = &WS2812FX::mode_glitter;
      _mode[FX_MODE_CANDLE]                  = &WS2812FX::mode_candle;
      _mode[FX_MODE_STARBURST]               = &WS2812FX::mode_starburst;
      _mode[FX_MODE_EXPLODING_FIREWORKS]     = &WS2812FX::mode_exploding_fireworks;
      _mode[FX_MODE_BOUNCINGBALLS]           = &WS2812FX::mode_bouncing_balls;
      _mode[FX_MODE_SINELON]                 = &WS2812FX::mode_sinelon;
      _mode[FX_MODE_SINELON_DUAL]            = &WS2812FX::mode_sinelon_dual;
      _mode[FX_MODE_SINELON_RAINBOW]         = &WS2812FX::mode_sinelon_rainbow;
      _mode[FX_MODE_POPCORN]                 = &WS2812FX::mode_popcorn;
      _mode[FX_MODE_DRIP]                    = &WS2812FX::mode_drip;
      _mode[FX_MODE_PLASMA]                  = &WS2812FX::mode_plasma;
      _mode[FX_MODE_PERCENT]                 = &WS2812FX::mode_percent;
      _mode[FX_MODE_RIPPLE_RAINBOW]          = &WS2812FX::mode_ripple_rainbow;
      _mode[FX_MODE_HEARTBEAT]               = &WS2812FX::mode_heartbeat;
      _mode[FX_MODE_PACIFICA]                = &WS2812FX::mode_pacifica;
      _mode[FX_MODE_CANDLE_MULTI]            = &WS2812FX::mode_candle_multi;
      _mode[FX_MODE_SOLID_GLITTER]           = &WS2812FX::mode_solid_glitter;
      _mode[FX_MODE_SUNRISE]                 = &WS2812FX::mode_sunrise;
      _mode[FX_MODE_PHASED]                  = &WS2812FX::mode_phased;
      _mode[FX_MODE_TWINKLEUP]               = &WS2812FX::mode_twinkleup;
      _mode[FX_MODE_NOISEPAL]                = &WS2812FX::mode_noisepal;
      _mode[FX_MODE_SINEWAVE]                = &WS2812FX::mode_sinewave;
      _mode[FX_MODE_PHASEDNOISE]             = &WS2812FX::mode_phased_noise;
      _mode[FX_MODE_FLOW]                    = &WS2812FX::mode_flow;
      _mode[FX_MODE_CHUNCHUN]                = &WS2812FX::mode_chunchun;
      _mode[FX_MODE_DANCING_SHADOWS]         = &WS2812FX::mode_dancing_shadows;
      _mode[FX_MODE_WASHING_MACHINE]         = &WS2812FX::mode_washing_machine;
      _mode[FX_MODE_CANDY_CANE]              = &WS2812FX::mode_candy_cane;
      _mode[FX_MODE_BLENDS]                  = &WS2812FX::mode_blends;
      _mode[FX_MODE_TV_SIMULATOR]            = &WS2812FX::mode_tv_simulator;
      _mode[FX_MODE_DYNAMIC_SMOOTH]          = &WS2812FX::mode_dynamic_smooth;

      _brightness = DEFAULT_BRIGHTNESS;
#ifndef XLIGHTS_FX
      currentPalette = CRGBPalette16(CRGB::Black);
#else
      currentPalette = CRGBPalette16(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
#endif
      targetPalette = CloudColors_p;
      ablMilliampsMax = 850;
      currentMilliamps = 0;
      timebase = 0;
      resetSegments();
    }

    void
      finalizeInit(),
      service(void),
      blur(uint8_t),
      fill(uint32_t),
      fade_out(uint8_t r, uint32_t toColour),
      setMode(uint8_t segid, uint8_t m),
      setColor(uint8_t slot, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      setColor(uint8_t slot, uint32_t c),
      setBrightness(uint8_t b),
      setRange(uint16_t i, uint16_t i2, uint32_t col),
      setShowCallback(show_callback cb),
      setTransition(uint16_t t),
      setTransitionMode(bool t),
      calcGammaTable(float),
      trigger(void),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t grouping = 0, uint8_t spacing = 0, uint16_t offset = UINT16_MAX),
      restartRuntime(),
      resetSegments(),
      makeAutoSegments(),
      fixInvalidSegments(),
      setPixelColor(uint16_t n, uint32_t c),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0),
      show(void),
			setTargetFps(uint8_t fps),
      setPixelSegment(uint8_t n),
      deserializeMap(uint8_t n=0);

    bool
      isRgbw = false,
      isOffRefreshRequred = false, //periodic refresh is required for the strip to remain off.
      gammaCorrectBri = false,
      gammaCorrectCol = false,
      applyToAllSelected = true,
      setEffectConfig(uint8_t m, uint8_t s, uint8_t i, uint8_t p),
      checkSegmentAlignment(void),
			hasCCTBus(void),
      // return true if the strip is being sent pixel updates
      isUpdating(void);

    uint8_t
      mainSegment = 0,
      paletteFade = 0,
      paletteBlend = 0, // Linear (wrap if moving)
      milliampsPerLed = 55,
			cctBlending = 0,
      getBrightness(void),
      getMode(void),
      getSpeed(void),
      getModeCount(void),
      getPaletteCount(void),
      getMaxSegments(void),
      getActiveSegmentsNum(void),
      //getFirstSelectedSegment(void),
      getMainSegmentId(void),
			getTargetFps(void),
      gamma8(uint8_t),
      gamma8_cal(uint8_t, float),
      sin_gap(uint16_t),
      get_random_wheel_index(uint8_t);

    int8_t
      tristate_square8(uint8_t x, uint8_t pulsewidth, uint8_t attdec);

    uint16_t
      ablMilliampsMax,
      currentMilliamps,
      triwave16(uint16_t),
      getLengthTotal(void),
      getLengthPhysical(void),
      getFps();

    uint32_t
      now,
      timebase,
      color_wheel(uint8_t),
      color_from_palette(uint16_t, bool mapping, bool wrap, uint8_t mcol, uint8_t pbri = 255),
      color_blend(uint32_t,uint32_t,uint16_t,bool b16=false),
      currentColor(uint32_t colorNew, uint8_t tNr),
      gamma32(uint32_t),
      getLastShow(void),
      getPixelColor(uint16_t),
      getColor(void);

    WS2812FX::Segment&
      getSegment(uint8_t n);

    WS2812FX::Segment_runtime
      getSegmentRuntime(void);

    WS2812FX::Segment*
      getSegments(void);

    // builtin modes
    uint16_t
      mode_static(void),
      mode_blink(void),
      mode_blink_rainbow(void),
      mode_strobe(void),
      mode_strobe_rainbow(void),
      mode_color_wipe(void),
      mode_color_sweep(void),
      mode_color_wipe_random(void),
      mode_color_sweep_random(void),
      mode_random_color(void),
      mode_dynamic(void),
      mode_breath(void),
      mode_fade(void),
      mode_scan(void),
      mode_dual_scan(void),
      mode_theater_chase(void),
      mode_theater_chase_rainbow(void),
      mode_rainbow(void),
      mode_rainbow_cycle(void),
      mode_running_lights(void),
      mode_saw(void),
      mode_twinkle(void),
      mode_dissolve(void),
      mode_dissolve_random(void),
      mode_sparkle(void),
      mode_flash_sparkle(void),
      mode_hyper_sparkle(void),
      mode_multi_strobe(void),
      mode_android(void),
      mode_chase_color(void),
      mode_chase_random(void),
      mode_chase_rainbow(void),
      mode_chase_flash(void),
      mode_chase_flash_random(void),
      mode_chase_rainbow_white(void),
      mode_colorful(void),
      mode_traffic_light(void),
      mode_running_color(void),
      mode_aurora(void),
      mode_running_random(void),
      mode_larson_scanner(void),
      mode_comet(void),
      mode_fireworks(void),
      mode_rain(void),
      mode_tetrix(void),
      mode_halloween(void),
      mode_fire_flicker(void),
      mode_gradient(void),
      mode_loading(void),
      mode_police(void),
      mode_fairy(void),
      mode_two_dots(void),
      mode_fairytwinkle(void),
      mode_running_dual(void),
      mode_bicolor_chase(void),
      mode_tricolor_chase(void),
      mode_tricolor_wipe(void),
      mode_tricolor_fade(void),
      mode_lightning(void),
      mode_icu(void),
      mode_multi_comet(void),
      mode_dual_larson_scanner(void),
      mode_random_chase(void),
      mode_oscillate(void),
      mode_fire_2012(void),
      mode_pride_2015(void),
      mode_bpm(void),
      mode_juggle(void),
      mode_palette(void),
      mode_colorwaves(void),
      mode_fillnoise8(void),
      mode_noise16_1(void),
      mode_noise16_2(void),
      mode_noise16_3(void),
      mode_noise16_4(void),
      mode_colortwinkle(void),
      mode_lake(void),
      mode_meteor(void),
      mode_meteor_smooth(void),
      mode_railway(void),
      mode_ripple(void),
      mode_twinklefox(void),
      mode_twinklecat(void),
      mode_halloween_eyes(void),
      mode_static_pattern(void),
      mode_tri_static_pattern(void),
      mode_spots(void),
      mode_spots_fade(void),
      mode_glitter(void),
      mode_candle(void),
      mode_starburst(void),
      mode_exploding_fireworks(void),
      mode_bouncing_balls(void),
      mode_sinelon(void),
      mode_sinelon_dual(void),
      mode_sinelon_rainbow(void),
      mode_popcorn(void),
      mode_drip(void),
      mode_plasma(void),
      mode_percent(void),
      mode_ripple_rainbow(void),
      mode_heartbeat(void),
      mode_pacifica(void),
      mode_candle_multi(void),
      mode_solid_glitter(void),
      mode_sunrise(void),
      mode_phased(void),
      mode_twinkleup(void),
      mode_noisepal(void),
      mode_sinewave(void),
      mode_phased_noise(void),
      mode_flow(void),
      mode_chunchun(void),
      mode_dancing_shadows(void),
      mode_washing_machine(void),
      mode_candy_cane(void),
      mode_blends(void),
      mode_tv_simulator(void),
      mode_dynamic_smooth(void);

  private:
    uint32_t crgb_to_col(CRGB fastled);
    CRGB col_to_crgb(uint32_t);
    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;

    uint16_t _length, _virtualSegmentLength;
    uint16_t _rand16seed;
    uint8_t _brightness;
    uint16_t _usedSegmentData = 0;
    uint16_t _transitionDur = 750;

		uint8_t _targetFps = 42;
		uint16_t _frametime = (1000/42);
    uint16_t _cumulativeFps = 2;

    bool
      _triggered = false;

    mode_ptr _mode[MODE_COUNT]; // SRAM footprint: 4 bytes per element

    show_callback _callback = nullptr;

    // mode helper functions
    uint16_t
      blink(uint32_t, uint32_t, bool strobe, bool),
      candle(bool),
      color_wipe(bool, bool),
      dynamic(bool),
      scan(bool),
      running_base(bool,bool),
      larson_scanner(bool),
      sinelon_base(bool,bool),
      dissolve(uint32_t),
      chase(uint32_t, uint32_t, uint32_t, bool),
      gradient_base(bool),
      ripple_base(bool),
      police_base(uint32_t, uint32_t),
      running(uint32_t, uint32_t, bool theatre=false),
      tricolor_chase(uint32_t, uint32_t),
      twinklefox_base(bool),
      spots_base(uint16_t),
      phased_base(uint8_t);

    CRGB twinklefox_one_twinkle(uint32_t ms, uint8_t salt, bool cat);
    CRGB pacifica_one_layer(uint16_t i, CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff);

    void
      blendPixelColor(uint16_t n, uint32_t color, uint8_t blend),
      startTransition(uint8_t oldBri, uint32_t oldCol, uint16_t dur, uint8_t segn, uint8_t slot),
      estimateCurrentAndLimitBri(void),
      load_gradient_palette(uint8_t),
      handle_palette(void);

    uint16_t* customMappingTable = nullptr;
    uint16_t  customMappingSize  = 0;
    
    uint32_t _lastPaletteChange = 0;
    uint32_t _lastShow = 0;

    uint32_t _colors_t[NUM_COLORS];
    uint8_t _bri_t;
    
    uint8_t _segment_index = 0;
    uint8_t _segment_index_palette_last = 99;
    segment _segments[MAX_NUM_SEGMENTS] = { // SRAM footprint: 24 bytes per element
      // start, stop, offset, speed, intensity, palette, mode, options, grouping, spacing, opacity (unused), color[]
      {this, 0, 7, 0, DEFAULT_SPEED, 128, 0, DEFAULT_MODE, NO_OPTIONS, 1, 0, 255, {DEFAULT_COLOR}}
    };
    segment_runtime _segment_runtimes[MAX_NUM_SEGMENTS]; // SRAM footprint: 28 bytes per element
    friend struct Segment_runtime;

    ColorTransition transitions[MAX_NUM_TRANSITIONS]; //12 bytes per element
    friend struct ColorTransition;

    uint16_t
      realPixelIndex(uint16_t i),
      transitionProgress(uint8_t tNr);
};

//10 names per line
const char JSON_mode_names[] PROGMEM = R"=====([
"Solid","Blink","Breathe","Wipe","Wipe Random","Random Colors","Sweep","Dynamic","Colorloop","Rainbow",
"Scan","Scan Dual","Fade","Theater","Theater Rainbow","Running","Saw","Twinkle","Dissolve","Dissolve Rnd",
"Sparkle","Sparkle Dark","Sparkle+","Strobe","Strobe Rainbow","Strobe Mega","Blink Rainbow","Android","Chase","Chase Random",
"Chase Rainbow","Chase Flash","Chase Flash Rnd","Rainbow Runner","Colorful","Traffic Light","Sweep Random","Chase 2","Aurora","Stream",
"Scanner","Lighthouse","Fireworks","Rain","Tetrix","Fire Flicker","Gradient","Loading","Police","Fairy",
"Two Dots","Fairytwinkle","Running Dual","Halloween","Chase 3","Tri Wipe","Tri Fade","Lightning","ICU","Multi Comet",
"Scanner Dual","Stream 2","Oscillate","Pride 2015","Juggle","Palette","Fire 2012","Colorwaves","Bpm","Fill Noise",
"Noise 1","Noise 2","Noise 3","Noise 4","Colortwinkles","Lake","Meteor","Meteor Smooth","Railway","Ripple",
"Twinklefox","Twinklecat","Halloween Eyes","Solid Pattern","Solid Pattern Tri","Spots","Spots Fade","Glitter","Candle","Fireworks Starburst",
"Fireworks 1D","Bouncing Balls","Sinelon","Sinelon Dual","Sinelon Rainbow","Popcorn","Drip","Plasma","Percent","Ripple Rainbow",
"Heartbeat","Pacifica","Candle Multi","Solid Glitter","Sunrise","Phased","Twinkleup","Noise Pal","Sine","Phased Noise",
"Flow","Chunchun","Dancing Shadows","Washing Machine","Candy Cane","Blends","TV Simulator","Dynamic Smooth"
])=====";


const char JSON_palette_names[] PROGMEM = R"=====([
"Default","* Random Cycle","* Color 1","* Colors 1&2","* Color Gradient","* Colors Only","Party","Cloud","Lava","Ocean",
"Forest","Rainbow","Rainbow Bands","Sunset","Rivendell","Breeze","Red & Blue","Yellowout","Analogous","Splash",
"Pastel","Sunset 2","Beech","Vintage","Departure","Landscape","Beach","Sherbet","Hult","Hult 64",
"Drywet","Jul","Grintage","Rewhi","Tertiary","Fire","Icefire","Cyane","Light Pink","Autumn",
"Magenta","Magred","Yelmag","Yelblu","Orange & Teal","Tiamat","April Night","Orangery","C9","Sakura",
"Aurora","Atlantica","C9 2","C9 New","Temperature","Aurora 2","Retro Clown","Candy","Toxy Reaf","Fairy Reaf",
"Semi Blue","Pink Candy","Red Reaf","Aqua Flash","Yelblu Hot","Lite Light","Red Flash","Blink Red","Red Shift","Red Tide",
"Candy2"
])=====";

#endif
