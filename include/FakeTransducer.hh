
#ifndef FAKE_TRANSDUCER_HH_
#define FAKE_TRANSDUCER_HH_

#include "SectorTransducer.hh"

#define SPEED_OF_SOUND     1.54 // mm/us
#define SAMPLE_RATE        24.0 // MHz
#define ANGLE_DEG          100.0
//#define ANGLE_DEG          180.0
#define ANGLE_RAD          (ANGLE_DEG * PI / 180.0)

#define MM_PER_SAMPLE      ( (1.0/SAMPLE_RATE) * SPEED_OF_SOUND * 0.5)
#define SAMPLES_PER_MM     ( 1.0 / MM_PER_SAMPLE )
#define TRANSDUCER_HEIGHT  7.43  // mm
#define TIP_TO_TRANSDUCER  0.5     // mm
#define TIP_THICKNESS      3.0     // mm
//#define ACOUSTIC_RANGE     (0.5 * SPEED_OF_SOUND * (SAMPLES/SAMPLE_RATE))
#define MAX_RADIUS         (ACOUSTIC_RANGE + TRANSDUCER_HEIGHT) // mm
#define MIN_RADIUS         TRANSDUCER_HEIGHT + TIP_TO_TRANSDUCER + TIP_THICKNESS// mm
#define SAG_WIDTH          60.0    // mm

namespace uslib
{
class FakeTransducer : public SectorTransducer
{
public:
   FakeTransducer(uint32 element_cnt, uint32 vectors, uint32 samples,
                  double frequency) :
      SectorTransducer(element_cnt, vectors, samples, frequency)
   {
      m_focus_map.map = new uint32*[m_element_cnt];
   }
      
   virtual ~FakeTransducer () 
   { 
      for (uint32 c = 0; c < m_element_cnt; c++)
      {
         delete [] m_focus_map.map[c];
      }
      delete [] m_focus_map.map;
   }

   virtual err CalculateFocusOffsets(uint32 upsample_factor)
   {
      m_focus_map.vectors = m_vectors;
      m_focus_map.samples = m_samples * upsample_factor;
      m_focus_map.channels = m_element_cnt;

      uint32 mod_cnt = 0;
      for (uint32 c = 0; c < m_element_cnt; c++)
      {
         m_focus_map.map[c] = new uint32[m_samples*upsample_factor];
         for (uint32 s = 0; s < m_samples*upsample_factor; s++)
         {
            m_focus_map.map[c][s] = s + (rand() % upsample_factor);
            if ( m_focus_map.map[c][s] % upsample_factor == 0)
            {
               mod_cnt++;
            }

         }
      }
      //printf("Fake Percent not interpolated %f\n", (double)mod_cnt / 
      //         (double)(m_element_cnt*m_samples*upsample_factor));
      return SUCCESS;
   }

   SectorParameters GetSectorDimensions()
   {
      const double ACOUSTIC_RANGE = 0.5 * SPEED_OF_SOUND * ((double)m_samples/SAMPLE_RATE);

      SectorParameters p;
      p.min_radius = MIN_RADIUS;
      p.max_radius = MAX_RADIUS;
      p.acoustic_range = ACOUSTIC_RANGE;
      p.xducer_height = TRANSDUCER_HEIGHT;
      p.scan_angle = ANGLE_RAD;

      return p;
   } 

   
}; // class FakeTransducer
} // namespace uslib

#endif
