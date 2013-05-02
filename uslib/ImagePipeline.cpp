
#include "types.h"
#include "Image.hh"
#include "ImagePipeline.hh"
#include "ImageTask.hh"

namespace uslib
{

PipelineStage::PipelineStage(uint32 id, FrameRing *input_ring, FrameRing *output_ring) :
   m_id(id), 
   m_input_ring(input_ring),
   m_output_ring(output_ring)
{

}

PipelineStage::~PipelineStage()
{

}

err 
PipelineStage::AddImageTask(ImageTask *new_task)
{
   m_tasks.push_back(new_task);
   return SUCCESS;
}

err
PipelineStage::Run()
{
   Frame *f = NULL;
   err rc = m_input_ring->Read(&f);
   if (rc != SUCCESS)
   {
      return rc;
   }

   TaskList::iterator it;
   for (it = m_tasks.begin(); it != m_tasks.end(); ++it)
   {
      err rc = (*it)->Run(f, m_id);
      if (rc != SUCCESS)
      {
         return rc;
      }
   }
 
   while (m_output_ring->Write(f) != SUCCESS)
   {
      // block
   } 
   return SUCCESS;
}

/* ------------------------------------------------------------------- */
ImagePipeline::ImagePipeline(uint32 num_stages, uint32 ring_size) :
   m_num_stages(num_stages),
   m_num_rings(num_stages+1),
   m_ring_size(ring_size)
{
   m_rings = new FrameRing*[m_num_rings]; 
   for (uint32 x = 0; x < m_num_stages+1; x++)
   {
      m_rings[x] = new FrameRing(ring_size); 
   }

   m_stages = new PipelineStage*[num_stages];
   for (uint32 x = 0; x < num_stages; x++)
   {
      m_stages[x] = new PipelineStage(x, m_rings[x], m_rings[x+1]);
   }
  
}

ImagePipeline::~ImagePipeline()
{
   for (uint32 x = 0; x < m_num_stages; x++)
   {
      delete m_stages[x];
   } 
   delete [] m_stages;

   for (uint32 x = 0; x < m_num_rings; x++)
   {
      delete m_rings[x];
   } 
   delete [] m_rings;
}

err 
ImagePipeline::AddImageTask(ImageTask *task, uint32 stage)
{
   if (stage >= m_num_stages)
   {
      return OUTOFRANGE;
   }
   return m_stages[stage]->AddImageTask(task);
}

err
ImagePipeline::RunStage(uint32 id)
{
   if (id >= m_num_stages)
   {
      return OUTOFRANGE;
   }
   return m_stages[id]->Run();
}
 
} // namespace uslib
