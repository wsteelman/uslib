
#ifndef IMAGE_MAP_HH_
#define IMAGE_MAP_HH_

#include <string.h>

#include "types.h"
#include "Image.hh"
#include "ImageTask.hh"

namespace uslib
{

class ImageMap : public ImageTask
{
public:
   ImageMap(const char *name, uint32 width, uint32 height) :
      m_width(width),
      m_height(height)
   {
      strncpy(m_name, name, 128);
   }

   virtual ~ImageMap() { }

   const char *GetName() const
   {
      return m_name;
   }

protected:
   char     m_name[128];
   uint32   m_width;
   uint32   m_height;  
 
}; // class ImageMap
} // namespace uslib

#endif
