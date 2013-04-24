
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "Utils.hh"
#include "USBProbe.hh"
#include "FrameList.hh"
#include "Transducer.hh"
#include "usapi.h"

namespace uslib
{
void *USBThreadStart(void *arg)
{
   USBProbe *probe = reinterpret_cast<USBProbe*>(arg);
   probe->Run();
   pthread_exit(NULL);
}


USBProbe::USBProbe() :
   m_vectors(0),
   m_samples(0),
   m_channels(0),
   m_sample_rate(0.0),
   m_ring(NULL),
   m_list(NULL),
   m_running(false),
   m_frame_capture(false)
{

}

USBProbe::~USBProbe()
{
   StopFrameCapture();
   StopImagingThread();
   Release();
}

err 
USBProbe::InitializeHW(uint32 frames)
{
   result_code rc = Initialize(frames);
   if (rc != USB_SUCCESS)
   {
      printf("Unable to open USB device, code = 0x%x\n", rc);
      return ERROR;
   }
   m_vectors = GetVectorCount();
   m_samples = GetSampleCount();
   m_channels = GetChannelCount();
   m_sample_rate = GetSamplingRate(); 

   return SUCCESS;   
}

err
USBProbe::StartImagingThread()
{
   m_running = true;
   pthread_create(&m_thread, (pthread_attr_t *)NULL, USBThreadStart, this);
   return SUCCESS;
}

err
USBProbe::StopImagingThread()
{
   if (m_running)
   {
      m_running = false;
      pthread_join(m_thread, NULL);
   } 
   return SUCCESS;
}

err 
USBProbe::StartFrameCapture()
{
   Start();
   m_frame_capture = true;
   return SUCCESS;
}


err 
USBProbe::StopFrameCapture()
{
   Stop();
   m_frame_capture = false;
   return SUCCESS;
}


void
USBProbe::Run()
{
   if (m_list == NULL || m_ring == NULL)
   {
      return;
   }


   while (m_running)
   {
      if (m_frame_capture)
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
         result_code code = GetNextFrame(f->GetChannelData());
         if (code != USB_SUCCESS)
         {
            m_list->ReplaceFrame(f);
            continue;
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
      }
      else
      {
         sleep(1);
      }
   }
}

} // namespace uslib
