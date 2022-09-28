// Created by Bradly Landucci

#include "userdelfx.h"
#include "delayline.hpp"
#include "osc_api.h"
#include <cmath>

// Calculated for the minimum bpm of Minilogue-XD //
#define BUF_SIZE 480000

// Initializing Variables // 
static dsp::DelayLine s_delay;
static __sdram float s_delay_ram[BUF_SIZE];

float s_len;
float s_mix;
float gain;
float wetXNL, wetXNR;
float bpm_valf;

float __fast_inline waveshape(float in) 
{
    return 1.5f * in - 0.5f * in *in * in;
}

void calculate_len(float bpm_valf)
{
    if (bpm_valf < 0.25f) 
    {
      // Full note
      s_len = (60 * k_samplerate / _fx_get_bpmf()) * 4.f;
    } 
    else if (bpm_valf < 0.5f) 
    {
      // 1/2 note
      s_len = ((60 * k_samplerate) / _fx_get_bpmf()) * 2.f;
    } 
    else if (bpm_valf < 0.75f) 
    {
      // 1/4 note
      s_len = ((60 * k_samplerate) / _fx_get_bpmf());
    } 
    else 
    {
      // 1/8 note
      s_len = ((60 * k_samplerate) / _fx_get_bpmf()) / 2.f;
    }
}

/*
  Give the s_delay_ram to s_delay object, instantiate variables
*/
void DELFX_INIT(uint32_t platform, uint32_t api)
{
  s_delay.setMemory(s_delay_ram, BUF_SIZE);  
  s_delay.clear();
  s_mix = .5f;
  gain = .5f;
  bpm_valf = .5f;
  calculate_len(bpm_valf);
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  float * __restrict x = xn;
  const float * x_e = x + 2*frames;
  const float wet = s_mix;
  const float feedback = gain;

  calculate_len(bpm_valf);

  for (; x != x_e ; x+=2) 
  {
    const float delSampleL = s_delay.readFrac(s_len);
    const float delSampleR = s_delay.readFrac(s_len);

    wetXNL = wet * delSampleL;
    wetXNR = wet * delSampleR;

    s_delay.write(feedback * (*x + delSampleL));
    s_delay.write(feedback * (*(x+1) + delSampleR));

    *x = (*x) + fastertanhf(waveshape(wetXNL));
    *(x+1) = *(x+1) + fastertanhf(waveshape(wetXNR));
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case k_user_delfx_param_time:
    gain = valf;
    break;
  case k_user_delfx_param_depth:
    bpm_valf = valf;
    break;
  case k_user_delfx_param_shift_depth:
    s_mix = valf;
    break;
  default:
    break;
  }
}
