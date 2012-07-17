
#ifndef COORDINATE_CONVERTER_HH_
#define COORDINATE_CONVERTER_HH_

#include "types.h"
#include "Point3Df.hh"
#include "Matrix4.hh"

namespace uslib
{ 


class CoordinateConverter
{
 
public: 
   CoordinateConverter(Point3Df origin, Point3Df rotation, double scale);

   ~CoordinateConverter();
   
   err FwdTransform(Point3Df* src, Point3Df* dst);
   
   err RevTransform(Point3Df* src, Point3Df* dst);

   err ConvToCylindrical(Point3Df* src, Point3Df* dst);

private: 
   err Transform(Point3Df* src, Point3Df* dst, Matrix4 m);

private:
   Point3Df m_origin;
   Point3Df m_rotation;
   double   m_scale;
   Matrix4  m_fwd_matrix;
   Matrix4  m_rev_matrix;

}; // class CoordinateConverter
} // namespace uslib
#endif
