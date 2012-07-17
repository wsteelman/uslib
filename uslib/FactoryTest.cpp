#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>

#include "types.h"
#include "ImageFactory.hh"
#include "FocusMapInt.hh"
#include "FocusMapFFT.hh"
#include "FocusMapSparse.hh"
#include "FocusMapSparse2.hh"
#include "SectorImageMapRF.hh"
#include "SectorImageMapFast.hh"
#include "SectorImageMapF2.hh"
#include "Utils.hh"
#include "FakeTransducer.hh"

using namespace std;
using namespace uslib;

#define MAX_RT_PRIO 90 

#define SPEED_OF_SOUND     1.54 // mm/us
#define RAW_SAMPLES        2048
#define INT_SAMPLES        2048
#define VECTORS            256
#define RAW_SIZE           (RAW_SAMPLES * VECTORS)

#define IMG_WIDTH          512
#define IMG_HEIGHT         512
#define UPSAMPLE_FACTOR    4
#define NUM_CHANNELS       6
#define FRAMES             1

long ntime(void)
/* ----| Returns a long integer scaled to milliseconds |---- */
{
    static struct timeval tp;

    gettimeofday(&tp, (struct timezone *)0);
    return (tp.tv_sec*1000 + tp.tv_usec / 1000);
}

// returns true if failed
inline bool SetThreadPrio(int prio)
{
#ifdef _POSIX_PRIORITY_SCHEDULING
   int policy = prio ? SCHED_RR : SCHED_OTHER;
   struct sched_param sch_param = {0};
   sch_param.sched_priority = (int) prio;
   if (prio > MAX_RT_PRIO)      // 2.6.36 has issues with +99s
   {
      sch_param.sched_priority = (int) MAX_RT_PRIO;
   }
   return (sched_setscheduler(0, policy, &sch_param));
#else
   return false;
#endif
}


int main(int argc, char **argv)
{
   if (argc < 2)
   {
      printf("Error image filename not specified");
      return -1;
   }

   uint8 *raw_data = new uint8[VECTORS * RAW_SAMPLES];
   //FILE *img_file = fopen(argv[1], "rb");
   //fread(raw_data, sizeof(uint8), VECTORS*SAMPLES, img_file);
   //fclose(img_file); 

   ImageFactory *factory = new ImageFactory();
   FakeTransducer *t = new FakeTransducer(NUM_CHANNELS, VECTORS, RAW_SAMPLES, UPSAMPLE_FACTOR,
                                          1.0);

   const uint32 num_maps = 5;

   FocusMapInt *fmap_int = new FocusMapInt(RAW_SAMPLES, INT_SAMPLES, VECTORS, UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_int->Calculate(t->GetFocusOffsets());
   
   FocusMapFFT *fmap_fft = new FocusMapFFT(RAW_SAMPLES, INT_SAMPLES, VECTORS, UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_fft->Calculate(t->GetFocusOffsets());
   
   FocusMapSparse *fmap_sparse0 = new FocusMapSparse(RAW_SAMPLES, INT_SAMPLES, VECTORS, UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_sparse0->Calculate(t->GetFocusOffsets());
   
   FocusMapSparse *fmap_sparse1 = new FocusMapSparse(RAW_SAMPLES, INT_SAMPLES, VECTORS, UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_sparse1->Calculate(t->GetFocusOffsets());
   
   FocusMapSparse2 *fmap_sparse2 = new FocusMapSparse2(RAW_SAMPLES, INT_SAMPLES, VECTORS, UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_sparse2->Calculate(t->GetFocusOffsets());

   //SectorImageMapRF *rf_map0 = new SectorImageMapRF("RF", IMG_WIDTH, IMG_HEIGHT);
   //rf_map0->CalculateMap(t);
   //SectorImageMapRF *rf_map1 = new SectorImageMapRF("RF", IMG_WIDTH, IMG_HEIGHT);
   //rf_map1->CalculateMap(t);
   //SectorImageMapRF *rf_map2 = new SectorImageMapRF("RF", IMG_WIDTH, IMG_HEIGHT);
   //rf_map2->CalculateMap(t);
   SectorImageMapFast *fast_map0 = new SectorImageMapFast("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map0->CalculateMap(t, INT_SAMPLES, VECTORS);
   SectorImageMapFast *fast_map1 = new SectorImageMapFast("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map1->CalculateMap(t, INT_SAMPLES, VECTORS);
   SectorImageMapFast *fast_map2 = new SectorImageMapFast("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map2->CalculateMap(t, INT_SAMPLES, VECTORS);
   SectorImageMapFast *fast_map3 = new SectorImageMapFast("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map3->CalculateMap(t, INT_SAMPLES, VECTORS);
   SectorImageMapF2 *fast_map4 = new SectorImageMapF2("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map4->CalculateMap(t, INT_SAMPLES, VECTORS);

   factory->AddImageTask(0, fmap_int);
   factory->AddImageTask(0, fast_map0);
  
   factory->AddImageTask(1, fmap_fft);
   factory->AddImageTask(1, fast_map1);

   factory->AddImageTask(2, fmap_sparse0);
   factory->AddImageTask(2, fast_map2);

   factory->AddImageTask(3, fmap_sparse1);
   factory->AddImageTask(3, fast_map3);
 
   factory->AddImageTask(4, fmap_sparse2);
   factory->AddImageTask(4, fast_map4);
 

   Frame *frame = new Frame(NUM_CHANNELS, 0, VECTORS, INT_SAMPLES, IMG_WIDTH*IMG_HEIGHT);
   uint8 **data = new uint8*[NUM_CHANNELS];
   for (uint32 x = 0; x < NUM_CHANNELS; x++)
   {
      data[x] = new uint8[RAW_SIZE];
      frame->AddChannelData(x, data[x]);
      //memcpy(data[x], raw_data, RAW_SIZE);
      GenerateFakeImage(VECTORS, RAW_SAMPLES, data[x]);
   }

   FILE *f = fopen("fake.bs", "wb");
   fwrite(data[0], sizeof(uint8), VECTORS*RAW_SAMPLES, f);
   fclose(f);


   SetThreadPrio(99);

   for (uint32 m = 0; m < num_maps; m++)
   {
      long start, end;
      frame->SetImageMapID(m);
      start = ntime();
      for (uint32 x = 0; x < FRAMES; x++)
      {  
         factory->GenerateImage(frame);
      }
      end = ntime();
      
      printf("Total time: %ld ms, %f frames/s\n", 
               end - start, 
               (double)FRAMES*1000.0/(double)(end-start));

      char filename[256];
      sprintf(filename, "img_%u.raw", m);
      FILE *f = fopen(filename, "wb");
      fwrite(frame->GetDisplayBuffer(), sizeof(uint8), 
             frame->GetDisplayWidth() * frame->GetDisplayHeight(), f);
      fclose(f);
   }

   

   for (uint32 x = 0; x < NUM_CHANNELS; x++)
   {
      delete [] data[x];
   }
   delete [] data;
   delete frame; 
   delete factory;
   delete raw_data;
   delete fmap_int;
   delete fmap_fft;
   delete fmap_sparse0;
   delete fmap_sparse1;
   return 0;
}
