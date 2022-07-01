#include "userdelfx.h"
#include "simplelfo.hpp"
#include "delayline.hpp"
#include "osc_api.h"
#include "fx_api.h"
#include <math.h>

// Calculated for the minimum bpm of Minilogue-XD //
#define BUF_SIZE 210000

// Initilizaing Variables // 
static dsp::DelayLine s_delay;
static __sdram float s_delay_ram[BUF_SIZE];

static dsp::SimpleLFO s_lfo;
float lfo_speed = 0.5f;

uint32_t s_len;
static float s_mix;
static float gain;
float wetXNL;
float wetXNR;
int bpm;

/*
  Give the s_delay_ram to s_delay object, instantiate variables
*/
void DELFX_INIT(uint32_t platform, uint32_t api)
{
  s_delay.setMemory(s_delay_ram, BUF_SIZE);  
  s_delay.clear();
  s_lfo.reset();
  bpm = _fx_get_bpm();
  s_len = (60 / (bpm / 10) * BUF_SIZE) * 32;
  s_mix = .5f;
  s_lfo.setF0(lfo_speed, k_samplerate_recipf);
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  float * __restrict x = xn;
  const float * x_e = x + 2*frames;

  const float wet = s_mix;
  const float dry = 1.f - s_mix;

  for (; x != x_e ; x+=2) 
  {
    const float delSampleL = gain * (s_delay.read(240) * s_lfo.sine_bi());
    s_lfo.cycle();
    const float delSampleR = gain * (s_delay.read(239) * s_lfo.sine_bi());
    s_lfo.cycle();
    wetXNL = wet * delSampleL;
    wetXNR = wet * delSampleR;
    // As of now I'm unsure why this is working properly, but my previous version wasn't..
    *x += 0.f;
    *(x + 1) += wetXNR;
    s_delay.write(*x);
    s_delay.write(*(x + 1));
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) 
  {
  case 0:
    // Gain == Feedback //
    gain = valf;
    break;
  case 1:
    lfo_speed = valf * 2.f;
    s_lfo.setF0(lfo_speed, k_samplerate_recipf);
    break;
  case 3:
    // Mix Val
    s_mix = valf;
    break;
  default:
    break;
  }
}