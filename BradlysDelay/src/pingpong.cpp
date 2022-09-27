#include "userdelfx.h"
#include "delayline.hpp"

// Calculated for the minimum bpm of Minilogue-XD //
#define BUF_SIZE 210000

static float *delay;

static float s_mix;
static float gain;
int mwriteidx = 0;
uint32_t s_len;
float wetXNL, wetXNR;
int bpm;

/*
  Give the s_delay_ram to s_delay object, instantiate variables
*/

void setMemory()
{
  for (int i = 0; i < BUF_SIZE; i++)
    delay[i] = 0.f;
}

void DELFX_INIT(uint32_t platform, uint32_t api)
{
  setMemory();
  s_mix = .5f;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  float * __restrict x = xn;
  const float * x_e = x + 2*frames;

  const float wet = s_mix;
  for (; x != x_e ; x+=2) 
  {
    float curSampleL = *x;
    float curSampleR = *(x+1);
    const float delSample1 = gain * s_delay.read0(s_len);
    const float delSample2 = gain * s_delay.read1(s_len / 2);
    const float delSample1 = gain * delay[s_len];
    const float delSample2 = gain * delay[s_len / 2];

    wetXNL = wet * delSample1;
    wetXNR = wet * delSample2;
    *x = curSampleL + wetXNL;
    *(x+1) = curSampleR + wetXNR;
    delay[mwriteidx % BUF_SIZE] = curSampleL;
    mwriteidx++;
    delay[mwriteidx % BUF_SIZE] = curSampleR;
    mwriteidx++;
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