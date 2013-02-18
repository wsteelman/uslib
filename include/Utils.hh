#ifndef UTILS_HH_
#define UTILS_HH_

#include <math.h>
#include "types.h"
#include "Transducer.hh"
#include <fftw3.h>

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
      hannw = 0.5 + 0.5*cos(((float)i-(float)num_upsample_taps/2.0) * PI/((float)num_upsample_taps/2.0));
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
            if (x > (frame*32 + 500) && x < (frame*32 + 1500))
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

inline void UnfocusImage(uint8 *in, uint8 *out, FocusOffsets *map, 
                  uint32 vectors, uint32 samples, uint32 channel)
{
   uint32 upsample = map->samples / samples;
   
   double         *upsample_out = (double*)fftw_malloc(map->samples*sizeof(double)); 
   double         *fft_in = (double*)fftw_malloc(samples*sizeof(double));   
   fftw_complex   *fft_tmp = (fftw_complex*)fftw_malloc((map->samples/2+1)*sizeof(fftw_complex));
   fftw_plan      fft_plan = fftw_plan_dft_r2c_1d(samples, fft_in, fft_tmp, FFTW_ESTIMATE);   
   fftw_plan      ifft_plan = fftw_plan_dft_c2r_1d(map->samples, fft_tmp, upsample_out, FFTW_ESTIMATE); 

   for (uint32 v = 0; v < vectors; v++)
   {
      for (uint32 s = 0; s < samples; s++)
      {
         fft_in[s] = (double)(signed char)(*in++);
      }
      memset(fft_tmp, 0x00, (map->samples/2+1)*sizeof(fftw_complex));
      fftw_execute_dft_r2c(fft_plan, fft_in, fft_tmp);
      fftw_execute_dft_c2r(ifft_plan, fft_tmp, upsample_out);
    
      for (int s = 0; s < (int)map->samples; s+=upsample)
      { 
         int idx = s - ((int)map->map[channel][s] - s);
         if (idx >= (int)map->samples)
         {
            idx = 0;
         }
         else if (idx < 0)
         {
            idx = 0;
         }
         double tmp = round(upsample_out[idx] / (double) samples);
         if (tmp > 127.0)
         {
            *out = 127;
         } 
         else if (tmp < -128.0)
         {
            *out = 128;
         }
         else
         {
            *out = (uint8)(signed char)tmp;
         }
         out++;
      }
   }  

   fftw_free(upsample_out);
   fftw_free(fft_in);
   fftw_free(fft_tmp);
}

} // namespace uslib

#endif
