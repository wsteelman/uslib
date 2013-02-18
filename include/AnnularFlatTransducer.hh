
#ifndef ANNULAR_FLAT_TRANSDUCER_HH_
#define ANNULAR_FLAT_TRANSDUCER_HH_

#include "SectorTransducer.hh"

#define SPEED_OF_SOUND     1.54 // mm/us

namespace uslib
{
struct AnnularFlatParameters
{
   uint32 element_cnt;
   uint32 vectors;
   uint32 samples;
   double sample_rate;
   double frequency;
   double sector_degrees;
   double transducer_height;
   double probe_radius;
   double radii[MAX_ELEMENTS];
};

class AnnularFlatTransducer : public SectorTransducer
{
public:
   AnnularFlatTransducer(AnnularFlatParameters &params);
      
   virtual ~AnnularFlatTransducer(); 

   virtual err CalculateFocusOffsets(uint32 upsample_factor);

   SectorParameters GetSectorDimensions();

private:
   AnnularFlatParameters m_params;
   double m_mm_per_sample;
   double m_samples_per_mm;
   double m_acoustic_range;  
   double m_max_radius;
   double m_min_radius;
   double m_angle_rad;
 
}; // class FakeTransducer
} // namespace uslib

#endif
