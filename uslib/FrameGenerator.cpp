
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "Utils.hh"
#include "FrameGenerator.hh"
#include "FrameList.hh"

namespace uslib
{
void *GeneratorThreadStart(void *arg)
{
   FrameGenerator *gen = reinterpret_cast<FrameGenerator*>(arg);
   gen->Run();
   pthread_exit(NULL);
}


FrameGenerator::FrameGenerator(uint32 frame_cnt, 
                               uint32 vectors, 
                               uint32 samples,
                               uint32 channels,
                               FrameList *free_list,
                               FrameRing *ring) :
   m_frame_idx(0),
   m_frame_cnt(frame_cnt),
   m_vectors(vectors),
   m_samples(samples),
   m_channels(channels),
   m_list(free_list),
   m_ring(ring),
   m_running(false)
{
   m_data = new uint8[frame_cnt * vectors * samples * channels]; 
   Generate();
}

FrameGenerator::~FrameGenerator()
{
   delete [] m_data;
   Stop();
}

void
FrameGenerator::Generate()
{
   uint8 *buf = m_data;
   for (uint32 x = 0; x < m_frame_cnt; x++)
   {
      for (uint32 c = 0; c < m_channels; c++)
      {
         GenerateFakeImage(m_vectors, m_samples, buf, x);
         buf += m_vectors * m_samples * sizeof(uint8);
      }
   }
}

err
FrameGenerator::Start()
{
   m_running = true;
   pthread_create(&m_thread, (pthread_attr_t *)NULL, GeneratorThreadStart, this);
   return SUCCESS;
}

err
FrameGenerator::Stop()
{
   if (m_running)
   {
      m_running = false;
      pthread_join(m_thread, NULL);
   } 
   return SUCCESS;
}

err 
FrameGenerator::GenerateSingleFrame(FrameRing *output_ring)
{
   Frame *f = m_list->GetFrame();
   while (f == NULL)
   {
      f = m_list->GetFrame(); 
   }

   uint8 *buf = &m_data[m_frame_idx * m_vectors * m_samples * m_channels];
   for (uint32 c = 0; c < m_channels; c++)
   { 
      f->AddChannelData(c, buf);
      buf += m_vectors * m_samples * sizeof(uint8);
   }
   
   err rc = output_ring->Write(f);
   while (rc == NOMEM)
   {
      rc = output_ring->Write(f);
   }
   m_frame_idx = (m_frame_idx + 1) % m_frame_cnt;  
   return SUCCESS;
}

void
FrameGenerator::Run()
{
   while (m_running)
   {
      Frame *f = m_list->GetFrame();
      while (f == NULL)
      {
         if (!m_running)
         {
            return;
         }
         f = m_list->GetFrame(); 
      }
      uint8 *buf = &m_data[m_frame_idx * m_vectors * m_samples * m_channels];
      for (uint32 c = 0; c < m_channels; c++)
      { 
         f->AddChannelData(c, buf);
         buf += m_vectors * m_samples * sizeof(uint8);
      }
      
      err rc = m_ring->Write(f);
      while (rc == NOMEM)
      {
         if (!m_running)
         {
            m_list->ReplaceFrame(f);
            return;
         }
         rc = m_ring->Write(f);
      }
      m_frame_idx = (m_frame_idx + 1) % m_frame_cnt;   
   }
}

} // namespace uslib
