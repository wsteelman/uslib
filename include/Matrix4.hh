
#ifndef MATRIX4_HH_
#define MATRIX4_HH_

#include <math.h>
#include <string.h>

namespace uslib
{
class Matrix4
{
   struct Row4
   {
      double m_data[4];

      Row4()
      {
         memset(m_data, 0x00, sizeof(m_data));
      }

      double& operator[] (int x)
      {
         return m_data[x];
      }
   };

public:
   Matrix4()
   {
	   for(int i = 0; i < 4; i++)
	   {
         m_data[i][0] = 0.0;
         m_data[i][1] = 0.0;
         m_data[i][2] = 0.0;
         m_data[i][3] = 0.0;
	   }
	   m_data[0][0] = 1.0;
	   m_data[1][1] = 1.0;
	   m_data[2][2] = 1.0;
	   m_data[3][3] = 1.0;
   }

   ~Matrix4() { }

   Row4& operator[] (int x) 
   {
      return m_data[x];
   }

   void Multiply(Matrix4 m)
   {
      int i,j;
      Row4 tmp[4];
      for(j = 0; j < 4; j++)
      {
         for(i = 0; i < 4; i++)
         {
            tmp[j][i] = m_data[j][0]*m[0][i] + m_data[j][1]*m[1][i] + 
                        m_data[j][2]*m[2][i] + m_data[j][3]*m[3][i];
         }
      } 
      memcpy(m_data, tmp, sizeof(m_data));   
   }

   void Translate(double x, double y, double z)
   {
      Matrix4 trans;
      trans[0][3] = x;
	   trans[1][3] = y;
	   trans[2][3] = z;
	   Multiply(trans);
   }

   void Scale(double scaleX, double scaleY, double scaleZ)
   {
   	Matrix4 scale;
   	scale[0][0] = scaleX;
   	scale[1][1] = scaleY;
   	scale[2][2] = scaleZ;
   	Multiply(scale);
   }

   void Rotate( double x, double y, double z)
   {
   	Matrix4 rx;
   	Matrix4 ry;
   	Matrix4 rz;
   
   	rx[1][1] = cos(x);
   	rx[2][1] = sin(x);
   	rx[1][2] = -sin(x);
   	rx[2][2] = cos(x);
   
   	ry[0][0] = cos(y);
   	ry[2][0] = -sin(y);
   	ry[0][2] = sin(y);
   	ry[2][2] = cos(y);
   
   	rz[0][0] = cos(z);
   	rz[1][0] = sin(z);
   	rz[0][1] = -sin(z);
   	rz[1][1] = cos(z);
   
   	Multiply(rx);
   	Multiply(ry);
   	Multiply(rz);
   }

private:
   Row4 m_data[4];
}; // class Matrix4
} // namespace uslib

#endif;
