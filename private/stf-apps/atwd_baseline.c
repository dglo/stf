/* atwd_baseline.c, skeleton file created by gendir
 */
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "stf/stf.h"
#include "stf-apps/atwd_baseline.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN atwd_baselineInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_baselineEntry(STF_DESCRIPTOR *d,
                    unsigned atwd_sampling_speed_dac,
                    unsigned atwd_ramp_top_dac,
                    unsigned atwd_ramp_bias_dac,
                    unsigned atwd_analog_ref_dac,
                    unsigned atwd_pedestal_dac,
                    unsigned atwd_chip_a_or_b,
                    unsigned atwd_channel,
                    unsigned atwd_trig_forced_or_spe,
                    unsigned spe_discriminator_uvolt,
                    unsigned loop_count,
                    BOOLEAN fill_output_arrays,
                    unsigned *atwd_baseline_mean,
                    unsigned *atwd_baseline_rms,
                    unsigned *atwd_baseline_min,
                    unsigned *atwd_baseline_max,
                    unsigned *atwd_baseline_histogram,
                    unsigned *atwd_disc_threshold_dac) {

   const int ch = 
      (atwd_chip_a_or_b) ? 
      DOM_HAL_DAC_ATWD1_TRIGGER_BIAS : DOM_HAL_DAC_ATWD0_TRIGGER_BIAS;
   int i;
   unsigned minv, maxv;
   const int cnt = 128;
   short *buffer = (short *) calloc(cnt, sizeof(short));
   short *channels[4] = { NULL, NULL, NULL, NULL };
   unsigned *values = (unsigned *) calloc(loop_count, sizeof(unsigned));
   unsigned sm = 0, sm2 = 0;
   const int trigger_mask = (atwd_chip_a_or_b) ?
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

   /* A. all five atwd dac settings are programmed...
    */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(DOM_HAL_DAC_ATWD_ANALOG_REF, atwd_analog_ref_dac);
   halWriteDAC(DOM_HAL_DAC_PMT_FE_PEDESTAL, atwd_pedestal_dac);

   /* C. if the SPE trigger was requested, calculate the SPE DAC that
    * corresponds to SPE_DISCRIMINATOR_UVOLT and program it...
    */
   if (atwd_trig_forced_or_spe==1) {
      *atwd_disc_threshold_dac = (unsigned) 
	 ((spe_discriminator_uvolt * 9.6 * (2200+1000)/1000 + 
	   atwd_pedestal_dac * 5000000 / 4096)*1024/5000000);
      halWriteDAC(DOM_HAL_DAC_MULTIPLE_SPE_THRESH, *atwd_disc_threshold_dac);
   }

   /* Thorsten recommends we wait a bit...
    */
   halUSleep(1000);

   for (i=0; i<(int)loop_count; i++) {
      int j;
      unsigned sum = 0;

      /* B. The ATWD trigger mask is written according to 
       * ATWD_TRIG_FORCED_OR_SPE
       */
      if (atwd_trig_forced_or_spe==0) {
	 /* forced ... */
	 hal_FPGA_TEST_trigger_forced(trigger_mask);
      }
      else {
	 /* discriminator... */
	 hal_FPGA_TEST_trigger_disc(trigger_mask);
      }

      /* D.  Take one waveform for the channel requested...
       */
      channels[atwd_channel] = buffer;
      hal_FPGA_TEST_readout(channels[0], channels[1], channels[2], channels[3],
			    NULL, NULL, NULL, NULL,
			    cnt, NULL, 0, atwd_chip_a_or_b);

      /* sum it... */
      for (j=0; j<cnt; j++) sum+=buffer[j];

      /* record it... */
      values[i] = sum/cnt;

      /* E. repeat...
       */
   }

   minv = maxv = values[0];
   for (i=0; i<loop_count; i++) {
      if (values[i]<minv) minv = values[i];
      else if (values[i]>maxv) maxv = values[i];
   }

   for (i=0; i<loop_count; i++) sm += values[i];
   sm /= loop_count;

   for (i=0; i<loop_count; i++) {
      const int diff = (int) values[i] - sm;
      sm2 += diff*diff;
   }

   *atwd_baseline_mean = sm;
   *atwd_baseline_rms = (int) sqrt( (1.0/(loop_count-1)) * sm2 );
   *atwd_baseline_min = minv;
   *atwd_baseline_max = maxv;
   *atwd_baseline_histogram = 0;

   free(buffer);
   free(values);
   
   return TRUE;
}
