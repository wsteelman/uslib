#include "SectorImageMapFast.hh"
#include "Point3Df.hh"
#include "CoordConverter.hh"
#include "SectorTransducer.hh"
#include "Utils.hh"

#define NUM_TAPS 31

namespace uslib
{
SectorImageMapFast::SectorImageMapFast(const char *name, uint32 width, uint32 height) :
   ImageMap(name, width, height),
   m_samples(0),
   m_vectors(0)
{
   m_map = new PixelData[width*height];
   memset(m_map, 0x00, width*height*sizeof(PixelData));
   m_num_taps = 31;
}

SectorImageMapFast::~SectorImageMapFast()
{
   //for (uint32 x = 0; x < m_width*m_height; x++)
   //{
   //   delete [] m_map[x].px1_samples;
   //   delete [] m_map[x].px2_samples;
   //   delete [] m_map[x].px1_weights;
   //   delete [] m_map[x].px2_weights;
   //}
   delete m_map;
   delete m_conv;
   delete [] m_envelope_taps; 
}

err
SectorImageMapFast::CalculateMap(SectorTransducer *t, uint32 samples, uint32 vectors)
{
   m_samples = samples;
   m_vectors = vectors;

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
   uint32	s,vf, s1,s2;
	Point3Df pixel, phys, polar;
	float	r,theta,v,weight;

	float samplesPerMM = (m_samples - 1) / sp.acoustic_range;
	float vectorsPerRad = (float)(m_vectors - 1) / sp.scan_angle;

   bool valid = false;
	PixelData *px_data = m_map;
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
                 
         px_data->valid = valid; 
         px_data->weight = weight; 
         px_data->num_samples = m_num_taps;
         px_data->s1 = s1 - m_num_taps/2;
         px_data->s2 = s2 - m_num_taps/2;;
         //px_data->px1_samples = new uint32[m_num_taps];
         //px_data->px1_weights = new float[m_num_taps];
         //FillSamplesAndWeights(s1, px_data->px1_samples, px_data->px1_weights, m_num_taps);
         //px_data->px2_samples = new uint32[m_num_taps];
         //px_data->px2_weights = new float[m_num_taps];
         //FillSamplesAndWeights(s2, px_data->px2_samples, px_data->px2_weights, m_num_taps);
         px_data++; 
      }
   }
   return SUCCESS;
}

err
SectorImageMapFast::FillSamplesAndWeights(uint32 target, uint32 *samples, float *weights, uint32 size)
{
   //samples = new uint32[size];
   //weights = new float[size];
   int sn = target - size/2; 
   for (uint32 s = 0; s < size; s++)
   {
      // bounds check
      if (sn < 0 || sn > (int)(m_samples*m_vectors))
      {
         *samples++ = 0;
         *weights++ = 0.0f;
      } 
      else
      {
         *samples++ = sn;
         *weights++ = m_envelope_taps[s]; 
      } 
      sn++;
   }
   return SUCCESS;
}

float 
SectorImageMapFast::EnvelopePixel(uint32 *samples, float *weights,
                                  uint32 num_samples, float *data)
{
   float sum = 0;
   for (uint32 s = 0; s < num_samples; s++)
   {
      sum += abs(data[*samples++]) * *weights++; 
   }
   return sum;
}

float 
SectorImageMapFast::EnvelopePixelF(uint32 sample, float *data)
{
   float sum = 0;
   for (uint32 s = 0; s < m_num_taps; s++)
   {
      sum += abs(data[sample++]) * m_envelope_taps[s];
   }
   return sum;
}


err
SectorImageMapFast::Run(Frame *f)
{
   f->SetDisplaySize(m_width, m_height);
   int i;
	PixelData* map_ptr;
	PixelData pmap;
	float s1,s2;
	uint8 * bitmap_ptr;
	float pixel;
	int size = m_width * m_height;
	
   map_ptr = m_map;
	bitmap_ptr = f->GetDisplayBuffer();
   float *in = f->GetFocusBuffer();
	for(i = 0; i < size; i++)
	{
		pmap	= *(map_ptr++);
      if (!pmap.valid)
      {
         pixel = 0;
      }
      else
      {
         //s1    = EnvelopePixel(pmap.px1_samples, pmap.px1_weights, pmap.num_samples, in);
         //s2    = EnvelopePixel(pmap.px2_samples, pmap.px2_weights, pmap.num_samples, in);
         s1 = EnvelopePixelF(pmap.s1, in); 
         s2 = EnvelopePixelF(pmap.s2, in); 
         pixel	= (s2 * pmap.weight) + (s1 * (1.0-pmap.weight));
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
      *bitmap_ptr++ = (uint8)pixel;
		//*(bitmap_ptr++) = *(lut + pixel);
	}

   return SUCCESS;
}


} // namespace uslib
