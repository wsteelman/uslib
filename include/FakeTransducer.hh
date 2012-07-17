
#ifndef FAKE_TRANSDUCER_HH_
#define FAKE_TRANSDUCER_HH_

#include "SectorTransducer.hh"

#define SPEED_OF_SOUND     1.54 // mm/us
#define SAMPLE_RATE        24.0 // MHz
#define ANGLE_DEG          100.0
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
                  uint32 upsample_factor, double frequency) :
      SectorTransducer(element_cnt, vectors, samples, upsample_factor, frequency)
   {
      m_focus_map.map = new uint32*[m_element_cnt];
      for (uint32 c = 0; c < m_element_cnt; c++)
      {
         m_focus_map.map[c] = new uint32[m_samples*m_upsample_factor];
         for (uint32 s = 0; s < m_samples*m_upsample_factor; s++)
         {
            m_focus_map.map[c][s] = s + (rand() % 4);
         }
      }

   }
      
   virtual ~FakeTransducer () 
   { 
      for (uint32 c = 0; c < m_element_cnt; c++)
      {
         delete [] m_focus_map.map[c];
      }
      delete [] m_focus_map.map;
   }

   virtual err CalculateFocusOffsets()
   {
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