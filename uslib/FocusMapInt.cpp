
#include "FocusMapInt.hh"
#include "Utils.hh"

namespace uslib
{
FocusMapInt::FocusMapInt(uint32 in_samples, uint32 out_samples, uint32 vectors, 
                         uint32 upsample_factor, uint32 channels) :
   FocusMap("FocusMapInt", in_samples, out_samples, vectors, upsample_factor, channels)
{
   m_up_samples = m_in_samples * m_upsample_factor;
   m_num_taps = 31;
}

FocusMapInt::~FocusMapInt()
{
   delete [] m_upsample_taps;
   //for (uint32 x = 0; x < m_channels; x++)
   //{
   //   fftw_free(m_upsample_out[x]); 
   //}
   delete [] m_upsample_out;
}

err
FocusMapInt::Calculate(FocusOffsets *offsets)
{
   m_upsample_taps = new float[m_num_taps];
   GenTaps(m_upsample_taps, m_num_taps, 3.0f, 24.0f, false); 
   m_focus_offsets = offsets;
   // setup upsampling plan for focusing
   m_upsample_out = new float[m_up_samples]; 
   //m_upsample_out = new float*[m_channels];
   //for (uint32 x = 0; x < m_channels; x++)
   //{
   //   m_upsample_out[x] = (float*)fftw_malloc(m_up_samples*sizeof(float));
   //}
   return SUCCESS;
}


err
FocusMapInt::UpsampleVector(uint8 *in, float* out)
{
   uint32 taps = (m_num_taps / m_upsample_factor) + 1; 
   uint32 len = ((m_num_taps / m_upsample_factor) + 1) * m_upsample_factor; 
   uint32 half_taps = len/2;

   for (uint32 i = 0; i < half_taps; i++)
   {
      *out++ = 0.0f;
   }

   for (uint32 i = 0; i < m_up_samples - len; i+=m_upsample_factor)
   {
      uint32 tap_start = m_upsample_factor - 2;
      out[0] = (float)(signed char)in[0];
      for (uint32 s = 1; s < m_upsample_factor; s++)
      {
         uint32 tap_offset = tap_start;
         int sample_offset = -m_upsample_factor+1;
         float  tmp = 0; 
         for (uint32 t = 0; t < taps; t++)
         { 
            // convert to signed byte, then to float
            signed char sample = (signed char)(in[sample_offset++]);
            tmp += (float) sample * m_upsample_taps[tap_offset];
            tap_offset += m_upsample_factor;
         }
         tap_start--;
         out[s] = tmp;
      }
      out += m_upsample_factor;
      in++;
   }

   for (uint32 i = 0; i < half_taps; i++)
   {
      *out++ = 0.0f;
   }

   return SUCCESS;
}



err 
FocusMapInt::Run(Frame *f)
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
            *output++ += m_upsample_out[m_focus_offsets->map[c][s]] / (float)m_channels; 
         }
         chnl += m_in_samples;
      }
   }

   //output = f->GetFocusBuffer();
   //uint8 tmp[m_total_out_samples];
   //for (uint32 x = 0; x < m_total_out_samples; x++)
   //{
   //   tmp[x] = (uint8)output[x];
   //}
   //FILE *file = fopen("focused.raw", "wb");
   //fwrite(tmp, sizeof(uint8), m_total_out_samples, file);
   //fclose(file);


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
   //         *output += m_upsample_out[c][m_focus_offsets->map[c][s]] / (float)m_channels;
   //      }
   //      output++;
   //   } 
   //}
   return SUCCESS;

}

} // namespace uslbi
