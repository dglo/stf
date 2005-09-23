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
   
   for (j=0; j<10; j++) {
      int rand = 0;
      int i;
      
      for (i=0; i<750 * 1000; i++) {
         rand = rand*69069 + 1;
         halUSleep(1);
         hal_FPGA_TEST_comm_bit_bang_dac( (rand&0xff)>>WIGGLE_SHIFT );
      }
      halUSleep(1);
      hal_FPGA_TEST_comm_bit_bang_dac(128);
      halUSleep(100*1000);
   }

   halBoardReboot();
   
   return 0;
}



