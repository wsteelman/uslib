
#ifndef FOCUS_MAP_SPARSE_HH
#define FOCUS_MAP_SPARSE_HH

#include "FocusMap.hh"
#include "Transducer.hh"
#include <fftw3.h>

namespace uslib
{
class FocusMapSparse : public FocusMap
{
   struct SampleData
   {
      bool valid;
      uint32 *samples;
      float *weights;
      uint32 channels;
      uint32 num_samples;
   };


public:
   FocusMapSparse(uint32 in_samples, uint32 out_samples, uint32 vectors, 
               uint32 upsample_factor, uint32 channels);
   
   virtual ~FocusMapSparse();

   virtual err Calculate(FocusOffsets *offsets);
    
   virtual err Run(Frame *f, uint32 thread_id);

private:
   inline float FocusPixel(uint32 *samples, float *weights,
                    uint32 channels, uint32 num_samples,
                    uint8 **data);
  
   err ComputeSampleData(FocusOffsets *map, uint32 sample_num, 
                         uint32 vector, uint32 taps,
                         uint32 *samples, float *weights);

   err ComputeUpsample(uint32 sample_num, uint32 vector, uint32 taps,
                       uint32 *samples, float* weights);
 
   uint32 m_up_samples;
   uint32 m_num_taps;
   float *m_upsample_taps;
   FocusOffsets *m_focus_offsets;
   SampleData *m_sample_map;

}; // class FocusMapSparse
} // nsmespace uslib

#endif
