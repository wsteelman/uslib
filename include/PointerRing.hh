

#ifndef FRAME_RING_HH_
#define FRAME_RING_HH_

#include "types.h"

namespace uslib
{

template <class T>
class PointerRing 
{
public:
   PointerRing(uint32 size) :
      m_size(size),
      m_read_idx(size-1),
      m_write_idx(0)
   {
      m_pointers = new T*[size];
      memset(m_pointers, 0x00, sizeof(m_pointers));
   }

   ~PointerRing()
   {
      delete [] m_pointers;
   }

   err Read(T **item)
   {
      // make local copy of volatile 
      uint32 nxt_idx = NextIndex(m_read_idx);
      uint32 write_idx = m_write_idx;
      if (nxt_idx == write_idx)
      {
         return NOMEM;
      }  
 
      *item = m_pointers[nxt_idx]; 
      m_read_idx = NextIndex(m_read_idx);
      return SUCCESS;

   }

   err Write(T *item)
   {
      uint32 write_idx = m_write_idx;
      uint32 nxt_idx = NextIndex(write_idx);
      if (nxt_idx == m_read_idx)
      {
         return NOMEM; 
      }
      
      m_pointers[write_idx] = item;
      m_write_idx = nxt_idx;
      return SUCCESS;
   }
   

private:
   inline uint32 NextIndex(uint32 idx)
   {
      return (idx+1) % m_size;
   }

   T **m_pointers;
   uint32  m_size;
   volatile uint32  m_read_idx;
   volatile uint32  m_write_idx; 

}; // class FrameRing
} // namespace uslib

#endif
