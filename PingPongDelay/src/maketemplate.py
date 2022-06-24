def template():
    template_name = input("Enter template name: ")
    file = open(template_name + ".cpp", "w+")
    file.write("""// Created by Bradly Landucci

#include <usermodfx.h>
#include "fx_api.h"
#include <math.h>
#include "simplelfo.hpp"
#include "osc_api.h"

// Initializing Params //
static float fxamt = 0.5f;
static int type = 01.f;

// Initializing Platform //
void MODFX_INIT(uint32_t platform, uint32_t api)
{
}

// Main DSP Process Block //
// Sub values to be ignored, only used for prologue
void MODFX_PROCESS(const float *xn, float *yn,
                   const float *sub_xn, float *sub_yn,
                   uint32_t frames)
{
  float base_main;

  // Effect processing loop //
  // For double frames (AKA samples cause each frame = sample pair)
  for (int i=0;i<frames*2;i++)
  {
    switch(type)
    {
      case 0:
        *yn++ = *xn++;
        break;
      case 1:
        *yn++ = *xn++;      
        break;
      case 2:
        *yn++ = *xn++;
        break;
      case 3:
        *yn++ = *xn++;
        break;
    }

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
      fxamt = valf;

    case 1:
      if (valf < 0.25) 
      {
        type = 0; 
      } 
      else if (valf < 0.5) 
      {
        type = 1; 
      } 
      else if (valf < 0.75) 
      {
        type = 2; 
      } 
      else 
      {
        type = 3; 
      }
      break;
    default:
      break;
  }
}
""")

if __name__ == "__main__":
    template()
