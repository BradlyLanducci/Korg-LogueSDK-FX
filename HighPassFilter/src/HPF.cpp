#include "usermodfx.h"
#include "fx_api.h"
#include "biquad.hpp"

float curSampleL, curSampleR;

static dsp::BiQuad filter_l, filter_r;
float resonance = 1.4041f;
float cutoff = 0.49f;
float z = 0.f;
float a = 0.05f;
float b = 1.f - a;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
  filter_l.flush();
  filter_l.mCoeffs.setSOHP(fx_tanpif(cutoff), resonance);
  filter_r.flush();
  filter_r.mCoeffs = filter_l.mCoeffs;
}

float absolute(float valf)
{
  if (valf < 0)
    return -valf;
  return valf;
}

void MODFX_PROCESS(const float *xn, float *yn,
                     const float *sub_xn, float *sub_yn,
                     uint32_t frames)
{
 for (int i = 0; i < frames; i++)
  {
    curSampleL = *xn++;
    curSampleR = *xn++;

    curSampleL = filter_l.process_so(curSampleL);
    curSampleR = filter_r.process_so(curSampleR);

    *yn++ = curSampleL;
    *yn++ = curSampleR;
  }
}

float __fast_inline smooth(float in)
{
  z = (in * b) + (z * a);
  return z;
}

void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  float invertedValf;
  float range = 0;
  switch (index) 
  {
  case 0:
    invertedValf = absolute(1-valf);
    range = clipminmaxf(0.001f, invertedValf, 1.f);
    cutoff = (1 / range) * 0.00046875;
    cutoff = smooth(cutoff);
    filter_l.mCoeffs.setSOHP(fx_tanpif(cutoff), resonance);
    filter_r.mCoeffs = filter_l.mCoeffs;
    break;
  case 1:
    break;
  case 3:
    // Mix Val
    break;
  default:
    break;
  }
}