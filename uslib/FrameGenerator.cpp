
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "Utils.hh"
#include "FrameGenerator.hh"
#include "FrameList.hh"
#include "Transducer.hh"

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
                               FrameRing *ring,
                               FocusOffsets *map,
                               Frame::data_type *data,
                               uint32 data_cnt) :
   m_frame_idx(0),
   m_frame_cnt(frame_cnt),
   m_vectors(vectors),
   m_samples(samples),
   m_channels(channels),
   m_list(free_list),
   m_ring(ring),
   m_running(false)
{
   m_data = new Frame::data_type[frame_cnt * vectors * samples * channels]; 
   Generate(data, data_cnt, map);
}

FrameGenerator::~FrameGenerator()
{
   delete [] m_data;
   Stop();
}

void
FrameGenerator::Generate(Frame::data_type *data, uint32 data_cnt, FocusOffsets *map)
{
   Frame::data_type *buf = m_data;
   uint32 frame_size = m_vectors*m_samples;     
 
   if (data == NULL || data_cnt == 0)
   {
      for (uint32 x = 0; x < m_frame_cnt; x++)
      {
         for (uint32 c = 0; c < m_channels; c++)
         {
            GenerateFakeImage(m_vectors, m_samples, buf, x);
            buf += frame_size;
         }
      }
   }
   else
   {
      Frame::data_type *in = data;
      for (uint32 f = 0; f < m_frame_cnt; f++)
      {
         for (uint32 c = 0; c < m_channels; c++)
         {
            //UnfocusImage(in, buf, map, m_vectors, m_samples, c);
            for (uint32 s = 0; s < m_vectors*m_samples; s++)
            {
               buf[s] = in[s]; 
            }
            in += frame_size;
            if (in > (data + frame_size*data_cnt))
            {
               in = data;
            }
            buf += frame_size;
         }
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

   Frame::data_type *buf = &m_data[m_frame_idx * m_vectors * m_samples * m_channels];
   for (uint32 c = 0; c < m_channels; c++)
   { 
      f->AddChannelData(c, buf);
      buf += m_vectors * m_samples;
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
      Frame::data_type *buf = &m_data[m_frame_idx * m_vectors * m_samples * m_channels];
      for (uint32 c = 0; c < m_channels; c++)
      { 
         f->AddChannelData(c, buf);
         buf += m_vectors * m_samples;
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
