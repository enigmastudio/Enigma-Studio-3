/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *
 *    ------  /   /  /\   /  /---  -----  /  ----  /   /
 *       /   /   /  /  \ /  /---   -/-   /   \--  /---/   version 3
 *      /    \---  /    /  /---    /    /  ----/ /   /.
 *
 *       t i n y   m u s i c   s y n t h e s i z e r
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TUNEFISH3_HPP
#define TUNEFISH3_HPP

#ifdef eHWSYNTH
#include <stdio.h>
#endif

#ifdef ePLAYER

#define NO_BANDPASS_FILTER
#define NO_NOTCH_FILTER
#define NO_MM_MODULATORS

#define NO_FX_FLANGER
#define NO_FX_CHORUS
#define NO_FX_EQ

#endif

#ifndef eHWSYNTH
#define TF_OVERSAMPLING
#endif

typedef eF32                        eSignal;
typedef eArray<eSignal>             eSignalArray;

const eU32  TF_ADDSYNTHTABLESIZE    = 128 * 1024;
const eU32  TF_NOISETABLESIZE       = 65536;
const eU32  TF_LFONOISETABLESIZE    = 256;
const eU32  TF_DISTTABLESIZE        = 32768;
const eU32  TF_FLANGERBUFFSIZE      = 4096;
const eU32  TF_NUMFREQS             = 128;
const eU32  TF_MAXVOICES            = 16;
const eU32  TF_OSCILLATOR_POINTS    = 6;
const eU32  TF_OSCILLATOR_SAMPLES   = 256;
const eU32  TF_SUBOSC               = 3;
const eU32  TF_MAXUNISONO           = 10;
const eU32  TF_MODMATRIXENTRIES     = 10;
const eU32  TF_EFFECTSLOTS          = 10;
const eU32  TF_INTERPOLATIONS       = 3;
const eU32  TF_MAXOCTAVES           = 9;
const eU32  TF_MAXSUBOSC            = 1;
const eU32  TF_LFOSHAPECOUNT        = 5;
const eU32  TF_ADDSYNTHPROFILES     = 4;
const eF32  TF_MM_MODRANGE          = 20.0f;
const eU32  TF_MAX_INPUTS           = 32;
const eU32  TF_BLOCKSIZE            = 384;
const eU32  TF_MAX_MODULATIONS      = 4;
const eU32  TF_MAX_OVERSAMPLING     = 8;
const eU32  TF_DESIRED_OVERSAMPLING = 32;
const eU32  TF_PLAYER_PEAK_MEMORY   = 6;

static const eF32 TF_OCTAVES[] =
{
    1.0f*16.0f,
    1.0f*8.0f,
    1.0f*4.0f,
    1.0f*2.0f,
    1.0f,
    1.0f/2.0f,
    1.0f/4.0f,
    1.0f/8.0f,
    1.0f/16.0f,
};

#ifndef ePLAYER

static const eChar * TF_NAMES[] =
{
    "OscPoints",
    "OscP1Off",
    "OscP1Val",
    "OscP1Int",
    "OscP2Off",
    "OscP2Val",
    "OscP2Int",
    "OscP3Off",
    "OscP3Val",
    "OscP3Int",
    "OscP4Off",
    "OscP4Val",
    "OscP4Int",
    "OscFiInt",
    "OscVol",
    "OscFreq",
    "OscPan",
    "OscDetun",
    "OscPoly",
    "OscUnisn",
    "OscSpread",
    "OscSubOsc",
    "OscGlide",
    "OscDrive",
    "OscOctave",
    "OscOffset",
    "OscSlop",

    "AddVolume",
    "AddProfile",
    "AddOctave",
    "AddBandw",
    "AddDamp",
    "AddHarmncs",
    "AddScale",
    "AddDrive",

    "NoiseOn",
    "NoiseFreq",
    "NoiseBw",

    "FltLPOn",
    "FltLPCut",
    "FltLPRes",

    "FltHPOn",
    "FltHPCut",
    "FltHPRes",

    "FltBPOn",
    "FltBPCut",
    "FltBPQ",

    "FltNTOn",
    "FltNTCut",
    "FltNTQ",

    "Env1Att",
    "Env1Dec",
    "Env1Sus",
    "Env1Rel",
    "Env1Slp",

    "Env2Att",
    "Env2Dec",
    "Env2Sus",
    "Env2Rel",
    "Env2Slp",

    "Lfo1Rate",
    "Lfo1Dep",
    "Lfo1Shp",
    "Lfo1Sync",

    "Lfo2Rate",
    "Lfo2Dep",
    "Lfo2Shp",
    "Lfo2Sync",

    "Mm1Source",
    "Mm1Mod",
    "Mm1Target",
    "Mm2Source",
    "Mm2Mod",
    "Mm2Target",
    "Mm3Source",
    "Mm3Mod",
    "Mm3Target",
    "Mm4Source",
    "Mm4Mod",
    "Mm4Target",
    "Mm5Source",
    "Mm5Mod",
    "Mm5Target",
    "Mm6Source",
    "Mm6Mod",
    "Mm6Target",
    "Mm7Source",
    "Mm7Mod",
    "Mm7Target",
    "Mm8Source",
    "Mm8Mod",
    "Mm8Target",
    "Mm9Source",
    "Mm9Mod",
    "Mm9Target",
    "Mm10Sourc",
    "Mm10Mod",
    "Mm10Targe",

    "Effect1",
    "Effect2",
    "Effect3",
    "Effect4",
    "Effect5",
    "Effect6",
    "Effect7",
    "Effect8",
    "Effect9",
    "Effect10",

    "DistAmnt",

    "ChrsRate",
    "ChrsDep",

    "DelayL",
    "DelayR",
    "DelayDc",

    "RevRoom",
    "RevDamp",
    "RevWet",
    "RevWidth",

    "FlangLfo",
    "FlangFreq",
    "FlangAmp",
    "FlangWet",

    "GainAmount",

    "ChrsGain",

    "FrmtType",
    "FrmtWet",

    "EqLow",
    "EqMid",
    "EqHigh",
    
};

