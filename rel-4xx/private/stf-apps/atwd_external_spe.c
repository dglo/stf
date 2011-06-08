/* atwd_external_spe.c, skeleton file created by gendir
 */

#include <stdlib.h>
#include <math.h>

#include "stf/stf.h"
#include "stf-apps/atwd_external_spe.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

#include "stf-apps/atwdUtils.h"

BOOLEAN atwd_external_speInit(STF_DESCRIPTOR *d) {
   return FALSE;
}

BOOLEAN atwd_external_speEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_ch0_clamp,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned triggerable_spe_dac,
                    unsigned loop_count,
                    unsigned *atwd_waveform_width,
                    unsigned *atwd_waveform_amplitude,
                    unsigned *atwd_waveform_position,
                    unsigned *atwd_waveform_external_spe) {
const int ch = (atwd_chip_a_or_b) ? 0 : 4;
   int i;
   const int cnt = 128;
   short *channels[4] = { NULL, NULL, NULL, NULL };
   short *buffer = (short *) calloc(128, sizeof(short));
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

   /* pretest 1) all six atwd dac settings are programmed... */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);
   halWriteDAC(DOM_HAL_DAC_SINGLE_SPE_THRESH, triggerable_spe_dac);

   /* clear atwd */
   prescanATWD(trigger_mask);

   /* 3) take loop_count waveforms... */
   for (i=0; i<cnt; i++) atwd_waveform_external_spe[i] = 0;
   for (i=0; i<loop_count; i++) {
      int j;
      
      hal_FPGA_TEST_trigger_disc(trigger_mask);
      channels[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], 
			    channels[3], 
			    channels[0], channels[1], channels[2], 
			    channels[3],
			    cnt, NULL, 0, trigger_mask);
      for (j=0; j<cnt; j++) atwd_waveform_external_spe[j] += buffer[j];
   }
   for (i=0; i<cnt; i++) atwd_waveform_external_spe[i] /= loop_count;

   /* 8) reverse waveform */
   reverseATWDIntWaveform(atwd_waveform_external_spe);

   /* 9) find max index (still need to make window 50-80) */
   {   int maxIdx = 0;
       unsigned maxValue = atwd_waveform_external_spe[maxIdx];
       int half_max, hmxIdx = 127, hmnIdx = 0;
       
       for (i=1; i<50; i++) {
	  if (maxValue < atwd_waveform_external_spe[i]) {
	     maxIdx = i;
	     maxValue = atwd_waveform_external_spe[i];
	  }
       }
       int minIdx = maxIdx;
       unsigned minValue = maxValue;

       for (i=1; i<50; i++) {
	  if (minValue > atwd_waveform_external_spe[i]) {
	     minIdx = i;
	     minValue = atwd_waveform_external_spe[i];
	  }
       }

       /* 10) */
       *atwd_waveform_amplitude = maxValue-minValue;

       /* 11) */
       half_max = 
	 (*atwd_waveform_amplitude)/2;

       for (i=maxIdx; i<50; i++) {
	  if (atwd_waveform_external_spe[i]<half_max) {
	     hmxIdx = i;
	     break;
	  }
       }
       for (i=maxIdx; i>=0; i--) {
	  if (atwd_waveform_external_spe[i]<half_max) {
	     hmnIdx = i;
	     break;
	  }
       }

       /* 14 */
       *atwd_waveform_width = hmxIdx - hmnIdx;

       *atwd_waveform_position = maxIdx + 1;
   }
   
   free(buffer);

   return TRUE; 
}
