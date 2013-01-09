#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <GLUT/glut.h>

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
#define RAW_SAMPLES        4096
#define INT_SAMPLES        2048
#define VECTORS            256
#define RAW_SIZE           (RAW_SAMPLES * VECTORS)

#define IMG_WIDTH          512
#define IMG_HEIGHT         512
#define UPSAMPLE_FACTOR    4
#define NUM_CHANNELS       6
#define FRAMES             32
#define CHNL_BUFFERS       (NUM_CHANNELS * FRAMES)

// globals
ImageFactory *g_factory;
Frame       **g_frames;
uint32        g_frame_index;
uint32        g_perf_cnt;
int           g_timebase;
float         g_fps;
char          g_fps_char[16];

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
   Frame *f = g_frames[g_frame_index];
   g_frame_index = (g_frame_index+1) % FRAMES;
   g_factory->GenerateImage(f);

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
}



void display(void)
{
   ImageGen();
   //long stime, etime;
   //stime = ntime();
   //for(int i = 0; i < 50000; i++)
   //{
   //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //   glEnable(GL_TEXTURE_2D);
   //   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
   //   glBindTexture(GL_TEXTURE_2D, texName[0]);
  
   //   glBegin(GL_QUAD_STRIP);
   //   int x1 = 0.0;
   //   int y1 = 2.0;
   //   float max_radius = 4.0;
   //   float min_radius = 0.0;
   //   float t = 512;
   //   for(float angle = PI + MIN_ANGLE; 
   //       angle < (PI + MAX_ANGLE/* + (SECTOR_RAD/t)*/); 
   //       angle += (SECTOR_RAD/t))
   //   {
   //      float vnorm = ((angle-PI-MIN_ANGLE)/SECTOR_RAD);
   //      glTexCoord2f(vnorm,1.0); 
   //      glVertex2f(x1 + cos(angle) * min_radius, y1 + sin(angle) * min_radius);
   //      glTexCoord2f(vnorm,0.0);
   //      glVertex2f(x1 + cos(angle) * max_radius, y1 + sin(angle) * max_radius);
   //   }
   //   glEnd();


   //   glFlush();
   //   glDisable(GL_TEXTURE_2D);
   //}
   //etime = ntime();
   //printf("num images = %i, t (ms) = %ld, frames/s = %f\n", 500, etime - stime, (double)500/(etime-stime) * 1000.0f);

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
  /*
  if(state == GLUT_DOWN)
  {
    float xf = ((float)x - ((float)wnd_width)/2) / float(wnd_width);
    float yf = (((float)wnd_height)/2 - (float)y) / float(wnd_height);
    //printf("x %f, y %f\n",xf,yf);
    //printf("scale %f\n", scale);

    trans.x += -1 * xf * scale;
    trans.y += -1 * yf * scale;
    //printf("x %f, y %f, z %f\n",trans.x, trans.y, trans.z);
    reshape(wnd_width, wnd_height);
    display();
  } 
  */
}

int main(int argc, char **argv)
{

   uint8 *raw_data = new uint8[VECTORS * RAW_SAMPLES];

   g_factory = new ImageFactory();
   FakeTransducer *t = new FakeTransducer(NUM_CHANNELS, VECTORS, RAW_SAMPLES, UPSAMPLE_FACTOR,
                                          1.0);

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

   g_factory->AddImageTask(0, fmap_int);
   g_factory->AddImageTask(0, fast_map0);
  
   g_factory->AddImageTask(1, fmap_fft);
   g_factory->AddImageTask(1, fast_map1);

   g_factory->AddImageTask(2, fmap_sparse0);
   g_factory->AddImageTask(2, fast_map2);

   g_factory->AddImageTask(3, fmap_sparse1);
   g_factory->AddImageTask(3, fast_map3);
 
   g_factory->AddImageTask(4, fmap_sparse2);
   g_factory->AddImageTask(4, fast_map4);
 
   uint8 **data = new uint8*[CHNL_BUFFERS];
   g_frames = new Frame*[FRAMES];   
   for (uint32 x = 0; x < FRAMES; x++)
   {
      g_frames[x] = new Frame(NUM_CHANNELS, 0, VECTORS, INT_SAMPLES, IMG_WIDTH*IMG_HEIGHT);
      g_frames[x]->SetImageMapID(4);
      for (uint32 c = 0; c < NUM_CHANNELS; c++)
      {
         uint8 *tmp = new uint8[RAW_SIZE];
         data[x*NUM_CHANNELS + c] = tmp;
         g_frames[x]->AddChannelData(c, tmp);
         GenerateFakeImage(VECTORS, RAW_SAMPLES, tmp, x);
      }
    }
  
   g_frame_index = 0;
   g_perf_cnt = 0;
   g_timebase = 0;
   g_fps = 0.0f;

   SetThreadPrio(99);


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

   for (uint32 x = 0; x < CHNL_BUFFERS; x++)
   {
      delete [] data[x];
   }
   delete [] data;
   for (uint32 x = 0; x < FRAMES; x++)
   {
      delete g_frames[x];
   }
   delete [] g_frames; 
   delete g_factory;
   delete raw_data;
   delete fmap_int;
   delete fmap_fft;
   delete fmap_sparse0;
   delete fmap_sparse1;
   return 0;
}
