
#include "FocusMapFFT.hh"

namespace uslib
{
FocusMapFFT::FocusMapFFT(uint32 in_samples, uint32 out_samples, uint32 vectors, 
                         uint32 upsample_factor, uint32 channels) :
   FocusMap("FocusMapFFT", in_samples, out_samples, vectors, upsample_factor, channels)
{
   m_up_samples = m_in_samples * m_upsample_factor;
   m_num_taps = 31;
}

FocusMapFFT::~FocusMapFFT()
{
   delete [] m_upsample_taps;
   //for (uint32 x = 0; x < m_channels; x++)
   //{
   //   fftw_free(m_upsample_out[x]); 
   //}
   //delete [] m_upsample_out;
   fftw_free(m_upsample_out); 
   fftw_free(m_fft_tmp);
   fftw_free(m_fft_in);
}

err
FocusMapFFT::Calculate(FocusOffsets *offsets)
{
   m_focus_offsets = offsets;
   // setup upsampling plan for focusing
   m_upsample_out = (double*)fftw_malloc(m_up_samples*sizeof(double)); 
   //m_upsample_out = new double*[m_channels];
   //for (uint32 x = 0; x < m_channels; x++)
   //{
   //   m_upsample_out[x] = (double*)fftw_malloc(m_up_samples*sizeof(double));
   //}
   m_fft_in = (double*)fftw_malloc(m_in_samples*sizeof(double));   
   m_fft_tmp = (fftw_complex*)fftw_malloc((m_up_samples/2+1)*sizeof(fftw_complex));
   m_fft_plan = fftw_plan_dft_r2c_1d(m_in_samples, m_fft_in, m_fft_tmp, FFTW_ESTIMATE);   
   m_ifft_plan = fftw_plan_dft_c2r_1d(m_up_samples, m_fft_tmp, m_upsample_out, FFTW_ESTIMATE);   

   return SUCCESS;
}


err
FocusMapFFT::UpsampleVector(uint8 *in, double* out)
{
   for (uint32 s = 0; s < m_in_samples; s++)
   {
      m_fft_in[s] = (double)(signed char)(*in++);
   }
   memset(m_fft_tmp, 0x00, (m_up_samples/2+1)
            *sizeof(fftw_complex));
   fftw_execute_dft_r2c(m_fft_plan, m_fft_in, m_fft_tmp);
   fftw_execute_dft_c2r(m_ifft_plan, m_fft_tmp, out);
 
   return SUCCESS;
}



err 
FocusMapFFT::Run(Frame *f, uint32 thread_id)
{
   if (m_channels != f->GetNumChannels())
   {
      printf("SectorImageMapRF::Focus(): frame contains incorrect number of channels %u, "
             "expected %u\n", f->GetNumChannels(), m_channels);
      return OUTOFRANGE;
   }

   float *output = f->GetFocusBuffer();
   memset(output, 0x00, m_total_out_samples * sizeof(float)); 
   for (uint32 c = 0; c < m_channels; c++)
   {
      uint8 *chnl = f->GetChannelData(c);
      output = f->GetFocusBuffer();
      for (uint32 v = 0; v < m_vectors; v++)
      {
         UpsampleVector(chnl, m_upsample_out);
         for (uint32 s = 0; s < m_up_samples; s+= m_out_upsample_factor)
         {
            *output++ += m_upsample_out[m_focus_offsets->map[c][s]]; 
         }
         chnl += m_in_samples;
      }
   }
   output = f->GetFocusBuffer();
   for (uint32 s = 0; s < m_total_out_samples; s++)
   {
      *output = *output / (float)m_in_samples / (float)m_channels;
      output++;
   }
   //// copy channel pointers to local storage
   //uint8 *chnls[m_channels];
   //for (uint32 c = 0; c < m_channels; c++)
   //{
   //   chnls[c] = f->GetChannelData(c); 
   //}
   //for (uint32 v = 0; v < m_vectors; v++)
   //{
   //   for (uint32 c = 0; c < m_channels; c++) 
   //   {
   //      UpsampleVector(chnls[c], m_upsample_out[c]); 
   //      chnls[c] += m_samples; 
   //   }
   //  
   //   for (uint32 s = 0; s < m_up_samples; s+=m_upsample_factor)
   //   {
   //      *output = 0;
   //      for (uint32 c = 0; c < m_channels; c++)
   //      {
   //         *output += (float)m_upsample_out[c][m_focus_offsets->map[c][s]] / 
   //                     (float) m_samples / (float)m_channels;
   //      }
   //      output++;
   //   } 
   //}
   return SUCCESS;

}

} // namespace uslbi
