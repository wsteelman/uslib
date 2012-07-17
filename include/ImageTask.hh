
#ifndef IMAGE_TASK_HH_
#define IMAGE_TASK_HH_

#include "Image.hh"

namespace uslib
{
class ImageTask
{
public: 
   ImageTask() : m_next(NULL) { }
   
   ~ImageTask() { }

   virtual err Run(Frame *f) = 0;

   err SetNext(ImageTask *next)
   {
      m_next = next;
      return SUCCESS;
   }

   ImageTask *Next() const
   {
      return m_next;
   }

private:
   ImageTask *m_next;

}; // class ImageTask
} // namespace uslibk

#endif
