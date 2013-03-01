
#ifndef IMAGE_HH_
#define IMAGE_HH_

#include <stdlib.h>
#include "types.h"
#include "PointerRing.hh"

namespace uslib
{
template <class T>
class Frame_t;

typedef Frame_t<RAW_TYPE> Frame;
typedef PointerRing<Frame> FrameRing;

//template <class T>
//struct FrameRing
//{
//   typedef PointerRing<Frame<T> > type;
//}

template <class T>
class Frame_t
{
public:
   typedef T data_type;
   
   Frame_t(uint32 id, uint32 num_channels, uint32 image_map_id,
         uint32 vectors, uint32 samples,
         uint32 max_display_size) :
      m_id(id),
      m_num_channels(num_channels),
      m_map_id(image_map_id),
      m_vectors(vectors),
      m_samples(samples),
      m_width(0),
      m_height(0),
      m_max_display_size(max_display_size),
      m_display(NULL),
      m_focused(NULL)
   {
      m_channels = new data_type*[num_channels];
      m_focused = new float[m_samples*m_vectors];
      m_display = new uint8[m_max_display_size];
   }

   ~Frame_t()
   {
      delete [] m_channels;
      delete [] m_display;
   }


   err AddChannelData(uint32 chnl, T *data)
   {
      if (chnl < m_num_channels)
      {
         m_channels[chnl] = data;
         return SUCCESS; 
      }
      return OUTOFRANGE;
   }

   void SetDisplaySize(uint32 width, uint32 height)
   {
      m_width = width;
      m_height = height;
   }

   void SetImageMapID(uint32 id)
   {
      m_map_id = id;
   }

   uint32 GetFrameID() const
   {
      return m_id;
   }

   uint32 GetImageMapID() const
   {
      return m_map_id;
   }

   uint32 GetDisplayWidth() const
   {
      return m_width;
   }

   uint32 GetDisplayHeight() const
   {
      return m_height;
   }

   uint8 *GetDisplayBuffer() const
   {
      return m_display;
   }

   float *GetFocusBuffer() const
   {
      return m_focused;
   }

   T *GetChannelData(uint32 chnl) const
   {
      if (chnl < m_num_channels)
      {
         return m_channels[chnl];
      }     
      return NULL;
   }

   T **GetChannelData() const
   {
      return m_channels;
   }

   uint32 GetNumChannels() const
   {
      return m_num_channels;
   }

private:
   uint32   m_id;
   uint32   m_num_channels;
   uint32   m_map_id;
   uint32   m_vectors;
   uint32   m_samples;
   uint32   m_width;
   uint32   m_height;
   T      **m_channels;
   uint32   m_max_display_size;
   uint8   *m_display;
   float   *m_focused;

}; // class Image
} // namespace uslib
#endif
