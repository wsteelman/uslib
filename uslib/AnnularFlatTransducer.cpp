#include "AnnularFlatTransducer.hh"
#include <math.h>
#include <stdio.h>

namespace uslib
{
   AnnularFlatTransducer::AnnularFlatTransducer(AnnularFlatParameters &params) :
      SectorTransducer(params.element_cnt, 
                       params.vectors, 
                       params.samples, 
                       params.frequency),
      m_params(params)
   {
      m_focus_map.map = new uint32*[m_element_cnt];
      
      // compute physical conversions
      m_acoustic_range = 0.5 * SPEED_OF_SOUND * ((double)m_samples/params.sample_rate);   
      m_mm_per_sample = ( (1.0/params.sample_rate) * SPEED_OF_SOUND * 0.5);
      m_samples_per_mm = 1.0 / m_mm_per_sample;
      m_max_radius = params.transducer_height + m_acoustic_range;
      m_min_radius = params.probe_radius;  
      m_angle_rad = params.sector_degrees  * PI / 180.0; 
   }

   AnnularFlatTransducer:: ~AnnularFlatTransducer () 
   { 
      for (uint32 c = 0; c < m_element_cnt; c++)
      {
         delete [] m_focus_map.map[c];
      }
      delete [] m_focus_map.map;
   }
 
   err 
   AnnularFlatTransducer::CalculateFocusOffsets(uint32 upsample_factor)
   {
      m_focus_map.vectors = m_vectors;
      m_focus_map.samples = m_samples * upsample_factor;
      m_focus_map.channels = m_element_cnt;

      const uint32 up_samples = m_samples * upsample_factor;
      for (uint32 c = 0; c < m_element_cnt; c++)
      {
         m_focus_map.map[c] = new uint32[up_samples];
      }     

      const double samples_per_mm = m_samples_per_mm * upsample_factor;
      const double mm_per_sample = 1.0 / samples_per_mm;
      uint32 mod_cnt = 0;
 
      for (uint32 s = 0; s < up_samples; s++)
      {
         double d0 = s * mm_per_sample;
         for (uint32 c = 0; c < m_element_cnt; c++)
         {
            double dc = sqrt( pow(d0, 2.0) + pow(m_params.radii[c], 2.0) );
            uint32 offset = round((dc - d0) * samples_per_mm);
            m_focus_map.map[c][s] = s + offset; 
            if ( (s+offset) % upsample_factor == 0)
            {
               mod_cnt++;
            }
         } 
      }
      //printf("AFT Percent not interpolated %f\n", (double)mod_cnt / (double)(m_element_cnt*up_samples));
      return SUCCESS;
   }

   SectorParameters 
   AnnularFlatTransducer::GetSectorDimensions()
   {
      SectorParameters p;
      p.min_radius = m_min_radius;
      p.max_radius = m_max_radius;
      p.acoustic_range = m_acoustic_range;
      p.xducer_height = m_params.transducer_height;
      p.scan_angle = m_angle_rad;

      return p;
   } 


} // namespace uslib
