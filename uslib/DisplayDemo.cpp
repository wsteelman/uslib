#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <GLUT/glut.h>

#include "types.h"
#include "Image.hh"
#include "ImageFactory.hh"
#include "FrameList.hh"
#include "FrameGenerator.hh"
#include "FocusMapInt.hh"
#include "FocusMapFFT.hh"
#include "FocusMapSparse.hh"
#include "FocusMapSparse2.hh"
#include "FocusMapSparseMulti.hh"
#include "FocusMapSparseMulti2.hh"
#include "SectorImageMapRF.hh"
#include "SectorImageMapFast.hh"
#include "SectorImageMapF2.hh"
#include "Utils.hh"
#include "FakeTransducer.hh"
#include "AnnularFlatTransducer.hh"
#include "perf.hh"

using namespace std;
using namespace uslib;

#define MAX_RT_PRIO 90 

#define PERF_TEST          1
#define SUB_CNT            10
#define SPEED_OF_SOUND     1.54 // mm/us
//#define RAW_SAMPLES        8192
//#define INT_SAMPLES        2048
//#define VECTORS            256
//#define RAW_SIZE           (RAW_SAMPLES * VECTORS)

#define IMG_WIDTH          512
#define IMG_HEIGHT         512
//#define UPSAMPLE_FACTOR    4
//#define NUM_CHANNELS       6
//#define FRAMES             32
//#define CHNL_BUFFERS       (NUM_CHANNELS * FRAMES)

uint32  g_FFT_PIPELINE;
uint32  g_SPARSE_PIPELINE;
uint32  g_SPARSE_STAGED_PIPELINE;

// globals
ImageFactory   *g_factory;
FrameGenerator *g_generator;
FrameList      *g_free_list;
FrameRing      *g_output_ring;
uint32          g_perf_cnt;
int             g_timebase;
float           g_fps;
char            g_fps_char[16];

// returns true if failed
//inline bool SetThreadPrio(int prio)
//{
//#ifdef _POSIX_PRIORITY_SCHEDULING
//   int policy = prio ? SCHED_RR : SCHED_OTHER;
//   struct sched_param sch_param = {0};
//   sch_param.sched_priority = (int) prio;
//   if (prio > MAX_RT_PRIO)      // 2.6.36 has issues with +99s
//   {
//      sch_param.sched_priority = (int) MAX_RT_PRIO;
//   }
//   return (sched_setscheduler(0, policy, &sch_param));
//#else
//   return false;
//#endif
//}

long ntime(void)
/* ----| Returns a long integer scaled to milliseconds |---- */
{
    static struct timeval tp;

    gettimeofday(&tp, (struct timezone *)0);
    return (tp.tv_sec*1000 + tp.tv_usec / 1000);
}

void drawBitmapText(char *s, float x, float y) 
{  
   glColor3f(1.0f, 1.0f, 1.0f);
   glWindowPos2i(x,y);

   glLoadIdentity();
   glTranslatef(0.0f, 0.0f, 1.0f);

   while (*s != '\0') 
   {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);
      s++; 
   }
}

void ImageGen()
{
   Frame *f = NULL;
   err rc = g_output_ring->Read(&f);
   if (rc != SUCCESS)
   {
      return;
   }

   glWindowPos2i(0,0);
   glDrawPixels(f->GetDisplayWidth(), 
                f->GetDisplayHeight(), 
                GL_LUMINANCE, 
                GL_UNSIGNED_BYTE, 
                f->GetDisplayBuffer());
   g_perf_cnt++;
   int time = glutGet(GLUT_ELAPSED_TIME);
   if (time - g_timebase > 2000)
   {
      g_fps = (float)g_perf_cnt*1000.0f/(float)(time-g_timebase);
      g_timebase = time;
      g_perf_cnt = 0;
   }
   sprintf(g_fps_char, "FPS: %2.2f", g_fps);
   drawBitmapText(g_fps_char, 10.0f, 10.0f);
   
   glutSwapBuffers();
   g_free_list->ReplaceFrame(f);
}



void display(void)
{
   ImageGen();
}

void reshape(int w, int h)
{
  //wnd_width = w;
  //wnd_height = h;

  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLfloat) w / (GLfloat) h, 0.01, 500.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //glTranslatef(trans.x, trans.y, trans.z);
}

void keyboard(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 27:
      exit(0);
      break;

    default:
      break;
  }
  //reshape(wnd_width, wnd_height);
  display();
}

void specialkeys(int key, int x, int y)
{
  switch(key)
  {
    default:
      break;
  }
  //reshape(wnd_width, wnd_height);
  display();
}

void mouse(int button, int state, int x, int y)
{

}

