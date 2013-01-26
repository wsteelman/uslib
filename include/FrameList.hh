
#ifndef FRAME_LIST_HH_
#define FRAME_LIST_HH_

#include <stdlib.h>

#include "types.h"
#include "Image.hh"
#include "FreeList.hh"

namespace uslib
{
class FrameList
{
public:
   FrameList(uint32 size,
             uint32 channels,
             uint32 vectors,
             uint32 samples,
             uint32 map_id,
             uint32 display_size):
      m_size(size),
      m_channels(channels),
      m_vectors(vectors),
      m_samples(samples),
      m_map_id(map_id),
      m_display_size(display_size),
      m_next_id(0)
   {
      for (uint32 x = 0; x < size; x++)
      {
         CreateNewFrame();
      }
   }

   ~FrameList()
   {
      while (!m_free_list.empty())
      {
         Frame *f = m_free_list.get();
         if (f != NULL)
         { 
            delete f;
         }
      }
   }

   inline void CreateNewFrame()
   {
      Frame *f = new Frame(m_next_id, m_channels, m_map_id, m_vectors, m_samples, m_display_size);
      m_free_list.replace(f); 
      m_next_id++;
   }

   Frame *GetFrame()
   {
      if (m_free_list.empty())
      {
         return NULL;
         //CreateNewFrame(); 
      }
      return m_free_list.get();  
   }  

   void ReplaceFrame(Frame *f)
   {
      m_free_list.replace(f);
   }

   uint32 GetCount()
   {
      return m_free_list.size();
   } 

private:
   FreeList<Frame> m_free_list;
   uint32 m_size;
   uint32 m_channels;
   uint32 m_vectors;
   uint32 m_samples;
   uint32 m_map_id;
   uint32 m_display_size;
   uint32 m_next_id;

}; // class FrameList
} // namespace uslib
#endif