#endif

enum tfParam
{
    TF_OSC_POINTCOUNT = 0,
    TF_OSC_POINT1_OFFSET,
    TF_OSC_POINT1_VALUE,
    TF_OSC_POINT1_INTERPOLATION,
    TF_OSC_POINT2_OFFSET,
    TF_OSC_POINT2_VALUE,
    TF_OSC_POINT2_INTERPOLATION,
    TF_OSC_POINT3_OFFSET,
    TF_OSC_POINT3_VALUE,
    TF_OSC_POINT3_INTERPOLATION,
    TF_OSC_POINT4_OFFSET,
    TF_OSC_POINT4_VALUE,
    TF_OSC_POINT4_INTERPOLATION,
    TF_OSC_FINAL_INTERPOLATION,
    TF_OSC_VOLUME,
    TF_OSC_FREQ,
    TF_OSC_PAN,
    TF_OSC_DETUNE,
    TF_OSC_POLYPHONY,
    TF_OSC_UNISONO,
    TF_OSC_SPREAD,
    TF_OSC_SUBOSC,
    TF_OSC_GLIDE,
    TF_OSC_DRIVE,
    TF_OSC_OCTAVE,
    TF_OSC_OFFSET_not_used,
    TF_OSC_SLOP,

    TF_ADD_VOLUME,
    TF_ADD_PROFILE,
    TF_ADD_OCTAVE,
    TF_ADD_BANDWIDTH,
    TF_ADD_DAMP,
    TF_ADD_HARMONICS,
    TF_ADD_SCALE,
    TF_ADD_DRIVE,

    TF_NOISE_AMOUNT,
    TF_NOISE_FREQ,
    TF_NOISE_BW,

    TF_LP_FILTER_ON,
    TF_LP_FILTER_CUTOFF,
    TF_LP_FILTER_RESONANCE,

    TF_HP_FILTER_ON,
    TF_HP_FILTER_CUTOFF,
    TF_HP_FILTER_RESONANCE,

    TF_BP_FILTER_ON,
    TF_BP_FILTER_CUTOFF,
    TF_BP_FILTER_Q,

    TF_NT_FILTER_ON,
    TF_NT_FILTER_CUTOFF,
    TF_NT_FILTER_Q,

    TF_ADSR1_ATTACK,
    TF_ADSR1_DECAY,
    TF_ADSR1_SUSTAIN,
    TF_ADSR1_RELEASE,
    TF_ADSR1_SLOPE,

    TF_ADSR2_ATTACK,
    TF_ADSR2_DECAY,
    TF_ADSR2_SUSTAIN,
    TF_ADSR2_RELEASE,
    TF_ADSR2_SLOPE,

    TF_LFO1_RATE,
    TF_LFO1_DEPTH,
    TF_LFO1_SHAPE,
    TF_LFO1_SYNC,

    TF_LFO2_RATE,
    TF_LFO2_DEPTH,
    TF_LFO2_SHAPE,
    TF_LFO2_SYNC,

