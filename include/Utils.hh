#ifndef UTILS_HH_
#define UTILS_HH_

#include <math.h>
#include "types.h"
#include "Transducer.hh"
#include "Image.hh"
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

inline void GenerateFakeImage(uint32 vectors, uint32 samples, Frame::data_type *img, uint32 frame)
{
   for (uint32 v = 0; v < vectors; v++)
   {
      uint32 x = 0;
      for(x = 0; x < samples/4; x++)
      {
         *(img++) = 0x40;
      }

      if (v > (vectors/2 - 30) && v < (vectors/2 + 30))
      {
         for(x = 0; x < samples/2; x++)
         {
            if (x > (frame*32 + 100) && x < (frame*32 + 400))
            {
               *(img++) = 0x40;
            }
            else
            {
               *(img++) = (Frame::data_type)(128 + 64*cos(x*1.0*PI/180.0));
            } 
         }
      }
      else
      {
         for(x = 0; x < samples/2; x++)
         {
            *(img++) = (Frame::data_type)(128 + 64*cos(x*1.0*PI/180.0));
         }
      }
      for(x = 0; x < samples/4; x++)
      {
         *(img++) = 0x40;
      }
   }
}

inline void UnfocusImage(Frame::data_type *in, Frame::data_type *out, FocusOffsets *map, 
                  uint32 vectors, uint32 samples, uint32 channel)
{
   uint32 upsample = map->samples / samples;
   
   double         *upsample_out = (double*)fftw_malloc(map->samples*sizeof(double)); 
   double         *fft_in = (double*)fftw_malloc(samples*sizeof(double));   
   fftw_complex   *fft_tmp = (fftw_complex*)fftw_malloc((map->samples/2+1)*sizeof(fftw_complex));
   fftw_plan      fft_plan = fftw_plan_dft_r2c_1d(samples, fft_in, fft_tmp, FFTW_ESTIMATE);   
   fftw_plan      ifft_plan = fftw_plan_dft_c2r_1d(map->samples, fft_tmp, upsample_out, FFTW_ESTIMATE); 

   uint32 rev_map[map->samples];
   uint32 min = map->map[channel][0];
   for (uint32 rev_sample = 0; rev_sample < map->samples; rev_sample++)
   {
      if (rev_sample < min)
      {
         rev_map[rev_sample] = 0;
      } 
      int diff = samples;
      uint32 best = 0;
      for (uint32 in = 0; in < map->samples; in++)
      {
         int tmp = abs((int)rev_sample - (int)map->map[channel][in]);
         if (tmp < diff)
         {
            best = in; 
            diff = tmp;
         } 
      } 
      rev_map[rev_sample] = best;
   }

   Frame::data_type *ptr = out;
   for (uint32 v = 0; v < vectors; v++)
   {
      for (uint32 s = 0; s < samples; s++)
      {
         fft_in[s] = (double)(*in++);
      }
      memset(fft_tmp, 0x00, (map->samples/2+1)*sizeof(fftw_complex));
      fftw_execute_dft_r2c(fft_plan, fft_in, fft_tmp);
      fftw_execute_dft_c2r(ifft_plan, fft_tmp, upsample_out);
  
      for (uint32 s = 0; s < map->samples; s+= upsample)
      { 
         uint32 idx = rev_map[s];
         double tmp = upsample_out[idx] / (double) samples;
         if (tmp > 127.0)
         {
            *ptr = 127;
         } 
         else if (tmp < -128.0)
         {
            *ptr = 128;
         }
         else
         {
            *ptr = (Frame::data_type)tmp;
         }
         ptr++;
      }
   }  

   fftw_free(upsample_out);
   fftw_free(fft_in);
   fftw_free(fft_tmp);

   char filename[64];
   sprintf(filename, "channel%u.rf", channel);
   FILE *file = fopen(filename, "wb");
   if (file != NULL)
   {
      fwrite(out, sizeof(uint8), vectors*samples, file);
      fclose(file);
   }
}

} // namespace uslib

#endif
