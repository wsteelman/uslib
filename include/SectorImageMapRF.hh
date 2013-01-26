
#ifndef SECTOR_IMAGE_MAP_RF_HH_
#define SECTOR_IMAGE_MAP_RF_HH_

#include "types.h"
#include "ImageMap.hh"
#include <fftw3.h>

namespace uslib
{

class CoordinateConverter;
class SectorTransducer;
class FocusMap;

class SectorImageMapRF : public ImageMap
{
   struct PixelData
   {
      bool valid;
      uint32 sample1;
      uint32 sample2;
      float weight;
   };

public:
   SectorImageMapRF(const char *name, uint32 width, uint32 height);
 
   ~SectorImageMapRF();

   err Run(Frame *f, uint32 thread_id);

   err CalculateMap(SectorTransducer *t);

private:
   err Focus(Frame *f);

   err ScanConvert(Frame *f);

   err Envelope(Frame *f);

   err UpsampleVector(uint8 *in, float *out);

   PixelData *m_map; 
   CoordinateConverter *m_conv;
   uint32 m_samples;
   uint32 m_vectors;
   uint32 m_num_taps;
   float *m_envelope_taps;
 
}; // class SectorImageMapRF
} // namespace uslib

#endif
