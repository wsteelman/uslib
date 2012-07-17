
#ifndef IMAGE_HH_
#define IMAGE_HH_

#include <stdlib.h>
#include "types.h"

namespace uslib
{
class Frame
{
public:
   Frame(uint32 num_channels, uint32 image_map_id,
         uint32 vectors, uint32 samples,
         uint32 max_display_size);

   ~Frame();

   err AddChannelData(uint32 chnl, uint8 *data);

   void SetDisplaySize(uint32 width, uint32 height)
   {
      m_width = width;
      m_height = height;
   }

   void SetImageMapID(uint32 id)
   {
      m_map_id = id;
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

   uint8 *GetChannelData(uint32 chnl) const
   {
      if (chnl < m_num_channels)
      {
         return m_channels[chnl];
      }     
      return NULL;
   }

   uint8 **GetChannelData() const
   {
      return m_channels;
   }

   uint32 GetNumChannels() const
   {
      return m_num_channels;
   }

private:
   uint32   m_num_channels;
   uint32   m_map_id;
   uint32   m_vectors;
   uint32   m_samples;
   uint32   m_width;
   uint32   m_height;
   uint8  **m_channels;
   uint32   m_max_display_size;
   uint8   *m_display;
   float   *m_focused;

}; // class Image
} // namespace uslib
#endif
