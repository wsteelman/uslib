
#ifndef IMAGE_PIPELINE_HH_
#define IMAGE_PIPELINE_HH_

#include "Image.hh"
#include <list>

namespace uslib
{
class ImageTask;

typedef std::list<ImageTask*> TaskList;

class PipelineStage
{
public:
   PipelineStage(uint32 id, FrameRing *input_ring, FrameRing *output_ring);

   ~PipelineStage();

   err AddImageTask(ImageTask *task);

   err Run();

private:
   uint32     m_id;
   FrameRing *m_input_ring;
   FrameRing *m_output_ring;
   TaskList m_tasks;

}; class PipelineStage;

class ImagePipeline
{
public:
   ImagePipeline(uint32 num_stages, uint32 ring_size);

   ~ImagePipeline();

   err AddImageTask(ImageTask *task, uint32 stage);

   err RunStage(uint32 id);

   FrameRing *GetInputRing()
   {
      return m_rings[0];
   }

   FrameRing *GetOutputRing()
   {
      return m_rings[m_num_rings-1];
   }

   uint32 GetNumStages() const
   {
      return m_num_stages;
   }

private:  
   uint32                  m_num_stages;
   uint32                  m_num_rings;
   uint32                  m_ring_size;
   PipelineStage         **m_stages;
   FrameRing             **m_rings;
    
}; // class ImagePipeline
} // namespace uslib

#endif
