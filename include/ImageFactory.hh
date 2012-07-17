
#ifndef IMAGE_FACTORY_HH_
#define IMAGE_FACTORY_HH_

#include "types.h"

#define MAX_MAPS 64

namespace uslib
{

class Frame;
class ImageTask;

class ImageFactory
{

public:
   ImageFactory();

   ~ImageFactory(); 

   err GenerateImage(Frame *f);

   err AddImageTask(uint32 map_id, ImageTask *new_task);

private:
   ImageTask *m_tasks[MAX_MAPS];

}; // class ImageFactory
} // namespace uslib

#endif
