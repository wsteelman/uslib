
#include "FocusMapSparseMulti.hh"
#include "Utils.hh"

namespace uslib
{
FocusMapSparseMulti::FocusMapSparseMulti(uint32 in_samples, 
                                         uint32 out_samples, 
                                         uint32 vectors, 
                                         uint32 upsample_factor, 
                                         uint32 channels,
                                         uint32 num_threads) :
   FocusMap("FocusMapSparseMulti", 
            in_samples, 
            out_samples, 
            vectors, 
            upsample_factor, 
            channels),
   m_num_taps(31),
   m_num_threads(num_threads)
{
   m_up_samples = m_in_samples * m_upsample_factor;
   m_num_taps = upsample_factor * 8 - 1;;
   m_sample_map = new SampleData[m_out_samples*m_vectors];
}

FocusMapSparseMulti::~FocusMapSparseMulti()
{
   delete [] m_upsample_taps;
   delete [] m_sample_map;
}


err
FocusMapSparseMulti::ComputeSampleData(FocusOffsets *map, uint32 sample_num, 
                                     uint32 vector, uint32 taps,
                                     SampleData *data)
{
   data->num_samples = taps;
   data->channels = m_channels; 
   for (uint32 c = 0; c < m_channels; c++)
   {
      ComputeUpsample(map->map[c][sample_num], vector, taps, c, data);
   }
   return SUCCESS; 
}

err
FocusMapSparseMulti::ComputeUpsample(uint32 sample_num, uint32 vector, uint32 taps,
                                 uint32 channel, SampleData *data)
{
   uint32 len = ((m_num_taps / m_upsample_factor) + 1) * m_upsample_factor; 
   uint32 half_taps = len/2;
   uint32 mod = sample_num % m_upsample_factor;

   if (sample_num < half_taps || sample_num > (m_up_samples - half_taps))
   {
      data->sample[channel] = 0;
      data->weight_offset[channel] = 0;
      data->interpolate[channel] = false;
   }
   else if (mod == 0)
   {
      data->sample[channel] = (m_in_samples * vector) + sample_num / m_upsample_factor;
      data->weight_offset[channel] = 0;
      data->interpolate[channel] = false; 
   } 
   else
   { 
      uint32 down_sample = sample_num / m_upsample_factor - taps/2 + 1; 
      data->sample[channel] = (m_in_samples * vector) + down_sample;
      data->weight_offset[channel] = m_upsample_factor - mod - 1;
      data->interpolate[channel] = true;
   }
   return SUCCESS;
}


err
FocusMapSparseMulti::Calculate(FocusOffsets *offsets)
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
         ComputeSampleData(offsets, s, v, taps_per_pixel, sample); 
         sample++; 
      }  
   }
   return SUCCESS;
}

err 
FocusMapSparseMulti::Run(Frame *f, uint32 thread_id)
{
   if (m_channels != f->GetNumChannels())
   {
      printf("SectorImageMapRF::Focus(): frame contains incorrect number of channels %u, "
             "expected %u\n", f->GetNumChannels(), m_channels);
      return OUTOFRANGE;
   }

   //printf("Beamforming tid: %u, frame: %u\n", thread_id, f->GetFrameID());

   float sum = 0;
   float *out = f->GetFocusBuffer();
   uint8 **data = f->GetChannelData();
   SampleData *sd = m_sample_map;

   const uint32 start_vector = m_vectors / m_num_threads * thread_id;
   const uint32 num_vectors = m_vectors / m_num_threads;

   const uint32 start_sample = start_vector * m_out_samples;
   const uint32 end_sample = start_sample + (num_vectors * m_out_samples) - 1;
  
   out += start_sample;
   sd  += start_sample;
    
   //printf("tid: %u, start: %u, end: %u\n", thread_id, start_sample, end_sample);

   for (uint32 s = start_sample; LIKELY(s < end_sample); s++)
   {
      sum = 0;
      for (uint32 c = 0; LIKELY(c < sd->channels); c++)
      {
         if (LIKELY(sd->interpolate[c]))
         {
            uint32 tap = sd->weight_offset[c];
            uint32 sample = sd->sample[c];
            for (uint32 s = 0; s < sd->num_samples; s++)
            {
               sum += (float)(signed char)(data[c][sample++]) * 
                      m_upsample_taps[tap]; 
               tap += m_upsample_factor;
            }
         }
         else
         {
            sum += (float)(signed char)data[c][sd->sample[c]];
         }
      }
      *out++ =  sum / (float)sd->channels;
      sd++; 
   } 

   return SUCCESS;

}

} // namespace uslbi
