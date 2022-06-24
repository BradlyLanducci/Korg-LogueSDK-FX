#include "userdelfx.h"
#include "simplelfo.hpp"
#include "delayline.hpp"
#include "osc_api.h"
#include "fx_api.h"
#include <math.h>


// Calculated for the minimum bpm of Minilogue-XD //
#define BUF_SIZE 210000

// Initilizaing Variables // 
static dsp::DualDelayLine s_delay;
static __sdram f32pair_t s_delay_ram[BUF_SIZE];

static dsp::SimpleLFO s_lfo;
float lfo_speed = 5.f;

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

float __fast_inline waveshape(float in) 
{
    return 1.5f * in - 0.5f * in *in * in;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  float * __restrict x = xn;
  const float * x_e = x + 2*frames;

  const float wet = s_mix;
  f32pair_t valf;

  for (; x != x_e ; x+=2) 
  {
    const float delSample1 = gain * s_delay.read0(1680) * (s_lfo.sine_bi() * 2);
    const float delSample2 = gain * s_delay.read1(2880) * s_lfo.sine_bi();
    wetXNL = wet * delSample1;
    wetXNR = wet * delSample2;
    *x = (*x) + wetXNL;
    *(x+1) = (*(x+1)) + wetXNR;
    valf.a = (*x);
    valf.b = (*x + 1);    
    s_delay.write(valf);
    s_lfo.cycle();
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
    bpm = _fx_get_bpm() / 10;
    // Calculate note intervals
    if (valf < 0.25) 
    {
      // Full note
      s_len = ((60 * 48000) / bpm) * 4;
    } 
    else if (valf < 0.5) 
    {
      // 1/2 note
      s_len = ((60 * 48000) / bpm) * 2;
    } 
    else if (valf < 0.75) 
    {
      // 1/4 note
      s_len = ((60 * 48000) / bpm);
    } 
    else 
    {
      // 1/8 note
      s_len = ((60 * 48000) / bpm) / 2;
    }
    break;
  case 3:
    // Mix Val
    s_mix = valf;
    break;
  default:
    break;
  }
}