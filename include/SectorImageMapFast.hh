
#ifndef SECTOR_IMAGE_MAP_FAST_HH_
#define SECTOR_IMAGE_MAP_FAST_HH_

#include "types.h"
#include "ImageMap.hh"
#include <fftw3.h>

namespace uslib
{

class CoordinateConverter;
class SectorTransducer;
class FocusMap;

class SectorImageMapFast : public ImageMap
{
   struct PixelData
   {
      bool valid;
      uint32 s1;
      uint32 s2;
      //uint32 *px1_samples;
      //uint32 *px2_samples;
      //float *px1_weights;
      //float *px2_weights;
      uint32 num_samples;
      float weight; 
   };

public:
   SectorImageMapFast(const char *name, uint32 width, uint32 height);
 
   ~SectorImageMapFast();

   err Run(Frame *f);

   err CalculateMap(SectorTransducer *t, uint32 samples, uint32 vectors);

private:
   err ScanConvert(Frame *f);

   err FillSamplesAndWeights(uint32 target, uint32 *samples, float *weights, uint32 size);

   inline float EnvelopePixel(uint32 *samples, float *weights,
                              uint32 num_samples, float *data);


   inline float EnvelopePixelF(uint32 sample, float *data);

   PixelData *m_map; 
   CoordinateConverter *m_conv;
   uint32 m_samples;
   uint32 m_vectors;
   uint32 m_num_taps;
   float *m_envelope_taps;
 
}; // class SectorImageMapFast
} // namespace uslib

#endif
