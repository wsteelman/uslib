
#ifndef IMAGE_FACTORY_HH_
#define IMAGE_FACTORY_HH_

#include "types.h"
#include "Image.hh"

#define MAX_MAPS 64

namespace uslib
{

class Frame;
class ImageTask;
class ImageFactory;
class ImagePipeline;

struct ThreadArgs
{
   uint32 id;
   ImageFactory *factory;   
};

class ImageFactory
{

public:
   ImageFactory(uint32 ring_size);

   ~ImageFactory(); 

   err Start();

   err Stop();
   
   void Run(uint32 id);

   err InitializePipeline(uint32 num_stages, uint32 *id);

   err AddImageTask(uint32 pipeline_id, uint32 stage, ImageTask *new_task);

   err InsertFrame(Frame *f);

   err ReadFrame(Frame **f, uint32 map_id);

   FrameRing *GetOutputRing(uint32 map_id);

   FrameRing *GetInputRing(uint32 map_id);

private:
   uint32         m_ring_size; 
   uint32         m_next_avail_id;
   ImagePipeline *m_pipelines[MAX_MAPS]; 
   uint32         m_max_pipeline_stages;
   uint32         m_num_threads; 
   pthread_t     *m_threads;
   ThreadArgs    *m_thread_args;
   volatile bool  m_running;

}; // class ImageFactory
} // namespace uslib

#endif
