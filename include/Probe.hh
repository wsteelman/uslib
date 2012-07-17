
#ifndef PROBE_HH_
#define PROBE_HH_

#include "types.h"

namespace uslib
{


class Probe
{
public:
   Probe(const char *name)
   {
      strncpy(m_name, name, 256);
   }

   virtual ~Probe()
   {

   }

    

private:
   char m_name[256];

}; // class probe
} // namespace uslib

#endif
