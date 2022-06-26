#include "usermodfx.h"
#include "fx_api.h"

float curSampleL = 0.f;
float curSampleR = 0.f;
float prevSampleL = 0.f;
float prevSampleR = 0.f;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
}

void MODFX_PROCESS(const float *xn, float *yn,
                     const float *sub_xn, float *sub_yn,
                     uint32_t frames)
{
 for (int i = 0; i < frames; i++)
  {
    curSampleL = *xn++;
    curSampleR = *xn++;
    *yn++ = curSampleL + (-1.f * prevSampleL);
    *yn++ = curSampleR + (-1.f * prevSampleR);
    prevSampleL = curSampleL;
    prevSampleR = curSampleR;
  }
}

void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);

}