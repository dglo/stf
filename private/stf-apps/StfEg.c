/* StfEg.c, skeleton file created by gendir
 */
#include "stf/stf.h"
#include "stf-apps/StfEg.h"

BOOLEAN StfEgInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN StfEgEntry(STF_DESCRIPTOR *d,
                    BOOLEAN Para1,
                    const char * Para2,
                    unsigned Para3,
                    unsigned long Para4,
                    BOOLEAN *Para5,
                    char * *Para6,
                    unsigned *Para7,
                    unsigned long *Para8,
                    unsigned *Para9) {
   /* must fill output array */
   int i;
   for (i=0; i<10; i++) Para9[i] = i;
   return TRUE; 
}
