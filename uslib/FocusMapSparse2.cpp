#include <stdio.h>
#include <string.h>
#include "FocusMapSparse2.hh"
#include "Utils.hh"
#include "perf.hh"

#ifdef CYCLE_PERF
uint64 g_sparse_up_cycles = 0;
uint64 g_sparse_noup_cycles = 0;
uint64 g_sparse_total_cycles = 0;
#endif


namespace uslib
{
FocusMapSparse2::FocusMapSparse2(uint32 in_samples, uint32 out_samples, uint32 vectors, 
                         uint32 upsample_factor, uint32 channels) :
   FocusMap("FocusMapSparse2", in_samples, out_samples, vectors, upsample_factor, channels)
{
   m_up_samples = m_in_samples * m_upsample_factor;
   m_num_taps = upsample_factor * 8 - 1;;
   m_sample_map = new SampleData[m_out_samples*m_vectors];
}

FocusMapSparse2::~FocusMapSparse2()
{
   delete [] m_upsample_taps;
   delete [] m_sample_map;
}


err
FocusMapSparse2::ComputeSampleData(FocusOffsets *map, uint32 sample_num, 
                                     uint32 vector, uint32 taps,
                                     SampleData *data)
{
   data->channels = m_channels; 
   for (uint32 c = 0; c < m_channels; c++)
   {
      ComputeUpsample(map->map[c][sample_num], vector, taps, c, data);
   }
   return SUCCESS; 
}

err
FocusMapSparse2::ComputeUpsample(uint32 sample_num, uint32 vector, uint32 taps,
                                 uint32 channel, SampleData *data)
{
   uint32 len = ((taps / m_upsample_factor) + 1) * m_upsample_factor; 
   uint32 half_taps = len/2;
   uint32 mod = sample_num % m_upsample_factor;

   if (mod == 0 || 
       (sample_num < half_taps || sample_num > (m_up_samples - half_taps)) )
   {
      data->num_samples[channel] = 1;
      if ((sample_num/m_upsample_factor) >= m_in_samples)
      {
         data->sample[channel] = 0;
      }
      else
      {
         data->sample[channel] = (m_in_samples * vector) + sample_num / m_upsample_factor;
      }
      data->weight_offset[channel] = 0;
      data->interpolate[channel] = false; 
   } 
   //else if (sample_num < half_taps || sample_num > (m_up_samples - half_taps))
   //{
   //   data->num_samples[channel] = 1; //(taps + m_upsample_factor - 1) / m_upsample_factor;
   //   data->sample[channel] = (m_in_samples * vector) + sample_num / m_upsample_factor - 1; // 0;
   //   data->weight_offset[channel] = 0;
   //   data->interpolate[channel] = false; //true;
   //}
   else
   { 
      uint32 offset = m_upsample_factor - mod;
      uint32 starting_sample = (sample_num - (taps/2) + offset) / m_upsample_factor;
      uint32 samples = (taps - offset + m_upsample_factor - 1) / m_upsample_factor;
      
      if ((starting_sample + samples) >= m_in_samples)
      {
         data->num_samples[channel] = 1;
         data->sample[channel] = (m_in_samples * vector) + sample_num / m_upsample_factor - 1;
         data->weight_offset[channel] = 0;
         data->interpolate[channel] = false; 
      }
      else
      { 
         data->num_samples[channel] = samples;
         data->sample[channel] = (m_in_samples * vector) + starting_sample;
         data->weight_offset[channel] = offset; 
         data->interpolate[channel] = true;
      }
   }
   return SUCCESS;
}


err
FocusMapSparse2::Calculate(FocusOffsets *offsets)
{
   m_upsample_taps = new float[m_num_taps];
   GenTaps(m_upsample_taps, m_num_taps, 3.0f, 24.0f, false); 
   //uint32 taps_per_pixel = (m_num_taps+1)/m_upsample_factor;
   m_focus_offsets = offsets;
   
   SampleData *sample = m_sample_map; 
   for (uint32 v = 0; v < m_vectors; v++)
   {
      for (uint32 s = 0; s < m_up_samples; s+= m_out_upsample_factor)
      {
         ComputeSampleData(offsets, s, v, m_num_taps, sample); 
         sample++; 
      }  
   }
   return SUCCESS;
}

err 
FocusMapSparse2::Run(Frame *f, uint32 thread_id)
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
#ifdef CYCLE_PERF
   uint64 start = GetPerformanceCount();
   uint32 noup = 0;
   uint32 total = 0;
#endif
   for (uint32 s = 0; LIKELY(s < m_total_out_samples); s++)
   {
      sum = 0;
      for (uint32 c = 0; LIKELY(c < sd->channels); c++)
      {
         if (LIKELY(sd->interpolate[c]))
         {
            uint32 tap = sd->weight_offset[c];
            uint32 sample = sd->sample[c];
            for (uint32 x = 0; x < sd->num_samples[c]; x++)
            {
               sum += ((float)(data[c][sample++])) * 
                      m_upsample_taps[tap]; 
               tap += m_upsample_factor;
#ifdef CYCLE_PERF
               total++;
#endif 
            }
         }
         else
         {
#ifdef CYCLE_PERF
            noup++;
            total++;
#endif 
            sum += (float)(data[c][sd->sample[c]]);
         }
      }
      sum =  sum / (float)sd->channels;
      //if (sum > 127.0f)
      //{
      //   sum = 127.0f;
      //}
      //else if (sum < -128.0f)
      //{
      //   sum = -128.0f;
      //}
      *out++ = sum;
      sd++; 
   } 
#ifdef CYCLE_PERF
   uint64 diff = GetPerformanceCount() - start;
   g_sparse_noup_cycles += (diff * noup) / total;
   g_sparse_up_cycles += (diff * (total-noup)) / total;
#endif
   
   return SUCCESS;

}

} // namespace uslbi
