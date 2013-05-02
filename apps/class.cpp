#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <GLUT/glut.h>

#include "usapi.h"
#include "types.h"
#include "Utils.hh"
#include "Image.hh"
#include "FrameList.hh"
#include "ImageFactory.hh"
#include "FakeTransducer.hh"
#include "SectorImageMapF2.hh"
#include "FocusMapSparseMulti.hh"
#include "USBProbe.hh"

using namespace std;
using namespace uslib;

#define IMG_WIDTH          512
#define IMG_HEIGHT         512

uint32          g_perf_cnt;
int             g_timebase;
float           g_fps;
char            g_fps_char[16];
FrameRing      *g_output_ring;
FrameList      *g_free_list;

void drawBitmapText(char *s, float x, float y) 
{  
   glColor3f(1.0f, 1.0f, 1.0f);
   //glWindowPos2i(x,y);

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

   //glWindowPos2i(0,0);
   glDrawPixels(f->GetDisplayWidth(), 
                f->GetDisplayHeight(), 
                GL_LUMINANCE, 
                GL_UNSIGNED_BYTE, 
                f->GetDisplayBuffer());
   g_perf_cnt++;
   int time = glutGet(GLUT_ELAPSED_TIME);
   if (time - g_timebase > 1000)
   {
      g_fps = (float)g_perf_cnt*1000.0f/(float)(time-g_timebase);
      g_timebase = time;
      g_perf_cnt = 0;
   }
   sprintf(g_fps_char, "FPS: %2.2f", g_fps);
   printf("FPS: %2.2f\r", g_fps);
   //drawBitmapText(g_fps_char, 10.0f, 10.0f);
   
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
  //gluPerspective(60.0, (GLfloat) w / (GLfloat) h, 0.01, 500.0);
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



void StartGLUT(int argc, char **argv)
{
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

void usage(char *prog)
{
  printf("%s [-s raw_samples] [-x intermediate samples] [-v vectors] [-u upsample factor] [-c channels] \
          [-f frames] [-h]\
          \n\t-u upsample factor   : upsample factor\
          \n\t-f frames            : number of frames\
          \n\t-i input file        : input file\
          \n\t-p pipeline stage    : number of pipline stages used for beamforming\
          \n\t-h                   : print this help message\n",prog);
  return;
}


int main(int argc, char **argv)
{
   char c;

   uint32 SAMPLES          = 2048;
   uint32 VECTORS          = 256; 
   uint32 CHANNELS         = 6;
   uint32 UPSAMPLE_FACTOR  = 4; 
   uint32 FRAMES           = 8;
   uint32 STAGES           = 4;
   char *filename          = NULL;
   while ((c = getopt(argc, argv, "s:v:x:u:c:f:t:i:p:")) != -1)
   {
      switch (c)
      {
      case 'u':
         UPSAMPLE_FACTOR = atoi(optarg);
         break;

      case 'f':
         FRAMES = atoi(optarg);
         break;
   
      case 'i':
         filename = optarg;
         break;
 
      case 'p':
         STAGES = atoi(optarg);
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

   USBProbe *probe = new USBProbe();
   err rc = probe->InitializeHW(8);
   if (rc != SUCCESS)
   {
      printf("Unable to create USB probe object\n");
      exit(-1);
   }
   CHANNELS = probe->GetChannelCount();
   VECTORS = probe->GetVectorCount();
   SAMPLES = probe->GetSampleCount();

   // create factory
   ImageFactory *factory = new ImageFactory(64);
   
   // create free list
   g_free_list = new FrameList(100, CHANNELS, VECTORS, SAMPLES, 
                               0, IMG_WIDTH * IMG_HEIGHT);
  
   // initialize transducer 
   FakeTransducer *t = new FakeTransducer(CHANNELS, VECTORS, SAMPLES, 
                                          1.0);
   t->CalculateFocusOffsets(UPSAMPLE_FACTOR);

   // initialize image tasks
   FocusMapSparseMulti *fmap_sparse_4stage = new FocusMapSparseMulti(SAMPLES, SAMPLES, VECTORS, 
                                                       UPSAMPLE_FACTOR, CHANNELS, 4);
   fmap_sparse_4stage->Calculate(t->GetFocusOffsets());

   SectorImageMapF2 *fast_map2 = new SectorImageMapF2("Fast", IMG_WIDTH, IMG_HEIGHT);
   fast_map2->CalculateMap(t, SAMPLES, VECTORS);

   // add tasks to pipeline
   uint32 id = -1;
   factory->InitializePipeline(5, &id); 
   for (uint32 x = 0; x < 4; x++)
   {
      factory->AddImageTask(id, x, fmap_sparse_4stage);
   }
   factory->AddImageTask(id, 4, fast_map2);

   // save output ring pointer
   FrameRing *input_ring = factory->GetInputRing(id);
   g_output_ring = factory->GetOutputRing(id);
   
   // start the factory
   factory->Start();

   probe->SetOutputRing(input_ring);
   probe->SetFrameList(g_free_list);
   probe->StartImagingThread();
   probe->StartFrameCapture();
    
   // start GLUT interface
   StartGLUT(argc, argv);

   probe->StopFrameCapture();
   factory->Stop();

   // draing queue
   Frame *f = NULL;
   rc = g_output_ring->Read(&f);
   while (rc == SUCCESS)
   {
      rc = g_output_ring->Read(&f);
   }

   probe->StopImagingThread();
   
   delete fast_map2;
   delete fmap_sparse_4stage;
   delete t; 
   delete g_free_list;
   delete factory;
   delete probe;

   return 0;
}
