#ifndef USB_PROBE_HH_
#define USB_PROBE_HH_

#include "types.h"
#include "Image.hh"
#include "Transducer.hh"

namespace uslib
{
class FrameList;

class USBProbe
{
public:
   USBProbe();

   ~USBProbe();

   err StartImagingThread();
   
   err StopImagingThread();

   err StartFrameCapture();
   
   err StopFrameCapture();

   err InitializeHW(uint32 frames);

   void SetOutputRing(FrameRing *ring)
   {
      m_ring = ring;
   }

   void SetFrameList(FrameList *list)
   {
      m_list = list;
   }

   void Run();   

   uint32 GetVectorCount() const
   {
      return m_vectors;
   }

   uint32 GetSampleCount() const
   {
      return m_samples;
   }

   uint32 GetChannelCount() const 
   {
      return m_channels;
   }

   double GetSampleRate() const
   {
      return m_sample_rate;
   }

private:
   uint32 m_vectors;
   uint32 m_samples;
   uint32 m_channels;
   double m_sample_rate;

   FrameRing *m_ring;
   FrameList *m_list;
   pthread_t m_thread;
   volatile bool m_running;
   volatile bool m_frame_capture;

}; // class USBProbe
} // namespace uslib

#endif
