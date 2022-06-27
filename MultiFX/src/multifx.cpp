// Created by Bradly Landucci

#include <usermodfx.h>
#include "fx_api.h"
#include <math.h>
#include "simplelfo.hpp"
#include "osc_api.h"
#include "delayline.hpp"

// Calculated for the minimum bpm of Minilogue-XD //
#define BUF_SIZE 48000

// Initializing Variables // 
static dsp::DelayLine s_delay;
static __sdram float s_delay_ram[BUF_SIZE];

uint32_t s_len;

// Initializing Params //
static float fxamt = 0.5f;
static int type = 01.f;
static float bit_rate = 16;
float thresh = 0.f;
float trem_rate = 0.5f;
int releaseCount = 0;
int bpm = 90;
float lfo_speed = 0.5f;

// Instantiating LFO
static dsp::SimpleLFO s_lfo;

// waveshaping algorithm //
// x' = 3/2x - 1/2(x^3)
// https://www.musicdsp.org/en/latest/Effects/114-waveshaper-simple-description.html
float __fast_inline waveshape(float in) 
{
    return 1.5f * in - 0.5f * in *in * in;
}

// bit reduction algorithm //
// x' = -1,                                                           {x = -1}
// x' = roundup(bits * input) * ( 1 / bits) <-- Reciprorocal          { Else }
float __fast_inline bitreduc(float in, float bits)
{
  bits = fastpowf(2, bits - 1.0f);
  return in == -1 ? -1 : (ceil(bits * in) * (1.0f / bits));
}

// gate helper algorithm //
float __fast_inline gateHelper(float in)
{
  if (releaseCount != 0)
  {
    in *= (1 / releaseCount);
    releaseCount--;
  }
  else
  {
    releaseCount = 60;
  }
  return in;
}

// gate algorithm //
float __fast_inline gate(float in)
{
  float tmp = ampdbf(in);
  if (tmp < thresh) return gateHelper(in);
  else return in;
}

// tremolo algorithm // 
// input *= current cycle of lfo
// then steps the phase one cycle forward 
float __fast_inline trem(float in)
{
  // Waveshaping before tremolo, turn to 0% waveshaping for just tremolo
  in *= ((fxamt * 10.0f) + 1.f);
  in = waveshape(in);
  in = bitreduc(in, bit_rate) + 0.0f; 
  in *= (s_lfo.sine_bi() * 0.5f); // Adjust this to change lfo DEPTH
  s_lfo.cycle();
  return in;
}

// Initializing Platform //
void MODFX_INIT(uint32_t platform, uint32_t api)
{
  s_delay.setMemory(s_delay_ram, BUF_SIZE);  
  s_delay.clear();
  s_lfo.reset();
  bpm = _fx_get_bpm();
  lfo_speed = 60 / (bpm * trem_rate);
  s_lfo.setF0(lfo_speed, k_samplerate_recipf * bpm);
}

// Main DSP Process Block //
// Sub values to be ignored, only used for prologue
void MODFX_PROCESS(const float *xn, float *yn,
                   const float *sub_xn, float *sub_yn,
                   uint32_t frames)
{
  float base_main;

  // Effect processing loop //
  // For double frames (AKA samples cause each frame = sample pair)
  for (int i=0;i<frames*2;i++)
  {
    switch(type)
    {
      // Bit Crushing Algorithm //
      case 0:
        *yn++ = bitreduc(*xn++, bit_rate) + 0.0f; // +0.0f for -0.0f values
        break;
      // Gate Algorithm //
      case 1:
        *yn++ = gate(*xn++);        
        break;
      // Tremolo Algorithm // 
      case 2:
        *yn++ = trem(*xn++);
        break;
      // Waveshaping algorithm // 
      case 3:
        base_main = *xn++ * ((fxamt * 10.0f) + 1.f);
        *yn++ = waveshape(base_main);
        break;
    }

  }
}

// Param Controls //
void MODFX_PARAM(uint8_t index, int32_t value)
{
  //Convert fixed-point q31 format to float
  const float valf = q31_to_f32(value);
  switch (index)
  {
    case 0:
      fxamt = valf;
      if (type == 0)
      {
        bit_rate = fxamt * 14.0f + 4.0f;
      }

      // Gate threshold
      //thresh = ampdbf(valf);

      // Grab bpm to calculate trem rate
      bpm = _fx_get_bpm();
      if (valf < 0.25) 
      {
        trem_rate = 0.125f;
      } 
      else if (valf < 0.5) 
      {
        trem_rate = 0.25f;
      } 
      else if (valf < 0.75) 
      {
        trem_rate = 0.5f;
      } 
      else 
      {
        trem_rate = 1.f;
      }
      // Calculate the amount of that rate of notes per minute based off bpm - lfo_speed is in Hz
      lfo_speed = 60 / (bpm * trem_rate);
      s_lfo.setF0(lfo_speed, k_samplerate_recipf);
      break;

    case 1:
      if (valf < 0.25) 
      {
        type = 0; // Bitcrusher
      } 
      else if (valf < 0.5) 
      {
        type = 1; // Gate
      } 
      else if (valf < 0.75) 
      {
        type = 2; // Tremolo
      } 
      else 
      {
        type = 3; // Waveshaper
      }
      break;
    default:
      break;
  }
}