    TF_MM1_SOURCE,
    TF_MM1_MOD,
    TF_MM1_TARGET,
    TF_MM2_SOURCE,
    TF_MM2_MOD,
    TF_MM2_TARGET,
    TF_MM3_SOURCE,
    TF_MM3_MOD,
    TF_MM3_TARGET,
    TF_MM4_SOURCE,
    TF_MM4_MOD,
    TF_MM4_TARGET,
    TF_MM5_SOURCE,
    TF_MM5_MOD,
    TF_MM5_TARGET,
    TF_MM6_SOURCE,
    TF_MM6_MOD,
    TF_MM6_TARGET,
    TF_MM7_SOURCE,
    TF_MM7_MOD,
    TF_MM7_TARGET,
    TF_MM8_SOURCE,
    TF_MM8_MOD,
    TF_MM8_TARGET,
    TF_MM9_SOURCE,
    TF_MM9_MOD,
    TF_MM9_TARGET,
    TF_MM10_SOURCE,
    TF_MM10_MOD,
    TF_MM10_TARGET,

    TF_EFFECT_1,
    TF_EFFECT_2,
    TF_EFFECT_3,
    TF_EFFECT_4,
    TF_EFFECT_5,
    TF_EFFECT_6,
    TF_EFFECT_7,
    TF_EFFECT_8,
    TF_EFFECT_9,
    TF_EFFECT_10,

    TF_DISTORT_AMOUNT,

    TF_CHORUS_RATE,
    TF_CHORUS_DEPTH,

    TF_DELAY_LEFT,
    TF_DELAY_RIGHT,
    TF_DELAY_DECAY,

    TF_REVERB_ROOMSIZE,
    TF_REVERB_DAMP,
    TF_REVERB_WET,
    TF_REVERB_WIDTH,

    TF_FLANGER_LFO,
    TF_FLANGER_FREQUENCY,
    TF_FLANGER_AMPLITUDE,
    TF_FLANGER_WET,

    TF_GAIN_AMOUNT,

    TF_CHORUS_GAIN,

    TF_FORMANT_MODE,
    TF_FORMANT_WET,

    TF_EQ_LOW,
    TF_EQ_MID,
    TF_EQ_HIGH,

    TF_PARAM_COUNT
};

#ifndef ePLAYER

static const eF32 TF_DEFAULTPROG[] = 
{
    // Oscillator
    0.33f,

    //p1
    0.25f,
    1.0f,
    1.0f,
    //p2
    0.66f,
    0.0f,
    1.0f,
    //p3
    0.6f,
    0.0f,
    1.0f,
    //p4
    0.8f,
    0.2f,
    1.0f,
    // last int
    1.0f,

    0.5f,
    0.0f,
    0.5f,
    0.1f,
    1.0f,
    0.0f,
    0.4f,
    0.0f,
    0.0f,   //"OscGlide",
    0.0f,   //"OscDrive",
    0.5f,   //"OscOctave",
    0.5f,   //"OscOffset",
    0.5f,   //"OscSlop",

    0.0f,   //"AddVolume",
    0.0f,   //"AddProfile",
    0.5f,   //"AddOctave",
    0.5f,   //"AddBandw",
    0.5f,   //"AddDamp",
    0.1f,   //"AddHarmncs",
    0.1f,   //"AddScale",
    0.0f,   //"AddDrive",

    // Noise
    0.0f,
    0.5f,
    1.0f,

    // LP Filter
    0.0f,
    0.5f,
    0.5f,

    // HP Filter
    0.0f,
    0.5f,
    0.5f,

    // BP Filter
    0.0f,
    0.5f,
    0.5f,

    // NT Filter
    0.0f,
    0.5f,
    0.5f,

    // ADSR1 (Osc/Noise)
    0.0f,
    0.5f,
    0.5f,
    0.2f,
    0.5f,

    // ADSR2 (Filter)
    0.0f,
    0.5f,
    0.5f,
    0.2f,
    0.5f,

    // LFO1 (Osc/Noise)
    0.5f,
    0.5f,
    0.0f,
    0.0f, 

    // LFO2 (Filter)
    0.5f,
    0.5f,
    0.0f,
    0.0f,

    // Modulation matrix
    0.0f,   //"Mm1Source",
    0.55f,  //"Mm1Mod,
    0.0f,   //"Mm1Target",
    0.0f,   //"Mm2Source",
    0.55f,  //"Mm2Mod,
    0.0f,   //"Mm2Target",
    0.0f,   //"Mm3Source",
    0.55f,  //"Mm3Mod,
    0.0f,   //"Mm3Target",
    0.0f,   //"Mm4Source",
    0.55f,  //"Mm4Mod,
    0.0f,   //"Mm4Target",
    0.0f,   //"Mm5Source",
    0.55f,  //"Mm5Mod,
    0.0f,   //"Mm5Target",
    0.0f,   //"Mm6Source",
    0.55f,  //"Mm6Mod,
    0.0f,   //"Mm6Target",
    0.0f,   //"Mm7Source",
    0.55f,  //"Mm7Mod,
    0.0f,   //"Mm7Target",
    0.0f,   //"Mm8Source",
    0.55f,  //"Mm8Mod,
    0.0f,   //"Mm8Target",
    0.0f,   //"Mm9Source",
    0.55f,  //"Mm9Mod,
    0.0f,   //"Mm9Target",
    0.0f,   //"Mm10Source",
    0.55f,  //"Mm10Mod,
    0.0f,   //"Mm10Target",

    // Effects section
    0.0f,   //"Effect1",
    0.0f,   //"Effect2",
    0.0f,   //"Effect3",
    0.0f,   //"Effect4",
    0.0f,   //"Effect5",
    0.0f,   //"Effect6",
    0.0f,   //"Effect7",
    0.0f,   //"Effect8",
    0.0f,   //"Effect9",
    0.0f,   //"Effect10",

    // Distortion
    0.2f,

    // Chorus
    0.2f,
    0.2f,

    // Delay
    0.2f,
    0.2f,
    0.2f,

    //Reverb
    0.2f,
    0.2f,
    0.5f,
    0.5f,

    //Flanger
    0.2f,
    0.2f,
    0.2f,
    0.5f,

     //Gain
    0.5f,

    //Chorus
    1.0f,

    //Formant
    0.0f,
    1.0f,

    //EQ
    0.5f,
    0.5f,
    0.5f,
   
};

