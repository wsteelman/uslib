#ifndef PERF_HH_
#define PERF_HH_

#include "types.h"

//#define CYCLE_PERF

#ifdef CYCLE_PERF
extern uint64 g_fft_cycles;
extern uint64 g_ifft_cycles;
extern uint64 g_sum_cycles;
extern uint64 g_total_cycles;
extern uint64 g_sparse_up_cycles;
extern uint64 g_sparse_noup_cycles;
extern uint64 g_sparse_total_cycles;
#endif

static inline uint64 GetPerformanceCount()
{
   uint32 _hi, _lo;
   asm volatile ("rdtsc":"=a" (_lo), "=d"(_hi));
   return ((uint64) _hi << 32) | _lo;
}

static inline uint64 GetPerformanceFrequency(void)
{
   return 2200ULL * 1000ULL * 1000ULL;  /* nanoseconds */
}

//#     define GetPerformanceFrequency() 1000000ULL
#endif;
