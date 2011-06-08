/* loopback_lc.c, skeleton file created by gendir
 *
 * FIXME: wtf?  direct register writes.  this thing
 * needs to get fixed up...
 */
#include "stf/stf.h"
#include "stf-apps/loopback_lc.h"

#include "hal/DOM_MB_hal.h"

BOOLEAN loopback_lcInit(STF_DESCRIPTOR *d) { return TRUE; }

BOOLEAN loopback_lcEntry(STF_DESCRIPTOR *d,
                    unsigned loop_count,
                    unsigned *lc_tx_quiet,
                    unsigned *lc_tx_dwn_hi,
                    unsigned *lc_tx_dwn_lo,
                    unsigned *lc_tx_up_hi,
                    unsigned *lc_tx_up_lo,
                    unsigned *lc_tx_up_hi_dwn_hi,
                    unsigned *lc_tx_up_hi_dwn_lo,
                    unsigned *lc_tx_up_lo_dwn_lo,
                    unsigned *lc_tx_up_lo_dwn_hi) {
   unsigned i;
   unsigned pulses[] = {
      0x000, 0x100, 0x200, 0x400, 0x800, /* each by itself */
      0x500, 0x900, 0xa00, 0x600  /* combination of two */
   };
   unsigned answers[] = {
      0xaa00, 0x9a00, 0x6a00, 0xa900, 0xa600,
      0x9900, 0x9600, 0x6600, 0x6900
   };
   unsigned *vars[] = {
      lc_tx_quiet, lc_tx_dwn_hi, lc_tx_dwn_lo, lc_tx_up_hi, lc_tx_up_lo,
      lc_tx_up_hi_dwn_hi, lc_tx_up_lo_dwn_hi, lc_tx_up_lo_dwn_lo,
      lc_tx_up_hi_dwn_lo
   };
   const int npulses = sizeof(pulses)/sizeof(pulses[0]);

   /* set variables to zero... */
   for (i=0; i<npulses; i++) *(vars[i]) = 0;
   
   for (i=0; i<loop_count; i++) {
      int j;

      for (j=0; j<npulses; j++) {
         /* reset */
         *(volatile unsigned *)0x90081018 = 0;

         /* wait a bit for line to be quiet... */
         halUSleep(2);

         /* send a cmd down -- and latch... */
         *(volatile unsigned *) 0x90081018 = 0xf004 | pulses[j];

         /* wait for it to arrive... */
         halUSleep(10);

         /* did it arrive correctly? */
         {  const unsigned answer = (*(volatile unsigned *) 0x9008101c)&0xff00;
            if (answer==answers[j]) (*vars[j])++;
         }
      }
   }

   for (i=0; i<npulses; i++) if (*vars[i]!=loop_count) return FALSE;

   return TRUE;
}






