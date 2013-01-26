#ifndef UTILS_HH_
#define UTILS_HH_

#include <math.h>
#include "types.h"

namespace uslib
{

#ifdef __GNUC__
#  define LIKELY( x )     __builtin_expect( !!(x), 1 )
#  define UNLIKELY( x )   __builtin_expect( !!(x), 0 )
#else
#  define LIKELY( x )     (x)
#  define UNLIKELY( x )   (x)
#endif

inline void GenTaps(float *taps, uint32 num_upsample_taps, float cutoff, float sample_rate,
             bool normalize)
{
   float w0 = 2.0*PI*cutoff/sample_rate;
   float w = -(w0 * (num_upsample_taps/2));
   float sincw;
   float hannw;

   float sum = 0.0f;
   for(int i = 0; i < (int)num_upsample_taps; i++)
   {
      //printf("w: %3.10f\t", w);
      sincw = sin(w) / w;
      //printf("sinc: %3.10f\t", sincw);
      hannw = 0.5 + 0.5*cos((float)(i-num_upsample_taps/2.0) * PI/(num_upsample_taps/2.0));
      //printf("han: %3.10f\t", hannw);
      taps[i] = sincw * hannw;
      w += w0;
      //printf("taps = %3.10f\n", taps[i]);
      sum += taps[i]; 
   }

   if (normalize)
   {
      // scale taps
      for (uint32 t = 0; t < num_upsample_taps; t++)
      {
         taps[t] = taps[t] / sum;
      }
   }
   return;
}

inline void GenerateFakeImage(uint32 vectors, uint32 samples, uint8 *img, uint32 frame)
{
   for (uint32 v = 0; v < vectors; v++)
   {
      uint32 x = 0;
      for(x = 0; x < samples/4; x++)
      {
         *(img++) = 0x80;
      }

      if (v > (vectors/2 - 30) && v < (vectors/2 + 30))
      {
         for(x = 0; x < samples/2; x++)
         {
            if (x > (frame*8 + 500) && x < (frame*8 + 1500))
            {
               *(img++) = 0x80;
            }
            else
            {
               *(img++) = (uint8)(128 + 64*cos(x*1.0*PI/180.0));
            } 
         }
      }
      else
      {
         for(x = 0; x < samples/2; x++)
         {
            *(img++) = (uint8)(128 + 64*cos(x*1.0*PI/180.0));
         }
      }
      for(x = 0; x < samples/4; x++)
      {
         *(img++) = 0x80;
      }
   }
}

} // namespace uslib

#endif
