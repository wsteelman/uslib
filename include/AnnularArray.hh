
#ifndef ANNULAR_ARRAY_HH_
#define ANNULAR_ARRAY_HH_

namespace uslib
{

#include "SectorTransducer.hh"
#include "Point3Df.hh"

class AnnularArray : public SectorTransducer
{
public:
   AnnularArray(uint32 element_cnt, uint32 vectors, uint32 samples,
                uint32 upsample_factor, double frequency) :
      SectorTransducer(element_cnt, vectors, samples, upsample_factor, frequency)
   {
      m_element_size.x = 1.0;
      m_element_size.y = 1.0;

      m_center.x = 0.0; 
      m_center.y = TRANSDUCER_HEIGHT + m_element_size.y/2.0;
      m_center.z = 0.0;
   
      m_top_center.x = 0.0;
      m_top_center.y = TRANSDUCER_HEIGHT + m_element_size.y;
      m_top_center.z = 0.0;
   }

   err GetElementPosition(uint32 element_index, Point3Df *loc)
   {
      if (element_index >= m_element_cnt)
      {
         return OUTOFRANGE;
      }
      double x = m_element_size.x * element_index + m_element_size.x / 2.0;
   }

private:
   Point3Df m_element_size;
   Point3Df m_center;
   Point3Df m_top_center;
      
}; // class AnnularArray
} // namespace uslib
#endif