#endif

#include "tf_functions.hpp"
#include "tf_adsr.hpp"
#include "tf_lfo.hpp"
#include "tf_modmatrix.hpp"
#include "tf_filter.hpp"
#include "tf_oscillator.hpp"
#include "tf_addsynth.hpp"
#include "tf_noise.hpp"
#include "tf_ieffect.hpp"
#include "tf_instrument.hpp"
#include "tf_song.hpp"
#include "tf_isoundout.hpp"
#include "tf_player.hpp"

#include "directx/tf_soundoutdx8.hpp"

#ifdef _WIN32
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#endif

#ifdef eUSE_ARM_NEON
#include <arm_neon.h>
#endif

static const eU8 TF_PARAM_VALUERANGE[] =
{
    TF_OSCILLATOR_POINTS-3,  // TF_OSC_POINTCOUNT,
    0,  // TF_OSC_POINT1_OFFSET,
    0,  // TF_OSC_POINT1_VALUE,
    TF_INTERPOLATIONS-1,  // TF_OSC_POINT1_INTERPOLATION,
    0,  // TF_OSC_POINT2_OFFSET,
    0,  // TF_OSC_POINT2_VALUE,
    TF_INTERPOLATIONS-1,  // TF_OSC_POINT2_INTERPOLATION,
    0,  // TF_OSC_POINT3_OFFSET,
    0,  // TF_OSC_POINT3_VALUE,
    TF_INTERPOLATIONS-1,  // TF_OSC_POINT3_INTERPOLATION,
    0,  // TF_OSC_POINT4_OFFSET,
    0,  // TF_OSC_POINT4_VALUE,
    TF_INTERPOLATIONS-1,  // TF_OSC_POINT4_INTERPOLATION,
    TF_INTERPOLATIONS-1,  // TF_OSC_FINAL_INTERPOLATION,
    0,  // TF_OSC_VOLUME,
    0,  // TF_OSC_FREQ,
    0,  // TF_OSC_PAN,
    0,  // TF_OSC_DETUNE,
    TF_MAXVOICES-1,  // TF_OSC_POLYPHONY,
    TF_MAXUNISONO-1,  // TF_OSC_UNISONO,
    0,  // TF_OSC_SPREAD,
    TF_MAXSUBOSC,  // TF_OSC_SUBOSC,
    0,  // TF_OSC_GLIDE,
    0,  // TF_OSC_DRIVE,
    TF_MAXOCTAVES-1,  // TF_OSC_OCTAVE,
    0,  // TF_OSC_OFFSET_not_used,
    0,  // TF_OSC_SLOP,

    0,  // TF_ADD_VOLUME,
    TF_ADDSYNTHPROFILES-1,  // TF_ADD_PROFILE,
    TF_MAXOCTAVES-1,  // TF_ADD_OCTAVE,
    0,  // TF_ADD_BANDWIDTH,
    0,  // TF_ADD_DAMP,
    0,  // TF_ADD_HARMONICS,
    0,  // TF_ADD_SCALE,
    0,  // TF_ADD_DRIVE,

    0,  // TF_NOISE_AMOUNT,
    0,  // TF_NOISE_FREQ,
    0,  // TF_NOISE_BW,

    1,  // TF_LP_FILTER_ON,
    0,  // TF_LP_FILTER_CUTOFF,
    0,  // TF_LP_FILTER_RESONANCE,

    1,  // TF_HP_FILTER_ON,
    0,  // TF_HP_FILTER_CUTOFF,
    0,  // TF_HP_FILTER_RESONANCE,

    1,  // TF_BP_FILTER_ON,
    0,  // TF_BP_FILTER_CUTOFF,
    0,  // TF_BP_FILTER_Q,

    1,  // TF_NT_FILTER_ON,
    0,  // TF_NT_FILTER_CUTOFF,
    0,  // TF_NT_FILTER_Q,

    0,  // TF_ADSR1_ATTACK,
    0,  // TF_ADSR1_DECAY,
    0,  // TF_ADSR1_SUSTAIN,
    0,  // TF_ADSR1_RELEASE,
    0,  // TF_ADSR1_SLOPE,

    0,  // TF_ADSR2_ATTACK,
    0,  // TF_ADSR2_DECAY,
    0,  // TF_ADSR2_SUSTAIN,
    0,  // TF_ADSR2_RELEASE,
    0,  // TF_ADSR2_SLOPE,

    0,  // TF_LFO1_RATE,
    0,  // TF_LFO1_DEPTH,
    TF_LFOSHAPECOUNT-1,  // TF_LFO1_SHAPE,
    1,  // TF_LFO1_SYNC,

    0,  // TF_LFO2_RATE,
    0,  // TF_LFO2_DEPTH,
    TF_LFOSHAPECOUNT-1,  // TF_LFO2_SHAPE,
    1,  // TF_LFO2_SYNC,

    tfModMatrix::INPUT_COUNT-1,  // TF_MM1_SOURCE,
    0,  // TF_MM1_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM1_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM2_SOURCE,
    0,  // TF_MM2_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM2_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM3_SOURCE,
    0,  // TF_MM3_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM3_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM4_SOURCE,
    0,  // TF_MM4_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM4_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM5_SOURCE,
    0,  // TF_MM5_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM5_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM6_SOURCE,
    0,  // TF_MM6_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM6_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM7_SOURCE,
    0,  // TF_MM7_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM7_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM8_SOURCE,
    0,  // TF_MM8_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM8_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM9_SOURCE,
    0,  // TF_MM9_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM9_TARGET,
    tfModMatrix::INPUT_COUNT-1,  // TF_MM10_SOURCE,
    0,  // TF_MM10_MOD,
    tfModMatrix::OUTPUT_COUNT-1,  // TF_MM10_TARGET,

    tfIEffect::FX_COUNT-1,  // TF_EFFECT_1,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_2,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_3,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_4,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_5,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_6,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_7,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_8,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_9,
    tfIEffect::FX_COUNT-1,  // TF_EFFECT_10,

    0,  // TF_DISTORT_AMOUNT,

    0,  // TF_CHORUS_RATE,
    0,  // TF_CHORUS_DEPTH,

    0,  // TF_DELAY_LEFT,
    0,  // TF_DELAY_RIGHT,
    0,  // TF_DELAY_DECAY,

    0,  // TF_REVERB_ROOMSIZE,
    0,  // TF_REVERB_DAMP,
    0,  // TF_REVERB_WET,
    0,  // TF_REVERB_WIDTH,

    0,  // TF_FLANGER_LFO,
    0,  // TF_FLANGER_FREQUENCY,
    0,  // TF_FLANGER_AMPLITUDE,
    0,  // TF_FLANGER_WET,

    0,  // TF_GAIN_AMOUNT,

    0,  // TF_CHORUS_GAIN,

    4,  // TF_FORMANT_MODE,
    0,  // TF_FORMANT_WET,

    0,  // TF_EQ_LOW,
    0,  // TF_EQ_MID,
    0,  // TF_EQ_HIGH,
};

#endif // TUNEFISH3_HPP