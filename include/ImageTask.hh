
#ifndef IMAGE_TASK_HH_
#define IMAGE_TASK_HH_

#include "Image.hh"

namespace uslib
{
class ImageTask
{
public: 
   ImageTask() { }
   
   ~ImageTask() { }

   virtual err Run(Frame *f, uint32 thread_id) = 0;

}; // class ImageTask
} // namespace uslibk

#endif
