#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "ImageFactory.hh"
#include "ImageTask.hh"
#include "Image.hh"
#include "ImagePipeline.hh"

namespace uslib
{
void *ThreadStart(void *arg)
{
   ThreadArgs *args = reinterpret_cast<ThreadArgs*>(arg);
   args->factory->Run(args->id);
   pthread_exit(NULL);
}

ImageFactory::ImageFactory(uint32 ring_size) :
   m_ring_size(ring_size),
   m_next_avail_id(0),
   m_max_pipeline_stages(0),
   m_num_threads(0),
   m_running(false)
{
   memset(m_pipelines, 0x00, sizeof(m_pipelines));   
}

ImageFactory::~ImageFactory()
{
   Stop();
   for (uint32 x = 0; x < m_next_avail_id; x++)
   {
      delete m_pipelines[x];
   } 
   free(m_threads); 
   delete [] m_thread_args;
}

err
ImageFactory::InitializePipeline(uint32 num_stages, uint32 *id)
{
   if (m_next_avail_id == MAX_MAPS)
   {
      return OUTOFRANGE;
   }
   ImagePipeline *pipe = new ImagePipeline(num_stages, m_ring_size);
   m_pipelines[m_next_avail_id] = pipe;
   *id = m_next_avail_id;
   m_next_avail_id++;

   if (num_stages > m_max_pipeline_stages)
   {
      m_max_pipeline_stages = num_stages;
   }
   return SUCCESS;
}

err
ImageFactory::Start()
{
   m_running = true;
   m_num_threads = m_max_pipeline_stages;
   m_threads = (pthread_t*)malloc(m_num_threads * sizeof(pthread_t));
   m_thread_args = new ThreadArgs[m_num_threads]; 
   for (uint32 t = 0; t < m_num_threads; t++)
   {
      m_thread_args[t].id = t;
      m_thread_args[t].factory = this;
      pthread_create(&m_threads[t], 
                     (pthread_attr_t *)NULL, 
                     ThreadStart, 
                     &m_thread_args[t]);
   }
   return SUCCESS;
}

err
ImageFactory::Stop()
{
   if (m_running)
   {
      m_running = false;
      for (uint32 t = 0; t < m_num_threads; t++)
      {
         pthread_join(m_threads[t], NULL);
      }
   } 
   return SUCCESS;
}

void
ImageFactory::Run(uint32 id)
{
   while (m_running)
   {
      for (uint32 x = 0; x < m_next_avail_id; x++)
      {
         m_pipelines[x]->RunStage(id); 
      } 
   }   
}

FrameRing *
ImageFactory::GetOutputRing(uint32 map_id)
{
   if (map_id >= m_next_avail_id)
   {
      return NULL;
   }
   return m_pipelines[map_id]->GetOutputRing();
}

FrameRing *
ImageFactory::GetInputRing(uint32 map_id)
{
   if (map_id >= m_next_avail_id)
   {
      return NULL;
   }
   return m_pipelines[map_id]->GetInputRing();
}


err
ImageFactory::InsertFrame(Frame *f)
{
   uint32 id = f->GetImageMapID();
   if (m_pipelines[id] == NULL)
   {
      return OUTOFRANGE;
   }
   return m_pipelines[id]->GetInputRing()->Write(f);
}

err 
ImageFactory::ReadFrame(Frame **f, uint32 map_id)
{
   if (m_pipelines[map_id] == NULL)
   {
      return OUTOFRANGE;
   }
   return m_pipelines[map_id]->GetOutputRing()->Read(f);

}

err 
ImageFactory::AddImageTask(uint32 id, uint32 stage, ImageTask *new_task)
{
   if (id >= m_next_avail_id)
   {
      return OUTOFRANGE;
   }

   ImagePipeline *pipe = m_pipelines[id];
   return pipe->AddImageTask(new_task, stage);
}
} // namespace uslib
