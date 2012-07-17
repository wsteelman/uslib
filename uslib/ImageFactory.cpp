#include "ImageFactory.hh"
#include "ImageTask.hh"
#include "Image.hh"
#include <string.h>

namespace uslib
{
ImageFactory::ImageFactory()
{
   memset(m_tasks, 0x00, sizeof(m_tasks));
}

ImageFactory::~ImageFactory()
{

}

err
ImageFactory::GenerateImage(Frame *f)
{
   ImageTask *task = m_tasks[f->GetImageMapID()];
   if (task == NULL)
   {
      return OUTOFRANGE;
   }
   err rc = 0;
   while (task != NULL)
   {
      rc = task->Run(f);
      if (rc != SUCCESS)
      {
         return rc;
      }
      task = task->Next(); 
   } 
   return SUCCESS;
}


err 
ImageFactory::AddImageTask(uint32 map_id, ImageTask *new_task)
{
   if (map_id >= MAX_MAPS)
   {
      return OUTOFRANGE;
   }

   ImageTask *task = m_tasks[map_id];
   if (task == NULL)
   {
      m_tasks[map_id] = new_task; 
   }
   else
   {
      // insert at the end of the list
      while (task->Next() != NULL)
      {
         task = task->Next();
      } 
      task->SetNext(new_task);
   } 
   return SUCCESS;
}

} // namespace uslib
