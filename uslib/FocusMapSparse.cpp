#include <stdio.h>
#include <string.h>
#include "FocusMapSparse.hh"
#include "Utils.hh"

namespace uslib
{
FocusMapSparse::FocusMapSparse(uint32 in_samples, uint32 out_samples, uint32 vectors, 
                         uint32 upsample_factor, uint32 channels) :
   FocusMap("FocusMapSparse", in_samples, out_samples, vectors, upsample_factor, channels)
{
   m_up_samples = m_in_samples * m_upsample_factor;
   m_num_taps = 31;
   m_sample_map = new SampleData[m_out_samples*m_vectors];
}

FocusMapSparse::~FocusMapSparse()
{
   delete [] m_upsample_taps;
   for (uint32 x = 0; x < m_out_samples*m_vectors; x++)
   {
      delete [] m_sample_map[x].samples;
      delete [] m_sample_map[x].weights;
   }
   delete [] m_sample_map;
}


err
FocusMapSparse::ComputeSampleData(FocusOffsets *map, uint32 sample_num, 
                                     uint32 vector, uint32 taps,
                                     uint32 *samples, float *weights)
{ 
   for (uint32 c = 0; c < m_channels; c++)
   {
      ComputeUpsample(map->map[c][sample_num], vector, taps, samples, weights);
      samples += taps;
      weights += taps;  
   }
   return SUCCESS; 
}

err
FocusMapSparse::ComputeUpsample(uint32 sample_num, uint32 vector, uint32 taps,
                                    uint32 *samples, float* weights)
{
   uint32 len = ((m_num_taps / m_upsample_factor) + 1) * m_upsample_factor; 
   uint32 half_taps = len/2;
   uint32 mod = sample_num % m_upsample_factor;

   if (sample_num < half_taps || sample_num > (m_up_samples - half_taps))
   {
      for (uint32 x = 0; x < taps; x++)
      {
         samples[x] = 0;
         weights[x] = 0.0f; 
      }
   }
   else if (mod == 0)
   {
      samples[0] = (m_in_samples * vector) + sample_num / m_upsample_factor;
      weights[0] = 1.0f;
      for (uint32 x = 1; x < taps; x++)
      {
         samples[x] = 0;
         weights[x] = 0.0f;
      }
   } 
   else
   { 
      uint32 tap_offset = m_upsample_factor - mod - 1;
      uint32 down_sample = sample_num / m_upsample_factor - taps/2 + 1; 
      for (uint32 t = 0; t < taps; t++)
      { 
         samples[t] = (m_in_samples * vector) + down_sample;
         weights[t] = m_upsample_taps[tap_offset]; 
         down_sample++;
         tap_offset += m_upsample_factor;
      }
   }
   return SUCCESS;
}


err
FocusMapSparse::Calculate(FocusOffsets *offsets)
{
   m_upsample_taps = new float[m_num_taps];
   GenTaps(m_upsample_taps, m_num_taps, 3.0f, 24.0f, false); 
   uint32 taps_per_pixel = (m_num_taps+1)/m_upsample_factor;
   m_focus_offsets = offsets;
   
   SampleData *sample = m_sample_map; 
   for (uint32 v = 0; v < m_vectors; v++)
   {
      for (uint32 s = 0; s < m_up_samples; s+= m_out_upsample_factor)
      {
         sample->samples = new uint32[m_channels * taps_per_pixel];
         sample->weights = new float[m_channels * taps_per_pixel];
         sample->channels = m_channels;
         sample->num_samples = taps_per_pixel;
         ComputeSampleData(offsets, s, v, taps_per_pixel, 
                          sample->samples, sample->weights);

         sample++; 
      }  
   }
   return SUCCESS;
}

float 
FocusMapSparse::FocusPixel(uint32 *samples, float *weights,
                                 uint32 channels, uint32 num_samples,
                                 Frame::data_type **data)
{
   float sum = 0;
   for (uint32 c = 0; c < channels; c++)
   {
      for (uint32 s = 0; s < num_samples; s++)
      {
         sum += (float)(data[c][*samples++]) * *weights++; 
      }
   }
   return sum / (float)channels;
}

err 
FocusMapSparse::Run(Frame *f, uint32 thread_id)
{
   if (m_channels != f->GetNumChannels())
   {
      printf("SectorImageMapRF::Focus(): frame contains incorrect number of channels %u, "
             "expected %u\n", f->GetNumChannels(), m_channels);
      return OUTOFRANGE;
   }

   float sum = 0;
   float *out = f->GetFocusBuffer();
   Frame::data_type **data = f->GetChannelData();
   SampleData *sd = m_sample_map;
   uint32 *samples;
   float *weights;

   memset(out, 0x00, m_total_out_samples * sizeof(float));

   for (uint32 c = 0; c < m_channels; c++)
   {
      sd = m_sample_map;
      out = f->GetFocusBuffer();
      for (uint32 s = 0; s < m_total_out_samples; s++)
      {
         sum = 0;
         samples = sd->samples + (c * sd->num_samples);
         weights = sd->weights + (c * sd->num_samples);
         for (uint32 s = 0; s < sd->num_samples; s++)
         {
            sum += (float)(data[c][*samples++]) * *weights++; 
         }
         *out++ += sum / (float) m_channels;
         sd++;
      }
   }

   //float *out = f->GetFocusBuffer();
   //// copy channel pointers to local storage
   //uint8 **data = f->GetChannelData();
   //SampleData *sample = m_sample_map;
   //for (uint32 v = 0; v < m_vectors; v++)
   //{
   //   for (uint32 s = 0; s < m_samples; s++)
   //   {
   //      *out++ = FocusPixel(sample->samples, sample->weights, sample->channels,
   //                          sample->num_samples, data);
   //      sample++; 
   //   } 
   //} 

   return SUCCESS;

}

} // namespace uslbi
