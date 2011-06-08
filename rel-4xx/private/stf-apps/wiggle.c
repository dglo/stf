/* wiggle.c -- wiggle comm dac in 
 */
#include "hal/DOM_MB_fpga.h"
#include "hal/DOM_MB_pld.h"

#ifndef WIGGLE_SHIFT
#define WIGGLE_SHIFT 0
#endif

/* wiggle for 1500ms break for 100ms -- wiggle rate is approx
 * 1MHz we're done after about 20s...
 */
int main(void) {
   int j;
   
   for (j=0; j<30; j++) {
      unsigned rand = 0;
      int i;
      
      for (i=0; i< 750 * 150; i++) {
         /* FIXME: shouldn't this be ~50? */
         const unsigned ns = 3 * (rand&0xf);

         hal_FPGA_TEST_comm_bit_bang_dac(0xff);
         halNanoSleep(ns);
         hal_FPGA_TEST_comm_bit_bang_dac(0);
         halNanoSleep(ns);
         rand = rand*69069 + 1;
      }
      hal_FPGA_TEST_comm_bit_bang_dac(128);
      halUSleep(100*1000);
   }

   halBoardReboot();
   
   return 0;
}



