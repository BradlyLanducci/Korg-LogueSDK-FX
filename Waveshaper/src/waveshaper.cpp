// Created by Bradly Landucci

#include <usermodfx.h>
#include "fx_api.h"

// Initializing Params //
float distAmt;

// Simple waveshaping algorithm //
// Audio Effects: Theory, Implementation and Application Pg. 184
float __fast_inline waveshape(float in) 
{
  float threshold1 = 1.0f/3.0f;
  float threshold2 = 2.0f/3.0f;

  if(in > threshold2)
    return 1.0f;
  else if(in > threshold1)
    return (3.0f - (2.0f - 3.0f*in) * (2.0f - 3.0f*in))/3.0f;
  else if(in < -threshold2)
    return -1.0f;
  else if(in < -threshold1)
    return -(3.0f - (2.0f + 3.0f*in) * (2.0f + 3.0f*in))/3.0f;
  else
    return 2.0f * in;
}

// Initializing Platform //
void MODFX_INIT(uint32_t platform, uint32_t api)
{
  float distAmt = 0.5f;
}

// Main DSP Process Block //
// Sub values to be ignored, only used for prologue
void MODFX_PROCESS(const float *xn, float *yn,
                   const float *sub_xn, float *sub_yn,
                   uint32_t frames)
{
  // Effect processing loop //
  // For double frames (AKA samples cause each frame = sample pair)
  for (int i=0;i<frames*2;i++)
  {
    float xn_cur = *xn++;
    const float dry = 1.f - distAmt;
    const float drive = 100.f;

    *yn++ = (dry * xn_cur) + (distAmt * waveshape(drive * xn_cur));
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
      distAmt = valf;
      break;
    case 1:
      break;
    default:
      break;
  }
}