#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

void prescanATWD(unsigned trigger_mask) {
   int i;
   
   /* Thorsten recommends we wait a bit...
    */
   halUSleep(1000);
   
   /* azriel recommends to throw away a few atwd captures first...
    */
   for (i=0; i<8; i++) {
      hal_FPGA_TEST_trigger_forced(trigger_mask);
      while (!hal_FPGA_TEST_readout_done(trigger_mask)) ;
      halUSleep(1000);
   }
}
