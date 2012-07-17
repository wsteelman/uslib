#ifndef POINT_3DF_HH
#define POINT_3DF_HH

#include "types.h"

class Point3Df
{
public:
   Point3Df() :
      x(0),
      y(0),
      z(0)
   {

   }

   Point3Df(double xi, double yi, double zi) :
      x(xi),
      y(yi),
      z(zi)
   {
      
   }

   ~Point3Df() { }

   double x;
   double y;
   double z;
};

#endif
