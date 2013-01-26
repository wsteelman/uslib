
#ifndef FOCUS_MAP_INT_HH
#define FOCUS_MAP_INT_HH

#include "FocusMap.hh"
#include "Transducer.hh"
#include <fftw3.h>

namespace uslib
{
class FocusMapInt : public FocusMap
{
public:
   FocusMapInt(uint32 in_samples, uint32 out_samples, uint32 vectors, 
               uint32 upsample_factor, uint32 channels);
   
   virtual ~FocusMapInt();

   virtual err Calculate(FocusOffsets *offsets);
    
   virtual err Run(Frame *f, uint32 thread_id);

private:
   err UpsampleVector(uint8 *in, float* out);

   uint32 m_up_samples;
   uint32 m_num_taps;
   float *m_upsample_taps;
   FocusOffsets *m_focus_offsets;
   float *m_upsample_out;

}; // class FocusMapInt
} // nsmespace uslib

#endif
