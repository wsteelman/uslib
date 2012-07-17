#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "CoordConverter.hh"

namespace uslib
{

CoordinateConverter::CoordinateConverter(Point3Df origin, Point3Df rotation, double scale) :
   m_origin(origin),
   m_rotation(rotation),
   m_scale(scale)
{
	m_fwd_matrix.Scale(m_scale,m_scale,m_scale);
	m_fwd_matrix.Translate(m_origin.x,m_origin.y,m_origin.z);
	m_fwd_matrix.Rotate(m_rotation.x,m_rotation.y,m_rotation.z);

	Point3Df zero;
	Point3Df revOrigin;
	Transform(&zero,&revOrigin,m_fwd_matrix);
	double revScale = 1.0 / m_scale;

	m_rev_matrix.Scale(revScale,revScale,revScale);
	m_rev_matrix.Rotate(-m_rotation.x,-m_rotation.y,-m_rotation.z);
	m_rev_matrix.Translate(-revOrigin.x,-revOrigin.y,-revOrigin.z);
}

CoordinateConverter::~CoordinateConverter()
{
}

err
CoordinateConverter::FwdTransform(Point3Df* src, Point3Df* dst)
{
	return Transform(src,dst,m_fwd_matrix);
}

err 
CoordinateConverter::RevTransform(Point3Df* src, Point3Df* dst)
{
	return Transform(src,dst,m_rev_matrix);
}

err
CoordinateConverter::ConvToCylindrical(Point3Df* src, Point3Df* dst)
{
	double theta;
	dst->x = sqrt(src->x*src->x + src->y*src->y);
	theta = atan(src->y / src->x);
	dst->y = (theta < 0) ? (PI + theta) : theta;
	dst->z = src->z;
   return SUCCESS;
}

err 
CoordinateConverter::Transform(Point3Df* src, Point3Df* dst, Matrix4 m)
{
	dst->x = (src->x*m[0][0]) + (src->y*m[0][1]) + (src->z*m[0][2]) + m[0][3];
	dst->y = (src->x*m[1][0]) + (src->y*m[1][1]) + (src->z*m[1][2]) + m[1][3];
	dst->z = (src->x*m[2][0]) + (src->y*m[2][1]) + (src->z*m[2][2]) + m[2][3];
   return SUCCESS;
}

} // namespace uslib
