#include "userdelfx.h"
#include "simplelfo.hpp"
#include "delayline.hpp"
#include "osc_api.h"
#include "fx_api.h"
#include <math.h>

// Calculated for the min of 56 bpm //
#define BUF_SIZE 192000

// Initilizaing Variables // 
static dsp::DelayLine s_delay;
static __sdram float s_delay_ram[BUF_SIZE];

static dsp::SimpleLFO s_lfo;
float lfo_speed = 0.5f;

float s_mix;
float gain;
float wetXNL, wetXNR;
float bpm_valf;

float __fast_inline waveshape(float in) 
{
    return 1.5f * in - 0.5f * in *in * in;
}

/*
  Give the s_delay_ram to s_delay object, instantiate variables
*/
void DELFX_INIT(uint32_t platform, uint32_t api)
{
  s_delay.setMemory(s_delay_ram, BUF_SIZE);  
  s_delay.clear();
  s_lfo.reset();
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
    const float lfo_cycle = s_lfo.sine_bi();

    const float delSampleL = gain * (s_delay.readFrac(288 + (240 * lfo_cycle)));
    const float delSampleR = gain * (s_delay.readFrac(288 + (240 * lfo_cycle)));

    s_lfo.cycle();

    wetXNL = wet * delSampleL;
    wetXNR = wet * delSampleR;

    *x += fastertanhf(waveshape(wetXNL));
    *(x+1) += fastertanhf(waveshape(wetXNR));

    s_delay.write(*x);
    s_delay.write(*(x + 1));
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) 
  {
  case k_user_delfx_param_time:
    gain = valf;
    break;
  case k_user_delfx_param_depth:
    lfo_speed = valf * 2.f;
    s_lfo.setF0(lfo_speed, k_samplerate_recipf);
    break;
  case k_user_delfx_param_shift_depth:
    s_mix = valf;
    break;
  default:
    break;
  }
}