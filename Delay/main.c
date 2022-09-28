#include "userdelfx.h"

void DELFX_INIT(uint32_t platform, uint32_t api)
{
    
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{

}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  switch (index) {
  case k_user_delfx_param_time:
    break;
  case k_user_delfx_param_depth:
    break;
  default:
    break;
  }
}