void SaveFrame(const char *filename, uint32 pipeline)
{
   // stop generator and drain
   g_generator->Stop();
   // draing queue
   Frame *f = NULL;
   while (g_free_list->GetCount() < 100)
   {
      err rc = g_output_ring->Read(&f);
      if (rc == SUCCESS)
      {
         g_free_list->ReplaceFrame(f);
      }
   }

   FrameRing *input_ring = g_factory->GetInputRing(pipeline);
   FrameRing *output_ring = g_factory->GetOutputRing(pipeline);

   FILE *file = fopen(filename, "wb");
   if (file == NULL)
   {
      return;
   }

   g_generator->GenerateSingleFrame(input_ring);
   f = NULL;
   err rc = output_ring->Read(&f);
   while (rc != SUCCESS)
   {
      rc = output_ring->Read(&f);
   }
   fwrite(f->GetDisplayBuffer(), sizeof(uint8),
          f->GetDisplayWidth() * f->GetDisplayHeight(), file);
   fclose(file);
   g_free_list->ReplaceFrame(f);
}

void RunPerformanceTest(uint32 pipeline, uint32 num_frames, double *results)
{
   // stop generator and drain
   g_generator->Stop();
   // draing queue
   Frame *f = NULL;
   while (g_free_list->GetCount() < 100)
   {
      err rc = g_output_ring->Read(&f);
      if (rc == SUCCESS)
      {
         g_free_list->ReplaceFrame(f);
      }
   }

   FrameRing *input_ring = g_factory->GetInputRing(pipeline);
   FrameRing *output_ring = g_factory->GetOutputRing(pipeline);

   // setup test
   memset(results, 0x00, num_frames*sizeof(float)); 
   long start, end;

   // collect samples 
   for (uint32 x = 0; x < num_frames; x++)
   { 
      start = ntime();
      for (uint32 y = 0; y < SUB_CNT; y++)
      {
         g_generator->GenerateSingleFrame(input_ring);
         Frame *f = NULL;
         err rc = output_ring->Read(&f);
         while (rc != SUCCESS)
         {
            rc = output_ring->Read(&f);
         }
         g_free_list->ReplaceFrame(f);
      }
      end = ntime();
      results[x] = (end - start) / (double) SUB_CNT;
   }
 
}

void ComputeStats(double *data, uint32 count, double *mean, double *std)
{
   double tmp = 0;
   for (uint32 x = 0; x < count; x++)
   {
      tmp += (double)data[x]; 
   } 
   *mean = tmp / count;

   // std
   tmp = 0;
   for (uint32 x = 0; x < count; x++)
   {
      tmp += pow(data[x] - *mean, 2.0); 
   }
   *std = sqrt(tmp/(double)count);
   

}

