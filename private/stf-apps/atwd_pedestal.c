/* atwd_pedestal.c, skeleton file created by gendir
 */
#include <stdlib.h>

#include "stf/stf.h"
#include "stf-apps/atwd_pedestal.h"

#include "hal/DOM_MB_hal.h"
#include "hal/DOM_MB_fpga.h"

BOOLEAN atwd_pedestalInit(STF_DESCRIPTOR *d) {
   return TRUE;
}

BOOLEAN atwd_pedestalEntry(STF_DESCRIPTOR *d,
			   unsigned atwd_sampling_speed_dac,
			   unsigned atwd_ramp_top_dac,
			   unsigned atwd_ramp_bias_dac,
			   unsigned atwd_analog_ref_dac,
			   unsigned atwd_pedestal_dac,
			   unsigned atwd_chip_a_or_b,
			   unsigned atwd_channel,
			   unsigned atwd_trig_forced_or_spe,
			   unsigned spe_descriminator_uvolt,
			   unsigned loop_count,
			   BOOLEAN fill_output_arrays,
			   unsigned *atwd_pedestal_amplitude,
			   unsigned *atwd_pedestal_pattern,
			   unsigned *atwd_disc_threshold_dac) {
   const int ch = (atwd_chip_a_or_b) ? 4 : 0;
   int i;
   unsigned minv, maxv;
   const int cnt = 128;
   short *buffer = (short *) calloc(cnt, sizeof(short));
   short *channels[4] = { NULL, NULL, NULL, NULL };
   unsigned *sum = (unsigned *) calloc(cnt, sizeof(unsigned));
   int trigger_mask = (atwd_chip_a_or_b) ? 
      HAL_FPGA_TEST_TRIGGER_ATWD0 : HAL_FPGA_TEST_TRIGGER_ATWD1;

   /* A. all five atwd dac settings are programmed...
    */
   halWriteDAC(ch, atwd_sampling_speed_dac);
   halWriteDAC(ch+1, atwd_ramp_top_dac);
   halWriteDAC(ch+2, atwd_ramp_bias_dac);
   halWriteDAC(3, atwd_analog_ref_dac);
   halWriteDAC(7, atwd_pedestal_dac);

   /* C. if the SPE trigger was requested, calculate the SPE DAC that
    * corresponds to SPE_DISCRIMINATOR_UVOLT and program it...
    */
   if (atwd_trig_forced_or_spe==1) {
      *atwd_disc_threshold_dac = (unsigned) 
	 ((spe_descriminator_uvolt * 9.6 * (2200+1000)/1000 + 
	   atwd_pedestal_dac * 5000000 / 4096)*1024/5000000);
      halWriteDAC(8, *atwd_disc_threshold_dac);
   }

   /* Thorsten recommends we wait a bit...
    */
   halUSleep(1000);

   for (i=0; i<(int)loop_count; i++) {
      int j;
      
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
      hal_FPGA_TEST_atwd_readout(channels[0], channels[1], channels[2],
				 channels[3], cnt, atwd_chip_a_or_b);

      /* get summed waveform... */
      for (j=0; j<cnt; j++) sum[j]+=buffer[j];

      /* E. repeat...
       */
   }

   /* F. divide the resulting sum waveform by LOOP_COUNT to get an average
    * waveform.
    */
   for (i=0; i<cnt; i++) sum[i]/=loop_count;
   
   /* FIXME: G. fill output arrays...
    */
   *atwd_pedestal_pattern = 0;

   /* H. Analyze pedestal waveform:
    *
    *  - obtain maximum and minimum value of the average pedestal waveform...
    *  - set pass to:
    *      -> minimum value of average pedestal waveform > 0
    *      -> maximum value of average pedestal waveform < 1023
    *      -> maximum-minimum < 20
    */
   minv = maxv = sum[0];
   for (i=1; i<cnt; i++) {
      if (sum[i]<minv) minv=sum[i];
      else if (sum[i]>maxv) maxv=sum[i];
   }

   *atwd_pedestal_amplitude = maxv-minv;

   free(buffer);
   free(sum);
   
   return minv>0 && maxv<1023 && (maxv-minv)<20;
}










