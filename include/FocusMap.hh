
#ifndef FOCUS_MAP_HH_
#define FOCUS_MAP_HH_

#include <string.h>

#include "types.h"
#include "Image.hh"
#include "ImageTask.hh"
#include "Transducer.hh"

namespace uslib
{

class FocusMap : public ImageTask
{
public:
   FocusMap(const char *name, uint32 in_samples, 
            uint32 out_samples, uint32 vectors, 
            uint32 upsample_factor, uint32 channels) :
      m_in_samples(in_samples),
      m_out_samples(out_samples),
      m_vectors(vectors),
      m_upsample_factor(upsample_factor),
      m_channels(channels)
   {
      m_total_in_samples = m_vectors*m_in_samples;
      m_total_out_samples = m_vectors*m_out_samples;
      m_out_upsample_factor = m_in_samples * m_upsample_factor / m_out_samples;
      strncpy(m_name, name, 128);
   }

   virtual ~FocusMap() { }

   virtual err Calculate(FocusOffsets *offsets) = 0;

   const char *GetName() const
   {
      return m_name;
   }

protected:
   char     m_name[128];
   uint32   m_in_samples;
   uint32   m_out_samples;
   uint32   m_vectors;
   uint32   m_upsample_factor;
   uint32   m_out_upsample_factor;
   uint32   m_channels;
   uint32   m_total_out_samples;
   uint32   m_total_in_samples;
 
}; // class ImageMap
} // namespace uslib

#endif
