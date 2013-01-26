#include "SectorImageMapRF.hh"
#include "Point3Df.hh"
#include "CoordConverter.hh"
#include "SectorTransducer.hh"
#include "Utils.hh"

#define NUM_TAPS 31

namespace uslib
{
SectorImageMapRF::SectorImageMapRF(const char *name, uint32 width, uint32 height) :
   ImageMap(name, width, height),
   m_samples(0),
   m_vectors(0)
{
   m_map = new PixelData[width*height];
   m_num_taps = 31;
}

SectorImageMapRF::~SectorImageMapRF()
{
   delete m_map;
   delete m_conv;
   delete [] m_envelope_taps; 
}

err
SectorImageMapRF::CalculateMap(SectorTransducer *t)
{
   m_samples = t->Samples();
   m_vectors = t->Vectors();

   SectorParameters sp = t->GetSectorDimensions();

   m_envelope_taps = new float[m_num_taps];
   GenTaps(m_envelope_taps, m_num_taps, 3.0f, 24.0f, true);
 
   // setup coordinate converter
   Point3Df origin;
   origin.x = -((float)m_width)/2.0f;
   origin.y = m_height;

   Point3Df rotation;
   rotation.x = PI; float scale = sp.max_radius / m_height;
   m_conv = new CoordinateConverter(origin, rotation, scale);

   // compute scan conversion map
   uint32	s,vf, ptr, s1,s2;
	Point3Df pixel, phys, polar;
	float	r,theta,v,weight;

	float samplesPerMM = (m_samples - 1) / sp.acoustic_range;
	float vectorsPerRad = (float)(m_vectors - 1) / sp.scan_angle;

   bool valid = false;
	ptr = 0;
   for(uint32 j = 0; j < m_height; j++)
   {
      for(uint32 i = 0; i < m_width; i++)
      {
         valid = false;
         pixel.x = (double)i;
         pixel.y = (double)j;
         pixel.z = 0.0;
         m_conv->FwdTransform(&pixel,&phys);
         m_conv->ConvToCylindrical(&phys,&polar);
         r = polar.x;
         theta = polar.y;

         double theta_min = (PI - sp.scan_angle)/2.0;
         double theta_max = (PI - sp.scan_angle)/2.0 + sp.scan_angle;
         if(r > sp.max_radius || 
            r < sp.min_radius || 
            theta < theta_min ||
            theta >= theta_max)
         {
         	s1 = 0;
         	s2 = 0;
         	weight = 1;
         }
         else
         {
            valid = true;
         	s = (int)round( (r-sp.xducer_height) * samplesPerMM);
         	v = (theta - theta_min) * vectorsPerRad;
         	vf = (int)floor(v);
         	s1	= (vf * m_samples) + s;
         	s2	= (v < m_vectors-1) ? ((vf+1) * m_samples) + s : 0;
         	weight = v - (float)vf;
         }
                 
         m_map[ptr].valid = valid; 
         m_map[ptr].sample1 = s1;
         m_map[ptr].sample2 = s2;
         m_map[ptr].weight = weight;
         ptr++;
      }
   }
   return SUCCESS;
}

err
SectorImageMapRF::Run(Frame *f, uint32 thread_id)
{
   f->SetDisplaySize(m_width, m_height);
   err rc = Envelope(f);
   if (rc != SUCCESS)
   {
      return rc;
   }
   return ScanConvert(f);
}


err
SectorImageMapRF::Envelope(Frame *f)
{
   float *in = f->GetFocusBuffer();
   float *out = in;
   float tmp_vector[m_samples];
   for (uint32 v = 0; v < m_vectors; v++)
   {
      // rectify the vector
      for (uint32 s = 0; s < m_samples; s++)
      {
         tmp_vector[s] = abs(*in++);
      } 
   
      // skip first few samples for the filter
      for (uint32 s = 0; s < NUM_TAPS/2; s++)
      {
         *out++ = 0.0f; 
      }
      
      uint32 index = 0;
      for (uint32 s = NUM_TAPS/2; s < m_samples - NUM_TAPS/2; s++)
      {
         float tmp = 0;
         for(uint32 t = 0; t < NUM_TAPS; t++)
         {
            tmp += tmp_vector[index+t] * m_envelope_taps[t];
         }
         if (tmp > 255.0f)
         {
            *out++ = 255.0f;
         }
         else if(tmp < 0.0f)
         {
            *out++ = 0.0f;
         }
         else
         {
            *out++ = tmp;
         }
         index++; 
      }

      // skip the last few samples
      for (uint32 s = 0; s < NUM_TAPS/2; s++)
      {
         *out++ = 0.0f;
      }
   } 
   return SUCCESS;
}

err
SectorImageMapRF::ScanConvert(Frame *f)
{
   int i;
	PixelData* map_ptr;
	PixelData pmap;
	uint8 s1,s2;
	uint8 * bitmap_ptr;
	int pixel;
	int size = m_width * m_height;
	
   map_ptr = m_map;
	bitmap_ptr = f->GetDisplayBuffer();
   float *in_float = f->GetFocusBuffer();
	for(i = 0; i < size; i++)
	{
		pmap	= *(map_ptr++);
      if (!pmap.valid)
      {
         pixel = 0;
      }
      else
      {
		   //s1		= m_focused ? (uint8)*(in_float + pmap.sample1) : *(in_int + pmap.sample1);
		   //s2		= m_focused ? (uint8)*(in_float + pmap.sample2) : *(in_int + pmap.sample2);
		   s1		= (uint8)*(in_float + pmap.sample1);
		   s2		= (uint8)*(in_float + pmap.sample2);
         pixel	= (int)((s2 * pmap.weight) + (s1 * (1.0-pmap.weight)));
		   if(pixel < 1)
		   {
		   	pixel = 0;
		   }
		   //pixel += *(tgc + (pmap.sample1 % m_samples));
	
		   if(pixel < 0)
		   {
		   	pixel = 0;
		   }
		   else if(pixel > 255)
		   {
		   	pixel = 255;
		   }
		   else
		   {
		   	// do nothing
		   }
      }
      *bitmap_ptr++ = pixel;
		//*(bitmap_ptr++) = *(lut + pixel);
	}

   return SUCCESS;
}

} // namespace uslib
