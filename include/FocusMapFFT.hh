
#ifndef FOCUS_MAP_FFT_HH
#define FOCUS_MAP_FFT_HH

#include "FocusMap.hh"
#include "Transducer.hh"
#include <fftw3.h>

namespace uslib
{
class FocusMapFFT : public FocusMap
{
public:
   FocusMapFFT(uint32 in_samples, uint32 out_samples, uint32 vectors, 
               uint32 upsample_factor, uint32 channels);
   
   virtual ~FocusMapFFT();

   virtual err Calculate(FocusOffsets *offsets);
    
   virtual err Run(Frame *f, uint32 thread_id);

private:
   inline err UpsampleVector(uint8 *in, double* out);

   uint32         m_up_samples;
   uint32         m_num_taps;
   float         *m_upsample_taps;
   FocusOffsets  *m_focus_offsets;
   double        *m_upsample_out;
   double        *m_fft_in;
   fftw_complex  *m_fft_tmp;
   fftw_plan      m_fft_plan;
   fftw_plan      m_ifft_plan;

}; // class FocusMapFFT
} // nsmespace uslib

#endif
