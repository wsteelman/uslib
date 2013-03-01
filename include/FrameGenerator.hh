
#ifndef FRAME_GENERATOR_HH_
#define FRAME_GENERATOR_HH_

#include "types.h"
#include "Image.hh"
#include "Transducer.hh"

namespace uslib
{

class FrameList;

class FrameGenerator
{
public:
   FrameGenerator(uint32 frame_cnt,
                  uint32 vectors, 
                  uint32 samples,
                  uint32 channels,
                  FrameList *free_list,
                  FrameRing *ring,
                  FocusOffsets *map,
                  Frame::data_type *data,
                  uint32 data_cnt);

   ~FrameGenerator();

   err Start();
   
   err Stop();

   void Run();

   void SetOutputRing(FrameRing *ring)
   {
      m_ring = ring;
   }

   err GenerateSingleFrame(FrameRing *output_ring);

private:
   void Generate(Frame::data_type *data, uint32 data_cnt, FocusOffsets *map);

   uint32   m_frame_idx; 
   uint32   m_frame_cnt;
   uint32   m_vectors;
   uint32   m_samples;
   uint32   m_channels; 
   Frame::data_type  *m_data;
   FrameList *m_list;
   FrameRing *m_ring;

   pthread_t m_thread;
   bool      m_running;

}; // class FrameGenerator
} // namespace uslib
#endif
