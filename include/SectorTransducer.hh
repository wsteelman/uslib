
#ifndef SECTOR_TRANSDUCER_HH_
#define SECTOR_TRANSDUCER_HH_

#include "Transducer.hh"

namespace uslib
{
struct SectorParameters
{
   double min_radius;
   double max_radius;
   double acoustic_range;
   double xducer_height;
   double scan_angle;
};

class SectorTransducer : public Transducer
{
public:
   SectorTransducer(uint32 element_cnt, uint32 vectors, uint32 samples,
                    uint32 upsample_factor, double frequency) :
      Transducer(element_cnt, vectors, samples, upsample_factor, frequency)
   {

   }

   virtual ~SectorTransducer()
   {

   }

   virtual SectorParameters GetSectorDimensions() = 0;
   
}; // class SectorTransducer
} // namespace uslib
#endif
