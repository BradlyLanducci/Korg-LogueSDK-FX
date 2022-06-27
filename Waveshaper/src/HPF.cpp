#include "usermodfx.h"
#include "fx_api.h"

float prevSample = 0;

void MODFX_INIT(uint32_t platform, uint32_t api)
{
}

void MODFX_PROCESS(const float *xn, float *yn,
                     const float *sub_xn, float *sub_yn,
                     uint32_t frames)
{
  for (int i = 0; i < frames * 2; i++)
  {
    const float curSample = *xn++;
    *yn++ = curSample + (-1 * prevSample);
    prevSample = curSample;
  }
}

void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);

}