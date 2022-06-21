#include "userdelfx.h"
#include "delayline.hpp"

#define BUF_SIZE 48000

static dsp::DelayLine s_delay;
static __sdram float s_delay_ram[BUF_SIZE];

uint32_t s_len;
static float s_mix;
static float gain;
float wetXN;
float bpm;

void DELFX_INIT(uint32_t platform, uint32_t api)
{
  s_delay.setMemory(s_delay_ram, BUF_SIZE);  
  s_delay.clear();
  bpm = _fx_get_bpmf();
  s_len = (60 / bpm * BUF_SIZE) * 32;
  s_mix = .5f;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  const float dry = 1.f - s_mix;
  const float wet = s_mix;
  

  for (int i=0;i<frames*2;i++)
  {
    const float delSample = gain * s_delay.read(s_len);
    wetXN = wet * delSample;
    xn[i] = xn[i] + wetXN;
    s_delay.write(xn[i]+ wetXN);
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case 0:
    if (valf == 0.f) gain = 0.1f;
    else gain = valf;
    break;
  case 1:

    bpm = _fx_get_bpm();
    if (valf < 0.25) 
    {
      s_len = (60 / bpm * BUF_SIZE) * 32;
    } 
    else if (valf < 0.5) 
    {
      s_len = (60 / bpm * BUF_SIZE) * 16;
    } 
    else if (valf < 0.75) 
    {
      s_len = (60 / bpm * BUF_SIZE) * 8;
    } 
    else 
    {
      s_len = (60 / bpm * BUF_SIZE) * 4;
    }
    break;

  case 3:
    s_mix = valf;
    break;
  default:
    break;
  }
}