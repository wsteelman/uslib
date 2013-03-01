
#ifndef FOCUS_MAP_SPARSE_MULTI_HH
#define FOCUS_MAP_SPARSE_MULTI_HH

#include "FocusMap.hh"
#include "Transducer.hh"
#include <fftw3.h>

#define MAX_CHANNELS 16

namespace uslib
{
class FocusMapSparseMulti : public FocusMap
{
   struct SampleData
   {
      bool interpolate[MAX_CHANNELS];
      uint32 sample[MAX_CHANNELS];
      uint32 weight_offset[MAX_CHANNELS];
      uint32 num_samples[MAX_CHANNELS];
      uint32 channels;
   };


public:
   FocusMapSparseMulti(uint32 in_samples, 
                       uint32 out_samples, 
                       uint32 vectors, 
                       uint32 upsample_factor, 
                       uint32 channels, 
                       uint32 num_threads);
   
   virtual ~FocusMapSparseMulti();

   virtual err Calculate(FocusOffsets *offsets);
    
   virtual err Run(Frame *f, uint32 thread_id);

private:
   err ComputeSampleData(FocusOffsets *map, uint32 sample_num, 
                         uint32 vector, uint32 taps, SampleData *data);

   err ComputeUpsample(uint32 sample_num, uint32 vector, uint32 taps,
                       uint32 channel, SampleData *data);
 
   uint32 m_up_samples;
   uint32 m_num_taps;
   float *m_upsample_taps;
   FocusOffsets *m_focus_offsets;
   SampleData *m_sample_map;
   uint32 m_num_threads;

}; // class FocusMapSparseMulti
} // nsmespace uslib

#endif
