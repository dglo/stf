/* temperature.c, skeleton file created by George
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "stf-apps/temperature.h"

BOOLEAN temperatureInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

static int readTemp(void) { return halReadTemp(); }

static float formatTemp(int temp) {
   signed char t = (temp>>8);
   float v = t;
   int i;
   int mask = 0x80;
   for (i=0; i<4; i++, mask>>=1) {
      const float frac = 1.0 / (1<<(i+1));
      if (temp&mask) v+=frac;
   }
   return v;
 }

BOOLEAN temperatureEntry(STF_DESCRIPTOR *d,
                    unsigned env_temp,
                    unsigned *temp) {

  *temp= (formatTemp(readTemp())+273.0)*1000.0;
  return TRUE;
}
