
#ifndef TRANSDUCER_HH_
#define TRANSDUCER_HH_

#include "types.h"
#include "Point3Df.hh"

namespace uslib
{

struct FocusOffsets
{
   uint32 vectors;
   uint32 samples;
   uint32 channels;
   uint32 **map;
};


class Transducer
{
public:
   Transducer(uint32 element_cnt, uint32 vectors, uint32 samples,
              uint32 upsample_factor, double frequency) :
      m_element_cnt(element_cnt),
      m_vectors(vectors),
      m_samples(samples),
      m_upsample_factor(upsample_factor),
      m_frequency(frequency)
   {
      m_focused = element_cnt > 1;
   }

   virtual ~Transducer()
   {

   }

   uint32 GetElementCount() const
   {
      return m_element_cnt;
   }

   FocusOffsets *GetFocusOffsets()
   {
      return &m_focus_map;
   }

   virtual err CalculateFocusOffsets() = 0;

   //virtual err GetElementPosition(uint32 index, Point3Df *loc) = 0;

   //virtual Point3Df ConvertSampleLocation(uint32 sample_num,
   //                                       uint32 element_index) = 0;

   //virtual uint32 ComputeSampleOffset(uint32 sample_num,
   //                                   uint32 element_index,
   //                                   uint32 vector) = 0;   

   uint32 NumElements() const
   {
      return m_element_cnt;
   }
      
   uint32 UpsampleFactor() const
   {
      return m_upsample_factor;
   }

   uint32 Vectors() const
   {
      return m_vectors;
   }

   uint32 Samples() const
   {
      return m_samples;
   }

   double Frequency() const
   {
      return m_frequency;
   }

protected:
   bool   m_focused;
   uint32 m_element_cnt;
   uint32 m_vectors;
   uint32 m_samples;
   uint32 m_upsample_factor;
  
   // physical parameters 
   double m_frequency;

   FocusOffsets m_focus_map;
       
}; // class Transducer
} // namespace uslib
#endif
