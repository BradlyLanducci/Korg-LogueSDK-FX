#include "userdelfx.h"
#include "delayline.hpp"

#define BUF_SIZE 96000

// Initilizaing Variables // 
static dsp::DelayLine s_delay;
static __sdram float s_delay_ram[BUF_SIZE];

uint32_t s_len;
static float s_mix;
static float gain;
float wetXN;
int bpm;

/*
  Give the s_delay_ram to s_delay object, instantiate variables
*/
void DELFX_INIT(uint32_t platform, uint32_t api)
{
  s_delay.setMemory(s_delay_ram, BUF_SIZE);  
  s_delay.clear();
  bpm = _fx_get_bpm();
  s_len = (60 / (bpm / 10) * BUF_SIZE) * 32;
  s_mix = .5f;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  const float dry = 1.f - s_mix;
  const float wet = s_mix;
  
  // Delay Algorithm //
  for (int i=0;i<frames*2;i++)
  {
    const float delSample = gain * s_delay.read(s_len);
    wetXN = wet * delSample;
    xn[i] = xn[i] + wetXN;
    s_delay.write(xn[i]);
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case 0:
    // Gain == Feedback //
    if (valf == 0.f) gain = 0.1f;
    else gain = valf;
    break;
  case 1:

    bpm = _fx_get_bpm() / 10;
    // Calculate note intervals
    if (valf < 0.25) 
    {
      // 1/2 note
      s_len = ((60 * 48000) / bpm) * 2;
    } 
    else if (valf < 0.5) 
    {
      // 1/4 note
      s_len = ((60 * 48000) / bpm);
    } 
    else if (valf < 0.75) 
    {
      // 1/8 note
      s_len = ((60 * 48000) / bpm) / 2;
    } 
    else 
    {
      // 1/16 note
      s_len = ((60 * 48000) / bpm) / 4;
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