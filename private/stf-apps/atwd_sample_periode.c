/* atwd_sample_periode.c, skeleton file created by gendir
 */

#include <stddef.h>
#include <stdlib.h>

#include "stf/stf.h"
#include "stf-apps/atwd_sample_periode.h"

#include "stf-apps/atwdUtils.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN atwd_sample_periodeInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_sample_periodeEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_chip_a_or_b,
                    unsigned loop_count,
                    unsigned *atwd_clock1x_waveform,
	            unsigned *correlation,
                    unsigned *rotate) {
   int i;
   const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;
   
   /* 1) all five atwd dac settings are programmed...
    */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);

   /* 2) select clock1x on analog_mux...
    */
   prescanATWD(trigger_mask);
   
   halSelectAnalogMuxInput(DOM_HAL_MUX_OSC_OUTPUT);

   halUSleep(100);
   
   /* test algorithm... */
   getSummedWaveform(loop_count, trigger_mask, 3, atwd_clock1x_waveform);

   /* get power... */
   {
      float power = 0, sum = 0;
      float wf[128];
      float cc=0;
      int offset = 0;
      
      for (i=0; i<128; i++) wf[i] = atwd_clock1x_waveform[i];
      for (i=0; i<128; i++) sum += wf[i];
      for (i=0; i<128; i++) wf[i] -= sum/128;
      for (i=0; i<128; i++) power += wf[i]*wf[i];
      if (power>=0.1) {
         int j;
         
         for (j=0; j<32; j++) {
            float c = 0;
            for (i=0; i<128; i++) c += wf[i] * wf[127 - (i+j)%128];
            c/=power;
            if (c>cc) {
               cc=c;
               offset=j;
            }
         }
      }
      else {
         cc = 0;
         offset=0;
      }
      *correlation = cc*1000;
      *rotate=offset;
   }
   
   return *correlation > 760;
}