void InitPipelines(SectorTransducer *t, uint32 RAW_SAMPLES, uint32 INT_SAMPLES, uint32 VECTORS,
                   uint32 NUM_CHANNELS, uint32 UPSAMPLE_FACTOR)
{

   FocusMapInt *fmap_int = new FocusMapInt(RAW_SAMPLES, INT_SAMPLES, VECTORS, 
                                           UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_int->Calculate(t->GetFocusOffsets());
   
   FocusMapFFT *fmap_fft = new FocusMapFFT(RAW_SAMPLES, INT_SAMPLES, VECTORS, 
                                           UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_fft->Calculate(t->GetFocusOffsets());
     
   FocusMapSparse2 *fmap_sparse2 = new FocusMapSparse2(RAW_SAMPLES, INT_SAMPLES, VECTORS, 
                                                       UPSAMPLE_FACTOR, NUM_CHANNELS);
   fmap_sparse2->Calculate(t->GetFocusOffsets());

   FocusMapSparseMulti *fmap_sparse_4stage = new FocusMapSparseMulti(RAW_SAMPLES, INT_SAMPLES, VECTORS, 
                                                       UPSAMPLE_FACTOR, NUM_CHANNELS, 4);
   fmap_sparse_4stage->Calculate(t->GetFocusOffsets());

   FocusMapSparseMulti *fmap_sparse_3stage = new FocusMapSparseMulti(RAW_SAMPLES, INT_SAMPLES, VECTORS, 
                                                       UPSAMPLE_FACTOR, NUM_CHANNELS, 3);
   fmap_sparse_3stage->Calculate(t->GetFocusOffsets());

   SectorImageMapFast *fast_map0 = new SectorImageMapFast("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map0->CalculateMap(t, INT_SAMPLES, VECTORS);
   SectorImageMapF2 *fast_map2 = new SectorImageMapF2("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map2->CalculateMap(t, INT_SAMPLES, VECTORS);

   uint32 id = -1;
   g_factory->InitializePipeline(1, &id);
   g_factory->AddImageTask(id, 0, fmap_fft);
   //g_factory->AddImageTask(id, 0, fast_map2);
   g_FFT_PIPELINE = id;

   g_factory->InitializePipeline(1, &id);
   g_factory->AddImageTask(id, 0, fmap_sparse2);
   //g_factory->AddImageTask(id, 0, fast_map2);
   g_SPARSE_PIPELINE = id;

   if (!PERF_TEST)
   {
      g_factory->InitializePipeline(5, &id); 
      g_factory->AddImageTask(id, 0, fmap_sparse_4stage);
      g_factory->AddImageTask(id, 1, fmap_sparse_4stage);
      g_factory->AddImageTask(id, 2, fmap_sparse_4stage);
      g_factory->AddImageTask(id, 3, fmap_sparse_4stage);
      g_factory->AddImageTask(id, 4, fast_map2);
      g_SPARSE_STAGED_PIPELINE = id;
   } 
   else 
   {  
      g_SPARSE_STAGED_PIPELINE = 0;
   }
}

void usage(char *prog)
{
  printf("%s [-s raw_samples] [-x intermediate samples] [-v vectors] [-u upsample factor] [-c channels] \
          [-f frames] [-t test frames] [-h]\
          \n\t-s raw samples       : number of samples per input vector\
          \n\t-x int samples       : number of samples per intermediate vector\
          \n\t-v vectors           : number of vectors\
          \n\t-u upsample factor   : upsample factor\
          \n\t-c channels          : number of channels\
          \n\t-f frames            : number of frames\
          \n\t-t test frames       : number of frames for performance test\
          \n\t-i input file        : input file\
          \n\t-h                   : print this help message\n",prog);
  return;
}


int main(int argc, char **argv)
{
   char c;

   uint32 RAW_SAMPLES      = 8192;
   uint32 INT_SAMPLES      = 2048;
   uint32 VECTORS          = 256; 
   uint32 RAW_SIZE         = RAW_SAMPLES * VECTORS;
   uint32 UPSAMPLE_FACTOR  = 4; 
   uint32 NUM_CHANNELS     = 6;
   uint32 FRAMES           = 2;
   uint32 TEST_FRAMES      = 100;
   char *filename          = NULL;
   while ((c = getopt(argc, argv, "s:v:x:u:c:f:t:i:")) != -1)
   {
      switch (c)
      {
      case 's':
         RAW_SAMPLES = atoi(optarg);
         RAW_SIZE    = RAW_SAMPLES * VECTORS;
         break;

      case 'x':
         INT_SAMPLES = atoi(optarg);
         break;

      case 'v':
         VECTORS  = atoi(optarg);
         RAW_SIZE = RAW_SAMPLES * VECTORS;
         break;

      case 'u':
         UPSAMPLE_FACTOR = atoi(optarg);
         break;

      case 'c':
         NUM_CHANNELS = atoi(optarg);
         break;

      case 'f':
         FRAMES = atoi(optarg);
         break;
   
      case 't':
         TEST_FRAMES = atoi(optarg);
         break;
      
      case 'i':
         filename = optarg;
         break;
   
      case 'h':
         usage((char*)argv[0]);
         exit(1);
         break;

      default:
         printf("Invalid argument %c\n", c);
         usage((char*)argv[0]);
         exit(1);
         break;
      }
   }

   g_factory = new ImageFactory(64);
   g_free_list = new FrameList(100, NUM_CHANNELS, VECTORS, RAW_SAMPLES, 
                               0, IMG_WIDTH * IMG_HEIGHT);
   
   FakeTransducer *t = new FakeTransducer(NUM_CHANNELS, VECTORS, RAW_SAMPLES, 
                                          1.0);
   t->CalculateFocusOffsets(UPSAMPLE_FACTOR);

   AnnularFlatParameters afp; 
   afp.element_cnt         = NUM_CHANNELS;
   afp.vectors             = VECTORS;
   afp.samples             = RAW_SAMPLES;
   afp.sample_rate         = 24.0;
   afp.frequency           = 1.0;
   afp.sector_degrees      = 180.0;
   afp.transducer_height   = 7.43;
   afp.probe_radius        = 10.93;
   for (uint32 r = 0; r < NUM_CHANNELS; r++)
   {
      afp.radii[r]         = (double)r * 1.5;
   } 
   AnnularFlatTransducer *aft = new AnnularFlatTransducer(afp);
   aft->CalculateFocusOffsets(UPSAMPLE_FACTOR);
 
   InitPipelines(t, RAW_SAMPLES, INT_SAMPLES, VECTORS, NUM_CHANNELS, UPSAMPLE_FACTOR);

   uint8 *file_img = NULL;
   if (filename != NULL)
   {
      file_img = new uint8[VECTORS * RAW_SAMPLES];
      FILE *in_file = fopen(filename, "rb");
      if (in_file != NULL)
      {
         fread(file_img, sizeof(uint8), VECTORS*RAW_SAMPLES, in_file);
         fclose(in_file); 
      }
      else
      {
         printf("Error loading file %s\n", filename);
      }
   }
 
   g_generator = new FrameGenerator(FRAMES, VECTORS, RAW_SAMPLES, NUM_CHANNELS, g_free_list, 
                                    g_factory->GetInputRing(g_SPARSE_STAGED_PIPELINE),
                                    t->GetFocusOffsets(), file_img, 1);


   g_output_ring = g_factory->GetOutputRing(g_SPARSE_STAGED_PIPELINE);
   
   g_factory->Start();

   if (PERF_TEST)
   {
      // run some perf tests
      double fft_results[TEST_FRAMES];
      double sparse_results[TEST_FRAMES];
      double mean, std;

      //SaveFrame("fft.raw", g_FFT_PIPELINE);
      //SaveFrame("sparse.raw", g_SPARSE_PIPELINE);
      //printf("Saved frames...\n");

      RunPerformanceTest(g_FFT_PIPELINE, TEST_FRAMES, fft_results);
      RunPerformanceTest(g_SPARSE_PIPELINE, TEST_FRAMES, sparse_results);

#ifdef CYCLE_PERF
   float freq = (float)GetPerformanceFrequency();
   printf("fft perf: %lu => %f, ifft perf: %lu => %f, sum perf %lu => %f\n",
          g_fft_cycles/(TEST_FRAMES*SUB_CNT),  ((float)g_fft_cycles)/freq/(float)(TEST_FRAMES*SUB_CNT),
          g_ifft_cycles/(TEST_FRAMES*SUB_CNT), ((float)g_ifft_cycles)/freq/(float)(TEST_FRAMES*SUB_CNT),
          g_sum_cycles/(TEST_FRAMES*SUB_CNT),  ((float)g_sum_cycles)/freq/(float)(TEST_FRAMES*SUB_CNT));
   printf("Est FFT frame rate: %f\n", 
          1.0f / (((float)g_total_cycles)/freq/(float)(TEST_FRAMES*SUB_CNT)));

   printf("sparse perf: %lu => %f, no up perf: %lu => %f\n",
          g_sparse_up_cycles/(TEST_FRAMES*SUB_CNT),  ((float)g_sparse_up_cycles)/freq/(float)(TEST_FRAMES*SUB_CNT),
          g_sparse_noup_cycles/(TEST_FRAMES*SUB_CNT), ((float)g_sparse_noup_cycles)/freq/(float)(TEST_FRAMES*SUB_CNT) );
#endif 
 
      ComputeStats(fft_results, TEST_FRAMES, &mean, &std);
      printf("FFT Pipeline - Mean time: %f ms, STD: %f ms, %f frames/s\n", 
             mean, std, 1000.0/mean);

      ComputeStats(sparse_results, TEST_FRAMES, &mean, &std);
      printf("Sparse Pipeline - Mean time: %f ms, STD: %f ms, %f frames/s\n", 
             mean, std, 1000.0/mean);

      // compute speedup
      double speedup[TEST_FRAMES];
      memset(speedup, 0x00, sizeof(speedup));
      for (uint32 x = 0; x < TEST_FRAMES; x++)
      {
         speedup[x] = fft_results[x] / sparse_results[x];
      }
      ComputeStats(speedup, TEST_FRAMES, &mean, &std);
      printf("Speedup - Mean: %f, STD: %f\n", mean, std);
   }
   else
   {
      printf("Running image pipeline %u\n", g_SPARSE_STAGED_PIPELINE);
      g_generator->Start();
      g_perf_cnt = 0;
      g_timebase = 0;
      g_fps = 0.0f;

      glutInit(&argc, argv);
      glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
      glutInitWindowSize(512,512);
      glutInitWindowPosition(100,100);
      glutCreateWindow(argv[0]);
      
      glutDisplayFunc(display);
      glutReshapeFunc(reshape);
      glutKeyboardFunc(keyboard);
      glutMouseFunc(mouse);
      glutSpecialFunc(specialkeys);
      glutIdleFunc(ImageGen); 
      glutMainLoop();
   }
   g_generator->Stop();
   g_factory->Stop();

   // draing queue
   Frame *f = NULL;
   err rc = g_output_ring->Read(&f);
   while (rc == SUCCESS)
   {
      rc = g_output_ring->Read(&f);
   }

   delete g_factory;
   delete g_generator;
   delete g_free_list;
   delete [] file_img;
   return 0;
}
